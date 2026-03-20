/**
 * PureOS PDF Reader - MuPDF-based PDF renderer
 *
 * Renders real PDF pages as bitmaps using the MuPDF library.
 * Supports multi-page navigation, zoom, scroll, and file open dialog.
 */

#include "../fs/fs.h"
#include "../kernel/heap.h"
#include "../kernel/spinlock.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "../kernel/window.h"

/* MuPDF headers */
#include "mupdf/fitz.h"
#include "mupdf/pdf.h"

extern void print_serial(const char *s);
extern void k_print_serial_hex(uint64_t val);
extern window_t *winmgr_create_window(int x, int y, int w, int h,
                                      const char *title);
extern void winmgr_bring_to_front(window_t *win);
extern void winmgr_close_window(window_t *win);
extern task_t *create_task(void (*entry)(), char *name);
extern void task_set_priority(int pid, int priority);
extern task_t *get_task_by_name(const char *name);
extern int msg_receive(msg_t *msg);
extern void exit(int status);

/* ======================== STATE ======================== */
static window_t *pdf_win = 0;
static int pdf_running = 0;
static int pdf_scroll_y = 0;
static int pdf_dialog_mode = 0; /* 0=none, 1=open file */
static int pdf_cursor_blink = 0;
static int was_btn = 0;
extern int ui_dirty;
static spinlock_irq_t pdf_lock;

static char pdf_filename[128] = "sample.pdf";
static char pdf_title[160] = "PDF Reader";

/* Raw file buffer */
#define PDF_FILE_SIZE (1024 * 1024 * 2) /* 2MB max PDF */
static uint8_t *pdf_file_buf = 0;
static int pdf_file_len = 0;

/* Page info */
static int pdf_page_count = 0;
static int pdf_current_page = 0; /* 0-based */

/* MuPDF context and document - persistent across page navigations */
static fz_context *pdf_ctx = 0;
static fz_document *pdf_doc = 0;

/* Rendered page bitmap cache */
static uint32_t *pdf_page_pixels = 0; /* ARGB pixel buffer */
static int pdf_page_w = 0;
static int pdf_page_h = 0;
static int pdf_rendered_page = -1; /* which page is currently rendered */

/* Error message buffer */
static char pdf_error_msg[256] = "";

/* Zoom level (percentage) */
static int pdf_zoom = 100;
#define PDF_ZOOM_MIN 50
#define PDF_ZOOM_MAX 300
#define PDF_ZOOM_STEP 25

/* ======================== MuPDF ALLOCATOR ======================== */
/* Route MuPDF allocations through kernel allocator */

static void *pureos_malloc(void *opaque, size_t size) {
  (void)opaque;
  return kmalloc(size);
}

static void *pureos_realloc(void *opaque, void *old, size_t size) {
  (void)opaque;
  return realloc(old, size);
}

static void pureos_free(void *opaque, void *ptr) {
  (void)opaque;
  if (ptr)
    kfree(ptr);
}

static fz_alloc_context pureos_alloc = {0, pureos_malloc, pureos_realloc,
                                        pureos_free};

/* ======================== MuPDF RENDERING ======================== */

static void pdf_cleanup_mupdf(void) {
  if (pdf_page_pixels) {
    kfree(pdf_page_pixels);
    pdf_page_pixels = 0;
  }
  pdf_page_w = 0;
  pdf_page_h = 0;
  pdf_rendered_page = -1;

  if (pdf_doc) {
    fz_drop_document(pdf_ctx, pdf_doc);
    pdf_doc = 0;
  }
  if (pdf_ctx) {
    fz_drop_context(pdf_ctx);
    pdf_ctx = 0;
  }
}

static int pdf_init_mupdf(void) {
  if (pdf_ctx)
    return 1;

  print_serial("MuPDF: Initializing context...\n");
  pdf_ctx = fz_new_context(&pureos_alloc, 0, 16 * 1024 * 1024);
  if (!pdf_ctx) {
    print_serial("MuPDF: Failed to create context!\n");
    strcpy(pdf_error_msg, "[Error] Failed to create MuPDF context");
    return 0;
  }

  /* Register PDF handler */
  fz_try(pdf_ctx) { fz_register_document_handlers(pdf_ctx); }
  fz_catch(pdf_ctx) {
    print_serial("MuPDF: Failed to register handlers!\n");
    strcpy(pdf_error_msg, "[Error] Failed to register document handlers");
    fz_drop_context(pdf_ctx);
    pdf_ctx = 0;
    return 0;
  }

  print_serial("MuPDF: Context ready.\n");
  return 1;
}

