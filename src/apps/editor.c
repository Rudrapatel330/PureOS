#include "../fs/fs.h"
#include "../kernel/heap.h"
#include "../kernel/spinlock.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "../kernel/window.h"

extern void print_serial(const char *s);
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
static window_t *editor_win = 0;
static char ed_filename[32] = "untitled.txt";
static int ed_scroll_y = 0;
static int ed_dialog_mode = 0;
static int ed_cursor_blink = 0;
static int ed_running = 0;
static int was_btn = 0;
extern int ui_dirty;
static spinlock_irq_t editor_lock;

static int ed_cursor_pos = 0;
static int ed_selected_all = 0;
extern int ctrl_pressed;

#define KEY_LEFT  0x4B
#define KEY_RIGHT 0x4D
#define KEY_UP    0x48
#define KEY_DOWN  0x50

/* Global buffer */
char editor_buffer[8192] =
    "Welcome to ProEditor!\nTry typing something...\n\nPureOS is moving faster "
    "now.\nScrolling testing line 1\nScrolling testing line 2\nScrolling "
    "testing line 3\nScrolling testing line 4\nScrolling testing line "
    "5\nScrolling testing line 6\nScrolling testing line 7\nScrolling testing "
    "line 8\nScrolling testing line 9\nScrolling testing line 10\nScrolling "
    "testing line 11\nScrolling testing line 12\nScrolling testing line "
    "13\nScrolling testing line 14\nScrolling testing line 15";

/* Temp buffer for safe file reading */
static uint8_t ed_read_buf[8192];

/* ======================== DRAW ======================== */

#define MENU_Y 24
#define MENU_H 22
#define TEXT_Y 46
#define STATUS_H 18

static void editor_handle_input(char c);
static void editor_on_mouse(void *w, int rx, int ry, int buttons);
static void editor_on_key(void *w, int key, char ascii);

