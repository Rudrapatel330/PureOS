#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/window.h"
#include "../fs/vfs.h"
#include "../drivers/camera.h"

extern int ui_dirty;
extern void print_serial(const char *);

typedef struct {
  camera_ctx_t *cam_ctx;
  int is_active;
} camera_app_state_t;

static void camera_draw_cb(void *w) {
  window_t *win = (window_t *)w;
  camera_app_state_t *state = (camera_app_state_t *)win->user_data;

  // Draw background (Opaque Black)
  winmgr_fill_rect(win, 2, 24, win->width - 4, win->height - 26, 0xFF000000);

  if (state && state->cam_ctx && state->cam_ctx->frame_buffer) {
    int draw_w = state->cam_ctx->width;
    int draw_h = state->cam_ctx->height;

    // Center video in window
    int off_x = (win->width - draw_w) / 2;
    int off_y = (win->height - draw_h) / 2 + 12;

    if (off_x < 2) off_x = 2;
    if (off_y < 24) off_y = 24;

    if (off_x >= win->surface_w || off_y >= win->surface_h) return;

    uint32_t *src = state->cam_ctx->frame_buffer;
    uint32_t *dst = win->surface;

    int max_h = (draw_h < (win->surface_h - off_y)) ? draw_h : (win->surface_h - off_y);
    int max_w = (draw_w < (win->surface_w - off_x)) ? draw_w : (win->surface_w - off_x);

    for (int y = 0; y < max_h; y++) {
      uint32_t *dst_row = &dst[(off_y + y) * win->surface_w];
      uint32_t *src_row = &src[y * draw_w];
      for (int x = 0; x < max_w; x++) {
        dst_row[off_x + x] = 0xFF000000 | src_row[x];
      }
    }
  } else {
    winmgr_draw_text(win, 50, 100, "Camera Not Found", 0xFFFFFF);
  }
}

static void camera_on_close(void *w) {
  window_t *win = (window_t *)w;
  camera_app_state_t *state = (camera_app_state_t *)win->user_data;
  if (state) {
    kfree(state);
    win->user_data = NULL;
  }
}

void camera_app_init() {
  window_t *win = winmgr_create_window(-1, -1, 360, 320, "Camera");
  if (!win) return;

  camera_app_state_t *state = kmalloc(sizeof(camera_app_state_t));
  memset(state, 0, sizeof(camera_app_state_t));

  state->cam_ctx = camera_get_ctx();
  if (state->cam_ctx) {
    state->is_active = 1;
  }

  win->user_data = state;
  win->draw = camera_draw_cb;
  win->on_close = camera_on_close;
  win->bg_color = 0xFF000000;
}

void camera_app_update() {
  extern window_t windows[];
  for (int i = 0; i < MAX_WINDOWS; i++) {
    if (windows[i].draw == camera_draw_cb) {
      camera_app_state_t *state = (camera_app_state_t *)windows[i].user_data;
      if (state && state->is_active && state->cam_ctx) {
        windows[i].needs_redraw = 1;
        ui_dirty = 1;
      }
    }
  }
}
