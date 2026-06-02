#include "../kernel/ui_layout.h"
#include "../fs/fs.h"
#include "../kernel/heap.h"
#include "../kernel/image.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"

#define MAX_PHOTOS 128
#define THUMB_SIZE 120
#define GRID_PADDING 24
#define GRID_TEXT_H 20
#define TITLE_H 32

typedef struct {
  uint32_t *data;
  int w, h;
} thumb_t;

typedef struct {
  char filenames[MAX_PHOTOS][32];
  int file_sizes[MAX_PHOTOS];
  thumb_t thumbs[MAX_PHOTOS];
  int count;
  int selected_idx; // -1 for gallery, 0+ for viewer

  // Loaded full image data
  uint32_t *img_data;
  int img_w;
  int img_h;

  int scroll_y;
  char status[64];
  int prev_buttons;
} photos_app_t;

extern void print_serial(const char *);
extern void k_itoa(int, char *);

static void photos_load_image(photos_app_t *app, const char *filename, int expected_size) {
  if (app->img_data) {
    stbi_image_free(app->img_data);
    app->img_data = 0;
  }

  print_serial("PHOTOS: Loading ");
  print_serial(filename);
  print_serial("\n");

  if (expected_size <= 0) {
    FileInfo files[MAX_PHOTOS];
    int count = fs_list_files("/", files, MAX_PHOTOS);
    for (int i = 0; i < count; i++) {
      if (strcmp(files[i].name, filename) == 0) {
        expected_size = files[i].size;
        break;
      }
    }
  }

  if (expected_size <= 0 || expected_size > 1024 * 1024 * 64) {
    strcpy(app->status, "File too large/invalid.");
    return;
  }

  uint8_t *raw_data = (uint8_t *)malloc(expected_size + 16);
  if (!raw_data) {
    strcpy(app->status, "Out of memory.");
    return;
  }

  int size = fs_read(filename, raw_data);
  if (size <= 0) {
    free(raw_data);
    strcpy(app->status, "Read failed.");
    return;
  }

  int w, h, n;
  unsigned char *pixels = stbi_load_from_memory(raw_data, size, &w, &h, &n, 4);
  free(raw_data);

  if (!pixels) {
    strcpy(app->status, "Decode failed.");
    return;
  }

  // Convert to ARGB (STB is RGBA)
  uint32_t *p32 = (uint32_t *)pixels;
  for (int i = 0; i < w * h; i++) {
    uint32_t p = p32[i];
    uint8_t r = p & 0xFF;
    uint8_t g = (p >> 8) & 0xFF;
    uint8_t b = (p >> 16) & 0xFF;
    uint8_t a = (p >> 24) & 0xFF;
    if (a == 0) a = 255;
    p32[i] = (a << 24) | (r << 16) | (g << 8) | b;
  }

  app->img_data = (uint32_t *)pixels;
  app->img_w = w;
  app->img_h = h;

  strcpy(app->status, filename);
}

static void photos_create_thumbnail(photos_app_t *app, int idx) {
  if (app->thumbs[idx].data) return;

  uint8_t *raw_data = (uint8_t *)malloc(app->file_sizes[idx] + 16);
  if (!raw_data) return;
  int size = fs_read(app->filenames[idx], raw_data);
  if (size <= 0) { free(raw_data); return; }

  int w, h, n;
  unsigned char *pixels = stbi_load_from_memory(raw_data, size, &w, &h, &n, 4);
  free(raw_data);
  if (!pixels) return;

  int tw = THUMB_SIZE;
  int th = (h * THUMB_SIZE) / w;
  if (th > THUMB_SIZE) {
    th = THUMB_SIZE;
    tw = (w * THUMB_SIZE) / h;
  }

  uint32_t *thumb_data = (uint32_t *)malloc(tw * th * 4);
  if (!thumb_data) { stbi_image_free(pixels); return; }

  uint32_t *src32 = (uint32_t *)pixels;
  for (int y = 0; y < th; y++) {
    int src_y = (y * h) / th;
    for (int x = 0; x < tw; x++) {
      int src_x = (x * w) / tw;
      uint32_t p = src32[src_y * w + src_x];
      uint8_t r = p & 0xFF;
      uint8_t g = (p >> 8) & 0xFF;
      uint8_t b = (p >> 16) & 0xFF;
      uint8_t a = (p >> 24) & 0xFF;
      if (a == 0) a = 255;
      thumb_data[y * tw + x] = (a << 24) | (r << 16) | (g << 8) | b;
    }
  }

  app->thumbs[idx].data = thumb_data;
  app->thumbs[idx].w = tw;
  app->thumbs[idx].h = th;

  stbi_image_free(pixels);
}