static void editor_draw(void *w) {
  window_t *win = (window_t *)w;
  if (!win)
    return;

  int width = win->width;
  int height = win->height;

  /* Menu bar - dark gray */
  winmgr_fill_rect(win, 0, MENU_Y, width, MENU_H, 0xFF2C313A);
  winmgr_fill_rect(win, 0, MENU_Y + MENU_H - 1, width, 1, 0xFF444B58);
  winmgr_draw_text(win, 10, MENU_Y + 4, "Save", 0xFFD4D8DF);
  winmgr_draw_text(win, 52, MENU_Y + 4, "Open", 0xFFD4D8DF);
  winmgr_draw_text(win, 94, MENU_Y + 4, "New", 0xFFD4D8DF);

  /* Filename centered */
  int fn_len = strlen(ed_filename);
  int fn_x = (width - fn_len * 8) / 2;
  if (fn_x < 130)
    fn_x = 130;
  winmgr_draw_text(win, fn_x, MENU_Y + 4, ed_filename, 0xFF6B737E);

  /* Text area - dark background */
  int text_h = height - TEXT_Y - STATUS_H;
  if (text_h < 12)
    text_h = 12;
  if (ed_selected_all) {
    winmgr_fill_rect(win, 0, TEXT_Y, width, text_h, 0xFF3E4451);
  } else {
    winmgr_fill_rect(win, 0, TEXT_Y, width, text_h, 0xFF282C34);
  }

  /* Draw text with scrolling */
  int cx = 8;
  int cy = TEXT_Y + 4;
  int max_y = TEXT_Y + text_h - 14;
  
  /* Copy buffer locally to avoid holding lock during long draw loop */
  char local_buf[8192];
  spinlock_irq_acquire(&editor_lock);
  int buf_len = strlen(editor_buffer);
  if (buf_len > 8191)
    buf_len = 8191;
  for (int i = 0; i < buf_len; i++)
    local_buf[i] = editor_buffer[i];
  local_buf[buf_len] = 0;
  int local_cursor = ed_cursor_pos;
  if (local_cursor > buf_len) local_cursor = buf_len;
  spinlock_irq_release(&editor_lock);

  // PASS 1: Auto-scroll
  char *text1 = local_buf;
  int lx = 8;
  int logical_line = 0;
  int c_index1 = 0;
  while (1) {
      if (c_index1 == local_cursor) {
          if (logical_line < ed_scroll_y) {
              ed_scroll_y = logical_line;
              ui_dirty = 1; win->needs_redraw = 1;
          } else {
              int ly = (TEXT_Y + 4) + (logical_line * 14);
              int screen_cy = ly - (ed_scroll_y * 14);
              if (screen_cy > max_y) {
                  ed_scroll_y += ((screen_cy - max_y) / 14) + 1;
                  ui_dirty = 1; win->needs_redraw = 1;
              }
          }
      }
      if (!*text1) break;
      
      if (*text1 == '\n') {
          lx = 8;
          logical_line++;
      } else {
          lx += 8;
          if (lx > width - 12) {
              lx = 8;
              logical_line++;
          }
      }
      text1++;
      c_index1++;
  }

  // PASS 2: Drawing
  char *text = local_buf;
  int current_line = 0;
  int c_index = 0;
  int cursor_draw_x = -1;
  int cursor_draw_y = -1;

  while (*text && cy <= max_y) {
    if (current_line >= ed_scroll_y) {
      if (c_index == local_cursor) {
        cursor_draw_x = cx;
        cursor_draw_y = cy;
      }
      if (*text == '\n') {
        cx = 8;
        cy += 14;
        current_line++;
      } else {
        char tmp[2] = {*text, 0};
        winmgr_draw_text(win, cx, cy, tmp, 0xD4D8DF);
        cx += 8;
        if (cx > width - 12) {
          cx = 8;
          cy += 14;
          current_line++;
        }
      }
    } else {
      // Still skipping lines before scroll
      if (*text == '\n') {
        current_line++;
      } else {
        cx += 8;
        if (cx > width - 12) {
          cx = 8;
          current_line++;
        }
      }
    }
    text++;
    c_index++;
  }
  
  if (c_index == local_cursor && current_line >= ed_scroll_y && cy <= max_y) {
    cursor_draw_x = cx;
    cursor_draw_y = cy;
  }

  /* Cursors and status bar... */
  /* (Skipping some redundant logic for brevity in replace, but keeping it in
   * the actual file) */
  /* Blinking cursor at cursor pos */
  ed_cursor_blink++;
  if ((ed_cursor_blink / 15) % 2 == 0) {
    // Only draw cursor if it's on a visible line
    if (cursor_draw_x >= 0 && cursor_draw_y >= 0) {
      winmgr_fill_rect(win, cursor_draw_x, cursor_draw_y, 2, 14, 0xFFE06C75);
    }
  }

  /* Status bar */
  int sy = height - STATUS_H;
  winmgr_fill_rect(win, 0, sy, width, STATUS_H, 0xFF21252B);
  winmgr_fill_rect(win, 0, sy, width, 1, 0xFF444B58);

  spinlock_irq_acquire(&editor_lock);
  int total = strlen(editor_buffer);
  spinlock_irq_release(&editor_lock);

  char sbuf[40];
  strcpy(sbuf, "Chars: ");
  char num[16];
  k_itoa(total, num);
  strcat(sbuf, num);
  winmgr_draw_text(win, 8, sy + 3, sbuf, 0x6B737E);
  winmgr_draw_text(win, width - 80, sy + 3, ed_filename, 0x6B737E);

  /* Dialog overlay */
  if (ed_dialog_mode > 0) {
    int dw = 260, dh = 100;
    int dx = (width - dw) / 2;
    int dy = (height - dh) / 2;

    winmgr_fill_rect(win, dx + 2, dy + 2, dw, dh, 0xFF111418);
    winmgr_fill_rect(win, dx, dy, dw, dh, 0xFF2C313A);
    winmgr_draw_rect(win, dx, dy, dw, dh, 0xFFE06C75);

    const char *title = (ed_dialog_mode == 1) ? "Save As" : "Open File";
    winmgr_draw_text(win, dx + 10, dy + 8, title, 0xFFE06C75);

    winmgr_fill_rect(win, dx + 10, dy + 30, dw - 20, 22, 0xFF282C34);
    winmgr_draw_rect(win, dx + 10, dy + 30, dw - 20, 22, 0xFF444B58);
    winmgr_draw_text(win, dx + 14, dy + 34, ed_filename, 0xFFD4D8DF);

    if ((ed_cursor_blink / 15) % 2 == 0) {
      int cl = strlen(ed_filename);
      winmgr_fill_rect(win, dx + 14 + cl * 8, dy + 32, 2, 16, 0xFFE06C75);
    }

    winmgr_draw_text(win, dx + 10, dy + 60, "Enter=OK  Esc=Cancel", 0xFF6B737E);

    winmgr_fill_rect(win, dx + dw - 60, dy + 72, 50, 20, 0xFFE06C75);
    winmgr_draw_text(win, dx + dw - 50, dy + 76, "OK", 0xFF1E2228);
  }
}

