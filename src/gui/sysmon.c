#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "../kernel/window.h"

extern int screen_width, screen_height;

// Constants
#define SYSMON_W 220
#define SYSMON_H 110
#define UPDATE_TICKS 15

static int last_cpu_pct = 0;
static int last_ram_pct = 0;
static uint32_t last_update_tick = 0;

// Simple fixed-point sine approximation for waves
// Returns -128 to 127
static int fast_sine(int x) {
  x = x & 255;
  if (x < 64)
    return (x * 2);
  if (x < 128)
    return (128 - (x - 64) * 2);
  if (x < 192)
    return (-(x - 128) * 2);
  return (-128 + (x - 192) * 2);
}

// Helper to draw a liquid-filled container
static void sysmon_fill_liquid(window_t *win, int x, int y, int w, int h,
                               int pct, uint32_t color1, uint32_t color2) {
  extern uint32_t tick;

  // Background/Border
  winmgr_fill_rect(win, x, y, w, h, 0x44222222);
  winmgr_draw_rect(win, x, y, w, h, 0xFF333333);

  if (pct < 1)
    return;
  if (pct > 100)
    pct = 100;

  int target_y = y + h - (h * pct) / 100;

  // Draw vertical lines to fill the liquid and create waves
  for (int lx = 0; lx < w; lx++) {
    // Primary Wave (Very Slow, Larger Amp)
    int wave1 = (fast_sine(lx * 3 + tick * 1) * 5) / 128; // Speed 1, Amp 5
    // Secondary Wave (Slowest, Larger Amp)
    int wave2 = (fast_sine(lx * 1 - tick / 2) * 3) / 128; // Speed 0.5, Amp 3

    int wave_y = target_y + wave1 + wave2;
    if (wave_y < y)
      wave_y = y;
    if (wave_y > y + h)
      wave_y = y + h;

    int fill_h = (y + h) - wave_y;
    if (fill_h > 0) {
      // Draw liquid column with a slight gradient
      uint32_t col = color1;
      // Fade to color2 at the bottom for depth
      winmgr_fill_rect(win, x + lx, wave_y, 1, fill_h, col);

      // Surface highlight (foam/glare)
      winmgr_fill_rect(win, x + lx, wave_y, 1, 1, 0xAAFFFFFF);
    }
  }
}

void sysmon_draw(void *w) {
  window_t *win = (window_t *)w;
  int ww = win->width;
  int wh = win->height;

  // Background - Frosted look
  winmgr_fill_rect(win, 0, 0, ww, wh, 0xDD111111);
  winmgr_draw_rect(win, 0, 0, ww, wh, 0xFF555555);

  int box_w = (ww - 30) / 2;
  int box_h = wh - 45;

  // CPU Container
  int x1 = 10;
  int y1 = 30;
  sysmon_fill_liquid(win, x1, y1, box_w, box_h, last_cpu_pct, 0xFFFF3300,
                     0xFF881100);
  winmgr_draw_text(win, x1 + 5, 8, "CPU", 0xFFFFFFFF);

  char buf[16];
  k_itoa(last_cpu_pct, buf);
  strcat(buf, "%");
  winmgr_draw_text(win, x1 + box_w - 35, 8, buf, 0xFFFFFF00);

  // RAM Container
  int x2 = x1 + box_w + 10;
  sysmon_fill_liquid(win, x2, y1, box_w, box_h, last_ram_pct, 0xFF00AAFF,
                     0xFF004488);
  winmgr_draw_text(win, x2 + 5, 8, "RAM", 0xFFFFFFFF);

  k_itoa(last_ram_pct, buf);
  strcat(buf, "%");
  winmgr_draw_text(win, x2 + box_w - 35, 8, buf, 0xFFFFFF00);

  // Resize handle (bottom right)
  winmgr_fill_rect(win, ww - 10, wh - 10, 8, 8, 0x88555555);
  winmgr_draw_rect(win, ww - 10, wh - 10, 8, 8, 0xFFAAAAAA);
}

void sysmon_on_mouse(void *w, int mx, int my, int buttons) {
  window_t *win = (window_t *)w;
  static int dragging = 0;
  static int resizing = 0;
  static int off_x = 0, off_y = 0;
  static int start_w = 0, start_h = 0;

  // Check for resize handle click
  if (buttons & 1) {
    if (mx >= win->width - 10 && my >= win->height - 10 && !dragging) {
      if (!resizing) {
        resizing = 1;
        off_x = mx;
        off_y = my;
        start_w = win->width;
        start_h = win->height;
      }
    } else if (!resizing) { // Only allow dragging if not resizing
      if (!dragging) {
        dragging = 1;
        off_x = mx;
        off_y = my;
      }
    }
  } else {
    dragging = 0;
    resizing = 0;
  }

  if (dragging) {
    extern int mouse_x, mouse_y;
    win->x = mouse_x - off_x;
    win->y = mouse_y - off_y;
    extern void compositor_invalidate_rect(int, int, int, int);
    compositor_invalidate_rect(0, 0, 1024, 768);
  } else if (resizing) {
    extern int mouse_x, mouse_y;
    int new_w = start_w + (mouse_x - (win->x + off_x));
    int new_h = start_h + (mouse_y - (win->y + off_y));

    // Minimum size
    if (new_w < 100)
      new_w = 100;
    if (new_h < 80)
      new_h = 80;

    win->width = new_w;
    win->height = new_h;

    // Safety: Reallocate surface to match new dimensions
    extern int winmgr_resize_surface(window_t *, int, int);
    winmgr_resize_surface(win, new_w, new_h);

    win->needs_redraw = 1; // Request redraw for new size
    extern void compositor_invalidate_rect(int, int, int, int);
    compositor_invalidate_rect(0, 0, 1024, 768);
  }
}

void sysmon_update(window_t *win) {
  extern uint32_t tick;
  if (tick - last_update_tick >= UPDATE_TICKS) {
    last_update_tick = tick;
    // RAM Stats
    uint32_t total = heap_get_total_bytes();
    uint32_t used = heap_get_used_bytes();
    if (total > 0)
      last_ram_pct = (used * 100) / total;

    // CPU Stats
    task_t *idle = get_task_by_name("kernel");
    if (idle) {
      last_cpu_pct = 100 - idle->cpu_usage_percent;
    }
  }

  // Always request redraw for wave animation
  win->needs_redraw = 1;
}

window_t *sysmon_create() {
  window_t *win = winmgr_create_window(screen_width - 240, 40, SYSMON_W,
                                       SYSMON_H, "SysMon");
  if (!win)
    return 0;

  win->flags |= WINDOW_FLAG_NO_TITLEBAR | WINDOW_FLAG_WIDGET | WINDOW_FLAG_NO_CLOSE;
  win->always_on_top = 0; // Desktop layer
  win->draw = sysmon_draw;
  win->on_mouse = sysmon_on_mouse;
  win->bg_color = 0x00000000;

  return win;
}