static void photos_refresh_list(photos_app_t *app) {
  FileInfo files[MAX_PHOTOS];
  int count = fs_list_files("/", files, MAX_PHOTOS);

  app->count = 0;
  for (int i = 0; i < count; i++) {
    const char *name = files[i].name;
    int len = strlen(name);

    if (len > 4) {
      const char *ext = name + len - 4;
      int match = 0;
      if (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0) match = 1;
      if (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0) match = 1;

      if (match) {
        strcpy(app->filenames[app->count], name);
        app->file_sizes[app->count] = files[i].size;
        app->count++;
        if (app->count >= MAX_PHOTOS) break;
      }
    }
  }

  if (app->count == 0) strcpy(app->status, "No images found.");
  else strcpy(app->status, "Gallery View");
}

static void photos_draw_gallery(window_t *win, photos_app_t *app) {
  const theme_t *theme = theme_get();
  int w = win->width;
  int h = win->height;

  int cell_w = THUMB_SIZE + GRID_PADDING;
  int cell_h = THUMB_SIZE + GRID_PADDING + GRID_TEXT_H;
  int cols = (w - GRID_PADDING) / cell_w;
  if (cols < 1) cols = 1;

  int start_x = (w - (cols * cell_w - GRID_PADDING)) / 2;
  int start_y = TITLE_H + GRID_PADDING - app->scroll_y;

  for (int i = 0; i < app->count; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = start_x + col * cell_w;
    int y = start_y + row * cell_h;

    // Viewport check
    if (y + cell_h < 0 || y > h) continue;

    // Load thumbnail if needed (Lazy Loading)
    if (!app->thumbs[i].data) {
      photos_create_thumbnail(app, i);
    }

    if (app->thumbs[i].data) {
      // Center thumbnail in its cell
      int tx = x + (THUMB_SIZE - app->thumbs[i].w) / 2;
      int ty = y + (THUMB_SIZE - app->thumbs[i].h) / 2;
      
      // Draw shadow/border
      winmgr_draw_rect(win, tx - 1, ty - 1, app->thumbs[i].w + 2, app->thumbs[i].h + 2, theme->border);
      
      // Draw image
      winmgr_blit(win, tx, ty, app->thumbs[i].data, app->thumbs[i].w, app->thumbs[i].h, 0, 0, app->thumbs[i].w, app->thumbs[i].h);
    } else {
      // Loading placeholder
      winmgr_fill_rect(win, x, y, THUMB_SIZE, THUMB_SIZE, theme->menu_bg);
    }

    // Filename
    char name_buf[16];
    strncpy(name_buf, app->filenames[i], 12);
    name_buf[12] = 0;
    if (strlen(app->filenames[i]) > 12) strcat(name_buf, "..");
    
    int tw = strlen(name_buf) * 8;
    winmgr_draw_text(win, x + (THUMB_SIZE - tw) / 2, y + THUMB_SIZE + 5, name_buf, theme->fg);
  }

  // Update max scroll
  int total_rows = (app->count + cols - 1) / cols;
  win->max_scroll = (total_rows * cell_h) - h + GRID_PADDING * 2;
  if (win->max_scroll < 0) win->max_scroll = 0;
}

static void photos_draw_viewer(window_t *win, photos_app_t *app) {
  const theme_t *theme = theme_get();
  if (!app->img_data) {
    winmgr_draw_text(win, 20, 20, "Loading...", theme->fg);
    return;
  }

  int vw = win->width;
  int vh = win->height - TITLE_H;
  int dw = app->img_w;
  int dh = app->img_h;

  int tw = dw;
  int th = dh;

  if (dw > vw || dh > vh) {
    float sw = (float)vw / dw;
    float sh = (float)vh / dh;
    float scale = (sw < sh) ? sw : sh;
    tw = (int)(dw * scale);
    th = (int)(dh * scale);
  }

  int dx = (vw - tw) / 2;
  int dy = TITLE_H + (vh - th) / 2;

  // Background for viewer (darker)
  winmgr_fill_rect(win, 0, TITLE_H, vw, vh, 0xFF000000);

  // Scaled drawing
  for (int y = 0; y < th; y++) {
    int sy = (y * dh) / th;
    uint32_t *src_row = &app->img_data[sy * dw];
    for (int x = 0; x < tw; x++) {
      int sx = (x * dw) / tw;
      winmgr_put_pixel(win, dx + x, dy + y, src_row[sx]);
    }
  }

  // Back button (top left of content area)
  int bx = 10;
  int by = TITLE_H + 10;
  winmgr_fill_rect(win, bx, by, 80, 30, 0x80000000);
  winmgr_draw_rect(win, bx, by, 80, 30, 0xFFFFFFFF);
  winmgr_draw_text(win, bx + 15, by + 8, "< Back", 0xFFFFFFFF);
}