static void editor_on_scroll(int direction) {
  ed_scroll_y -= direction; // Scroll 1 line per notch for smooth feel
  if (ed_scroll_y < 0)
    ed_scroll_y = 0;
  // TODO: Clamp max scroll if we know the line count
  ui_dirty = 1;
  if (editor_win)
    editor_win->needs_redraw = 1;
}

static void editor_handle_input(char c) {

  if (!editor_win)
    return;

  if (ed_dialog_mode > 0) {
    int len = strlen(ed_filename);
    if (c == '\n' || c == '\r') {
      if (ed_dialog_mode == 1) {
        fs_write(ed_filename, (uint8_t *)editor_buffer, strlen(editor_buffer));
      } else if (ed_dialog_mode == 2) {
        char *fname = (char *)kmalloc(32);
        if (fname) {
          strcpy(fname, ed_filename);
          msg_t m = {0};
          m.type = MSG_USER + 1;
          m.ptr = fname;
          msg_send_to_name("Editor", &m);
        }
      }
      ed_dialog_mode = 0;
    } else if (c == 27) {
      ed_dialog_mode = 0;
    } else if (c == '\b') {
      if (len > 0)
        ed_filename[len - 1] = 0;
    } else if (len < 31 && c >= 32 && c < 127) {
      ed_filename[len] = c;
      ed_filename[len + 1] = 0;
    }
    ui_dirty = 1;
    editor_win->needs_redraw = 1;
    return;
  }

  spinlock_irq_acquire(&editor_lock);
  if (ed_selected_all) {
      editor_buffer[0] = 0;
      ed_cursor_pos = 0;
      ed_scroll_y = 0;
      ed_selected_all = 0;
      if (c == '\b') {
          ui_dirty = 1;
          editor_win->needs_redraw = 1;
          spinlock_irq_release(&editor_lock);
          return;
      }
  }
  int len = strlen(editor_buffer);
  if (c == '\b') {
    if (ed_cursor_pos > 0) {
      for (int i = ed_cursor_pos - 1; i <= len; i++) {
          editor_buffer[i] = editor_buffer[i + 1];
      }
      ed_cursor_pos--;
      ui_dirty = 1;
      editor_win->needs_redraw = 1;
      // Scroll up if we deleted enough to see previous lines?
      // Simplified: just reset scroll if buffer is small
      if (len < 10) ed_scroll_y = 0;
    }
  } else if (c == '\n' || (c >= 32 && c < 127)) {
    if (len < 8191) {
      for (int i = len; i >= ed_cursor_pos; i--) {
          editor_buffer[i + 1] = editor_buffer[i];
      }
      editor_buffer[ed_cursor_pos] = c;
      ed_cursor_pos++;
      ui_dirty = 1;
      editor_win->needs_redraw = 1;
    }
  }
  spinlock_irq_release(&editor_lock);
}

