#include "../kernel/ui_layout.h"
#include "../drivers/video_dev.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/window.h"

extern int ui_dirty;
extern void print_serial(const char *);

typedef struct {
  video_ctx_t *v_ctx;
  uint32_t last_tick;
  char current_file[128];
} videoplayer_state_t;

static void videoplayer_draw_cb(void *w) {
  window_t *win = (window_t *)w;
  videoplayer_state_t *state = (videoplayer_state_t *)win->user_data;

  // Draw background (Opaque Black)
  winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, 0xFF000000);

  if (state && state->v_ctx && state->v_ctx->frame_buffer) {
    int src_w = state->v_ctx->width;
    int src_h = state->v_ctx->height;
    
    int dst_w = win->width;
    int dst_h = win->height - 24;

    if (dst_w <= 0 || dst_h <= 0) return;

    // Scaling blit
    uint32_t *src = state->v_ctx->frame_buffer;
    uint32_t *dst = win->surface;

    for (int y = 0; y < dst_h; y++) {
      int sy = (y * src_h) / dst_h;
      uint32_t *src_row = &src[sy * src_w];
      uint32_t *dst_row = &dst[(y + 24) * win->surface_w];
      
      for (int x = 0; x < dst_w; x++) {
        int sx = (x * src_w) / dst_w;
        dst_row[x] = 0xFF000000 | src_row[sx];
      }
    }
  } else {
    int fs = ui_get_font_scale();
    winmgr_draw_text(win, (win->width - 120) / 2, (win->height - fs) / 2, "No Video Loaded", 0xFFFFFF);
  }
}

static void videoplayer_on_close(void *w) {
  window_t *win = (window_t *)w;
  videoplayer_state_t *state = (videoplayer_state_t *)win->user_data;
  if (state) {
    if (state->v_ctx) {
      video_close(state->v_ctx);
      state->v_ctx = NULL;
    }
    kfree(state);
    win->user_data = NULL;
  }
}

static void videoplayer_on_key(void *w, int key, char c) {
  // Media controls could go here
}

void videoplayer_init(const char *path) {
  window_t *win = winmgr_create_window(-1, -1, 640, 480, "Video Player");
  if (!win)
    return;

  videoplayer_state_t *state = kmalloc(sizeof(videoplayer_state_t));
  memset(state, 0, sizeof(videoplayer_state_t));

  win->user_data = state;
  win->draw = videoplayer_draw_cb;
  win->on_key = videoplayer_on_key;
  win->on_close = videoplayer_on_close;
  win->app_type = 8;          // APP_VIDEOPLAYER
  win->bg_color = 0xFF000000; // Black background for video

  if (!path)
    path = "/disk/WALKTHROUGH.MPG";

  if (path) {
    state->v_ctx = video_open(path);
    if (state->v_ctx) {
      strncpy(win->title, "Video Player - Playing", 31);
    } else {
      strncpy(win->title, "Video Player - Error", 31);
    }
  }
}

// In kernel loop, we need a way to update the video frame
void videoplayer_update() {
  for (int i = 0; i < MAX_WINDOWS; i++) {
    if (windows[i].draw == videoplayer_draw_cb) {
      videoplayer_state_t *state = (videoplayer_state_t *)windows[i].user_data;
      if (state && state->v_ctx && state->v_ctx->is_playing) {
        if (video_decode_frame(state->v_ctx) > 0) {
          windows[i].needs_redraw = 1;
          ui_dirty = 1;
        }
      }
    }
  }
}
