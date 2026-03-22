#include "compositor.h"
#include "../drivers/bga.h"
#include "../drivers/vga.h"
#include "../gui/startmenu.h"
#include "../gui/sysmenu.h"
#include "../gui/taskbar.h"
#include "../ui/ctxmenu.h"
#include "desktop.h"
#include "gfx_2d.h"
#include "screen.h"
#include "string.h"
#include "window.h"

extern void startmenu_draw(uint32_t *buffer, rect_t clip);

int alt_tab_active = 0;
int mission_control_active = 0;

static inline uint64_t rdtsc(void) {
  uint32_t lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

int current_fps = 0;
uint64_t compositor_cycles = 0;
static uint32_t last_fps_tick = 0;
static int frames_this_second = 0;

// Fast 32-bit fill using rep stosd (fills count uint32_t values)
static inline void fast_fill_32(uint32_t *dest, uint32_t value, int count) {
  __asm__ volatile("rep stosl"
                   : "+D"(dest), "+c"(count)
                   : "a"(value)
                   : "memory");
}

#define MAX_DIRTY_RECTS 64

void compositor_blur_rect(int x, int y, int w, int h, int radius);

static rect_t dirty_rects[MAX_DIRTY_RECTS];
static int dirty_count = 0;

static rect_t prev_dirty_rects[MAX_DIRTY_RECTS];
static int prev_dirty_count = 0;

static rect_t prev_2_dirty_rects[MAX_DIRTY_RECTS];
static int prev_2_dirty_count = 0;

static rect_t combined_rects[MAX_DIRTY_RECTS * 3];

// Global references from kernel.c / screen.h
extern uint32_t *backbuffer;
extern uint32_t *desktop_buffer;
extern int screen_width;
extern int screen_height;
extern uint32_t *real_lfb;

int disable_animations = 0; // Default to enabled

static int debug_mode = 0;
static rect_t overlay_rect = {0, 0, 0, 0};
static int overlay_active = 0;
static int force_full_redraw = 0;

int magnifier_enabled = 0; // DISABLED BY DEFAULT
float magnifier_scale = 2.0f;

// 32-bit ARGB Blending (Optimized)
static inline uint32_t blend_color_32(uint32_t src, uint32_t dst,
                                      uint8_t global_alpha) {
  uint32_t src_alpha = (src >> 24) & 0xFF;
  uint32_t sum_a = src_alpha * global_alpha;
  uint32_t a = (sum_a + (sum_a >> 8) + 1) >> 8;

  if (a >= 255)
    return src;
  if (a <= 0)
    return dst;

  uint32_t inv_a = 255 - a;

  uint32_t sum_r = ((src >> 16) & 0xFF) * a + ((dst >> 16) & 0xFF) * inv_a;
  uint32_t sum_g = ((src >> 8) & 0xFF) * a + ((dst >> 8) & 0xFF) * inv_a;
  uint32_t sum_b = (src & 0xFF) * a + (dst & 0xFF) * inv_a;

  uint32_t r = (sum_r + (sum_r >> 8) + 1) >> 8;
  uint32_t g = (sum_g + (sum_g >> 8) + 1) >> 8;
  uint32_t b = (sum_b + (sum_b >> 8) + 1) >> 8;

  return 0xFF000000 | (r << 16) | (g << 8) | b;
}

// 32-bit ARGB Scaled Blending
void compositor_scaled_blend(window_t *win, rect_t clip) {
  if (!win || win->id == 0 || !win->surface || win->scale <= 0.05f)
    return;
  if (win->opacity <= 1)
    return;
  if (win->surface_w <= 0 || win->surface_h <= 0)
    return;

  int dw = (int)(win->width * win->scale);
  int dh = (int)(win->height * win->scale);
  if (dw <= 0 || dh <= 0)
    return;

  // Center the scaled window on its original position for a zoom effect
  int dx, dy;
  if (win->anim_mode == 2 || win->anim_mode == 3 || win->anim_mode == 1) {
    // Minimize, Restore, or Launch: Use animated top-left
    dx = (int)win->anim_x.current_val;
    dy = (int)win->anim_y.current_val;
  } else {
    dx = win->x + (win->width - dw) / 2;
    dy = win->y + (win->height - dh) / 2;
  }

  // Fixed-point scaling (16.16)
  int step_x = (win->surface_w << 16) / dw;
  int step_y = (win->surface_h << 16) / dh;

  for (int y = 0; y < dh; y++) {
    int cur_dy = dy + y;
    if (cur_dy < 0 || cur_dy >= screen_height)
      continue;
    if (cur_dy < clip.y || cur_dy >= clip.y + clip.h)
      continue;

    int src_y = (y * step_y) >> 16;
    if (src_y >= win->surface_h)
      src_y = win->surface_h - 1;

    uint32_t *dest_row = &backbuffer[cur_dy * screen_width];
    uint32_t *src_row = &win->surface[src_y * win->surface_w];

    for (int x = 0; x < dw; x++) {
      int cur_dx = dx + x;
      if (cur_dx < 0 || cur_dx >= screen_width)
        continue;
      if (cur_dx < clip.x || cur_dx >= clip.x + clip.w)
        continue;

      int src_x = (x * step_x) >> 16;
      if (src_x >= win->surface_w)
        src_x = win->surface_w - 1;

      uint32_t src_px = src_row[src_x];
      uint8_t src_alpha = (src_px >> 24) & 0xFF;

      if (src_alpha > 0) {
        dest_row[cur_dx] =
            blend_color_32(src_px, dest_row[cur_dx], win->opacity);
      }
    }
  }
}
static inline float cubic_bezier(float t, float p0, float p1, float p2, float p3) {
  float u = 1.0f - t;
  float tt = t * t;
  float uu = u * u;
  float uuu = uu * u;
  float ttt = tt * t;
  return uuu * p0 + 3.0f * uu * t * p1 + 3.0f * u * tt * p2 + ttt * p3;
}

// S-curve sigmoid: maps [0,1] -> [0,1] with a smooth inflection at the midpoint.
// steepness controls how sharp the transition is (6-10 is a good range).
static inline float s_curve(float t, float steepness) {
  // Logistic sigmoid remapped to [0,1] -> [0,1]
  // f(t) = 1 / (1 + e^(-steepness*(t-0.5)))
  // Normalized so f(0)=0, f(1)=1
  float raw = 1.0f / (1.0f + 1.0f / (t * steepness + 0.001f));
  // Use smoothstep as a fast, well-behaved approximation
  float ss = t * t * (3.0f - 2.0f * t); // smoothstep
  // Blend: sharpen the smoothstep with an extra cubic push
  float s = ss * ss * (3.0f - 2.0f * ss); // double-smoothstep for tighter S
  (void)raw; (void)steepness; // suppress unused
  return s;
}

// Fast fractional Bilinear Texture Sampling for high density continuous mesh resolution
static inline uint32_t bilinear_blend_argb(uint32_t c00, uint32_t c10, uint32_t c01, uint32_t c11, int u_frac, int v_frac) {
    if (u_frac == 0 && v_frac == 0) return c00;

    int u_inv = 256 - u_frac;
    int v_inv = 256 - v_frac;

    int w00 = (u_inv * v_inv) >> 8;
    int w10 = (u_frac * v_inv) >> 8;
    int w01 = (u_inv * v_frac) >> 8;
    int w11 = (u_frac * v_frac) >> 8;

    int a = ((c00 >> 24) * w00 + (c10 >> 24) * w10 + (c01 >> 24) * w01 + (c11 >> 24) * w11) >> 8;
    int r = (((c00 >> 16) & 0xFF) * w00 + ((c10 >> 16) & 0xFF) * w10 + ((c01 >> 16) & 0xFF) * w01 + ((c11 >> 16) & 0xFF) * w11) >> 8;
    int g = (((c00 >> 8) & 0xFF) * w00 + ((c10 >> 8) & 0xFF) * w10 + ((c01 >> 8) & 0xFF) * w01 + ((c11 >> 8) & 0xFF) * w11) >> 8;
    int b = ((c00 & 0xFF) * w00 + (c10 & 0xFF) * w10 + (c01 & 0xFF) * w01 + (c11 & 0xFF) * w11) >> 8;

    return (a << 24) | (r << 16) | (g << 8) | b;
}

// 32-bit ARGB Genie Effect (Warp) Blending
void compositor_warp_blend(window_t *win, rect_t clip) {
  if (!win || win->id == 0 || !win->surface)
    return;

  // Skip rendering if fully transparent (early animation frames)
  if (win->opacity <= 1)
    return;

  // Validate surface dimensions are sane
  if (win->surface_w <= 0 || win->surface_h <= 0)
    return;

  // Failsafe: If marked as animating but no spatial animations are active,
  // do not attempt to warp/stretch the window. This prevents "Black Box" desyncs.
  if (!win->anim_x.active && !win->anim_y.active && !win->anim_w.active &&
      !win->anim_h.active) {
    win->is_animating = 0;
    return;
  }

  // anim_x, anim_y drive the top-left position
  // anim_w, anim_h drive the current dimensions
  int cur_x = (int)win->anim_x.current_val;
  int cur_y = (int)win->anim_y.current_val;
  int cur_w = (int)win->anim_w.current_val;
  int cur_h = (int)win->anim_h.current_val;

  // Safety check: large or NaN animations can cause huge loop counts
  if (cur_w <= 2 || cur_h <= 2 || cur_w > 4000 || cur_h > 4000)
    return;

  // Additional sanity checks for positions
  if (cur_x < -2000 || cur_x > 4000 || cur_y < -2000 || cur_y > 4000)
    return;

  // Genie progress: how far along the animation is (0 = full window, 1 = icon)
  // Use the width shrinkage as the absolute source of truth for progression
  float progress = 0.0f;
  if (win->width > 0)
    progress = 1.0f - (float)(cur_w) / (float)win->width;
  if (progress < 0) progress = 0;
  if (progress > 1) progress = 1;

  // === SEPARATE VERTICAL TIMING (Stretching Effect) ===
  // Delay the rigid edge's descent, and anchor the vacuum tip.
  float inv_p = 1.0f - progress;
  float inv_p2 = inv_p * inv_p;
  float inv_p5 = inv_p2 * inv_p2 * inv_p;
  float p2 = progress * progress;
  float p5 = p2 * p2 * progress;

  float prog_top, prog_bot;
  if (win->pinch_top) {
      // Dock is above: top edge anchors faster, bottom edge lags exponentially
      prog_top = 1.0f - inv_p5;
      prog_bot = p5;
  } else {
      // Dock is below (standard): bottom edge anchors faster, top edge lags exponentially
      prog_top = p5;
      prog_bot = 1.0f - inv_p5;
  }

  int ty_target = (win->launch_y >= 0) ? win->launch_y : win->taskbar_y;
  if (ty_target < 0) ty_target = screen_height;

  int start_y = win->y;
  int start_bottom = win->y + win->height;
  int target_y = ty_target - 16;
  int target_bottom = ty_target + 16; // Icon size (32) centered

  // Override cur_y and cur_h with independent stretching bounds
  cur_y = start_y + (int)((target_y - start_y) * prog_top);
  int cur_bottom = start_bottom + (int)((target_bottom - start_bottom) * prog_bot);
  cur_h = cur_bottom - cur_y;
  if (cur_h < 2) cur_h = 2;

  // === PRE-LOOP: Hoist loop-invariant calculations ===
  // The dock icon X as the horizontal attractor for the Bézier bend
  int tx = (win->launch_x >= 0) ? win->launch_x : win->taskbar_x;
  int win_center = win->x + win->width / 2;
  float tx_dist = (tx >= 0) ? (float)(tx - win_center) : 0.0f;
  float bend_limit = (float)win->width * 0.6f;

  // Ease-in-out warp progress: smooth acceleration/deceleration of the funnel
  float wp = progress * progress * (3.0f - 2.0f * progress);

  // Horizontal anchor ratio — where on the width the funnel converges
  float rel_x = 0.5f;
  if (tx >= 0 && win->width > 0) {
    rel_x = (float)(tx - win->x) / (float)win->width;
    if (rel_x < 0.0f) rel_x = 0.0f;
    if (rel_x > 1.0f) rel_x = 1.0f;
  }

  // Pre-calculate fixed-point constants for vertical loop
  int h_fixed = (win->surface_h << 16) / cur_h;

  // === MESH-DRIVEN QUAD RENDERING (63 Slices) ===
  for (int i = 0; i < 63; i++) {
    // Screen Y boundaries for this horizontal strip (quad)
    int y0 = cur_y + (i * cur_h) / 63;
    int y1 = cur_y + ((i + 1) * cur_h) / 63;

    // Skip quad if it's entirely outside the clip/screen
    if (y1 < clip.y || y0 >= clip.y + clip.h) continue;
    if (y1 < 0 || y0 >= screen_height) continue;

    // Mesh boundaries for top and bottom of this strip
    float lx0 = win->mesh_lx[i];
    float rx0 = win->mesh_rx[i];
    float lx1 = win->mesh_lx[i+1];
    float rx1 = win->mesh_rx[i+1];

    // Texture Y (warped) boundaries from mesh
    float ty0 = win->mesh_ty[i];
    float ty1 = win->mesh_ty[i+1];

    // Draw rows within this quad
    for (int sy = y0; sy < y1; sy++) {
      if (sy < clip.y || sy >= clip.y + clip.h) continue;
      if (sy < 0 || sy >= screen_height) continue;

      // Linear interpolation factor within the quad
      float t = (y1 > y0) ? (float)(sy - y0) / (float)(y1 - y0) : 0.0f;

      // Interpolate horizontal bounds and vertical texture mapping
      float clx = lx0 * (1.0f - t) + lx1 * t;
      float crx = rx0 * (1.0f - t) + rx1 * t;
      float cty = ty0 * (1.0f - t) + ty1 * t;

      int irx = (int)clx;
      int irw = (int)(crx - clx);
      if (irw < 2) continue;

      // Vertical source Y mapping (fractional for bilinear)
      int src_y_fixed = (int)(cty * (float)win->surface_h * 256.0f);
      int src_y0 = src_y_fixed >> 8;
      if (src_y0 >= win->surface_h) src_y0 = win->surface_h - 1;
      int src_y1 = (src_y0 + 1 < win->surface_h) ? src_y0 + 1 : src_y0;
      int v_frac = src_y_fixed & 0xFF;

      // Horizontal source step
      int step_x = (win->surface_w << 16) / irw;

      uint32_t *dest_row = &backbuffer[sy * screen_width];
      uint32_t *src_row0 = &win->surface[src_y0 * win->surface_w];
      uint32_t *src_row1 = &win->surface[src_y1 * win->surface_w];

      for (int x = 0; x < irw; x++) {
        int cur_dx = irx + x;
        if (cur_dx < clip.x || cur_dx >= clip.x + clip.w) continue;
        if (cur_dx < 0 || cur_dx >= screen_width) continue;

        int u_fixed = x * step_x;
        int src_x0 = u_fixed >> 16;
        if (src_x0 >= win->surface_w) src_x0 = win->surface_w - 1;
        int src_x1 = (src_x0 + 1 < win->surface_w) ? src_x0 + 1 : src_x0;
        int u_frac = (u_fixed >> 8) & 0xFF;

        uint32_t c00 = src_row0[src_x0];
        uint32_t c10 = src_row0[src_x1];
        uint32_t c01 = src_row1[src_x0];
        uint32_t c11 = src_row1[src_x1];

        uint32_t src_px = bilinear_blend_argb(c00, c10, c01, c11, u_frac, v_frac);

        if ((src_px >> 24) > 0) {
          dest_row[cur_dx] = blend_color_32(src_px, dest_row[cur_dx], win->opacity);
        }
      }
    }
  }
}

// Helper: Intersect two rects
int rect_intersect(rect_t a, rect_t b, rect_t *out) {
  int x1 = (a.x > b.x) ? a.x : b.x;
  int y1 = (a.y > b.y) ? a.y : b.y;
  int x2 = (a.x + a.w < b.x + b.w) ? a.x + a.w : b.x + b.w;
  int y2 = (a.y + a.h < b.y + b.h) ? a.y + a.h : b.y + b.h;

  if (x1 < x2 && y1 < y2) {
    if (out) {
      out->x = x1;
      out->y = y1;
      out->w = x2 - x1;
      out->h = y2 - y1;
    }
    return 1;
  }
  return 0;
}

// Helper: Union of two rects (Bounding Box)
void rect_union(rect_t a, rect_t b, rect_t *out) {
  int x1 = (a.x < b.x) ? a.x : b.x;
  int y1 = (a.y < b.y) ? a.y : b.y;
  int x2 = (a.x + a.w > b.x + b.w) ? a.x + a.w : b.x + b.w;
  int y2 = (a.y + a.h > b.y + b.h) ? a.y + a.h : b.y + b.h;
  out->x = x1;
  out->y = y1;
  out->w = x2 - x1;
  out->h = y2 - y1;
}

// Helper: Clip rect to screen
static void rect_clip_screen(rect_t *r) {
  if (r->x < 0) {
    r->w += r->x;
    r->x = 0;
  }
  if (r->y < 0) {
    r->h += r->y;
    r->y = 0;
  }
  if (r->x + r->w > screen_width)
    r->w = screen_width - r->x;
  if (r->y + r->h > screen_height)
    r->h = screen_height - r->y;

  if (r->w < 0)
    r->w = 0;
  if (r->h < 0)
    r->h = 0;
}

// Helper: Check if rect A contains rect B
static int rect_contains(rect_t a, rect_t b) {
  return (b.x >= a.x && b.y >= a.y && (b.x + b.w) <= (a.x + a.w) &&
          (b.y + b.h) <= (a.y + a.h));
}

void compositor_reset_history() {
  dirty_count = 0;
  prev_dirty_count = 0;
  prev_2_dirty_count = 0;
  force_full_redraw = 5; // Force next 5 frames to be full-screen
}

void compositor_init() {
  compositor_reset_history();
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
}

void compositor_invalidate_rect(int x, int y, int w, int h) {
  rect_t r = {x, y, w, h};
  rect_clip_screen(&r);
  if (r.w <= 0 || r.h <= 0)
    return;

  if (dirty_count == 1 && dirty_rects[0].x == 0 && dirty_rects[0].y == 0 &&
      dirty_rects[0].w == screen_width && dirty_rects[0].h == screen_height) {
    return;
  }

  for (int i = 0; i < dirty_count; i++) {
    if (rect_contains(dirty_rects[i], r))
      return;
  }

  for (int i = 0; i < dirty_count; i++) {
    if (rect_contains(r, dirty_rects[i])) {
      dirty_rects[i] = r;
      return;
    }
  }

  if (dirty_count >= MAX_DIRTY_RECTS) {
    dirty_count = 1;
    dirty_rects[0].x = 0;
    dirty_rects[0].y = 0;
    dirty_rects[0].w = screen_width;
    dirty_rects[0].h = screen_height;
    return;
  }

  dirty_rects[dirty_count++] = r;
}

void compositor_invalidate_window(window_t *win) {
  compositor_invalidate_rect(win->x - 4, win->y - 4, win->width + 8,
                             win->height + 8);
}

void compositor_set_debug(int enabled) { debug_mode = enabled; }

int compositor_is_dirty() {
  return dirty_count > 0 || prev_dirty_count > 0 || prev_2_dirty_count > 0;
}

static void present_rect(rect_t r) {
  uint32_t *dest = bga_get_render_buffer();
  if (!dest)
    return;

  for (int y = 0; y < r.h; y++) {
    int offset = (r.y + y) * screen_width + r.x;
    memcpy(&dest[offset], &backbuffer[offset], r.w * 4);
  }
}

void compositor_clear_vram() {
  uint32_t *page0 = real_lfb;
  uint32_t *page1 =
      (uint32_t *)((uint8_t *)real_lfb + (screen_width * screen_height * 4));
  uint32_t *page2 =
      (uint32_t *)((uint8_t *)real_lfb + (screen_width * screen_height * 8));

  if (backbuffer) {
    if (page0)
      memcpy(page0, backbuffer, screen_width * screen_height * 4);
    if (page1)
      memcpy(page1, backbuffer, screen_width * screen_height * 4);
    if (page2)
      memcpy(page2, backbuffer, screen_width * screen_height * 4);
  }
}

static void compositor_render_rect(rect_t clip) {
  extern int screen_width;
  extern int screen_height;
  extern uint32_t *backbuffer;
  extern uint32_t *desktop_buffer;

  if (!backbuffer)
    return;

  extern window_t windows[];
  extern int window_z_order[];
  extern int window_count;
  extern int mouse_x, mouse_y;
  extern int ui_dirty;
  extern window_t *active_window;
  extern void desktop_render_icons(uint32_t *target, rect_t clip);
  extern void menubar_draw(uint32_t *buffer, rect_t clip);
  extern void taskbar_draw(uint32_t *buffer, rect_t clip);
  extern void startmenu_draw(uint32_t *buffer, rect_t clip);

  // 1. Desktop/Clear Pass
  int skip_desktop = 0;
  if (active_window && active_window->id != 0 && active_window->is_maximized &&
      active_window->opacity == 255 && !active_window->is_animating &&
      active_window->x == 0 && active_window->y == 0) {
    // Check if the maximized window covers the clip rect
    rect_t win_r = {0, 0, active_window->width, active_window->height};
    if (rect_contains(win_r, clip))
      skip_desktop = 1;
  }

  if (!skip_desktop) {
    if (desktop_buffer) {
      uint32_t *dst = &backbuffer[clip.y * screen_width + clip.x];
      uint32_t *src = &desktop_buffer[clip.y * screen_width + clip.x];
      gfx_blit_rect(dst, src, clip.w, clip.h, screen_width, screen_width);
    } else {
      // Fallback: Fill with solid black if wallpaper isn't loaded or memory
      // failed
      extern void simd_fill_32(uint32_t *dest, uint32_t value, size_t count);
      for (int i = 0; i < clip.h; i++) {
        uint32_t *dst = &backbuffer[(clip.y + i) * screen_width + clip.x];
        simd_fill_32(dst, 0xFF000000, clip.w);
      }
    }
    desktop_render_icons(backbuffer, clip);
    extern void desktop_render_widgets(uint32_t *target, rect_t clip);
    desktop_render_widgets(backbuffer, clip);
  }

  // 2. Window Pass (3-layered)
  for (int pass = 0; pass < 3; pass++) {
    for (int w_idx = 0; w_idx < window_count; w_idx++) {
      int win_idx = window_z_order[w_idx];
      if (win_idx < 0) continue;
      window_t *win = &windows[win_idx];
      if (win->id == 0 || (win->is_minimized && !win->is_animating))
        continue;

      // Pass 0: Desktop Widgets (always_on_top == 0 && WIDGET)
      // Pass 1: Standard Windows (always_on_top == 0 && !WIDGET)
      // Pass 2: Always-on-top Windows (always_on_top == 1)
      if (pass == 0) {
        if (win->always_on_top || !(win->flags & WINDOW_FLAG_WIDGET))
          continue;
      } else if (pass == 1) {
        if (win->always_on_top || (win->flags & WINDOW_FLAG_WIDGET))
          continue;
      } else if (pass == 2) {
        if (!win->always_on_top)
          continue;
      }

      rect_t win_r = {win->x, win->y, win->width, win->height};
      rect_t overlap;
      int intersects = rect_intersect(clip, win_r, &overlap);
      if (!intersects && !win->is_animating)
        continue;

      // Occlusion culling (Standard pass only)
      int occluded = 0;
      if (pass == 1 && intersects && !win->is_animating) {
        for (int hi = w_idx + 1; hi < window_count; hi++) {
          int upper_idx = window_z_order[hi];
          if (upper_idx < 0) continue;
          window_t *upper = &windows[upper_idx];
          if (upper->id == 0 || upper->is_minimized || upper->is_animating ||
              upper->opacity < 255 || upper->scale != 1.0f)
            continue;
          rect_t ur = {upper->x, upper->y, upper->width, upper->height};
          if (rect_contains(ur, overlap)) {
            occluded = 1;
            break;
          }
        }
      }
      if (occluded)
        continue;

      if (win->needs_redraw) {
        winmgr_render_window(win);
        win->needs_redraw = 0;
      }

      // Guard: surface might be NULL if app's draw callback caused OOM
      // or if the window was never properly initialized
      if (!win->surface) {
        continue;
      }

      if (win->is_animating && !disable_animations) {
        // Use Warp/Genie effect for Open and Minimize ONLY (Modes 1 and 2)
        if (win->anim_mode == 1 || win->anim_mode == 2)
          compositor_warp_blend(win, clip);
        else
          compositor_scaled_blend(win, clip);
        continue;
      }

      if (win->scale != 1.0f) {
        compositor_scaled_blend(win, clip);
        continue;
      }

      int surf_off_x = overlap.x - win->x;
      int surf_off_y = overlap.y - win->y;

      uint32_t *src_start =
          &win->surface[surf_off_y * win->surface_w + surf_off_x];
      uint32_t *dst_start = &backbuffer[overlap.y * screen_width + overlap.x];

      // Always use blend Pass for windows, even if opacity is 255,
      // because window surfaces have per-pixel alpha (e.g. rounded corners).
      gfx_blend_rect(dst_start, src_start, win->opacity, overlap.w, overlap.h,
                     screen_width, win->surface_w);
    }
  }

  // 3. UI Overlays
  menubar_draw(backbuffer, clip);
  taskbar_draw(backbuffer, clip);
  startmenu_draw(backbuffer, clip);
  sysmenu_draw(backbuffer, clip);
  extern void desktop_render_search(uint32_t *target, rect_t clip);
  desktop_render_search(backbuffer, clip);

  // 4. Context Menu (if visible)
  extern void ctxmenu_draw_target(int mx, int my, uint32_t *target);
  ctxmenu_draw_target(mouse_x, mouse_y, backbuffer);

  // 5. Present to VRAM
  present_rect(clip);
}

void compositor_draw_magnifier(uint32_t *target, int mx, int my) {
  if (!magnifier_enabled || !backbuffer)
    return;

  int lens_size = 160;
  int src_size = (int)(lens_size / magnifier_scale);
  int half_lens = lens_size / 2;
  int half_src = src_size / 2;

  int lx = mx - half_lens;
  int ly = my - half_lens;

  // Fixed-point scaling (16.16)
  int step = (src_size << 16) / lens_size;

  for (int y = 0; y < lens_size; y++) {
    int gy = ly + y;
    if (gy < 0 || gy >= screen_height)
      continue;

    int sy = (my - half_src) + ((y * step) >> 16);
    if (sy < 0 || sy >= screen_height)
      continue;

    uint32_t *dst_row = &target[gy * screen_width];
    uint32_t *src_row = &backbuffer[sy * screen_width];

    for (int x = 0; x < lens_size; x++) {
      int gx = lx + x;
      if (gx < 0 || gx >= screen_width)
        continue;

      int sx = (mx - half_src) + ((x * step) >> 16);
      if (sx < 0 || sx >= screen_width)
        continue;

      // Simple border logic
      int dx = x - half_lens;
      int dy = y - half_lens;
      int dist_sq = dx * dx + dy * dy;
      int r_inner = half_lens - 2;
      int r_outer = half_lens;

      if (dist_sq < r_inner * r_inner) {
        dst_row[gx] = src_row[sx];
      } else if (dist_sq < r_outer * r_outer) {
        dst_row[gx] = 0xFFCCCCCC; // Silver metal border
      }
    }
  }
}

void compositor_render() {
  uint64_t comp_start = rdtsc();

  if (dirty_count == 0 && prev_dirty_count == 0 && prev_2_dirty_count == 0 &&
      force_full_redraw <= 0)
    return;

  extern int mouse_x, mouse_y;
  extern int mission_control_active;
  extern ctxmenu_t context_menu;

  // Collect all rects from 3 frames (for triple buffering swap)
  int combined_count = 0;
  for (int i = 0; i < dirty_count && combined_count < MAX_DIRTY_RECTS * 3; i++)
    combined_rects[combined_count++] = dirty_rects[i];
  for (int i = 0; i < prev_dirty_count && combined_count < MAX_DIRTY_RECTS * 3;
       i++)
    combined_rects[combined_count++] = prev_dirty_rects[i];
  for (int i = 0; i < prev_2_dirty_count && combined_count < MAX_DIRTY_RECTS * 3;
       i++)
    combined_rects[combined_count++] = prev_2_dirty_rects[i];

  // OPTIMIZATION: Simple rect merging to reduce draw calls
  int final_count = combined_count;
  for (int i = 0; i < final_count; i++) {
    for (int j = i + 1; j < final_count; j++) {
      rect_t merge;
      rect_union(combined_rects[i], combined_rects[j], &merge);
      uint32_t area_i = combined_rects[i].w * combined_rects[i].h;
      uint32_t area_j = combined_rects[j].w * combined_rects[j].h;
      uint32_t area_merge = merge.w * merge.h;

      if (area_merge < (area_i + area_j) * 1.5f) {
        combined_rects[i] = merge;
        combined_rects[j] = combined_rects[final_count - 1];
        final_count--;
        j = i;
      }
    }
  }

  // Full screen special cases
  if (mission_control_active || final_count > 16 || force_full_redraw > 0) {
    if (force_full_redraw > 0)
      force_full_redraw--;

    rect_t screen = {0, 0, screen_width, screen_height};
    compositor_render_rect(screen);

    if (mission_control_active) {
      int total = screen_width * screen_height;
      uint32_t *p = backbuffer;
      for (int i = 0; i < total; i++) {
        uint32_t src = p[i];
        uint32_t rb = (src & 0x00FF00FF) * 80;
        uint32_t g = (src & 0x0000FF00) * 80;
        p[i] = 0xFF000000 | ((rb >> 8) & 0x00FF00FF) | ((g >> 8) & 0x0000FF00);
      }

      extern int window_z_order[];
      extern int window_count;
      extern window_t windows[];
      int open_count = 0;
      int open_indices[32];
      for (int i = 0; i < window_count && open_count < 32; i++) {
        window_t *win = &windows[window_z_order[i]];
        if (win->id != 0)
          open_indices[open_count++] = window_z_order[i];
      }

      if (open_count > 0) {
        int cols = (open_count > 6) ? 4 : (open_count > 2 ? 3 : 2);
        int rows = (open_count + cols - 1) / cols;
        int pad = 24;
        int top_margin = 50;
        int thumb_w = (screen_width - pad * (cols + 1)) / cols;
        int thumb_h =
            (screen_height - top_margin - pad * (rows + 1) - 60) / rows;
        if (thumb_h > 300)
          thumb_h = 300;

        vga_draw_string_lfb(screen_width / 2 - 60, 20, "Mission Control",
                            0xFFFFFFFF, backbuffer);

        for (int i = 0; i < open_count; i++) {
          int col = i % cols;
          int row = i / cols;
          int tx = pad + col * (thumb_w + pad);
          int ty = top_margin + pad + row * (thumb_h + pad + 20);
          window_t *win = &windows[open_indices[i]];
          extern window_t *active_window;
          int is_active_win = (win == active_window);

          uint32_t card_col = is_active_win ? 0xFF2A4A6A : 0xFF2A2A38;
          for (int cy = 0; cy < thumb_h + 20; cy++) {
            for (int cx = 0; cx < thumb_w; cx++) {
              int gx = tx + cx, gy = ty + cy;
              if (gx < 0 || gx >= screen_width || gy < 0 || gy >= screen_height)
                continue;
              int r = 8, draw = 1;
              if (cx < r && cy < r) {
                int ddx = r - cx - 1, ddy = r - cy - 1;
                if (ddx * ddx + ddy * ddy >= r * r) draw = 0;
              } else if (cx >= thumb_w - r && cy < r) {
                int ddx = cx - (thumb_w - r), ddy = r - cy - 1;
                if (ddx * ddx + ddy * ddy >= r * r) draw = 0;
              } else if (cx < r && cy >= thumb_h + 20 - r) {
                int ddx = r - cx - 1, ddy = cy - (thumb_h + 20 - r);
                if (ddx * ddx + ddy * ddy >= r * r) draw = 0;
              } else if (cx >= thumb_w - r && cy >= thumb_h + 20 - r) {
                int ddx = cx - (thumb_w - r), ddy = cy - (thumb_h + 20 - r);
                if (ddx * ddx + ddy * ddy >= r * r) draw = 0;
              }
              if (draw)
                backbuffer[gy * screen_width + gx] = card_col;
            }
          }

          if (is_active_win) {
            for (int bx = 0; bx < thumb_w; bx++) {
              backbuffer[ty * screen_width + tx + bx] = 0xFF4488CC;
              backbuffer[(ty + thumb_h + 19) * screen_width + tx + bx] =
                  0xFF4488CC;
            }
            for (int by = 0; by < thumb_h + 20; by++) {
              backbuffer[(ty + by) * screen_width + tx] = 0xFF4488CC;
              backbuffer[(ty + by) * screen_width + tx + thumb_w - 1] =
                  0xFF4488CC;
            }
          }

          if (win->surface && win->surface_w > 0 && win->surface_h > 0) {
            for (int dy = 0; dy < thumb_h; dy++) {
              int gy = ty + 18 + dy;
              if (gy < 0 || gy >= screen_height) continue;
              int src_y = (dy * win->surface_h) / thumb_h;
              uint32_t *src_row = &win->surface[src_y * win->surface_w];
              uint32_t *dst_row = &backbuffer[gy * screen_width];
              for (int dx = 0; dx < thumb_w - 8; dx++) {
                int gx = tx + 4 + dx;
                if (gx < 0 || gx >= screen_width) continue;
                int src_x = (dx * win->surface_w) / (thumb_w - 8);
                dst_row[gx] = src_row[src_x];
              }
            }
          }
        }
      }
    }
  } else {
    for (int i = 0; i < final_count; i++) {
      compositor_render_rect(combined_rects[i]);
    }
  }

  uint32_t *vram = bga_get_render_buffer();
  if (vram) {
    if (magnifier_enabled) {
      compositor_draw_magnifier(vram, mouse_x, mouse_y);
    }
    extern void kernel_draw_mouse_to_buffer(uint32_t *target, int mx, int my);
    kernel_draw_mouse_to_buffer(vram, mouse_x, mouse_y);
    bga_flip();
  }

  prev_2_dirty_count = prev_dirty_count;
  for (int i = 0; i < prev_dirty_count; i++)
    prev_2_dirty_rects[i] = prev_dirty_rects[i];

  prev_dirty_count = dirty_count;
  for (int i = 0; i < dirty_count; i++)
    prev_dirty_rects[i] = dirty_rects[i];

  dirty_count = 0;

  uint64_t comp_end = rdtsc();
  compositor_cycles = comp_end - comp_start;

  extern uint32_t get_timer_ticks(void);
  uint32_t now = get_timer_ticks();
  if (now - last_fps_tick >= 1000) {
    current_fps = frames_this_second;
    frames_this_second = 0;
    last_fps_tick = now;
    compositor_invalidate_rect(210, 0, 320, 24);
  }
  frames_this_second++;
}

void compositor_set_overlay(int x, int y, int w, int h, int enabled) {
  if (!enabled) {
    if (overlay_active) {
      compositor_invalidate_rect(overlay_rect.x, overlay_rect.y, overlay_rect.w,
                                 overlay_rect.h);
      overlay_active = 0;
    }
    return;
  }
  if (!overlay_active) {
    overlay_active = 1;
    overlay_rect.x = x;
    overlay_rect.y = y;
    overlay_rect.w = w;
    overlay_rect.h = h;
    compositor_invalidate_rect(x, y, w, h);
  } else if (x != overlay_rect.x || y != overlay_rect.y ||
             w != overlay_rect.w || h != overlay_rect.h) {
    compositor_invalidate_rect(overlay_rect.x, overlay_rect.y, overlay_rect.w,
                               overlay_rect.h);
    overlay_rect.x = x;
    overlay_rect.y = y;
    overlay_rect.w = w;
    overlay_rect.h = h;
    compositor_invalidate_rect(x, y, w, h);
  }
}

void compositor_blur_rect(int x, int y, int w, int h, int radius) {
  if (!backbuffer || w <= radius * 2 || h <= radius * 2)
    return;

  int x1 = (x < radius) ? radius : x;
  int y1 = (y < radius) ? radius : y;
  int x2 = (x + w >= screen_width - radius) ? screen_width - radius - 1 : x + w;
  int y2 =
      (y + h >= screen_height - radius) ? screen_height - radius - 1 : y + h;

  int bw = x2 - x1;
  int bh = y2 - y1;
  if (bw <= 0 || bh <= 0 || bw >= 4096 || bh >= 4096)
    return;

  const int r = radius;
  const int div = (2 * r + 1);
  static uint32_t line_buf[4096];

  for (int j = y1; j < y2; j++) {
    uint32_t r_sum = 0, g_sum = 0, b_sum = 0;
    uint32_t *row = &backbuffer[j * screen_width];

    for (int i = -r; i <= r; i++) {
      uint32_t p = row[x1 + i];
      r_sum += (p >> 16) & 0xFF;
      g_sum += (p >> 8) & 0xFF;
      b_sum += p & 0xFF;
    }

    for (int i = 0; i < bw; i++) {
      line_buf[i] = 0xFF000000 | ((r_sum / div) << 16) | ((g_sum / div) << 8) |
                    (b_sum / div);
      if (i + 1 < bw) {
        uint32_t p_out = row[x1 + i - r];
        uint32_t p_in = row[x1 + i + r + 1];
        r_sum = r_sum - ((p_out >> 16) & 0xFF) + ((p_in >> 16) & 0xFF);
        g_sum = g_sum - ((p_out >> 8) & 0xFF) + ((p_in >> 8) & 0xFF);
        b_sum = b_sum - (p_out & 0xFF) + (p_in & 0xFF);
      }
    }
    memcpy(&row[x1], line_buf, bw * 4);
  }

  for (int i = x1; i < x2; i++) {
    uint32_t r_sum = 0, g_sum = 0, b_sum = 0;
    for (int j = -r; j <= r; j++) {
      uint32_t p = backbuffer[(y1 + j) * screen_width + i];
      r_sum += (p >> 16) & 0xFF;
      g_sum += (p >> 8) & 0xFF;
      b_sum += p & 0xFF;
    }

    for (int j = 0; j < bh; j++) {
      line_buf[j] = 0xFF000000 | ((r_sum / div) << 16) |
                         ((g_sum / div) << 8) | (b_sum / div);
      if (j + 1 < bh) {
        uint32_t p_out = backbuffer[(y1 + j - r) * screen_width + i];
        uint32_t p_in = backbuffer[(y1 + j + r + 1) * screen_width + i];
        r_sum = r_sum - ((p_out >> 16) & 0xFF) + ((p_in >> 16) & 0xFF);
        g_sum = g_sum - ((p_out >> 8) & 0xFF) + ((p_in >> 8) & 0xFF);
        b_sum = b_sum - (p_out & 0xFF) + (p_in & 0xFF);
      }
    }
    for (int j = 0; j < bh; j++) {
      backbuffer[(y1 + j) * screen_width + i] = line_buf[j];
    }
  }
}