static void editor_on_mouse(void *w, int rx, int ry, int buttons) {
  window_t *win = (window_t *)w;
  if (!(buttons & 1)) {
    was_btn = 0;
    return;
  }
  int click = !was_btn;
  was_btn = 1;

  if (ed_dialog_mode > 0 && click) {
    int dw = 260, dh = 100;
    int dx = (win->width - dw) / 2;
    int dy = (win->height - dh) / 2;
    if (rx >= dx + dw - 60 && rx < dx + dw - 10 && ry >= dy + 72 &&
        ry < dy + 92) {
      if (ed_dialog_mode == 1)
        fs_write(ed_filename, (uint8_t *)editor_buffer, strlen(editor_buffer));
      else if (ed_dialog_mode == 2) {
        char *fname = (char *)kmalloc(32);
        if (fname) {
          strcpy(fname, ed_filename);
          msg_t m = {0};
          m.type = MSG_USER + 1;
          m.ptr = fname;
          msg_send_to_name("Editor", &m);
        }
      }
      ed_dialog_mode = 0;
      ui_dirty = 1;
      win->needs_redraw = 1;
    }
    return;
  }

  if (ry >= TEXT_Y && click) {
     ed_selected_all = 0;
     ui_dirty = 1;
     win->needs_redraw = 1;
  }

  if (ry >= MENU_Y && ry < MENU_Y + MENU_H && click) {
    if (rx >= 6 && rx < 48) {
      ed_dialog_mode = 1;
    } else if (rx >= 48 && rx < 90) {
      ed_dialog_mode = 2;
    } else if (rx >= 90 && rx < 128) {
      spinlock_irq_acquire(&editor_lock);
      editor_buffer[0] = 0;
      ed_cursor_pos = 0;
      spinlock_irq_release(&editor_lock);
      strcpy(ed_filename, "untitled.txt");
      ed_scroll_y = 0;
    }
    ui_dirty = 1;
    win->needs_redraw = 1;
  }
}

