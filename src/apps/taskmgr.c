#include "taskmgr.h"
#include "../kernel/smp.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"

static window_t *taskmgr_win = 0;
static int tm_scroll_offset = 0;

void taskmgr_draw(window_t *win) {
  if (!win)
    return;

  const theme_t *theme = theme_get();
 
  // Background
  winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, theme->bg);
 
  // 1. TOP HEADER / TOOLBAR
  winmgr_fill_rect(win, 0, 24, win->width, 30, theme->titlebar);
  winmgr_fill_rect(win, 0, 53, win->width, 1, theme->border);
  winmgr_draw_text(win, 10, 32, "PID", theme->fg_secondary);
  winmgr_draw_text(win, 50, 32, "Process Name", theme->fg_secondary);
  winmgr_draw_text(win, 200, 32, "Status", theme->fg_secondary);

  // 2. TASK LIST WITH ZEBRA STRIPING
  int y = 54;
  int row_h = 24;
  extern int window_count;
  extern window_t windows[];

  int list_idx = 0;
  int skipped = 0;
  for (int i = 0; i < window_count; i++) {
    if (windows[i].id == 0 || &windows[i] == win)
      continue;
    if (skipped < tm_scroll_offset) {
      skipped++;
      continue;
    }

    // Zebra Striping
    if (list_idx % 2 == 1) {
      winmgr_fill_rect(win, 0, y, win->width, row_h,
                       theme->menu_bg);
    }

    char pid_str[8];
    k_itoa(windows[i].id, pid_str);
    winmgr_draw_text(win, 10, y + 6, pid_str, theme->fg);

    char name[32];
    if (windows[i].title[0] == 0)
      strcpy(name, "System.bin");
    else
      strncpy(name, windows[i].title, 18);
    winmgr_draw_text(win, 50, y + 6, name, theme->fg);

    const char *status = windows[i].is_minimized ? "Suspended" : "Running";
    winmgr_draw_text(win, 200, y + 6, status,
                     windows[i].is_minimized ? theme->fg_secondary : 0xFF00AA00);

    // End Task Button (Subtle theme-aware button)
    winmgr_fill_rect(win, win->width - 50, y + 2, 45, 20, theme->button);
    winmgr_draw_rect(win, win->width - 50, y + 2, 45, 20, theme->border);
    winmgr_draw_text(win, win->width - 42, y + 6, "End", theme->button_text);

    y += row_h;
    list_idx++;
    if (y > win->height - 60)
      break;
  }

  // 3. STATS FOOTER
  int footer_y = win->height - 50;
  winmgr_fill_rect(win, 0, footer_y, win->width, 50, theme->titlebar);
  winmgr_fill_rect(win, 0, footer_y, win->width, 1, theme->border);

  extern uint32_t get_used_memory();
  extern uint32_t get_total_memory();
  uint32_t used_mem = get_used_memory();
  uint32_t total_mem = get_total_memory();

  char mem_info[64];
  strcpy(mem_info, "Memory: ");
  // Simple PCT
  int pct = (used_mem * 100) / total_mem;
  char pct_str[16];
  k_itoa(pct, pct_str);
  strcat(mem_info, pct_str);
  strcat(mem_info, "% Used");
  winmgr_draw_text(win, 10, footer_y + 10, mem_info, theme->fg);

  winmgr_draw_text(win, 10, footer_y + 25, "CPUs: 1 Active Core", theme->fg);
}

void taskmgr_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  extern int window_count;
  extern window_t windows[];

  // Wheel UP (direction > 0) -> scroll viewport UP -> decrease tm_scroll_offset
  tm_scroll_offset -= direction;

  // Bounds checking
  if (tm_scroll_offset < 0)
    tm_scroll_offset = 0;

  int visible_rows = (win->height - 54 - 20) / 24;
  int total_items = 0;
  for (int i = 0; i < window_count; i++) {
    if (windows[i].id != 0 && &windows[i] != win)
      total_items++;
  }
  int max_scroll =
      (total_items > visible_rows) ? (total_items - visible_rows) : 0;
  if (tm_scroll_offset > max_scroll)
    tm_scroll_offset = max_scroll;

  win->needs_redraw = 1;
  extern int ui_dirty;
  ui_dirty = 1;
}

void taskmgr_handle_mouse(window_t *win, int mx, int my, int buttons) {
  if (!(buttons & 1))
    return;
  int rx = mx;
  int ry = my;

  int y = 54;
  int row_h = 24;
  extern int window_count;
  extern window_t windows[];

  for (int i = 0; i < window_count; i++) {
    if (windows[i].id == 0 || &windows[i] == win)
      continue;

    // End Task Button hit test (matching draw coordinates)
    if (rx >= win->width - 50 && rx <= win->width - 5 && ry >= y + 2 &&
        ry <= y + 22) {
      winmgr_close_window(
          &windows[i]); // Trigger correct OS close animation/cleanup
      win->needs_redraw = 1;
      return;
    }

    y += row_h;
    if (y > win->height - 60)
      break;
  }
}

static void taskmgr_on_close(void *w) {
  (void)w;
  taskmgr_win = 0;
}

void taskmgr_init() {
  if (taskmgr_win && taskmgr_win->id != 0) {
    taskmgr_win->is_minimized = 0;
    extern void winmgr_bring_to_front(window_t * win);
    winmgr_bring_to_front(taskmgr_win);
    return;
  }
  taskmgr_win = winmgr_create_window(-1, -1, 500, 600, "Activity Monitor");
  if (!taskmgr_win)
    return;

  taskmgr_win->draw = (void (*)(void *))taskmgr_draw;
  taskmgr_win->on_mouse = (void (*)(void *, int, int, int))taskmgr_handle_mouse;
  taskmgr_win->on_scroll = (void (*)(void *, int))taskmgr_on_scroll;
  taskmgr_win->on_close = taskmgr_on_close;
  taskmgr_win->app_type = 6; // APP_TASKMGR

  tm_scroll_offset = 0;
  taskmgr_win->blur_enabled = 0; // Opaque as requested
  taskmgr_win->bg_color = 0xFFFFFFFF;
}
