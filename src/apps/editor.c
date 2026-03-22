#include "../fs/fs.h"
#include "../kernel/heap.h"
#include "../kernel/spinlock.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "../kernel/theme.h"
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
extern int ui_dirty;
extern int ctrl_pressed;

typedef struct {
  window_t *win;
  char buffer[8192];
  char filename[32];
  int scroll_y;
  int dialog_mode;
  int cursor_blink;
  int running;
  int cursor_pos;
  int selected_all;
  spinlock_irq_t lock;
  int was_btn;
} editor_app_t;

static inline editor_app_t *get_editor(void *w) {
  return (editor_app_t *)((window_t *)w)->user_data;
}

static uint8_t ed_read_buf[8192];

/* ======================== DRAW ======================== */

#define MENU_Y 24
#define MENU_H 22
#define TEXT_Y 46
#define STATUS_H 18

static void editor_handle_input(window_t *win, char c);
static void editor_on_mouse(void *w, int rx, int ry, int buttons);
static void editor_on_key(void *w, int key, char ascii);

static void editor_draw(void *w) {
  window_t *win = (window_t *)w;
  editor_app_t *ed = get_editor(win);
  if (!win || !ed)
    return;

  const theme_t *theme = theme_get();

  int width = win->width;
  int height = win->height;

  /* Menu bar */
  winmgr_fill_rect(win, 0, MENU_Y, width, MENU_H, theme->menu_bg);
  winmgr_fill_rect(win, 0, MENU_Y + MENU_H - 1, width, 1, theme->border);
  winmgr_draw_text(win, 10, MENU_Y + 4, "Save", theme->fg);
  winmgr_draw_text(win, 52, MENU_Y + 4, "Open", theme->fg);
  winmgr_draw_text(win, 94, MENU_Y + 4, "New", theme->fg);

  /* Filename centered */
  int fn_len = strlen(ed->filename);
  int fn_x = (width - fn_len * 8) / 2;
  if (fn_x < 130)
    fn_x = 130;
  winmgr_draw_text(win, fn_x, MENU_Y + 4, ed->filename, 0xFF6B737E);

  /* Text area - dark background */
  int text_h = height - TEXT_Y - STATUS_H;
  if (text_h < 12)
    text_h = 12;
  if (ed->selected_all) {
    winmgr_fill_rect(win, 0, TEXT_Y, width, text_h, theme->accent);
  } else {
    winmgr_fill_rect(win, 0, TEXT_Y, width, text_h, theme->bg);
  }

  /* Draw text with scrolling */
  int cx = 8;
  int cy = TEXT_Y + 4;
  int max_y = TEXT_Y + text_h - 14;
  
  /* Copy buffer locally to avoid holding lock during long draw loop */
  char local_buf[8192];
  spinlock_irq_acquire(&ed->lock);
  int buf_len = strlen(ed->buffer);
  if (buf_len > 8191)
    buf_len = 8191;
  for (int i = 0; i < buf_len; i++)
    local_buf[i] = ed->buffer[i];
  local_buf[buf_len] = 0;
  int local_cursor = ed->cursor_pos;
  if (local_cursor > buf_len) local_cursor = buf_len;
  spinlock_irq_release(&ed->lock);

  // PASS 1: Auto-scroll
  char *text1 = local_buf;
  int lx = 8;
  int logical_line = 0;
  int c_index1 = 0;
  while (1) {
      if (c_index1 == local_cursor) {
          if (logical_line < ed->scroll_y) {
              ed->scroll_y = logical_line;
              ui_dirty = 1; win->needs_redraw = 1;
          } else {
              int ly = (TEXT_Y + 4) + (logical_line * 14);
              int screen_cy = ly - (ed->scroll_y * 14);
              if (screen_cy > max_y) {
                  ed->scroll_y += ((screen_cy - max_y) / 14) + 1;
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
    if (current_line >= ed->scroll_y) {
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
        winmgr_draw_text(win, cx, cy, tmp, theme->fg);
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
  
  if (c_index == local_cursor && current_line >= ed->scroll_y && cy <= max_y) {
    cursor_draw_x = cx;
    cursor_draw_y = cy;
  }

  /* Blinking cursor at cursor pos */
  ed->cursor_blink++;
  if ((ed->cursor_blink / 15) % 2 == 0) {
    if (cursor_draw_x >= 0 && cursor_draw_y >= 0) {
      winmgr_fill_rect(win, cursor_draw_x, cursor_draw_y, 2, 14, theme->accent);
    }
  }

  /* Status bar */
  int sy = height - STATUS_H;
  winmgr_fill_rect(win, 0, sy, width, STATUS_H, 0xFF21252B);
  winmgr_fill_rect(win, 0, sy, width, 1, 0xFF444B58);

  spinlock_irq_acquire(&ed->lock);
  int total = strlen(ed->buffer);
  spinlock_irq_release(&ed->lock);

  char sbuf[40];
  strcpy(sbuf, "Chars: ");
  char num[16];
  k_itoa(total, num);
  strcat(sbuf, num);
  winmgr_draw_text(win, 8, sy + 3, sbuf, theme->fg_secondary);
  winmgr_draw_text(win, width - 80, sy + 3, ed->filename, theme->fg_secondary);

  /* Dialog overlay */
  if (ed->dialog_mode > 0) {
    int dw = 260, dh = 100;
    int dx = (width - dw) / 2;
    int dy = (height - dh) / 2;

    winmgr_fill_rect(win, dx + 2, dy + 2, dw, dh, 0xFF000000); // Shadow
    winmgr_fill_rect(win, dx, dy, dw, dh, theme->menu_bg);
    winmgr_draw_rect(win, dx, dy, dw, dh, theme->accent);

    const char *title = (ed->dialog_mode == 1) ? "Save As" : "Open File";
    winmgr_draw_text(win, dx + 10, dy + 8, title, theme->accent);

    winmgr_fill_rect(win, dx + 10, dy + 30, dw - 20, 22, theme->input_bg);
    winmgr_draw_rect(win, dx + 10, dy + 30, dw - 20, 22, theme->input_border);
    winmgr_draw_text(win, dx + 14, dy + 34, ed->filename, theme->fg);

    if ((ed->cursor_blink / 15) % 2 == 0) {
      int cl = strlen(ed->filename);
      winmgr_fill_rect(win, dx + 14 + cl * 8, dy + 32, 2, 16, theme->accent);
    }

    winmgr_draw_text(win, dx + 10, dy + 60, "Enter=OK  Esc=Cancel", theme->fg_secondary);

    winmgr_fill_rect(win, dx + dw - 60, dy + 72, 50, 20, theme->accent);
    winmgr_draw_text(win, dx + dw - 50, dy + 76, "OK", theme->button_text);
  }
}

static void editor_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  editor_app_t *ed = get_editor(win);
  if (!ed) return;

  ed->scroll_y -= direction; // Scroll 1 line per notch for smooth feel
  if (ed->scroll_y < 0)
    ed->scroll_y = 0;
  // TODO: Clamp max scroll if we know the line count
  ui_dirty = 1;
  win->needs_redraw = 1;
}

static void editor_handle_input(window_t *win, char c) {
  editor_app_t *ed = get_editor(win);
  if (!ed)
    return;

  if (ed->dialog_mode > 0) {
    int len = strlen(ed->filename);
    if (c == '\n' || c == '\r') {
      if (ed->dialog_mode == 1) {
        fs_write(ed->filename, (uint8_t *)ed->buffer, strlen(ed->buffer));
      } else if (ed->dialog_mode == 2) {
        char *fname = (char *)kmalloc(32);
        if (fname) {
          strcpy(fname, ed->filename);
          msg_t m = {0};
          m.type = MSG_USER + 1;
          m.ptr = fname;
          msg_send_to_name("Editor", &m);
        }
      }
      ed->dialog_mode = 0;
    } else if (c == 27) {
      ed->dialog_mode = 0;
    } else if (c == '\b') {
      if (len > 0)
        ed->filename[len - 1] = 0;
    } else if (len < 31 && c >= 32 && c < 127) {
      ed->filename[len] = c;
      ed->filename[len + 1] = 0;
    }
    ui_dirty = 1;
    win->needs_redraw = 1;
    return;
  }

  spinlock_irq_acquire(&ed->lock);
  if (ed->selected_all) {
      ed->buffer[0] = 0;
      ed->cursor_pos = 0;
      ed->scroll_y = 0;
      ed->selected_all = 0;
      if (c == '\b') {
          ui_dirty = 1;
          win->needs_redraw = 1;
          spinlock_irq_release(&ed->lock);
          return;
      }
  }
  int len = strlen(ed->buffer);
  if (c == '\b') {
    if (ed->cursor_pos > 0) {
      for (int i = ed->cursor_pos - 1; i <= len; i++) {
          ed->buffer[i] = ed->buffer[i + 1];
      }
      ed->cursor_pos--;
      ui_dirty = 1;
      win->needs_redraw = 1;
      // Scroll up if we deleted enough to see previous lines?
      // Simplified: just reset scroll if buffer is small
      if (len < 10) ed->scroll_y = 0;
    }
  } else if (c == '\n' || (c >= 32 && c < 127)) {
    if (len < 8191) {
      for (int i = len; i >= ed->cursor_pos; i--) {
          ed->buffer[i + 1] = ed->buffer[i];
      }
      ed->buffer[ed->cursor_pos] = c;
      ed->cursor_pos++;
      ui_dirty = 1;
      win->needs_redraw = 1;
    }
  }
  spinlock_irq_release(&ed->lock);
}

static void editor_on_mouse(void *w, int rx, int ry, int buttons) {
  window_t *win = (window_t *)w;
  editor_app_t *ed = get_editor(win);
  if (!win || !ed)
    return;

  if (!(buttons & 1)) {
    ed->was_btn = 0;
    return;
  }
  int click = !ed->was_btn;
  ed->was_btn = 1;

  if (ed->dialog_mode > 0 && click) {
    int dw = 260, dh = 100;
    int dx = (win->width - dw) / 2;
    int dy = (win->height - dh) / 2;
    if (rx >= dx + dw - 60 && rx < dx + dw - 10 && ry >= dy + 72 &&
        ry < dy + 92) {
      if (ed->dialog_mode == 1)
        fs_write(ed->filename, (uint8_t *)ed->buffer, strlen(ed->buffer));
      else if (ed->dialog_mode == 2) {
        char *fname = (char *)kmalloc(32);
        if (fname) {
          strcpy(fname, ed->filename);
          msg_t m = {0};
          m.type = MSG_USER + 1;
          m.ptr = fname;
          msg_send_to_name("Editor", &m);
        }
      }
      ed->dialog_mode = 0;
      ui_dirty = 1;
      win->needs_redraw = 1;
    }
    return;
  }

  if (ry >= TEXT_Y && click) {
     ed->selected_all = 0;
     ui_dirty = 1;
     win->needs_redraw = 1;
  }

  if (ry >= MENU_Y && ry < MENU_Y + MENU_H && click) {
    if (rx >= 6 && rx < 48) {
      ed->dialog_mode = 1;
    } else if (rx >= 48 && rx < 90) {
      ed->dialog_mode = 2;
    } else if (rx >= 90 && rx < 128) {
      spinlock_irq_acquire(&ed->lock);
      ed->buffer[0] = 0;
      ed->cursor_pos = 0;
      spinlock_irq_release(&ed->lock);
      strcpy(ed->filename, "untitled.txt");
      ed->scroll_y = 0;
    }
    ui_dirty = 1;
    win->needs_redraw = 1;
  }
}

static void editor_on_key(void *w, int key, char ascii) {
  window_t *win = (window_t *)w;
  editor_app_t *ed = get_editor(win);
  if (!ed) return;

  ed->cursor_blink = 0;
  if (ed->dialog_mode > 0) {
      if (ascii) editor_handle_input(win, ascii);
      return;
  }

  spinlock_irq_acquire(&ed->lock);
  int len = strlen(ed->buffer);

  if (ctrl_pressed) {
      if (ascii == 'a' || ascii == 'A') {
          ed->selected_all = 1;
          ui_dirty = 1; win->needs_redraw = 1;
          spinlock_irq_release(&ed->lock);
          return;
      } else if (ascii == 'n' || ascii == 'N') {
          ed->buffer[0] = 0;
          strcpy(ed->filename, "untitled.txt");
          ed->scroll_y = 0;
          ed->cursor_pos = 0;
          ui_dirty = 1;
          win->needs_redraw = 1;
      } else if (ascii == 's' || ascii == 'S') {
          int blen = len;
          spinlock_irq_release(&ed->lock);
          fs_write(ed->filename, (uint8_t *)ed->buffer, blen);
          ui_dirty = 1;
          win->needs_redraw = 1;
          return;
      } else if (key == 0x4B) { // KEY_LEFT
          ed->selected_all = 0;
          while (ed->cursor_pos > 0 && ed->buffer[ed->cursor_pos - 1] == ' ') ed->cursor_pos--;
          while (ed->cursor_pos > 0 && ed->buffer[ed->cursor_pos - 1] != ' ' && ed->buffer[ed->cursor_pos - 1] != '\n') ed->cursor_pos--;
          ui_dirty = 1; win->needs_redraw = 1;
      } else if (key == 0x4D) { // KEY_RIGHT
          ed->selected_all = 0;
          while (ed->cursor_pos < len && ed->buffer[ed->cursor_pos] == ' ') ed->cursor_pos++;
          while (ed->cursor_pos < len && ed->buffer[ed->cursor_pos] != ' ' && ed->buffer[ed->cursor_pos] != '\n') ed->cursor_pos++;
          ui_dirty = 1; win->needs_redraw = 1;
      } else if (ascii == '\b') {
          if (ed->selected_all) {
              ed->buffer[0] = 0;
              ed->cursor_pos = 0;
              ed->scroll_y = 0;
              ed->selected_all = 0;
              ui_dirty = 1; win->needs_redraw = 1;
              spinlock_irq_release(&ed->lock);
              return;
          }
          int old_pos = ed->cursor_pos;
          while (ed->cursor_pos > 0 && ed->buffer[ed->cursor_pos - 1] == ' ') ed->cursor_pos--;
          while (ed->cursor_pos > 0 && ed->buffer[ed->cursor_pos - 1] != ' ' && ed->buffer[ed->cursor_pos - 1] != '\n') ed->cursor_pos--;
          int chars_to_remove = old_pos - ed->cursor_pos;
          if (chars_to_remove > 0) {
              for (int i = ed->cursor_pos; i <= len - chars_to_remove; i++) {
                  ed->buffer[i] = ed->buffer[i + chars_to_remove];
              }
              ui_dirty = 1; win->needs_redraw = 1;
          }
      }
      spinlock_irq_release(&ed->lock);
      return;
  }

  if (key == 0x4B) { // KEY_LEFT
      ed->selected_all = 0;
      if (ed->cursor_pos > 0) ed->cursor_pos--;
      ui_dirty = 1; win->needs_redraw = 1;
      spinlock_irq_release(&ed->lock);
      return;
  } else if (key == 0x4D) { // KEY_RIGHT
      ed->selected_all = 0;
      if (ed->cursor_pos < len) ed->cursor_pos++;
      ui_dirty = 1; win->needs_redraw = 1;
      spinlock_irq_release(&ed->lock);
      return;
  } else if (key == 0x48) { // KEY_UP
      ed->selected_all = 0;
      int p = ed->cursor_pos - 1;
      while(p > 0 && ed->buffer[p-1] != '\n') p--;
      if (p > 0) {
          int p2 = p - 1;
          if (p2 > 0 && ed->buffer[p2] == '\n') p2--;
          while(p2 > 0 && ed->buffer[p2-1] != '\n') p2--;
          ed->cursor_pos = p2;
      } else {
          ed->cursor_pos = 0;
      }
      ui_dirty = 1; win->needs_redraw = 1;
      spinlock_irq_release(&ed->lock);
      return;
  } else if (key == 0x50) { // KEY_DOWN
      ed->selected_all = 0;
      int p = ed->cursor_pos;
      while(p < len && ed->buffer[p] != '\n') p++;
      if (p < len) {
          p++;
          ed->cursor_pos = p;
      } else {
          ed->cursor_pos = len;
      }
      ui_dirty = 1; win->needs_redraw = 1;
      spinlock_irq_release(&ed->lock);
      return;
  }

  spinlock_irq_release(&ed->lock);
  if (ascii) editor_handle_input(win, ascii);
}

void editor_thread_entry(void) {
  print_serial("Editor thread started.\n");

  task_t *self = get_current_task();
  window_t *win = 0;
  
  // Find the window owned by this task
  for (int i = 0; i < 32; i++) {
    extern window_t windows[];
    if (windows[i].id != 0 && windows[i].owner_pid == self->id) {
      win = &windows[i];
      break;
    }
  }

  msg_t msg;
  int running = 1;
  while (running) {
    if (msg_receive(&msg)) {
      if (!win) {
          // If we didn't find it yet, try again
          for (int i = 0; i < 32; i++) {
            extern window_t windows[];
            if (windows[i].id != 0 && windows[i].owner_pid == self->id) {
              win = &windows[i];
              break;
            }
          }
          if (!win) continue;
      }
      
      if (msg.type == MSG_KEYBOARD) {
        editor_on_key(win, msg.d1, (char)msg.d2);
      } else if (msg.type == MSG_MOUSE_CLICK) {
        editor_on_mouse(win, msg.d1, msg.d2, msg.d3);
      } else if (msg.type == MSG_MOUSE_SCROLL) {
        editor_on_scroll(win, msg.d4);
      } else if (msg.type == MSG_QUIT) {
        running = 0;
        break;
      } else if (msg.type == MSG_USER + 1) {
        char *fname = (char *)msg.ptr;
        if (fname) {
          extern void editor_open_internal(window_t * win, const char *fn);
          editor_open_internal(win, fname);
          kfree(fname);
        }
      }
    } else {
      if (self) self->state = TASK_STOPPED;
      __asm__ volatile("int $32");
    }
  }

  print_serial("Editor thread exiting.\n");
  exit(0);
}

static void editor_on_close(void *w) {
  window_t *win = (window_t *)w;
  editor_app_t *ed = get_editor(win);
  if (ed) {
    ed->running = 0;
    // Window will be cleaned up by winmgr
    kfree(ed);
    win->user_data = 0;
  }
}

void editor_init(void) {
  print_serial("EDITOR: Calling winmgr_create_window\n");
  window_t *win = winmgr_create_window(-1, -1, 900, 700, "Editor");
  if (!win) return;

  editor_app_t *ed = (editor_app_t *)kmalloc(sizeof(editor_app_t));
  if (!ed) {
    winmgr_close_window(win);
    return;
  }

  for (int i = 0; i < (int)sizeof(editor_app_t); i++) ((char *)ed)[i] = 0;
  ed->win = win;
  strcpy(ed->filename, "untitled.txt");
  const char *welcome = "Welcome to PureOS multitasking!";
  strcpy(ed->buffer, welcome);
  ed->cursor_pos = strlen(welcome);
  ed->running = 1;
  spinlock_irq_init(&ed->lock);

  win->user_data = ed;
  win->app_type = 2;
  win->bg_color = 0xFF1E2228;
  win->draw = (void (*)(void *))editor_draw;
  win->on_mouse = (void (*)(void *, int, int, int))editor_on_mouse;
  win->on_key = (void (*)(void *, int, char))editor_on_key;
  win->on_close = editor_on_close;
  win->needs_redraw = 1;

  create_task(editor_thread_entry, "Editor");
}

/* ======================== FILE OPEN ======================== */

void editor_open_internal(window_t *win, const char *filename) {
  editor_app_t *ed = get_editor(win);
  if (!ed || !filename || filename[0] == 0) return;

  int i = 0;
  while (filename[i] && i < 31) { ed->filename[i] = filename[i]; i++; }
  ed->filename[i] = 0;

  ed_read_buf[0] = 0;
  int bytes = fs_read(ed->filename, ed_read_buf);
  spinlock_irq_acquire(&ed->lock);
  if (bytes > 0 && bytes <= 8192) {
    if (bytes > 8191) bytes = 8191;
    for (int j = 0; j < bytes; j++) ed->buffer[j] = (char)ed_read_buf[j];
    ed->buffer[bytes] = 0;
    ed->cursor_pos = 0;
  } else {
    ed->buffer[0] = 0;
    ed->cursor_pos = 0;
  }
  spinlock_irq_release(&ed->lock);

  ui_dirty = 1;
  win->needs_redraw = 1;
  winmgr_bring_to_front(win);
}

void editor_open(const char *filename) {
  if (!filename || filename[0] == 0) return;
  
  // Find a window to use or create one
  window_t *target = 0;
  for (int i = 0; i < 32; i++) {
    extern window_t windows[];
    if (windows[i].id != 0 && windows[i].app_type == 2) {
      target = &windows[i];
      break;
    }
  }
  
  if (!target) {
    editor_init();
    // Wait/retry or find the one we just made
    for (int i = 0; i < 32; i++) {
      extern window_t windows[];
      if (windows[i].id != 0 && windows[i].app_type == 2) {
        target = &windows[i];
        break;
      }
    }
  }
  
  if (target) {
    char *fname = (char *)kmalloc(32);
    if (fname) {
      strcpy(fname, filename);
      msg_t m = {0};
      m.type = MSG_USER + 1;
      m.ptr = fname;
      extern int msg_send(int pid, msg_t *msg);
      msg_send(target->owner_pid, &m);
    }
  }
}