static void editor_on_key(void *w, int key, char ascii) {
  (void)w;
  ed_cursor_blink = 0;
  if (ed_dialog_mode > 0) {
      if (ascii) editor_handle_input(ascii);
      return;
  }

  spinlock_irq_acquire(&editor_lock);
  int len = strlen(editor_buffer);

  if (ctrl_pressed) {
      if (ascii == 'a' || ascii == 'A') {
          ed_selected_all = 1;
          ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
          spinlock_irq_release(&editor_lock);
          return;
      } else if (ascii == 'n' || ascii == 'N') {
          editor_buffer[0] = 0;
          strcpy(ed_filename, "untitled.txt");
          ed_scroll_y = 0;
          ed_cursor_pos = 0;
          ui_dirty = 1;
          if (editor_win) editor_win->needs_redraw = 1;
      } else if (ascii == 's' || ascii == 'S') {
          int blen = len;
          spinlock_irq_release(&editor_lock);
          fs_write(ed_filename, (uint8_t *)editor_buffer, blen);
          ui_dirty = 1;
          if (editor_win) editor_win->needs_redraw = 1;
          return;
      } else if (key == KEY_LEFT) {
          ed_selected_all = 0;
          while (ed_cursor_pos > 0 && editor_buffer[ed_cursor_pos - 1] == ' ') ed_cursor_pos--;
          while (ed_cursor_pos > 0 && editor_buffer[ed_cursor_pos - 1] != ' ' && editor_buffer[ed_cursor_pos - 1] != '\n') ed_cursor_pos--;
          ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
      } else if (key == KEY_RIGHT) {
          ed_selected_all = 0;
          while (ed_cursor_pos < len && editor_buffer[ed_cursor_pos] == ' ') ed_cursor_pos++;
          while (ed_cursor_pos < len && editor_buffer[ed_cursor_pos] != ' ' && editor_buffer[ed_cursor_pos] != '\n') ed_cursor_pos++;
          ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
      } else if (ascii == '\b') {
          if (ed_selected_all) {
              editor_buffer[0] = 0;
              ed_cursor_pos = 0;
              ed_scroll_y = 0;
              ed_selected_all = 0;
              ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
              spinlock_irq_release(&editor_lock);
              return;
          }
          int old_pos = ed_cursor_pos;
          while (ed_cursor_pos > 0 && editor_buffer[ed_cursor_pos - 1] == ' ') ed_cursor_pos--;
          while (ed_cursor_pos > 0 && editor_buffer[ed_cursor_pos - 1] != ' ' && editor_buffer[ed_cursor_pos - 1] != '\n') ed_cursor_pos--;
          int chars_to_remove = old_pos - ed_cursor_pos;
          if (chars_to_remove > 0) {
              for (int i = ed_cursor_pos; i <= len - chars_to_remove; i++) {
                  editor_buffer[i] = editor_buffer[i + chars_to_remove];
              }
              ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
          }
      }
      spinlock_irq_release(&editor_lock);
      return;
  }

  if (key == KEY_LEFT) {
      ed_selected_all = 0;
      if (ed_cursor_pos > 0) ed_cursor_pos--;
      ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
      spinlock_irq_release(&editor_lock);
      return;
  } else if (key == KEY_RIGHT) {
      ed_selected_all = 0;
      if (ed_cursor_pos < len) ed_cursor_pos++;
      ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
      spinlock_irq_release(&editor_lock);
      return;
  } else if (key == KEY_UP) {
      ed_selected_all = 0;
      int p = ed_cursor_pos - 1;
      while(p > 0 && editor_buffer[p-1] != '\n') p--;
      if (p > 0) {
          int p2 = p - 1;
          if (p2 > 0 && editor_buffer[p2] == '\n') p2--;
          while(p2 > 0 && editor_buffer[p2-1] != '\n') p2--;
          ed_cursor_pos = p2;
      } else {
          ed_cursor_pos = 0;
      }
      ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
      spinlock_irq_release(&editor_lock);
      return;
  } else if (key == KEY_DOWN) {
      ed_selected_all = 0;
      int p = ed_cursor_pos;
      while(p < len && editor_buffer[p] != '\n') p++;
      if (p < len) {
          p++;
          ed_cursor_pos = p;
      } else {
          ed_cursor_pos = len;
      }
      ui_dirty = 1; if (editor_win) editor_win->needs_redraw = 1;
      spinlock_irq_release(&editor_lock);
      return;
  }

  spinlock_irq_release(&editor_lock);
  if (ascii) editor_handle_input(ascii);
}

void editor_thread_entry(void) {
  print_serial("Editor thread started.\n");

  msg_t msg;
  while (ed_running) {
    if (msg_receive(&msg)) {
      if (msg.type == MSG_KEYBOARD) {
        editor_on_key(editor_win, msg.d1, (char)msg.d2);
      } else if (msg.type == MSG_MOUSE_CLICK) {
        editor_on_mouse(editor_win, msg.d1, msg.d2, msg.d3);
      } else if (msg.type == MSG_MOUSE_SCROLL) {
        editor_on_scroll(msg.d4);
      } else if (msg.type == MSG_QUIT) {
        ed_running = 0;
        break;
      } else if (msg.type == MSG_USER + 1) {
        char *fname = (char *)msg.ptr;
        if (fname) {
          extern void editor_open_internal(const char *fn);
          editor_open_internal(fname);
          kfree(fname);
        }
      }
      // Process other messages if needed
    } else {
      /* No messages — put this task to sleep.
         msg_send() will wake us (set TASK_READY) when a message arrives.
         This prevents starving the desktop task. */
      task_t *self = get_current_task();
      if (self)
        self->state = TASK_STOPPED;
      __asm__ volatile("int $32");
    }
  }

  print_serial("Editor thread exiting.\n");
  ed_running = 0;
  exit(0);
}