static void photos_draw(void *w) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  const theme_t *theme = theme_get();

  winmgr_fill_rect(win, 0, TITLE_H, win->width, win->height - TITLE_H, theme->bg);

  if (app->selected_idx == -1) {
    photos_draw_gallery(win, app);
  } else {
    photos_draw_viewer(win, app);
  }
}

static void photos_on_mouse(void *w, int mx, int my, int buttons) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;

  if (buttons && !app->prev_buttons) {
    if (app->selected_idx == -1) {
      // Gallery interaction
      int cell_w = THUMB_SIZE + GRID_PADDING;
      int cell_h = THUMB_SIZE + GRID_PADDING + GRID_TEXT_H;
      int cols = (win->width - GRID_PADDING) / cell_w;
      if (cols < 1) cols = 1;

      int start_x = (win->width - (cols * cell_w - GRID_PADDING)) / 2;
      int start_y = TITLE_H + GRID_PADDING - app->scroll_y;

      for (int i = 0; i < app->count; i++) {
        int col = i % cols;
        int row = i / cols;
        int x = start_x + col * cell_w;
        int y = start_y + row * cell_h;

        if (mx >= x && mx < x + THUMB_SIZE && my >= y && my < y + THUMB_SIZE) {
          app->selected_idx = i;
          photos_load_image(app, app->filenames[i], app->file_sizes[i]);
          win->needs_redraw = 1;
          break;
        }
      }
    } else {
      // Back button click (10, TITLE_H + 10, 80, 30)
      if (mx >= 10 && mx <= 90 && my >= TITLE_H + 10 && my <= TITLE_H + 40) {
        app->selected_idx = -1;
        if (app->img_data) {
          stbi_image_free(app->img_data);
          app->img_data = 0;
        }
        win->needs_redraw = 1;
      }
    }
  }
  app->prev_buttons = buttons;
}

static void photos_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  if (!app) return;

  if (app->selected_idx == -1) {
    app->scroll_y -= direction * 40;
    if (app->scroll_y < 0) app->scroll_y = 0;
    if (app->scroll_y > win->max_scroll) app->scroll_y = win->max_scroll;
    win->needs_redraw = 1;
  }
}

static void photos_on_close(void *w) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  if (app) {
    if (app->img_data) stbi_image_free(app->img_data);
    for (int i = 0; i < MAX_PHOTOS; i++) {
      if (app->thumbs[i].data) free(app->thumbs[i].data);
    }
    kfree(app);
    win->user_data = 0;
  }
}

void photos_open(const char *path) {
  window_t *win = winmgr_get_window_by_app_type(12);
  photos_app_t *app;

  if (win) {
    app = (photos_app_t *)win->user_data;
    winmgr_bring_to_front(win);
  } else {
    win = winmgr_create_window(-1, -1, 800, 600, "Photos");
    if (!win) return;

    app = (photos_app_t *)malloc(sizeof(photos_app_t));
    memset(app, 0, sizeof(photos_app_t));
    app->selected_idx = -1;

    win->user_data = app;
    win->draw = photos_draw;
    win->on_mouse = photos_on_mouse;
    win->on_scroll = (void (*)(void *, int))photos_on_scroll;
    win->on_close = photos_on_close;
    win->app_type = 12;

    photos_refresh_list(app);
  }

  if (path && path[0] != 0) {
    // Open specific file
    for (int i = 0; i < app->count; i++) {
      if (strstr(path, app->filenames[i])) {
        app->selected_idx = i;
        photos_load_image(app, path, 0);
        break;
      }
    }
  }

  win->needs_redraw = 1;
}

void photos_init() { photos_open(0); }
