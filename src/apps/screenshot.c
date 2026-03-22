#include "../kernel/heap.h"
#include "../kernel/screen.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"

extern void capture_screenshot(void);
extern uint32_t get_timer_ticks(void);

typedef struct {
  int delay_seconds;
  int capture_pending;
  uint32_t capture_tick;
  char status[64];
} screenshot_app_t;

static void screenshot_draw(void *w) {
  window_t *win = (window_t *)w;
  screenshot_app_t *app = (screenshot_app_t *)win->user_data;

  const theme_t *theme = theme_get();
 
  // Background
  winmgr_fill_rect(win, 0, 0, win->width, win->height, theme->bg);
 
  // Title/Header
  winmgr_draw_text(win, 10, 10, "Screenshot Utility", theme->fg);

  // Buttons
  winmgr_draw_button(win, 10, 40, 140, 25, "Capture Now");
  winmgr_draw_button(win, 10, 75, 140, 25, "Delay 5s");

  // Status
  uint32_t status_col = 0xFF444444;
  if (app->capture_pending) {
    status_col = 0xFFFF0000;
    char count[16];
    int remaining = (app->capture_tick - get_timer_ticks()) / 100; // Roughly
    if (remaining < 0)
      remaining = 0;
    strcpy(app->status, "Capturing in ");
    k_itoa(remaining, count);
    strcat(app->status, count);
    strcat(app->status, "s...");
  }
  winmgr_draw_text(win, 10, 110, app->status, theme->fg_secondary);
}

static void screenshot_on_mouse(void *w, int mx, int my, int buttons) {
  window_t *win = (window_t *)w;
  screenshot_app_t *app = (screenshot_app_t *)win->user_data;

  static int prev_buttons = 0;
  if (buttons && !prev_buttons) {
    // Capture Now
    if (mx >= 10 && mx <= 150 && my >= 40 && my <= 65) {
      strcpy(app->status, "Captured!");
      win->needs_redraw = 1;
      capture_screenshot();
    }
    // Delay 5s
    else if (mx >= 10 && mx <= 150 && my >= 75 && my <= 100) {
      app->capture_pending = 1;
      app->capture_tick = get_timer_ticks() + 500; // 5 seconds (100 ticks = 1s)
      strcpy(app->status, "Waiting...");
      win->needs_redraw = 1;
    }
  }
  prev_buttons = buttons;
}

void screenshot_update(window_t *win) {
  screenshot_app_t *app = (screenshot_app_t *)win->user_data;
  if (app->capture_pending) {
    if (get_timer_ticks() >= app->capture_tick) {
      app->capture_pending = 0;
      strcpy(app->status, "Captured!");
      capture_screenshot();
    }
    win->needs_redraw = 1;
  }
}

static void screenshot_on_close(void *w) {
  window_t *win = (window_t *)w;
  if (win->user_data) {
    kfree(win->user_data);
    win->user_data = 0;
  }
}

void screenshot_init() {
  window_t *win = winmgr_create_window(-1, -1, 300, 200, "Screenshot");
  if (!win)
    return;

  screenshot_app_t *app = (screenshot_app_t *)malloc(sizeof(screenshot_app_t));
  memset(app, 0, sizeof(screenshot_app_t));
  strcpy(app->status, "Ready.");

  win->user_data = app;
  win->draw = screenshot_draw;
  win->on_mouse = screenshot_on_mouse;
  win->on_close = screenshot_on_close;
  win->app_type = 11; // New type
}