static void editor_on_close(void *w) {
  (void)w;
  ed_running = 0;
  editor_win = 0;
}

void editor_init(void) {
  if (ed_running) {
    task_t *t = get_task_by_name("Editor");
    if (t && t->state != TASK_STOPPED && t->state != TASK_ZOMBIE) {
      if (editor_win) {
        editor_win->is_minimized = 0;
        winmgr_bring_to_front(editor_win);
      }
      return;
    } else {
      // Stale running flag or dead task
      ed_running = 0;
    }
  }

  ed_cursor_pos = strlen(editor_buffer);

  // Create window
  print_serial("EDITOR: Calling winmgr_create_window\n");
  window_t *win = winmgr_create_window(100, 100, 600, 450, "Editor");
  if (!win) {
    print_serial("EDITOR: Failed to create window (OOM)\n");
    return;
  }
  print_serial("EDITOR: Window created\n");
  editor_win = win;

  spinlock_irq_init(&editor_lock);

  editor_win->app_type = 2; // Editor

  editor_win->bg_color = 0xFF1E2228;
  editor_win->draw = (void (*)(void *))editor_draw;
  editor_win->needs_redraw = 1;
  editor_win->on_mouse = (void (*)(void *, int, int, int))editor_on_mouse;
  editor_win->on_key = (void (*)(void *, int, char))editor_on_key;
  editor_win->on_close = editor_on_close;
  // NOTE: Do NOT set owner_pid here! It was already set by winmgr_create_window
  // to the desktop task (the parent). Changing it to our own PID would cause
  // the app to quit itself when the window is closed!

  // Start editor thread
  ed_running = 1;

  task_t *t = create_task(editor_thread_entry, "Editor");
  if (t) {
    task_set_priority(t->id, 3); // Must match desktop priority (3) for fairness
    // Do NOT change owner_pid - keep it as the desktop (parent)
  } else {
    print_serial("EDITOR: Failed to start thread\n");
    ed_running = 0;
    winmgr_close_window(editor_win);
    editor_win = 0;
  }
}

/* ======================== FILE OPEN ======================== */

void editor_open_internal(const char *filename) {
  if (!filename || filename[0] == 0)
    return;

  int i = 0;
  while (filename[i] && i < 31) {
    ed_filename[i] = filename[i];
    i++;
  }
  ed_filename[i] = 0;

  ed_read_buf[0] = 0;
  int bytes = fs_read(ed_filename, ed_read_buf);
  if (bytes > 0 && bytes <= 8192) {
    if (bytes > 8191)
      bytes = 8191;
    spinlock_irq_acquire(&editor_lock);
    for (int j = 0; j < bytes; j++)
      editor_buffer[j] = (char)ed_read_buf[j];
    editor_buffer[bytes] = 0;
    ed_cursor_pos = 0;
    spinlock_irq_release(&editor_lock);
  } else {
    spinlock_irq_acquire(&editor_lock);
    editor_buffer[0] = 0;
    ed_cursor_pos = 0;
    spinlock_irq_release(&editor_lock);
  }

  ui_dirty = 1;
  if (editor_win) {
    editor_win->needs_redraw = 1;
    editor_win->is_minimized = 0;
    winmgr_bring_to_front(editor_win);
  }
}

void editor_open(const char *filename) {
  if (!filename || filename[0] == 0)
    return;

  if (!editor_win || editor_win->id == 0)
    editor_init();
  if (!editor_win)
    return;

  char *fname = (char *)kmalloc(32);
  if (fname) {
    int i = 0;
    while (filename[i] && i < 31) {
      fname[i] = filename[i];
      i++;
    }
    fname[i] = 0;

    msg_t m = {0};
    m.type = MSG_USER + 1;
    m.ptr = fname;
    msg_send_to_name("Editor", &m);
  }

  editor_win->is_minimized = 0;
  winmgr_bring_to_front(editor_win);
}