static int pdf_load_document(const uint8_t *data, int len) {
  if (!pdf_ctx)
    return 0;

  /* Drop any previously loaded document */
  if (pdf_doc) {
    fz_drop_document(pdf_ctx, pdf_doc);
    pdf_doc = 0;
  }
  if (pdf_page_pixels) {
    kfree(pdf_page_pixels);
    pdf_page_pixels = 0;
  }
  pdf_rendered_page = -1;
  pdf_page_count = 0;

  print_serial("MuPDF: Loading document buffer...\n");
  fz_try(pdf_ctx) {
    fz_buffer *buf = fz_new_buffer_from_shared_data(pdf_ctx, data, (size_t)len);
    pdf_doc = fz_open_document_with_buffer(pdf_ctx, "application/pdf", buf);
    fz_drop_buffer(pdf_ctx, buf);

    pdf_page_count = fz_count_pages(pdf_ctx, pdf_doc);
    print_serial("MuPDF: Document loaded, pages=");
    char num[8];
    k_itoa(pdf_page_count, num);
    print_serial(num);
    print_serial("\n");
  }
  fz_catch(pdf_ctx) {
    print_serial("MuPDF: Load failed: ");
    print_serial(fz_caught_message(pdf_ctx));
    print_serial("\n");
    strcpy(pdf_error_msg, "[Error] Failed to open PDF: ");
    strcat(pdf_error_msg, fz_caught_message(pdf_ctx));
    pdf_doc = 0;
    pdf_page_count = 0;
    return 0;
  }

  return 1;
}

static int pdf_render_page(int page_num) {
  spinlock_irq_acquire(&pdf_lock);
  if (!pdf_ctx || !pdf_doc) {
    spinlock_irq_release(&pdf_lock);
    return 0;
  }
  if (page_num < 0 || page_num >= pdf_page_count) {
    spinlock_irq_release(&pdf_lock);
    return 0;
  }
  if (page_num == pdf_rendered_page && pdf_page_pixels) {
    spinlock_irq_release(&pdf_lock);
    return 1; /* Already rendered */
  }

  /* Release lock during slow rendering! */
  fz_context *local_ctx = pdf_ctx;
  fz_document *local_doc = pdf_doc;
  int local_zoom = pdf_zoom;
  spinlock_irq_release(&pdf_lock);

  print_serial("MuPDF: Rendering page ");
  char num[8];
  k_itoa(page_num + 1, num);
  print_serial(num);
  print_serial("...\n");

  fz_pixmap *pix = 0;
  uint32_t *new_pixels = 0;
  int w = 0, h = 0;
  char err_buf[128] = {0};

  fz_try(local_ctx) {
    /* Scale factor based on zoom percentage */
    float scale = (float)local_zoom / 100.0f;
    fz_matrix ctm = fz_scale(scale, scale);

    /* Render to RGB pixmap (no alpha) */
    pix = fz_new_pixmap_from_page_number(local_ctx, local_doc, page_num, ctm,
                                         fz_device_rgb(local_ctx), 0);

    w = fz_pixmap_width(local_ctx, pix);
    h = fz_pixmap_height(local_ctx, pix);
    int n = fz_pixmap_components(local_ctx, pix);
    unsigned char *samples = fz_pixmap_samples(local_ctx, pix);

    /* Convert RGB pixmap to ARGB pixel buffer for window surface */
    new_pixels = (uint32_t *)kmalloc(w * h * sizeof(uint32_t));
    if (!new_pixels) {
      strcpy(err_buf, "[Error] Out of memory for page render");
      fz_drop_pixmap(local_ctx, pix);
      pix = 0;
    } else {
      for (int y = 0; y < h; y++) {
        unsigned char *row = samples + y * fz_pixmap_stride(local_ctx, pix);
        for (int x = 0; x < w; x++) {
          unsigned char r = row[x * n + 0];
          unsigned char g = row[x * n + 1];
          unsigned char b = row[x * n + 2];
          new_pixels[y * w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
      fz_drop_pixmap(local_ctx, pix);
      pix = 0;

      print_serial("MuPDF: Rendered ");
      k_itoa(w, num);
      print_serial(num);
      print_serial("x");
      k_itoa(h, num);
      print_serial(num);
      print_serial("\n");
    }
  }
  fz_catch(local_ctx) {
    if (pix)
      fz_drop_pixmap(local_ctx, pix);
    strcpy(err_buf, "[Error] Render failed: ");
    strcat(err_buf, fz_caught_message(local_ctx));
  }

  /* Now reacquire the lock to update the global state */
  spinlock_irq_acquire(&pdf_lock);
  if (err_buf[0]) {
    strcpy(pdf_error_msg, err_buf);
    spinlock_irq_release(&pdf_lock);
    return 0;
  }

  /* Free previous render */
  if (pdf_page_pixels) {
    kfree(pdf_page_pixels);
  }

  pdf_page_pixels = new_pixels;
  pdf_page_w = w;
  pdf_page_h = h;
  pdf_rendered_page = page_num;

  spinlock_irq_release(&pdf_lock);
  return 1;
}

/* ======================== OPEN FILE ======================== */

void pdfreader_open_file(const char *filename) {
  if (!filename || filename[0] == 0)
    return;

  /* We do NOT acquire pdf_lock here anymore because fs_read takes a long time
     and needs interrupts. This function runs inside the PDFReader thread. */

  /* Copy filename and convert to uppercase for FAT */
  char temp_name[128];
  memset(temp_name, 0, sizeof(temp_name));
  int i = 0;
  while (filename[i] && i < 127) {
    char c = filename[i];
    if (c >= 'a' && c <= 'z')
      c -= 32;
    temp_name[i] = c;
    i++;
  }
  temp_name[i] = 0;

  /* Safely update global filename */
  spinlock_irq_acquire(&pdf_lock);
  strcpy(pdf_filename, temp_name);
  spinlock_irq_release(&pdf_lock);

  print_serial("PDF: Opening file: ");
  print_serial(temp_name);
  print_serial("\n");

  /* Allocate file buffer if needed */
  if (!pdf_file_buf) {
    pdf_file_buf = (uint8_t *)kmalloc(PDF_FILE_SIZE);
    if (!pdf_file_buf) {
      spinlock_irq_acquire(&pdf_lock);
      strcpy(pdf_error_msg, "[Error] Out of memory for file buffer");
      spinlock_irq_release(&pdf_lock);
      goto show_error;
    }
  }

  /* Read file WITHOUT LOCKS */
  pdf_file_buf[0] = 0;
  pdf_file_len = fs_read(temp_name, pdf_file_buf);
  if (pdf_file_len <= 0) {
    /* Fallback: try prefixing with / if not present */
    if (temp_name[0] != '/') {
      char fallback_name[140];
      fallback_name[0] = '/';
      strcpy(fallback_name + 1, temp_name);
      print_serial("PDF: Trying fallback: ");
      print_serial(fallback_name);
      print_serial("\n");
      pdf_file_len = fs_read(fallback_name, pdf_file_buf);
    }
  }

  if (pdf_file_len <= 0) {
    print_serial("PDF: [Error] fs_read failed for: ");
    print_serial(temp_name);
    print_serial("\n");
    spinlock_irq_acquire(&pdf_lock);
    strcpy(pdf_error_msg, "[Error] Could not read file: ");
    strcat(pdf_error_msg, temp_name);
    pdf_file_len = 0;
    spinlock_irq_release(&pdf_lock);
    goto show_error;
  }
  if (pdf_file_len > PDF_FILE_SIZE) {
    spinlock_irq_acquire(&pdf_lock);
    strcpy(pdf_error_msg, "[Error] File too large (max 2MB)");
    pdf_file_len = 0;
    spinlock_irq_release(&pdf_lock);
    goto show_error;
  }

  /* MuPDF init and document load — run WITHOUT holding the lock
     so the compositor can still draw the window frame. */
  if (!pdf_ctx) {
    if (!pdf_init_mupdf()) {
      goto show_error;
    }
  }

  /* Load the document */
  if (!pdf_load_document(pdf_file_buf, pdf_file_len)) {
    goto show_error;
  }

  /* Brief lock to update shared state */
  spinlock_irq_acquire(&pdf_lock);
  pdf_current_page = 0;
  pdf_scroll_y = 0;
  pdf_error_msg[0] = 0;
  strcpy(pdf_title, "PDF Reader - ");
  strcat(pdf_title, pdf_filename);
  spinlock_irq_release(&pdf_lock);

  /* Render first page without lock held */
  if (!pdf_render_page(0)) {
    // Error msg set in render_page
  }
  goto update_window;

show_error:
  spinlock_irq_acquire(&pdf_lock);
  pdf_page_count = 0;
  spinlock_irq_release(&pdf_lock);

update_window:
  spinlock_irq_acquire(&pdf_lock);
  if (pdf_win) {
    /* Update window title */
    int j = 0;
    while (pdf_title[j] && j < 159) {
      pdf_win->title[j] = pdf_title[j];
      j++;
    }
    pdf_win->title[j] = 0;
    pdf_win->needs_redraw = 1;
    ui_dirty = 1;
  }
  spinlock_irq_release(&pdf_lock);
}

/* ======================== DRAW ======================== */

#define PDF_MENU_Y 24
#define PDF_MENU_H 26
#define PDF_TEXT_Y (PDF_MENU_Y + PDF_MENU_H)
#define PDF_STATUS_H 22

/* Colors - Dark theme with PDF red accent (Opaque) */
#define PDF_COL_BG 0xFF1A1D23
#define PDF_COL_MENU 0xFF252830
#define PDF_COL_MENU_BORDER 0xFF3A3F4B
#define PDF_COL_TEXT 0xFFD4D8DF
#define PDF_COL_DIM 0xFF6B737E
#define PDF_COL_ACCENT 0xFFE04040
#define PDF_COL_STATUS 0xFF1E2228
#define PDF_COL_PAGE_BG 0xFF383C46
#define PDF_COL_DIALOG_BG 0xFF2C313A
#define PDF_COL_DIALOG_BORDER 0xFFE04040
#define PDF_COL_INPUT_BG 0xFF282C34
#define PDF_COL_WHITE_PAGE 0xFFFFFFFF
#define PDF_COL_BTN 0xFF353A45
#define PDF_COL_BTN_HOVER 0xFF454B58

static void pdfreader_draw(void *w) {
  window_t *win = (window_t *)w;
  if (!win)
    return;

  /* Try to acquire the lock WITHOUT disabling interrupts.
     If the PDFReader thread holds it (file load / render), just skip
     this frame — the compositor will retry next tick.  This prevents
     the desktop task from spinning forever with IRQs off. */
  uint32_t old_val;
  __asm__ volatile("lock xchgl %0, %1"
                   : "=r"(old_val), "+m"(pdf_lock.lock.locked)
                   : "0"(1)
                   : "memory");
  if (old_val != 0)
    return; /* lock is busy — skip this frame */

  int width = win->width;
  int height = win->height;

  /* ---- Menu Bar ---- */
  winmgr_fill_rect(win, 0, PDF_MENU_Y, width, PDF_MENU_H, PDF_COL_MENU);
  winmgr_fill_rect(win, 0, PDF_MENU_Y + PDF_MENU_H - 1, width, 1,
                   PDF_COL_MENU_BORDER);

  /* Open button */
  winmgr_fill_rect(win, 6, PDF_MENU_Y + 3, 48, 20, PDF_COL_ACCENT);
  winmgr_draw_text(win, 14, PDF_MENU_Y + 7, "Open", 0xFFFFFF);

  /* Page navigation */
  if (pdf_page_count > 0) {
    /* Prev button */
    int nav_x = 64;
    winmgr_fill_rect(win, nav_x, PDF_MENU_Y + 3, 20, 20, PDF_COL_BTN);
    winmgr_draw_text(win, nav_x + 6, PDF_MENU_Y + 7, "<", PDF_COL_TEXT);

    /* Page number */
    char page_str[24];
    k_itoa(pdf_current_page + 1, page_str);
    strcat(page_str, "/");
    char num2[8];
    k_itoa(pdf_page_count, num2);
    strcat(page_str, num2);
    int ps_len = strlen(page_str);
    winmgr_draw_text(win, nav_x + 26, PDF_MENU_Y + 7, page_str, PDF_COL_TEXT);

    /* Next button */
    int next_x = nav_x + 28 + ps_len * 8 + 4;
    winmgr_fill_rect(win, next_x, PDF_MENU_Y + 3, 20, 20, PDF_COL_BTN);
    winmgr_draw_text(win, next_x + 6, PDF_MENU_Y + 7, ">", PDF_COL_TEXT);

    /* Zoom controls on the right */
    int zx = width - 120;
    winmgr_fill_rect(win, zx, PDF_MENU_Y + 3, 20, 20, PDF_COL_BTN);
    winmgr_draw_text(win, zx + 6, PDF_MENU_Y + 7, "-", PDF_COL_TEXT);

    char zoom_str[8];
    k_itoa(pdf_zoom, zoom_str);
    strcat(zoom_str, "%");
    winmgr_draw_text(win, zx + 26, PDF_MENU_Y + 7, zoom_str, PDF_COL_DIM);

    int zoom_len = strlen(zoom_str);
    int px = zx + 28 + zoom_len * 8 + 4;
    winmgr_fill_rect(win, px, PDF_MENU_Y + 3, 20, 20, PDF_COL_BTN);
    winmgr_draw_text(win, px + 6, PDF_MENU_Y + 7, "+", PDF_COL_TEXT);
  }

  /* ---- Content Area ---- */
  int content_y = PDF_TEXT_Y;
  int content_h = height - content_y - PDF_STATUS_H;
  if (content_h < 12)
    content_h = 12;

  /* Background */
  winmgr_fill_rect(win, 0, content_y, width, content_h, PDF_COL_PAGE_BG);

  if (pdf_error_msg[0]) {
    /* Show error message */
    winmgr_draw_text(win, 16, content_y + 20, pdf_error_msg, PDF_COL_ACCENT);
  } else if (pdf_page_pixels && pdf_page_w > 0 && pdf_page_h > 0) {
    /* Blit the rendered page centered horizontally */
    int page_x = (width - pdf_page_w) / 2;
    if (page_x < 4)
      page_x = 4;
    int page_y = content_y + 4 - pdf_scroll_y;

    /* Draw white page shadow/background */
    if (page_x > 6) {
      winmgr_fill_rect(win, page_x - 2, page_y - 2, pdf_page_w + 4,
                       pdf_page_h + 4, 0xFF111111);
    }

    /* Blit pixels using fast row block-copy */
    int vis_top = content_y;
    int vis_bot = content_y + content_h;

    int src_x = 0;
    int dst_x = page_x;
    int copy_w = pdf_page_w;

    /* Horizontal clipping */
    if (dst_x < 0) {
      src_x = -dst_x;
      copy_w -= src_x;
      dst_x = 0;
    }
    if (dst_x + copy_w > width) {
      copy_w = width - dst_x;
    }

    if (copy_w > 0 && win->surface) {
      for (int py = 0; py < pdf_page_h; py++) {
        int screen_y = page_y + py;
        if (screen_y < vis_top)
          continue;
        if (screen_y >= vis_bot)
          break;

        /* Row copy */
        uint32_t *src_row = pdf_page_pixels + (size_t)py * pdf_page_w + src_x;
        uint32_t *dst_row =
            (uint32_t *)win->surface + (size_t)screen_y * win->width + dst_x;
        memcpy(dst_row, src_row, copy_w * sizeof(uint32_t));
      }
    }
  }

  /* ---- Status Bar ---- */
  int status_y = height - PDF_STATUS_H;
  winmgr_fill_rect(win, 0, status_y, width, PDF_STATUS_H, PDF_COL_STATUS);
  winmgr_fill_rect(win, 0, status_y, width, 1, PDF_COL_MENU_BORDER);

  char status_text[128];
  if (pdf_page_count > 0) {
    char pg[8], pc[8], zm[8], len[16];
    k_itoa(pdf_current_page + 1, pg);
    k_itoa(pdf_page_count, pc);
    k_itoa(pdf_zoom, zm);
    k_itoa(pdf_file_len / 1024, len);

    strcpy(status_text, "Page ");
    strcat(status_text, pg);
    strcat(status_text, " of ");
    strcat(status_text, pc);
    strcat(status_text, " | Zoom: ");
    strcat(status_text, zm);
    strcat(status_text, "% | Size: ");
    strcat(status_text, len);
    strcat(status_text, " KB");
  } else {
    strcpy(status_text, "PDF Reader Ready.");
  }
  winmgr_draw_text(win, 8, status_y + 6, status_text, PDF_COL_DIM);

  if (pdf_dialog_mode == 1) {
    int dw = 320, dh = 130;
    int dx = (width - dw) / 2;
    int dy = (height - dh) / 2;
    winmgr_fill_rect(win, dx, dy, dw, dh, PDF_COL_DIALOG_BG);
    winmgr_draw_rect(win, dx, dy, dw, dh, PDF_COL_DIALOG_BORDER);
    winmgr_draw_text(win, dx + 12, dy + 12, "Open PDF File", 0xFFFFFF);

    winmgr_fill_rect(win, dx + 12, dy + 40, dw - 24, 24, PDF_COL_INPUT_BG);
    winmgr_draw_text(win, dx + 20, dy + 46, pdf_filename, 0xFFFFFF);

    /* OK button */
    winmgr_fill_rect(win, dx + dw - 64, dy + dh - 32, 52, 22, PDF_COL_ACCENT);
    winmgr_draw_text(win, dx + dw - 52, dy + dh - 26, "OK", 0xFFFFFF);
  }

  /* Release lock (no IRQ restore needed — we never disabled IRQs) */
  __asm__ volatile("movl $0, %0" : "+m"(pdf_lock.lock.locked)::"memory");
}

/* ======================== INPUT ======================== */

static void pdfreader_on_scroll(int direction) {
  spinlock_irq_acquire(&pdf_lock);

  // Wheel UP (direction > 0) -> scroll viewport UP -> decrease scroll_y
  pdf_scroll_y -= direction * 20;

  if (pdf_scroll_y < 0)
    pdf_scroll_y = 0;

  int content_h = pdf_win ? (pdf_win->height - PDF_TEXT_Y - PDF_STATUS_H) : 400;
  int max_scroll = pdf_page_h - content_h + 8;
  if (max_scroll < 0)
    max_scroll = 0;
  if (pdf_scroll_y > max_scroll)
    pdf_scroll_y = max_scroll;

  ui_dirty = 1;
  if (pdf_win)
    pdf_win->needs_redraw = 1;

  spinlock_irq_release(&pdf_lock);
}

static void pdfreader_go_to_page(int page) {
  if (page < 0)
    page = 0;
  if (page >= pdf_page_count)
    page = pdf_page_count - 1;
  if (page == pdf_current_page && pdf_rendered_page == page)
    return;

  pdf_current_page = page;
  pdf_scroll_y = 0;
  pdf_error_msg[0] = 0;

  if (!pdf_render_page(page)) {
    /* Error already set in pdf_error_msg */
  }

  ui_dirty = 1;
  if (pdf_win)
    pdf_win->needs_redraw = 1;
}

static void pdfreader_zoom(int delta) {
  int new_zoom = pdf_zoom + delta;
  if (new_zoom < PDF_ZOOM_MIN)
    new_zoom = PDF_ZOOM_MIN;
  if (new_zoom > PDF_ZOOM_MAX)
    new_zoom = PDF_ZOOM_MAX;
  if (new_zoom == pdf_zoom)
    return;

  pdf_zoom = new_zoom;
  pdf_rendered_page = -1; /* Force re-render */
  pdf_scroll_y = 0;
  pdf_error_msg[0] = 0;

  if (pdf_doc) {
    if (!pdf_render_page(pdf_current_page)) {
      /* Error already set */
    }
  }

  ui_dirty = 1;
  if (pdf_win)
    pdf_win->needs_redraw = 1;
}

static void pdfreader_handle_dialog_input(char c) {
  int len = strlen(pdf_filename);
  if (c == '\n' || c == '\r') {
    /* Open the file */
    pdf_dialog_mode = 0;

    char *fname = (char *)kmalloc(128);
    if (fname) {
      strcpy(fname, pdf_filename);
      msg_t m = {0};
      m.type = MSG_USER + 1;
      m.ptr = fname;
      msg_send_to_name("PDFReader", &m);
    }
  } else if (c == 27) {
    /* Cancel */
    pdf_dialog_mode = 0;
  } else if (c == '\b') {
    if (len > 0)
      pdf_filename[len - 1] = 0;
  } else if (len < 31 && c >= 32 && c < 127) {
    pdf_filename[len] = c;
    pdf_filename[len + 1] = 0;
  }
  ui_dirty = 1;
  if (pdf_win)
    pdf_win->needs_redraw = 1;
}

static void pdfreader_on_mouse(void *w, int rx, int ry, int buttons) {
  window_t *win = (window_t *)w;
  if (!(buttons & 1)) {
    was_btn = 0;
    return;
  }
  int click = !was_btn;
  was_btn = 1;

  if (!click)
    return;

  /* Dialog mode clicks */
  if (pdf_dialog_mode == 1) {
    int dw = 280, dh = 110;
    int dx = (win->width - dw) / 2;
    int dy = (win->height - dh) / 2;
    /* OK button */
    if (rx >= dx + dw - 64 && rx < dx + dw - 12 && ry >= dy + 82 &&
        ry < dy + 104) {
      pdf_dialog_mode = 0;

      char *fname = (char *)kmalloc(128);
      if (fname) {
        strcpy(fname, pdf_filename);
        msg_t m = {0};
        m.type = MSG_USER + 1;
        m.ptr = fname;
        msg_send_to_name("PDFReader", &m);
      }
    }
    ui_dirty = 1;
    win->needs_redraw = 1;
    return;
  }

  /* Menu bar clicks */
  if (ry >= PDF_MENU_Y && ry < PDF_MENU_Y + PDF_MENU_H) {
    /* Open button */
    if (rx >= 6 && rx < 54) {
      pdf_dialog_mode = 1;
      ui_dirty = 1;
      win->needs_redraw = 1;
      return;
    }

    if (pdf_page_count > 0) {
      /* Prev button */
      int nav_x = 64;
      if (rx >= nav_x && rx < nav_x + 20) {
        pdfreader_go_to_page(pdf_current_page - 1);
        return;
      }

      /* Next button - calculate position */
      char page_str[24];
      k_itoa(pdf_current_page + 1, page_str);
      strcat(page_str, "/");
      char num2[8];
      k_itoa(pdf_page_count, num2);
      strcat(page_str, num2);
      int ps_len = strlen(page_str);
      int next_x = nav_x + 28 + ps_len * 8 + 4;
      if (rx >= next_x && rx < next_x + 20) {
        pdfreader_go_to_page(pdf_current_page + 1);
        return;
      }

      /* Zoom out (-) */
      int zx = win->width - 120;
      if (rx >= zx && rx < zx + 20) {
        pdfreader_zoom(-PDF_ZOOM_STEP);
        return;
      }

      /* Zoom in (+) */
      char zoom_str[8];
      k_itoa(pdf_zoom, zoom_str);
      strcat(zoom_str, "%");
      int zoom_len = strlen(zoom_str);
      int px = zx + 28 + zoom_len * 8 + 4;
      if (rx >= px && rx < px + 20) {
        pdfreader_zoom(PDF_ZOOM_STEP);
        return;
      }
    }

    ui_dirty = 1;
    win->needs_redraw = 1;
  }
}

static void pdfreader_on_key(void *w, int key, char ascii) {
  (void)w;
  (void)key;

  if (pdf_dialog_mode > 0) {
    if (ascii)
      pdfreader_handle_dialog_input(ascii);
    return;
  }

  /* Navigation keys */
  if (ascii == 0) {
    /* Arrow keys via scancode */
    if (key == 0x48) { /* Up */
      pdfreader_on_scroll(1);
    } else if (key == 0x50) { /* Down */
      pdfreader_on_scroll(-1);
    } else if (key == 0x4B) { /* Left = prev page */
      pdfreader_go_to_page(pdf_current_page - 1);
    } else if (key == 0x4D) { /* Right = next page */
      pdfreader_go_to_page(pdf_current_page + 1);
    }
  } else if (ascii == ' ') {
    /* Space = page down */
    pdfreader_on_scroll(-10);
  } else if (ascii == '+' || ascii == '=') {
    pdfreader_zoom(PDF_ZOOM_STEP);
  } else if (ascii == '-') {
    pdfreader_zoom(-PDF_ZOOM_STEP);
  }
}

/* ======================== THREAD ======================== */

static void pdfreader_thread_entry(void) {
  print_serial("PDFReader thread started.\n");

  msg_t msg;
  while (pdf_running) {
    if (msg_receive(&msg)) {
      if (msg.type == MSG_KEYBOARD) {
        pdfreader_on_key(pdf_win, msg.d1, (char)msg.d2);
      } else if (msg.type == MSG_MOUSE_CLICK) {
        pdfreader_on_mouse(pdf_win, msg.d1, msg.d2, msg.d3);
      } else if (msg.type == MSG_MOUSE_SCROLL) {
        pdfreader_on_scroll(msg.d4);
      } else if (msg.type == MSG_QUIT) {
        pdf_running = 0;
        break;
      } else if (msg.type == MSG_USER + 1) {
        char *fname = (char *)msg.ptr;
        if (fname) {
          pdfreader_open_file(fname);
          kfree(fname);
        }
      }
    } else {
      /* No messages — put this task to sleep.
         msg_send() will wake us (set TASK_READY) when a message arrives. */
      task_t *self = get_current_task();
      if (self)
        self->state = TASK_STOPPED;
      __asm__ volatile("int $32");
    }
  }

  /* Cleanup MuPDF resources */
  pdf_cleanup_mupdf();
  if (pdf_file_buf) {
    kfree(pdf_file_buf);
    pdf_file_buf = 0;
  }

  print_serial("PDFReader thread exiting.\n");
  pdf_running = 0;
  exit(0);
}

/* ======================== INIT ======================== */

void pdfreader_init(void) {
  if (pdf_running) {
    task_t *t = get_task_by_name("PDFReader");
    if (t && t->state != TASK_STOPPED && t->state != TASK_ZOMBIE) {
      if (pdf_win) {
        pdf_win->is_minimized = 0;
        winmgr_bring_to_front(pdf_win);
      }
      return;
    } else {
      pdf_running = 0;
    }
  }

  /* Create window */
  pdf_win = winmgr_create_window(120, 80, 550, 500, "PDF Reader");
  if (!pdf_win) {
    print_serial("PDFREADER: Failed to create window (OOM)\n");
    return;
  }

  spinlock_irq_init(&pdf_lock);

  pdf_win->app_type = 10; /* APP_PDFREADER */
  pdf_win->bg_color = PDF_COL_BG;
  pdf_win->draw = (void (*)(void *))pdfreader_draw;
  pdf_win->on_mouse = (void (*)(void *, int, int, int))pdfreader_on_mouse;
  pdf_win->on_key = (void (*)(void *, int, char))pdfreader_on_key;
  pdf_win->owner_pid = 0;

  /* Reset state */
  pdf_file_len = 0;
  pdf_scroll_y = 0;
  pdf_dialog_mode = 0;
  pdf_page_count = 0;
  pdf_current_page = 0;
  pdf_zoom = 100;
  pdf_error_msg[0] = 0;
  pdf_rendered_page = -1;
  pdf_page_pixels = 0;
  strcpy(pdf_filename, "sample.pdf");

  print_serial("PDF: Window created.\n");

  /* NOTE: Do NOT call pdf_init_mupdf() here!
     It allocates 16MB and does heavy init which would freeze the
     desktop task (our caller).  MuPDF is initialized lazily in
     pdfreader_open_file() which runs in the PDFReader thread. */

  /* Start thread */
  pdf_running = 1;

  task_t *t = create_task(pdfreader_thread_entry, "PDFReader");
  if (t) {
    task_set_priority(t->id, 3);
    pdf_win->owner_pid = t->id;
  } else {
    print_serial("PDFREADER: Failed to start thread\n");
    pdf_running = 0;
    winmgr_close_window(pdf_win);
    pdf_win = 0;
  }
}

/* Open a PDF file directly (called from Explorer/File Manager) */
void pdfreader_open(const char *filename) {
  if (!filename || filename[0] == 0)
    return;

  if (!pdf_win || pdf_win->id == 0)
    pdfreader_init();
  if (!pdf_win)
    return;

  char *fname = (char *)kmalloc(128);
  if (fname) {
    int i = 0;
    while (filename[i] && i < 127) {
      fname[i] = filename[i];
      i++;
    }
    fname[i] = 0;

    msg_t m = {0};
    m.type = MSG_USER + 1;
    m.ptr = fname;
    msg_send_to_name("PDFReader", &m);
  }

  pdf_win->is_minimized = 0;
  winmgr_bring_to_front(pdf_win);
}
