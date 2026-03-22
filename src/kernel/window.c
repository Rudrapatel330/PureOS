#include "window.h"
#include "../gui/startmenu.h"
#include "../gui/taskbar.h"
#include "../ui/ctxmenu.h" // Logic for Right Click
#include "anim.h"
#include "clipboard.h"
#include "compositor.h"
#include "config.h"
#include "heap.h"
#include "screen.h"
#include "string.h"
#include "task.h"
#include "theme.h"
#include <stddef.h>

// Integer-based Supersampled SDF alpha mask helper (4x4)
static uint8_t get_corner_alpha_int(int x, int y, int r) {
  int total = 0;
  // 4x4 supersampling is sufficient and much faster than 8x8
  for (int sy = 0; sy < 4; sy++) {
    for (int sx = 0; sx < 4; sx++) {
      int dx = r * 8 - (x * 8 + sx * 2 + 1);
      int dy = r * 8 - (y * 8 + sy * 2 + 1);
      if (dx * dx + dy * dy <= r * r * 64)
        total++;
    }
  }
  return (uint8_t)(total * 255 / 16);
}

// Helper to blend color (used for SDF masking)
static inline uint32_t blend_color(uint32_t src, uint32_t dst, uint8_t alpha) {
  if (alpha == 255)
    return src;
  if (alpha == 0)
    return dst;

  uint32_t s_r = (src >> 16) & 0xFF;
  uint32_t s_g = (src >> 8) & 0xFF;
  uint32_t s_b = src & 0xFF;

  uint32_t d_r = (dst >> 16) & 0xFF;
  uint32_t d_g = (dst >> 8) & 0xFF;
  uint32_t d_b = dst & 0xFF;

  uint32_t r = ((s_r - d_r) * alpha >> 8) + d_r;
  uint32_t g = ((s_g - d_g) * alpha >> 8) + d_g;
  uint32_t b = ((s_b - d_b) * alpha >> 8) + d_b;

  return 0xFF000000 | (r << 16) | (g << 8) | b;
}

// Forward declarations

void winmgr_bring_to_front(window_t *win);
void start_window_anim(window_t *win, int sx, int sy, int sw, int sh, int ex,
                       int ey, int ew, int eh, float duration, int type);
void start_window_spring(window_t *win, int sx, int sy, int sw, int sh, int ex,
                         int ey, int ew, int eh, float stiffness,
                         float damping);

// Hack for header issue
void compositor_invalidate_window(window_t *win);

// Supersampled SDF alpha mask helper (REPLACED BY INTEGER VERSION)
// (Function removed to save space and prevent FPU use)

// 16bpp Window Manager Drawing - use backbuffer for compositing
extern uint32_t *backbuffer;

// 16bpp Text Drawing from Kernel
extern void draw_text_16bpp(int x, int y, const char *text, uint16_t color);
extern void vga_draw_scaled_bitmap(int dx, int dy, int dw, int dh,
                                   uint32_t *src_buf, int sw, int sh,
                                   uint32_t *dest_buf, int dest_w, int dest_h);
extern void print_serial(const char *);

// Accept 32-bit ARGB colors directly
static uint32_t validate_color_32(uint32_t color) {
  // If no alpha specified, assume opaque
  if (color <= 0xFFFFFF)
    return 0xFF000000 | color;
  return color;
}

// Helper to draw pixel efficiently in 32bpp
void put_pixel_32(int x, int y, uint32_t color) {
  if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
    backbuffer[y * screen_width + x] = color;
  }
}

// Draw rect 32bpp
void draw_rect_32(int x, int y, int w, int h, uint32_t color) {
  // Clip to screen
  if (x >= screen_width || y >= screen_height)
    return;
  if (x + w < 0 || y + h < 0)
    return;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }

  if (x + w > screen_width)
    w = screen_width - x;
  if (y + h > screen_height)
    h = screen_height - y;

  if (w <= 0 || h <= 0)
    return;

  for (int cy = 0; cy < h; cy++) {
    uint32_t offset = (y + cy) * screen_width + x;
    uint32_t *dest = &backbuffer[offset];
    for (int cx = 0; cx < w; cx++) {
      dest[cx] = color;
    }
  }
}

// Simple implementation globals
window_t windows[MAX_WINDOWS];
int window_z_order[MAX_WINDOWS]; // Array of indices into windows[]
int window_count = 0;

// Removed local str_cpy to use standard strcpy from string.h

int str_len(const char *str) {
  int len = 0;
  while (str[len])
    len++;
  return len;
}

int next_anim_origin_x = -1;
int next_anim_origin_y = -1;

void winmgr_init() {
  window_count = 0;
  for (int i = 0; i < MAX_WINDOWS; i++) {
    windows[i].id = 0;
    window_z_order[i] = -1;
  }
}

int winmgr_resize_surface(window_t *win, int new_w, int new_h) {
  if (new_w == win->surface_w && new_h == win->surface_h && win->surface)
    return 1;

  // Safety
  if (new_w < 1)
    new_w = 1;
  if (new_h < 1)
    new_h = 1;

  // Try to allocate NEW surface first - don't free old one until success!
  int total = new_w * new_h;
  uint32_t *new_surface = (uint32_t *)kmalloc(total * 4);

  if (!new_surface) {
    print_serial("WINMGR: surface allocation FAILED for size ");
    char num[16];
    k_itoa(new_w * new_h * 4, num);
    print_serial(num);
    print_serial("\n");
    // Keep old surface intact - don't leave window black!
    return 0;
  }

  // Allocation succeeded - now we can safely free old surface
  if (win->surface) {
    kfree(win->surface);
  }

  win->surface = new_surface;
  win->surface_w = new_w;
  win->surface_h = new_h;

  // DEBUG: confirm successful allocation
  print_serial("WINMGR: surface alloc OK bytes=");
  char _dbg_sz[20];
  k_itoa(new_w * new_h * 4, _dbg_sz);
  print_serial(_dbg_sz);
  print_serial("\n");

  // CRITICAL: Always force full opaque alpha on the fill color.
  // Some apps set bg_color without the 0xFF alpha prefix, which makes
  // the surface transparent and causes black window rendering.
  uint32_t bg32 =
      0xFF000000 | (validate_color_32(win->bg_color) & 0x00FFFFFF);
  extern void simd_fill_32(uint32_t *dest, uint32_t value, size_t count);
  simd_fill_32(win->surface, bg32, total);
  win->needs_redraw = 1;
  return 1;
}

extern void print_serial(const char *);

// ============================================================
// Window Querying Helpers
// ============================================================

window_t *winmgr_get_window_by_app_type(int app_type) {
  for (int i = 0; i < window_count; i++) {
    if (windows[i].id != 0 && windows[i].app_type == app_type &&
        windows[i].fading_mode != 2) {
      return &windows[i];
    }
  }
  return 0; // null pointer
}

void winmgr_invalidate_rect(window_t *win, int x, int y, int w, int h) {
  if (!win || win->id == 0)
    return;

  rect_t r = {x, y, w, h};
  if (!win->sub_dirty_active) {
    win->sub_dirty = r;
    win->sub_dirty_active = 1;
  } else {
    // Union
    int x1 = (win->sub_dirty.x < r.x) ? win->sub_dirty.x : r.x;
    int y1 = (win->sub_dirty.y < r.y) ? win->sub_dirty.y : r.y;
    int x2 = (win->sub_dirty.x + win->sub_dirty.w > r.x + r.w)
                 ? win->sub_dirty.x + win->sub_dirty.w
                 : r.x + r.w;
    int y2 = (win->sub_dirty.y + win->sub_dirty.h > r.y + r.h)
                 ? win->sub_dirty.y + win->sub_dirty.h
                 : r.y + r.h;
    win->sub_dirty.x = x1;
    win->sub_dirty.y = y1;
    win->sub_dirty.w = x2 - x1;
    win->sub_dirty.h = y2 - y1;
  }

  // Also notify compositor of global change
  extern void compositor_invalidate_rect(int x, int y, int w, int h);
  compositor_invalidate_rect(win->x + x, win->y + y, w, h);
}

window_t *winmgr_create_window(int x, int y, int w, int h, const char *title) {
  // print_serial("WINMGR: create_window start\n");

  // Find empty slot
  window_t *win = 0;
  int slot_idx = -1;
  for (int i = 0; i < MAX_WINDOWS; i++) {
    if (windows[i].id == 0) {
      win = &windows[i];
      slot_idx = i;
      win->id = i + 1; // Use 1-based ID
      extern task_t *get_current_task();
      win->owner_pid = get_current_task()->id;
      break;
    }
  }

  if (!win) {
    // print_serial("WINMGR: max windows reached!\n");
    return 0;
  }

  // Add the new window's index to the top of the Z-order
  window_z_order[window_count] = slot_idx;
  window_count++;

  // print_serial("WINMGR: got window slot\n");
  win->app_type = -1; // Default
  win->draw = 0;      // Important: clear reused slot
  win->on_mouse = 0;
  win->on_key = 0;
  win->on_scroll = 0;
  win->on_copy = 0;
  win->on_cut = 0;
  win->on_paste = 0;
  win->on_close = 0;  // CRITICAL: Clear stale on_close from reused slot
  win->user_data = 0; // CRITICAL: Clear stale user_data from reused slot
  win->exists = 1;    // Mark slot as in use
  win->workspace = 0; // Default to workspace 0
  win->ws_hidden = 0; // Visible by default
  win->flags = WINDOW_FLAG_NONE; // CRITICAL: Reset flags (No Titlebar, etc.)
  win->always_on_top = 0;
  extern int screen_width;
  extern int screen_height;

  // Handle auto-centering
  if (x == -1 && y == -1) {
    x = (screen_width - w) / 2;
    y = (screen_height - h) / 2;
  }

  win->x = x;
  win->y = y;
  win->prev_x = x;
  win->prev_y = y;
  win->width = w;
  win->height = h;
  win->bg_color = 0xFFC0C0C0; // Default Gray (32bpp)

  extern os_config_t global_config;
  if (global_config.theme_mode == 1) {
    win->style = STYLE_DEFAULT; // Light Mode
  } else {
    win->style = STYLE_DARK; // Dark Mode
  }

  win->is_dragging = 0;
  win->drag_offset_x = 0;
  win->drag_offset_y = 0;
  win->is_resizing = 0;
  win->resize_edge = 0;

  // Init States
  win->is_maximized = 0;
  win->sub_dirty_active = 0;
  win->sub_dirty = (rect_t){0, 0, 0, 0};
  win->always_on_top = 0;

  // CRITICAL: Initialize opacity and scale
  win->opacity = 255;
  win->scale = 1.0f;
  win->fading_mode = 0;
  win->anim_opacity.active = 0;
  win->anim_scale.active = 0;

  // Ensure surface is NULL before resizing to prevent invalid reuse
  // (winmgr_resize_surface handles the allocation)
  win->surface = 0;
  win->surface_w = 0;
  win->surface_h = 0;

  if (!winmgr_resize_surface(win, w, h)) {
    // If resize fails, we MUST abort window creation.
    // Otherwise we have a window with a NULL surface that will cause OOB/glitches.
    print_serial("WINMGR: create_window ABORTED (surface alloc failed)\n");
    win->id = 0;
    win->exists = 0;
    return 0;
  }
  win->is_snapped = 0;

  win->saved_x = x;
  win->saved_y = y;
  win->saved_w = w;
  win->saved_h = h;

  // Init Buffer
  win->text_buffer[0] = 0;
  win->cursor_pos = 0;
  win->needs_redraw = 1;

  // Init Animation State
  win->is_animating = 1;
  win->watchdog = 0; // RESET WATCHDOG
  win->taskbar_x = -1;
  win->taskbar_y = -1;
  win->launch_x = -1;
  win->launch_y = -1;

  // Start "Pop-in" Open Animation - Smooth scale up and spring
  win->anim_mode = 1; // Open
  int sx = x + w / 2;
  int sy = y + h; // Bottom anchored by default for "rising up"
  int sw = 0;
  int sh = 0;

  extern int next_anim_origin_x;
  extern int next_anim_origin_y;
  extern int screen_width;

  if (next_anim_origin_x >= 0) {
    win->taskbar_x = next_anim_origin_x;
    win->taskbar_y = next_anim_origin_y >= 0 ? next_anim_origin_y : y + h;
    win->launch_x = win->taskbar_x;
    win->launch_y = win->taskbar_y;
    // Perfect Fountain bounds mapping to Dock location
    sx = win->taskbar_x;
    sy = win->taskbar_y;

    // Reset for next launch
    next_anim_origin_x = -1;
    next_anim_origin_y = -1;
  } else {
    // Default taskbar-centered fountain fallback for Start Menu / CLI launches
    win->taskbar_x = screen_width / 2;
    win->taskbar_y = screen_height;
    win->launch_x = win->taskbar_x;
    win->launch_y = win->taskbar_y;
    sx = win->taskbar_x;
    sy = win->taskbar_y;
  }

  // macOS Genie: fast initial pull, soft decelerating spring finish
  float kw = 280.0f;
  float dw = 30.0f;

  extern int screen_height;
  win->pinch_top = (win->launch_y >= 0 && win->launch_y < screen_height / 2);

  // Ease-in-out position/size animation from icon origin to final window rect
  anim_start_spring(&win->anim_x, (float)sx, (float)x, kw, dw);
  anim_start_spring(&win->anim_y, (float)sy, (float)y, kw, dw);
  anim_start_spring(&win->anim_w, (float)sw, (float)w, kw, dw);
  anim_start_spring(&win->anim_h, (float)sh, (float)h, kw, dw);

  // Phase 1: Allocate Surface
  win->opacity = 0;
  win->fading_mode = 1; // Fade in
  anim_start_spring(&win->anim_opacity, 0.0f, 255.0f, kw, dw);

  // Phase 3: Blur Support
  win->blur_enabled = 0;
  win->blur_strength = 3;

  // Init Momentum
  win->vel_x = 0;
  win->vel_y = 0;

  // Phase 4: Scaling & Warp Support — Genie scale-up
  win->scale = 0.01f; // Start from near-zero for true genie emergence
  anim_start_spring(&win->anim_scale, 0.01f, 1.0f, kw, dw);

  win->scroll_position = 0;
  win->content_height = 0;
  win->visible_height = h;
  win->max_scroll = 0;
  win->scroll_line_height = 16;

  // Surface allocation with retry on heap-fragmentation failure.
  // Each attempt reduces dimensions by ~10%, down to a 64-pixel floor.
  {
    int alloc_w = w;
    int alloc_h = h;
    int attempts = 0;
    while (!winmgr_resize_surface(win, alloc_w, alloc_h) && attempts < 3) {
      print_serial("WINMGR: surface alloc failed, retrying smaller...\n");
      alloc_w = alloc_w * 9 / 10;
      alloc_h = alloc_h * 9 / 10;
      if (alloc_w < 64) alloc_w = 64;
      if (alloc_h < 64) alloc_h = 64;
      attempts++;
    }
    if (!win->surface) {
      print_serial("WINMGR: create failed, surface OOM after retries\n");
      win->id = 0; // Reclaim slot
      return 0;
    }
    // If we settled on a smaller size, update the window's geometry to match
    if (alloc_w != w || alloc_h != h) {
      print_serial("WINMGR: window created at reduced size due to heap pressure\n");
      win->width        = alloc_w;
      win->height       = alloc_h;
      win->saved_w      = alloc_w;
      win->saved_h      = alloc_h;
      win->visible_height = alloc_h;

      // CRITICAL: Restart animations targeting the actual (smaller) size.
      anim_start_spring(&win->anim_w, 0.0f, (float)alloc_w, kw, dw);
      anim_start_spring(&win->anim_h, 0.0f, (float)alloc_h, kw, dw);

      // Re-anchor position so the smaller window is still centered at (x,y)
      int new_sx = x + alloc_w / 2; // Default center anchor
      if (win->launch_x >= 0) {
        extern int screen_width;
        if (win->launch_x < screen_width / 3)
          new_sx = x;
        else if (win->launch_x > (2 * screen_width) / 3)
          new_sx = x + alloc_w;
        else
          new_sx = x + alloc_w / 2;
      }
      int new_sy = (win->launch_y >= 0) ? win->launch_y : y + alloc_h;
      anim_start_spring(&win->anim_x, (float)new_sx, (float)x, kw, dw);
      anim_start_spring(&win->anim_y, (float)new_sy, (float)y, kw, dw);
    }
  }

  // Use standard strcpy from string.h
  strcpy(win->title, title);
  // print_serial("WINMGR: title set\n");

  // Auto-focus new window
  extern void winmgr_bring_to_front(window_t * win);
  winmgr_bring_to_front(win);

  // NOTE: Do NOT call winmgr_render_window() here!
  // The app must set its draw callback first, then trigger a redraw.
  // Calling render here before draw callback is set results in blank windows!
  win->needs_redraw = 1;

  // Force compositor to do a full redraw after window creation
  // This helps ensure the window renders correctly after app close/open cycles
  extern void compositor_reset_history();
  compositor_reset_history();

  // print_serial("WINMGR: create_window done\n");
  return win;
}

void winmgr_blit(window_t *dst, int dx, int dy, const uint32_t *src, int sw,
                 int sh, int sx, int sy, int w, int h) {
  if (!dst || !dst->surface || !src || w <= 0 || h <= 0)
    return;

  // Clip to destination surface
  if (dx < 0) {
    w += dx;
    sx -= dx;
    dx = 0;
  }
  if (dy < 0) {
    h += dy;
    sy -= dy;
    dy = 0;
  }
  if (dx + w > dst->surface_w)
    w = dst->surface_w - dx;
  if (dy + h > dst->surface_h)
    h = dst->surface_h - dy;

  // Clip to source surface
  if (sx < 0) {
    w += sx;
    dx -= sx;
    sx = 0;
  }
  if (sy < 0) {
    h += sy;
    dy -= sy;
    sy = 0;
  }
  if (sx + w > sw)
    w = sw - sx;
  if (sy + h > sh)
    h = sh - sy;

  if (w <= 0 || h <= 0)
    return;

  extern void blit_copy_rect_32(uint32_t *dst, int dst_stride,
                                const uint32_t *src, int src_stride, int dx,
                                int dy, int sx, int sy, int w, int h);
  blit_copy_rect_32(dst->surface, dst->surface_w, src, sw, dx, dy, sx, sy, w,
                    h);
  winmgr_invalidate_rect(dst, dx, dy, w, h);
}

void winmgr_blend_blit(window_t *dst, int dx, int dy, const uint32_t *src,
                       int sw, int sh, int sx, int sy, int w, int h,
                       uint8_t alpha) {
  if (!dst || !dst->surface || !src || w <= 0 || h <= 0 || alpha == 0)
    return;

  // Generic clipping (same as winmgr_blit)
  if (dx < 0) {
    w += dx;
    sx -= dx;
    dx = 0;
  }
  if (dy < 0) {
    h += dy;
    sy -= dy;
    dy = 0;
  }
  if (dx + w > dst->surface_w)
    w = dst->surface_w - dx;
  if (dy + h > dst->surface_h)
    h = dst->surface_h - dy;
  if (sx < 0) {
    w += sx;
    dx -= sx;
    sx = 0;
  }
  if (sy < 0) {
    h += sy;
    dy -= sy;
    sy = 0;
  }
  if (sx + w > sw)
    w = sw - sx;
  if (sy + h > sh)
    h = sh - sy;

  if (w <= 0 || h <= 0)
    return;

  extern void blit_blend_rect_32(
      uint32_t *dst, int dst_stride, const uint32_t *src, int src_stride,
      int dx, int dy, int sx, int sy, int w, int h, uint8_t alpha);
  blit_blend_rect_32(dst->surface, dst->surface_w, src, sw, dx, dy, sx, sy, w,
                     h, alpha);
  winmgr_invalidate_rect(dst, dx, dy, w, h);
}

extern const uint8_t font8x8_basic[256][8];

void winmgr_fill_rect(window_t *win, int x, int y, int w, int h,
                      uint32_t color) {
  if (!win || !win->surface)
    return;

  // Clip to Surface
  if (x >= win->surface_w || y >= win->surface_h)
    return;
  if (x + w < 0 || y + h < 0)
    return;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }

  if (x + w > win->surface_w)
    w = win->surface_w - x;
  if (y + h > win->surface_h)
    h = win->surface_h - y;

  if (w <= 0 || h <= 0)
    return;

  uint32_t c32 = validate_color_32(color);
  int stride = win->surface_w;

  extern void simd_fill_32(uint32_t *dest, uint32_t value, size_t count);

  for (int cy = 0; cy < h; cy++) {
    uint32_t *line = &win->surface[(y + cy) * stride + x];
    simd_fill_32(line, c32, w);
  }

  winmgr_invalidate_rect(win, x, y, w, h);
}

void winmgr_fill_rect_gradient_v(window_t *win, int x, int y, int w, int h,
                                 uint32_t col_top, uint32_t col_bot) {
  if (!win || !win->surface)
    return;

  // Clip to Surface
  if (x >= win->surface_w || y >= win->surface_h)
    return;
  if (x + w < 0 || y + h < 0)
    return;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }

  if (x + w > win->surface_w)
    w = win->surface_w - x;
  if (y + h > win->surface_h)
    h = win->surface_h - y;

  if (w <= 0 || h <= 0)
    return;

  // Integer gradient — no float per row
  int sr = (col_top >> 16) & 0xFF;
  int sg = (col_top >> 8) & 0xFF;
  int sb = col_top & 0xFF;
  int sa = (col_top >> 24) & 0xFF;

  int er = (col_bot >> 16) & 0xFF;
  int eg = (col_bot >> 8) & 0xFF;
  int eb = col_bot & 0xFF;
  int ea = (col_bot >> 24) & 0xFF;

  int stride = win->surface_w;
  for (int cy = 0; cy < h; cy++) {
    uint8_t r = (uint8_t)(sr + (er - sr) * cy / h);
    uint8_t g = (uint8_t)(sg + (eg - sg) * cy / h);
    uint8_t b = (uint8_t)(sb + (eb - sb) * cy / h);
    uint8_t a = (uint8_t)(sa + (ea - sa) * cy / h);
    uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;

    uint32_t *row = &win->surface[(y + cy) * stride + x];
    for (int cx = 0; cx < w; cx++) {
      row[cx] = color;
    }
  }

  winmgr_invalidate_rect(win, x, y, w, h);
}

void winmgr_draw_rect(window_t *win, int x, int y, int w, int h,
                      uint32_t color) {
  // Outline
  winmgr_fill_rect(win, x, y, w, 1, color);         // Top
  winmgr_fill_rect(win, x, y, 1, h, color);         // Left
  winmgr_fill_rect(win, x, y + h - 1, w, 1, color); // Bottom
  winmgr_fill_rect(win, x + w - 1, y, 1, h, color); // Right
}

// Bresenham Line for Window
void winmgr_draw_line(window_t *win, int x1, int y1, int x2, int y2,
                      uint32_t color) {
  int start_x = x1, start_y = y1;
  int end_x = x2, end_y = y2;

  int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
  int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  while (1) {
    winmgr_put_pixel(win, x1, y1, color);
    if (x1 == x2 && y1 == y2)
      break;
    int e2 = err * 2;
    if (e2 > -dy) {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }

  int bx = (start_x < end_x) ? start_x : end_x;
  int by = (start_y < end_y) ? start_y : end_y;
  int bw = abs(end_x - start_x) + 1;
  int bh = abs(end_y - start_y) + 1;
  winmgr_invalidate_rect(win, bx, by, bw, bh);
}

void winmgr_put_pixel(window_t *win, int x, int y, uint32_t color) {
  if (!win || !win->surface)
    return;
  if (x < 0 || x >= win->surface_w || y < 0 || y >= win->surface_h)
    return;

  win->surface[y * win->surface_w + x] = validate_color_32(color);
}

// Anti-aliasing helper: blend a pixel with alpha (0-255)
static void winmgr_put_pixel_aa(window_t *win, int x, int y, uint32_t color,
                                uint8_t alpha) {
  if (!win || !win->surface || alpha == 0)
    return;
  if (x < 0 || x >= win->surface_w || y < 0 || y >= win->surface_h)
    return;

  if (alpha == 255) {
    win->surface[y * win->surface_w + x] = validate_color_32(color);
    return;
  }

  uint32_t dst = win->surface[y * win->surface_w + x];
  uint32_t src = validate_color_32(color);

  // Extract channels
  uint32_t s_r = (src >> 16) & 0xFF;
  uint32_t s_g = (src >> 8) & 0xFF;
  uint32_t s_b = src & 0xFF;

  uint32_t d_r = (dst >> 16) & 0xFF;
  uint32_t d_g = (dst >> 8) & 0xFF;
  uint32_t d_b = dst & 0xFF;

  // Blend
  uint32_t r = ((s_r - d_r) * alpha >> 8) + d_r;
  uint32_t g = ((s_g - d_g) * alpha >> 8) + d_g;
  uint32_t b = ((s_b - d_b) * alpha >> 8) + d_b;

  // We set the pixel with full alpha (0xFF) because we are blending INTO the
  // surface If we wanted to preserve surface alpha, we'd blend the alpha
  // channel too.
  win->surface[y * win->surface_w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
}

// Fixed point math helpers for Xiaolin Wu
#define FX_SHIFT 8
#define FX_ONE (1 << FX_SHIFT)
#define FX_PART(x) ((x) & (FX_ONE - 1))
#define FX_INV_PART(x) (FX_ONE - FX_PART(x))

// Xiaolin Wu's Anti-Aliased Line
void winmgr_draw_line_aa(window_t *win, int x1, int y1, int x2, int y2,
                         uint32_t color) {
  int start_x = x1, start_y = y1;
  int end_x = x2, end_y = y2;

  int steep = abs(y2 - y1) > abs(x2 - x1);
  if (steep) {
    int t;
    t = x1;
    x1 = y1;
    y1 = t;
    t = x2;
    x2 = y2;
    y2 = t;
  }
  if (x1 > x2) {
    int t;
    t = x1;
    x1 = x2;
    x2 = t;
    t = y1;
    y1 = y2;
    y2 = t;
  }

  int dx = x2 - x1;
  int dy = y2 - y1;
  int gradient = (dx == 0) ? FX_ONE : (dy << FX_SHIFT) / dx;

  // Handle first endpoint
  int xend = x1;
  int yend = y1 << FX_SHIFT;
  int xpxl1 = xend;
  int ypxl1 = yend >> FX_SHIFT;
  if (steep) {
    winmgr_put_pixel_aa(win, ypxl1, xpxl1, color, FX_INV_PART(yend));
    winmgr_put_pixel_aa(win, ypxl1 + 1, xpxl1, color, FX_PART(yend));
  } else {
    winmgr_put_pixel_aa(win, xpxl1, ypxl1, color, FX_INV_PART(yend));
    winmgr_put_pixel_aa(win, xpxl1, ypxl1 + 1, color, FX_PART(yend));
  }
  int intery = yend + gradient;

  // Main loop
  for (int x = x1 + 1; x < x2; x++) {
    if (steep) {
      winmgr_put_pixel_aa(win, intery >> FX_SHIFT, x, color,
                          FX_INV_PART(intery));
      winmgr_put_pixel_aa(win, (intery >> FX_SHIFT) + 1, x, color,
                          FX_PART(intery));
    } else {
      winmgr_put_pixel_aa(win, x, intery >> FX_SHIFT, color,
                          FX_INV_PART(intery));
      winmgr_put_pixel_aa(win, x, (intery >> FX_SHIFT) + 1, color,
                          FX_PART(intery));
    }
    intery += gradient;
  }

  // Handle second endpoint
  int xpxl2 = x2;
  int ypxl2 = y2 << FX_SHIFT;
  if (steep) {
    winmgr_put_pixel_aa(win, ypxl2 >> FX_SHIFT, xpxl2, color,
                        FX_INV_PART(ypxl2));
    winmgr_put_pixel_aa(win, (ypxl2 >> FX_SHIFT) + 1, xpxl2, color,
                        FX_PART(ypxl2));
  } else {
    winmgr_put_pixel_aa(win, xpxl2, ypxl2 >> FX_SHIFT, color,
                        FX_INV_PART(ypxl2));
    winmgr_put_pixel_aa(win, xpxl2, (ypxl2 >> FX_SHIFT) + 1, color,
                        FX_PART(ypxl2));
  }

  int bx = (start_x < end_x) ? start_x : end_x;
  int by = (start_y < end_y) ? start_y : end_y;
  int bw = abs(end_x - start_x) + 2; // +2 for AA spillover
  int bh = abs(end_y - start_y) + 2;
  winmgr_invalidate_rect(win, bx, by, bw, bh);
}

// Solid Arc for corners (used before masking)
void winmgr_draw_arc_solid(window_t *win, int cx, int cy, int radius,
                           int quadrant, uint32_t color) {
  uint32_t c32 = validate_color_32(color);
  for (int y = 0; y <= radius; y++) {
    for (int x = 0; x <= radius; x++) {
      if (x * x + y * y <= radius * radius) {
        int px = 0, py = 0;
        if (quadrant == 0) {
          px = cx - x;
          py = cy - y;
        } // TL
        else if (quadrant == 1) {
          px = cx + x;
          py = cy - y;
        } // TR
        else if (quadrant == 2) {
          px = cx - x;
          py = cy + y;
        } // BL
        else if (quadrant == 3) {
          px = cx + x;
          py = cy + y;
        } // BR
        if (px >= 0 && px < win->surface_w && py >= 0 && py < win->surface_h) {
          win->surface[py * win->surface_w + px] = c32;
        }
      }
    }
  }
}

// Local helper for drawing char to surface
static void draw_char_surface(window_t *win, int x, int y, char c,
                              uint32_t color) {
  if (!win || !win->surface)
    return;

  const uint8_t *glyph = font8x8_basic[(unsigned char)c];
  uint32_t c32 = validate_color_32(color);
  int stride = win->surface_w;

  for (int cy = 0; cy < 8; cy++) {
    uint8_t row = glyph[cy];
    int py = y + cy;
    if (py < 0 || py >= win->surface_h)
      continue;

    for (int cx = 0; cx < 8; cx++) {
      if (row & (1 << (7 - cx))) {
        int px = x + cx;
        if (px >= 0 && px < win->surface_w) {
          win->surface[py * stride + px] = c32;
        }
      }
    }
  }
}

// 16bpp Text Drawing - Render to Surface
void winmgr_draw_text(window_t *win, int x, int y, const char *text,
                      uint32_t color) {
  if (!win || !text)
    return;

  int start_x = x;
  int safety = 0;
  while (*text && safety < 1024) {
    draw_char_surface(win, x, y, *text++, color);
    x += 8;
    safety++;
  }

  winmgr_invalidate_rect(win, start_x, y, (x - start_x), 8);
}

void winmgr_draw_button(window_t *win, int x, int y, int w, int h,
                        const char *label) {
  // Button Body
  winmgr_fill_rect(win, x, y, w, h, 0xFFCE79CE); // Grayish
  // Button Border (High/Low for 3D effect)
  winmgr_fill_rect(win, x, y, w, 1, 0xFFFFFFFF);         // Top
  winmgr_fill_rect(win, x, y, 1, h, 0xFFFFFFFF);         // Left
  winmgr_fill_rect(win, x, y + h - 1, w, 1, 0xFF424242); // Bottom
  winmgr_fill_rect(win, x + w - 1, y, 1, h, 0xFF424242); // Right

  // Label
  int len = str_len(label);
  int txt_x = x + (w - (len * 8)) / 2; // 8 pixels per char
  int txt_y = y + (h - 8) / 2;
  winmgr_draw_text(win, txt_x, txt_y, label, 0x0000); // Black
}

void winmgr_render_window(window_t *win) {
  if (!win || !win->surface || win->surface_w <= 0 || win->surface_h <= 0) {
    print_serial("WINMGR: render_window SKIPPED (no surface or bad dims)\n");
    return;
  }

  // DEBUG: confirm render entry and bg_color
  print_serial("WINMGR: render_window id=");
  char _dbg_id[16];
  k_itoa(win->id, _dbg_id);
  print_serial(_dbg_id);
  print_serial(" bg=0x");
  char _dbg_bg[16];
  k_itoa((int)win->bg_color, _dbg_bg);
  print_serial(_dbg_bg);
  print_serial(" draw_cb=");
  print_serial(win->draw ? "SET" : "NULL");
  print_serial("\n");

  // Partial Redraw Logic
  if (win->sub_dirty_active && !win->needs_redraw) {
    // Only clear sub-dirty if transparent
    if (win->opacity < 255) {
      winmgr_fill_rect(win, win->sub_dirty.x, win->sub_dirty.y,
                       win->sub_dirty.w, win->sub_dirty.h, 0);
    }
    // Note: The app's draw() callback should technically only draw inside
    // sub_dirty. We don't force clip yet but we might in the future.
    if (win->draw) {
      print_serial("WINMGR: calling draw cb (partial)\n");
      win->draw(win);
      print_serial("WINMGR: draw cb returned (partial)\n");
    } else {
      print_serial("WINMGR: draw cb NULL (partial)\n");
    }

    win->sub_dirty_active = 0;
    return;
  }

  // Full Redraw Logic
  if (win->flags & WINDOW_FLAG_NO_TITLEBAR) {
    // Frameless: Fill base and call app draw immediately
    winmgr_fill_rect(win, 0, 0, win->width, win->height, win->bg_color);
    if (win->draw)
      win->draw(win);
    return;
  }

  // === Opaque Window Frame (Perfect Rounded 8px) ===
  const theme_t *theme = theme_get();
  uint32_t glass_edge = theme->border;
  int radius = 8;

  // 1. Fill base frame and title bar area (Full width to fix misalignment)
  winmgr_fill_rect(win, 0, 0, win->width, win->height, theme->bg);

  extern window_t *active_window;
  // Title bar colors (Forced Dark for Terminal)
  uint32_t title_col;
  if (win->app_type == 0) {
    title_col = (win == active_window) ? 0xFF181825 : 0xFF313244;
  } else {
    title_col = (win == active_window) ? theme->titlebar : theme->titlebar_inactive;
  }
  winmgr_fill_rect(win, 0, 0, win->width, 24, title_col);

  // 2. Draw border lines (Solid, only for straight sections)
  // Shorten them by 2px more to ensure they don't even touch the curves
  winmgr_draw_line(win, radius + 2, 0, win->width - 1 - radius - 2, 0,
                   glass_edge); // Top
  winmgr_draw_line(win, radius + 2, win->height - 1,
                   win->width - 1 - radius - 2, win->height - 1,
                   glass_edge); // Bottom
  winmgr_draw_line(win, 0, radius + 2, 0, win->height - 1 - radius - 2,
                   glass_edge); // Left
  winmgr_draw_line(win, win->width - 1, radius + 2, win->width - 1,
                   win->height - 1 - radius - 2, glass_edge); // Right

  // REMOVED winmgr_draw_arc_solid calls - they are redundant and cause
  // artifacts!

  // === Content Area (Fully Opaque) ===
  // CRITICAL: Always use fully-opaque colors (0xFF prefix) to prevent
  // transparent content areas that appear as black rectangles.
  if (win->app_type == 0) {
    // Terminal ALWAYS black regardless of theme
    winmgr_fill_rect(win, 2, 24, win->width - 4, win->height - 26, 0xFF000000);
  } else if (win->app_type == 2) {
    // Editor uses theme input_bg (staying dark for now or following theme)
    winmgr_fill_rect(win, 2, 24, win->width - 4, win->height - 26, theme->input_bg);
  } else {
    // Solid base from theme
    winmgr_fill_rect(win, 2, 24, win->width - 4, win->height - 26, theme->bg);
  }

  win->blur_strength = 0; // Disable blur globally

  // === Window Buttons (Shifted to avoid curve) ===
  int btn_x = win->width - 24; // 24px from right to avoid the 8px curve

  // Minimize button (yellow with border)
  uint32_t min_col = (win->hover_btn == 1) ? 0xFFFFFF00 : 0xFFC0C000;
  winmgr_draw_rect(win, btn_x - 36, 5, 14, 14, min_col);
  winmgr_draw_rect(win, btn_x - 36, 5, 14, 1, 0xFFFFFF80);
  winmgr_draw_rect(win, btn_x - 36, 5, 1, 14, 0xFFFFFF80);
  winmgr_draw_rect(win, btn_x - 36, 18, 14, 1, 0xFF808000);
  winmgr_draw_rect(win, btn_x - 23, 5, 1, 14, 0xFF808000);

  // Maximize button (green with border)
  uint32_t max_col = (win->hover_btn == 2) ? 0xFF00FFFF : 0xFF00FF00;
  winmgr_draw_rect(win, btn_x - 18, 5, 14, 14, max_col);
  winmgr_draw_rect(win, btn_x - 18, 5, 14, 1, 0xFF80FF80);
  winmgr_draw_rect(win, btn_x - 18, 5, 1, 14, 0xFF80FF80);
  winmgr_draw_rect(win, btn_x - 18, 18, 14, 1, 0xFF008000);
  winmgr_draw_rect(win, btn_x - 5, 5, 1, 14, 0xFF008000);

  // Close button (red with border)
  uint32_t close_col = (win->hover_btn == 3) ? 0xFFFF6666 : 0xFFFF0000;
  winmgr_draw_rect(win, btn_x, 5, 14, 14, close_col);
  winmgr_draw_rect(win, btn_x, 5, 14, 1, 0xFFFF8080);      // Light top
  winmgr_draw_rect(win, btn_x, 5, 1, 14, 0xFFFF8080);      // Light left
  winmgr_draw_rect(win, btn_x, 18, 14, 1, 0xFF800000);     // Dark bottom
  winmgr_draw_rect(win, btn_x + 13, 5, 1, 14, 0xFF800000); // Dark right

  // Button labels
  // Use better glyphs if possible, or just center them better
  winmgr_draw_text(win, btn_x - 32, 8, "_", 0xFFFFFFFF);
  winmgr_draw_text(win, btn_x - 14, 8, "#", 0xFFFFFFFF); // # as square
  winmgr_draw_text(win, btn_x + 3, 8, "x", 0xFFFFFFFF);

  // === Custom Draw ===
  if (win->draw) {
    print_serial("WINMGR: calling draw cb (full)\n");
    win->draw(win);
    print_serial("WINMGR: draw cb returned (full)\n");
  } else {
    print_serial("WINMGR: draw cb NULL (full) - window will be blank!\n");
  }

  // === Perfectionist SDF Corner Masking (3x3 Supersampled) ===
  for (int y = 0; y < radius; y++) {
    for (int x = 0; x < radius; x++) {
      uint8_t mask = get_corner_alpha_int(x, y, radius);

      // Coordinates for 4 corners
      int tx = x, ty = y;
      int trx = win->width - 1 - x, try_ = y;
      int blx = x, bly = win->height - 1 - y;
      int brx = win->width - 1 - x, bry = win->height - 1 - y;

      // Apply mask safely
      if (tx < win->surface_w && ty < win->surface_h)
        win->surface[ty * win->surface_w + tx] =
            (mask << 24) |
            (win->surface[ty * win->surface_w + tx] & 0x00FFFFFF);

      if (trx >= 0 && trx < win->surface_w && try_ >= 0 &&
          try_ < win->surface_h)
        win->surface[try_ * win->surface_w + trx] =
            (mask << 24) |
            (win->surface[try_ * win->surface_w + trx] & 0x00FFFFFF);

      if (blx < win->surface_w && bly >= 0 && bly < win->surface_h)
        win->surface[bly * win->surface_w + blx] =
            (mask << 24) |
            (win->surface[bly * win->surface_w + blx] & 0x00FFFFFF);

      if (brx >= 0 && brx < win->surface_w && bry >= 0 && bry < win->surface_h)
        win->surface[bry * win->surface_w + brx] =
            (mask << 24) |
            (win->surface[bry * win->surface_w + brx] & 0x00FFFFFF);
    }
  }

  // === Title Text (with shadow) ===
  winmgr_draw_text(win, 7, 7, win->title, 0x80101010); // Subtle dark shadow
  uint32_t title_text_col = (win->app_type == 0) ? 0xFFFFFFFF : theme->titlebar_text;
  winmgr_draw_text(win, 6, 6, win->title, title_text_col); // Theme or Fixed text color

  // Text Buffer Draw
  if (win->text_buffer[0] != 0 || win->cursor_pos >= 0) {
    int cx = 6;
    int cy = 30;
    char *ptr = win->text_buffer;
    while (*ptr) {
      char c = *ptr++;
      char s[2] = {c, 0};
      if (c == '\n') {
        cx = 6;
        cy += 10;
      } else {
        uint32_t txt_col =
            (win->bg_color == 0xFF000000) ? 0xFF00FF00 : 0xFF000000;
        winmgr_draw_text(win, cx, cy, s, txt_col);
        cx += 10;
        if (cx > win->width - 16) {
          cx = 6;
          cy += 10;
        }
      }
    }
  }

  // Draw resize grip (bottom-right corner triangle)
  if (!win->is_maximized) {
    int gx = win->width - 12;
    int gy = win->height - 12;
    for (int i = 0; i < 10; i++) {
      for (int j = 10 - i; j < 10; j++) {
        winmgr_put_pixel(win, gx + j, gy + i, 0xFF808080);
      }
    }
  }
}

// Pre-calculate 64-slice mesh for Genie effect warping
static float genie_bezier(float t, float p1, float p2) {
  float u = 1.0f - t;
  return 3.0f * u * u * t * p1 + 3.0f * u * t * t * p2 + t * t * t;
}

void winmgr_update_genie_mesh(window_t *win) {
  if (!win->is_animating)
    return;

  int cur_w = (int)win->anim_w.current_val;
  int cur_h = (int)win->anim_h.current_val;

  // progress 0=full window, 1=icon
  float progress = 0.0f;
  if (win->width > 0)
    progress = 1.0f - (float)(cur_w) / (float)win->width;
  if (progress < 0) progress = 0;
  if (progress > 1) progress = 1;

  float wp = progress * progress * (3.0f - 2.0f * progress);

  // Vertical scaling bounds for compression logic
  float R = (float)cur_h / (float)win->height;
  float optimal_p = R / 3.0f;
  if (optimal_p > 0.8f) optimal_p = 0.8f;
  float p1_sy, p2_sy;
  if (!win->pinch_top) {
      p1_sy = optimal_p; p2_sy = 0.05f;
  } else {
      p1_sy = 0.95f; p2_sy = 1.0f - optimal_p;
  }

  int tx = (win->launch_x >= 0) ? win->launch_x : win->taskbar_x;
  float rel_x = 0.5f;
  if (tx >= 0 && win->width > 0) {
    rel_x = (float)(tx - win->x) / (float)win->width;
    if (rel_x < 0.0f) rel_x = 0.0f;
    if (rel_x > 1.0f) rel_x = 1.0f;
  }

  float bend_limit = (float)win->width * 0.6f;

  for (int i = 0; i < 64; i++) {
    float ty_raw = (float)i / 63.0f;
    
    // Calculate Textured Y (curved)
    win->mesh_ty[i] = genie_bezier(ty_raw, p1_sy, p2_sy);

    float ty_funnel = win->pinch_top ? (1.0f - ty_raw) : ty_raw;

    // S-curve Sigmoid approximation (Double-smoothstep)
    float ss = ty_funnel * ty_funnel * (3.0f - 2.0f * ty_funnel);
    float ty_s = ss * ss * (3.0f - 2.0f * ss);

    float eval_t = ty_s + (wp * 2.0f) - 1.0f;
    if (eval_t < 0.0f) eval_t = 0.0f;
    if (eval_t > 1.0f) eval_t = 1.0f;

    float target_w_ratio = 32.0f / (float)win->width;
    if (target_w_ratio < 0.01f)
      target_w_ratio = 0.01f;

    // S-curve width scaling
    float st_w = eval_t * eval_t * (3.0f - 2.0f * eval_t);
    st_w = st_w * st_w * (3.0f - 2.0f * st_w);

    float row_scale = 1.0f + st_w * (target_w_ratio - 1.0f);
    int rw = (int)((float)win->width * row_scale);
    if (rw < 2) rw = 2;

    float base_rx = (float)win->x + (float)(win->width - rw) * rel_x;
    float bend_target = (tx - 16.0f) - ((float)win->x + (float)(win->width - 32) * rel_x);

    float st_b = eval_t * eval_t * (3.0f - 2.0f * eval_t);
    st_b = st_b * st_b * (3.0f - 2.0f * st_b);

    float bend_factor = st_b * bend_target;
    if (bend_factor > bend_limit) bend_factor = bend_limit;
    if (bend_factor < -bend_limit) bend_factor = -bend_limit;

    win->mesh_lx[i] = base_rx + bend_factor;
    win->mesh_rx[i] = win->mesh_lx[i] + (float)rw;
  }
}

void winmgr_tick_animations(float dt) {
  extern int ui_dirty;
  extern int disable_animations;
  int any_active = 0;

  if (disable_animations) {
    for (int i = 0; i < window_count; i++) {
      window_t *win = &windows[i];
      if (win->id == 0)
        continue;

      if (win->is_animating) {
        win->is_animating = 0;
        if (win->anim_mode == 2) {
          win->is_minimized = 1;
        } else {
          win->is_minimized = 0;
          win->x = (int)win->anim_x.end_val;
          win->y = (int)win->anim_y.end_val;
          win->width = (int)win->anim_w.end_val;
          win->height = (int)win->anim_h.end_val;
          winmgr_resize_surface(win, win->width, win->height);
          win->needs_redraw = 1;
        }
        win->anim_x.active = win->anim_y.active = win->anim_w.active =
            win->anim_h.active = 0;
        win->anim_x.current_val = win->anim_x.end_val;
        win->anim_y.current_val = win->anim_y.end_val;
        win->anim_w.current_val = win->anim_w.end_val;
        win->anim_h.current_val = win->anim_h.end_val;
      }

      if (win->fading_mode != 0) {
        if (win->fading_mode == 2) {
          // Immediately destroy
          if (win->on_close) {
            win->on_close(win);
          }
          // Safety-net: free user_data if app forgot to
          if (win->user_data) {
            kfree(win->user_data);
            win->user_data = 0;
          }
          if (win->surface) {
            kfree(win->surface);
            win->surface = 0;
            win->surface_w = 0;
            win->surface_h = 0;
          }
          win->id = 0;
          win->fading_mode = 0;
          win->is_animating = 0;
          win->anim_mode = 0;
          win->draw = 0;
          win->on_mouse = 0;
          win->on_key = 0;
          win->on_scroll = 0;
          win->on_copy = 0;
          win->on_cut = 0;
          win->on_paste = 0;
          win->on_close = 0;
          win->exists = 0;
          // Shrink window_count if trailing slots are empty
          while (window_count > 0 && windows[window_count - 1].id == 0) {
            window_count--;
          }
          if (active_window == win) {
            active_window = NULL;
            for (int k = window_count - 1; k >= 0; k--) {
              if (windows[k].id != 0 && windows[k].fading_mode != 2 &&
                  !windows[k].is_minimized) {
                active_window = &windows[k];
                break;
              }
            }
          }
        } else {
          win->opacity = 255;
          win->fading_mode = 0;
        }
        win->anim_opacity.active = 0;
        win->anim_opacity.current_val = win->anim_opacity.end_val;
      }

      if (win->anim_scale.active) {
        win->scale = win->anim_scale.end_val;
        win->anim_scale.active = 0;
        win->anim_scale.current_val = win->anim_scale.end_val;
      }

      if (win->vel_x != 0 || win->vel_y != 0) {
        // Snap momentum to stop
        win->vel_x = 0;
        win->vel_y = 0;
      }
    }
    ui_dirty = 1;
    return;
  }

  for (int i = 0; i < window_count; i++) {
    int win_idx = window_z_order[i];
    if (win_idx < 0) continue;
    window_t *win = &windows[win_idx];
    if (!win->is_animating && win->fading_mode == 0 && win->vel_x == 0 &&
        win->vel_y == 0 && !win->anim_scale.active)
      continue;

    any_active = 1;

    // --- Opacity Animation ---
    if (win->fading_mode != 0) {
      anim_tick(&win->anim_opacity, dt);
      win->opacity = (uint8_t)win->anim_opacity.current_val;
      compositor_invalidate_window(win);

      if (!win->anim_opacity.active) {
        if (win->fading_mode == 2) {
          // Fade Out Complete -> Destroy Window
          if (win->on_close) {
            win->on_close(win);
          }

          // Safety-net: free user_data if app forgot to
          if (win->user_data) {
            kfree(win->user_data);
            win->user_data = 0;
          }

          // FREE the surface memory (this was the memory leak!)
          if (win->surface) {
            kfree(win->surface);
            win->surface = 0;
            win->surface_w = 0;
            win->surface_h = 0;
          }

          // Remove from Z-order
          int z_idx_to_remove = -1;
          for (int k = 0; k < window_count; k++) {
            if (window_z_order[k] == win_idx) {
              z_idx_to_remove = k;
              break;
            }
          }
          if (z_idx_to_remove >= 0) {
            for (int k = z_idx_to_remove; k < window_count - 1; k++) {
              window_z_order[k] = window_z_order[k + 1];
            }
            window_z_order[window_count - 1] = -1;
            window_count--;
            i--; // Adjust loop index
          }

          win->id = 0;
          win->fading_mode = 0;
          win->is_animating = 0;
          win->anim_mode = 0;
          win->draw = 0;
          win->on_mouse = 0;
          win->on_key = 0;
          win->on_scroll = 0;
          win->on_copy = 0;
          win->on_cut = 0;
          win->on_paste = 0;
          win->on_close = 0;
          win->exists = 0;

          // If this was the active window, we need to find a new one
          if (active_window == win) {
            active_window = NULL;
            for (int k = window_count - 1; k >= 0; k--) {
              window_t *cand = &windows[window_z_order[k]];
              if (cand->id != 0 && cand->fading_mode != 2 && !cand->is_minimized) {
                active_window = cand;
                break;
              }
            }
          }

          // Invalidate full screen for cleanup
          extern int screen_width, screen_height;
          compositor_invalidate_rect(0, 0, screen_width, screen_height);

          ui_dirty = 1;
          continue; // Stop processing this window
        } else {
          // Fade In Complete
          win->fading_mode = 0;
        }
      }
    }

    // --- Scale Animation ---
    if (win->anim_scale.active) {
      anim_tick(&win->anim_scale, dt);
      win->scale = win->anim_scale.current_val;
      compositor_invalidate_window(win);
      ui_dirty = 1;
    }

    // --- Momentum Physics ---
    if (!win->is_dragging && (win->vel_x != 0 || win->vel_y != 0)) {
      // Invalidate Old
      compositor_invalidate_window(win);

      // Integrate
      win->x += (int)win->vel_x;
      win->y += (int)win->vel_y;

      // Friction (0.95 for smooth, long slides)
      win->vel_x *= 0.95f;
      win->vel_y *= 0.95f;

      // Threshold (stop when slow enough)
      if (win->vel_x > -0.3f && win->vel_x < 0.3f)
        win->vel_x = 0;
      if (win->vel_y > -0.3f && win->vel_y < 0.3f)
        win->vel_y = 0;

      // Wall Bounce
      extern int screen_width, screen_height;
      int max_x = screen_width - win->width;
      int max_y = screen_height - win->height;

      if (win->x < 0) {
        win->x = 0;
        win->vel_x *= -0.5f;
      }
      if (win->y < 0) {
        win->y = 0;
        win->vel_y *= -0.5f;
      }
      if (win->x > max_x) {
        win->x = max_x;
        win->vel_x *= -0.5f;
      }
      if (win->y > max_y) {
        win->y = max_y;
        win->vel_y *= -0.5f;
      }

      // Invalidate New
      compositor_invalidate_window(win);
      ui_dirty = 1;

      // Stop if settled
      if (win->vel_x == 0 && win->vel_y == 0) {
        // Momentum settled - no need to keep animating for this
      }
    }

    // --- Spatial Animation (Move/Resize) ---
    // Tick spatial properties if the window is marked as animating.
    // The completion block below will clear win->is_animating when all properties finish.
    if (win->is_animating) {
      if (win->anim_mode == 1 || win->anim_mode == 2) {
        winmgr_update_genie_mesh(win);
      }

      // Invalidate OLD ghost area
      int ox = (int)win->anim_x.current_val;
      int oy = (int)win->anim_y.current_val;
      int ow = (int)win->anim_w.current_val;
      int oh = (int)win->anim_h.current_val;
      compositor_invalidate_rect(ox - 8, oy - 8, ow + 16, oh + 16);

      // Tick individual property animations
      anim_tick(&win->anim_x, dt);
      anim_tick(&win->anim_y, dt);
      anim_tick(&win->anim_w, dt);
      anim_tick(&win->anim_h, dt);

      // Calculate the ACTUAL rendered bounds (matching compositor_warp_blend's p^5 logic)
      float progress = 0.0f;
      if (win->width > 0)
        progress = 1.0f - (float)(ow) / (float)win->width;
      if (progress < 0) progress = 0;
      if (progress > 1) progress = 1;

      float p2 = progress * progress;
      float p5 = p2 * p2 * progress;
      float inv_p = 1.0f - progress;
      float inv_p2 = inv_p * inv_p;
      float inv_p5 = inv_p2 * inv_p2 * inv_p;

      float prog_top, prog_bot;
      if (win->pinch_top) {
          prog_top = 1.0f - inv_p5;
          prog_bot = p5;
      } else {
          prog_top = p5;
          prog_bot = 1.0f - inv_p5;
      }

      int ty_target = (win->launch_y >= 0) ? win->launch_y : win->taskbar_y;
      if (ty_target < 0) ty_target = screen_height;
      int tx_target = (win->launch_x >= 0) ? win->launch_x : win->taskbar_x;

      int start_y = win->y;
      int start_bottom = win->y + win->height;
      int target_y = ty_target - 16;
      int target_bottom = ty_target + 16;

      // Actual rendered Y bounds
      int cur_y = start_y + (int)((target_y - start_y) * prog_top);
      int cur_bottom = start_bottom + (int)((target_bottom - start_bottom) * prog_bot);
      int cur_h = cur_bottom - cur_y;

      // Horizontal bounds: Mesh logic (lx/rx)
      // At progress=0, bounds are [win->x, win->x + win->width]
      // At progress=1, bounds are [tx_target-16, tx_target+16]
      int cur_x_left = (int)win->mesh_lx[0];
      int cur_x_right = (int)win->mesh_rx[0];
      for(int m=1; m<64; m++) {
          if (win->mesh_lx[m] < cur_x_left) cur_x_left = (int)win->mesh_lx[m];
          if (win->mesh_rx[m] > cur_x_right) cur_x_right = (int)win->mesh_rx[m];
      }

      compositor_invalidate_rect(cur_x_left - 8, cur_y - 8, (cur_x_right - cur_x_left) + 16, cur_h + 16);
      
      // Also invalidate the icon area to ensure the neck connects smoothly
      compositor_invalidate_rect(tx_target - 24, ty_target - 24, 48, 48);

      // Watchdog: If spatial animations take too long (e.g. 120 frames), force complete.
      // This prevents the "Big Black Box" warp mode from getting stuck forever.
      win->watchdog++;
      int force_complete = (win->watchdog > 120);

      if ((!win->anim_x.active && !win->anim_y.active && !win->anim_w.active &&
           !win->anim_h.active) || force_complete) {
        
        if (force_complete) {
          print_serial("WINMGR: Animation watchdog triggered! Forcing completion.\n");
        }

        win->is_animating = 0;
        win->watchdog = 0;

        // Explicit mode check
        if (win->anim_mode == 2 && win->fading_mode != 2) {
          // Minimize (but not when closing/fading out)
          win->is_minimized = 1;
        } else {
          // Open/Restore
          win->is_minimized = 0;
          // Snap to final
          win->x = (int)win->anim_x.end_val;
          win->y = (int)win->anim_y.end_val;
          win->width = (int)win->anim_w.end_val;
          win->height = (int)win->anim_h.end_val;

          // CRITICAL: Resize surface to match final animated dimensions
          // This prevents "reverting" or content clipping after snap/maximize
          if (!winmgr_resize_surface(win, (int)win->anim_w.end_val, (int)win->anim_h.end_val)) {
            print_serial("WINMGR: final anim resize FAILED, reverting to surface size\n");
            // Revert logical dimensions to match the existing surface so it renders correctly
            win->width = win->surface_w;
            win->height = win->surface_h;
          } else {
            win->width = (int)win->anim_w.end_val;
            win->height = (int)win->anim_h.end_val;
          }
          win->needs_redraw = 1;

          // FORCE COMPLETE REDRAW on animation end to prevent ghosting/desync
          extern int screen_width, screen_height;
          compositor_invalidate_rect(0, 0, screen_width, screen_height);

          extern int ui_dirty;
          ui_dirty = 1;
        }
      } else {
        // Ongoing...
        // Invalidate new ghost area handled above
      }

      // Invalidate standard window area too (just in case)
      compositor_invalidate_window(win);
    }
    ui_dirty = 1;
  }

  // Tick Start Menu animation
  extern int startmenu_is_animating();
  if (startmenu_is_animating()) {
    extern void startmenu_tick_animation(float dt);
    startmenu_tick_animation(dt);
    ui_dirty = 1;
  }

  // Tick System Menu animation
  extern int sysmenu_is_animating(void);
  if (sysmenu_is_animating()) {
    extern void sysmenu_tick_animation(float dt);
    sysmenu_tick_animation(dt);
    ui_dirty = 1;
  }
}

// Draw animation proxy (solid window look) using current anim values
// Draw animation proxy (High Fidelity) using current anim values
void draw_anim_ghost(window_t *win, rect_t clip) {
  int ax = (int)win->anim_x.current_val;
  int ay = (int)win->anim_y.current_val;
  int aw = (int)win->anim_w.current_val;
  int ah = (int)win->anim_h.current_val;

  if (aw < 2)
    aw = 2;
  if (ah < 2)
    ah = 2;

  // Intersect ghost area with clip
  rect_t ghost_r = {ax, ay, aw, ah};
  rect_t overlap;
  extern int rect_intersect(rect_t a, rect_t b, rect_t * out);
  if (!rect_intersect(clip, ghost_r, &overlap))
    return;

  if (win->surface) {
    // print_serial("WINMGR: Ghost drawing scaled surface\n");
    vga_draw_scaled_bitmap(ax, ay, aw, ah, win->surface, win->surface_w,
                           win->surface_h, backbuffer, screen_width,
                           screen_height);
  } else {
    // Fallback if no surface (shouldn't happen with Phase 3 engine)
    uint32_t body_col = validate_color_32(win->bg_color);
    int total_title_h = (ah > 24) ? 22 : ah / 2;

    for (int j = overlap.y; j < overlap.y + overlap.h; j++) {
      uint32_t *dst = &backbuffer[j * screen_width + overlap.x];
      int relative_y = j - ay;
      uint32_t row_col = (relative_y < total_title_h)
                             ? validate_color_32(win->style.title_color)
                             : body_col;

      for (int i = 0; i < overlap.w; i++) {
        dst[i] = row_col;
      }
    }
  }
}

// Helper to start standard genie animation
void start_window_anim(window_t *win, int sx, int sy, int sw, int sh, int ex,
                       int ey, int ew, int eh, float duration, int type) {

  win->is_animating = 1;

  // Setup X
  anim_start(&win->anim_x, (float)sx, (float)ex, duration, type);

  // Setup Y
  anim_start(&win->anim_y, (float)sy, (float)ey, duration, type);

  // Setup W
  anim_start(&win->anim_w, (float)sw, (float)ew, duration, type);

  // Setup H
  anim_start(&win->anim_h, (float)sh, (float)eh, duration, type);

  // Setup Scale
  if (sx == ex && sy == ey && sw == ew && sh == eh) {
    // Scale-only or no-op spatial anim
  } else {
    // If we are moving/resizing, usually we want scale to be 1.0
    // but if we are "popping in", we might want to drive scale too.
    // For now, let's just make sure scale is reset if not explicitly handled.
  }
}

// Helper to start spring genie animation
void start_window_spring(window_t *win, int sx, int sy, int sw, int sh, int ex,
                         int ey, int ew, int eh, float stiffness,
                         float damping) {

  win->is_animating = 1;

  // Setup X
  anim_start_spring(&win->anim_x, (float)sx, (float)ex, stiffness, damping);

  // Setup Y
  anim_start_spring(&win->anim_y, (float)sy, (float)ey, stiffness, damping);

  // Setup W
  anim_start_spring(&win->anim_w, (float)sw, (float)ew, stiffness, damping);

  // Setup H
  anim_start_spring(&win->anim_h, (float)sh, (float)eh, stiffness, damping);
}

// Stray brace removed

// Check for pending surface updates and flush them
void winmgr_flush_updates() {
  for (int i = 0; i < window_count; i++) {
    window_t *win = &windows[window_z_order[i]];
    if (win->id != 0 && win->needs_redraw) {
      win->needs_redraw = 0;
      winmgr_render_window(win);
      compositor_invalidate_window(win);
    }
  }
}
// Update Snap Preview Overlay via Compositor
void winmgr_draw_drag_overlay() {
  int active = 0;
  int ox = 0, oy = 0, ow = 0, oh = 0;

  extern int mouse_x, mouse_y;

  for (int i = 0; i < window_count; i++) {
    window_t *win = &windows[window_z_order[i]];
    if (win->id != 0 && win->is_dragging) {
      // Check snap regions
      if (mouse_x <= 40) { // Left Snap
        active = 1;
        ox = 0;
        oy = 0;
        ow = screen_width / 2;
        oh = screen_height;                      // -30 for taskbar (approx)
      } else if (mouse_x >= screen_width - 50) { // Right Snap
        active = 1;
        ox = screen_width / 2;
        oy = 0;
        ow = screen_width / 2;
        oh = screen_height;
      } else if (mouse_y <= 30) { // Top Snap (Maximize)
        active = 1;
        ox = 0;
        oy = 0;
        ow = screen_width;
        oh = screen_height;
      }
      break;
    }
  }

  // Update compositor state
  compositor_set_overlay(ox, oy, ow, oh, active);
}

// Deprecated - Text is now rendered into surface
void winmgr_render_text_all() {
  // No-op
}

window_t *active_window = 0;
extern int ui_dirty;

// Key/Mouse Handlers (Unchanged logic, just ensure 16bpp doesn't break logic)
// Logic uses coordinates, untouched by bpp.
void window_handle_key(int key, char c) {
  if (!active_window) {
    // Only print if not in lock screen phase (heuristic)
    // Actually, let's just ignore it for now to avoid spam
    // print_serial("WINMGR: No active window for key\n");
    return;
  }

  // Debug
  // char buf[32];
  // print_serial("WINMGR: Key -> Window ");
  // print_serial(active_window->title);
  // print_serial("\n");

  task_t *curr = get_current_task();
  if (active_window->owner_pid > 1 &&
      (!curr || curr->id != active_window->owner_pid)) {
    msg_t msg;
    msg.type = MSG_KEYBOARD;
    msg.d1 = key;
    msg.d2 = c;
    msg_send(active_window->owner_pid, &msg);
    return;
  }

  if (active_window->on_key) {
    active_window->on_key(active_window, key, c);
    return;
  }
  // Default buffer
  if (c == '\b') {
    if (active_window->cursor_pos > 0) {
      active_window->cursor_pos--;
      active_window->text_buffer[active_window->cursor_pos] = 0;
      ui_dirty = 3; // 3 = Active Window Only
    }
  } else if (c >= 32 || c == '\n') {
    if (active_window->cursor_pos < 255) {
      active_window->text_buffer[active_window->cursor_pos] = c;
      active_window->cursor_pos++;
      active_window->text_buffer[active_window->cursor_pos] = 0;
      ui_dirty = 3; // 3 = Active Window Only
    }
  }
}

#define RESIZE_BORDER 6
#define MIN_WIN_W 120
#define MIN_WIN_H 80

static int get_resize_edge(window_t *win, int mx, int my) {
  if (win->is_maximized)
    return 0;

  int edge = 0;
  if (mx < win->x + RESIZE_BORDER && mx >= win->x - 2)
    edge |= 1;
  if (mx >= win->x + win->width - RESIZE_BORDER && mx < win->x + win->width + 2)
    edge |= 2;
  if (my < win->y + RESIZE_BORDER && my >= win->y - 2)
    edge |= 4;
  if (my >= win->y + win->height - RESIZE_BORDER &&
      my < win->y + win->height + 2)
    edge |= 8;

  return edge;
}

int window_handle_mouse(window_t *win, int mx, int my, int buttons) {
  // 0. Skip inactive or minimized windows
  if (win->id == 0 || win->is_minimized)
    return 0;

  // If dragging or resizing, we capture mouse even if outside
  int hit = (mx >= win->x && mx < win->x + win->width && my >= win->y &&
             my < win->y + win->height);

  if (win->is_dragging || win->is_resizing)
    hit = 1; // Force hit if dragging/resizing

  if (hit) {
    // Focus
    extern window_t *active_window;
    if (active_window != win && !win->is_dragging) {
      active_window = win;
      winmgr_bring_to_front(win);
      // bring_to_front moves the window struct in the array,
      // so update win to point to the new location
      win = active_window;
    }

    // Hover detection
    int old_hover = win->hover_btn;
    win->hover_btn = 0;
    if (my < win->y + 24) {
      int rx = mx - win->x;
      int btn_x_close = win->width - 20;
      if (rx >= btn_x_close && rx < btn_x_close + 14)
        win->hover_btn = 3;
      else if (rx >= btn_x_close - 18 && rx < btn_x_close - 4)
        win->hover_btn = 2;
      else if (rx >= btn_x_close - 36 && rx < btn_x_close - 22)
        win->hover_btn = 1;
    }
    if (win->hover_btn != old_hover) {
      win->needs_redraw = 1; // Direct render might be safer but let's try this
      ui_dirty = 1;
    }

    // Right Click: Context Menu
    if (buttons & 2) {
      if (win->flags & WINDOW_FLAG_NO_CLOSE)
        return 1; // Protected window
      static ctxmenu_item_t items[] = {
          {"Minimize", 0}, {"Maximize", 0}, {"Close", 0}};
      ctxmenu_show(mx, my, items, 3);
      return 1;
    }

    if (buttons & 1) {
      // 1. Check for Click on Min/Max/Close Buttons FIRST (Priority)
      if (!win->is_dragging && my < win->y + 24) {
        int rx = mx - win->x;
        int btn_x_close = win->width - 20;

        // Close
        if (rx >= btn_x_close && rx < btn_x_close + 14) {
          if (!(win->flags & WINDOW_FLAG_NO_CLOSE)) {
            winmgr_close_window(win);
            ui_dirty = 1;
          }
          return 1;
        }
        // Maximize
        else if (rx >= btn_x_close - 18 && rx < btn_x_close - 4) {
          compositor_invalidate_window(win); // Invalidate old
          if (!win->is_maximized) {
            win->saved_x = win->x;
            win->saved_y = win->y;
            win->saved_w = win->width;
            win->saved_h = win->height;

            int target_w = screen_width;
            int target_h = screen_height - 30;
            if (winmgr_resize_surface(win, target_w, target_h)) {
              win->width = target_w;
              win->height = target_h;
              win->x = 0;
              win->y = 0;
              win->is_maximized = 1;
            } else {
              print_serial("WINMGR: maximize resize FAILED\n");
            }
          } else {
            if (winmgr_resize_surface(win, win->saved_w, win->saved_h)) {
              win->x = win->saved_x;
              win->y = win->saved_y;
              win->width = win->saved_w;
              win->height = win->saved_h;
              win->is_maximized = 0;
            } else {
              print_serial("WINMGR: restore resize FAILED\n");
            }
          }
          compositor_invalidate_window(win); // Invalidate new
          ui_dirty = 1;
          return 1;
        }
        // Minimize (Animated)
        else if (rx >= btn_x_close - 36 && rx < btn_x_close - 22) {
          compositor_invalidate_window(win); // Invalidate current

          // Don't set is_minimized = 1 yet! Wait for anim.

          // Configure Genie minimize animation
          win->anim_mode = 2; // Minimize

          // Target: Taskbar icon position (win->taskbar_x)
          int tx = (win->taskbar_x > 0) ? win->taskbar_x : 20;
          int ty = screen_height - 28;

          // Set pinch direction based on dock position
          win->pinch_top = (ty < win->y);

          // macOS Genie minimize: fast pull, soft decelerating spring
          float min_k = 280.0f;
          float min_d = 30.0f;
          win->is_animating = 1;
          anim_start_spring(&win->anim_x, (float)win->x, (float)tx, min_k, min_d);
          anim_start_spring(&win->anim_y, (float)win->y, (float)ty, min_k, min_d);
          anim_start_spring(&win->anim_w, (float)win->width, 32.0f, min_k, min_d);
          anim_start_spring(&win->anim_h, (float)win->height, 32.0f, min_k, min_d);

          // Scale animation for minimize
          anim_start_spring(&win->anim_scale, win->scale, 0.01f, min_k, min_d);

          ui_dirty = 1;
          return 1;
        }
      }

      // 2. Check for resize (edge grab) - but not on title bar
      if (!win->is_dragging && !win->is_resizing && my >= win->y + 24 &&
          !(win->flags & (WINDOW_FLAG_FIXED_SIZE | WINDOW_FLAG_WIDGET))) {
        int edge = get_resize_edge(win, mx, my);
        if (edge) {
          win->is_resizing = 1;
          win->resize_edge = edge;
          ui_dirty = 1;
          return 1;
        }
      }

      // 3. Handle active resize
      if (win->is_resizing) {
        int edge = win->resize_edge;

        if (edge & 2) { // right
          int new_w = mx - win->x;
          if (new_w >= MIN_WIN_W)
            win->width = new_w;
        }
        if (edge & 8) { // bottom
          int new_h = my - win->y;
          if (new_h >= MIN_WIN_H)
            win->height = new_h;
        }
        if (edge & 1) { // left
          int delta = mx - win->x;
          if (win->width - delta >= MIN_WIN_W) {
            win->x += delta;
            win->width -= delta;
          }
        }
        if (edge & 4) { // top
          int delta = my - win->y;
          if (win->height - delta >= MIN_WIN_H) {
            win->y += delta;
            win->height -= delta;
          }
        }

        // Invalidate both old and new positions to prevent ghosting (Safe
        // margin 20px)
        compositor_invalidate_rect(win->prev_x - 20, win->prev_y - 20,
                                   win->width + 40, win->height + 40);
        compositor_invalidate_window(win);

        // Update prev for next frame
        win->prev_x = win->x;
        win->prev_y = win->y;

        // Resize surface to match new dimensions
        winmgr_resize_surface(win, win->width, win->height);

        win->needs_redraw = 1;
        ui_dirty = 1;
        return 1;
      }

      // 4. Handle Dragging
      if (win->is_dragging || (my < win->y + 24)) {
        if (!win->is_dragging) {
          win->is_dragging = 1;
          win->drag_offset_x = mx - win->x;
          win->drag_offset_y = my - win->y;
          // Invalidate start position
          compositor_invalidate_window(win);
        } else {
          // --- Un-maximize on drag ---
          if (win->is_maximized) {
            // Calculate relative position before shrinking
            float rel_x = (float)(mx - win->x) / (float)win->width;

            win->is_maximized = 0;
            win->is_snapped = 0;
            win->width = win->saved_w;
            win->height = win->saved_h;

            // Adjust drag offset so window stays under cursor correctly
            win->drag_offset_x = (int)(win->width * rel_x);
            win->drag_offset_y = 12; // Snap to title bar area

            win->x = mx - win->drag_offset_x;
            win->y = my - win->drag_offset_y;

            winmgr_resize_surface(win, win->width, win->height);
            compositor_invalidate_window(win);
          }

          // Invalidate OLD position (shadow margin included + safety)
          compositor_invalidate_rect(win->x - 20, win->y - 20, win->width + 40,
                                     win->height + 40);

          win->prev_x = win->x;
          win->prev_y = win->y;

          // Calculate "raw" position based on mouse
          int raw_x = mx - win->drag_offset_x;
          int raw_y = my - win->drag_offset_y;

          // --- Magnetic Snapping (20px) ---
          int snap_dist = 20;

          // Left
          if (abs(raw_x) < snap_dist)
            raw_x = 0;

          // Top
          if (abs(raw_y) < snap_dist)
            raw_y = 0;

          // Right
          if (abs(raw_x + win->width - screen_width) < snap_dist) {
            raw_x = screen_width - win->width;
          }

          // Bottom (Taskbar area)
          int safe_bottom = screen_height - 28;
          if (abs(raw_y + win->height - safe_bottom) < snap_dist) {
            raw_y = safe_bottom - win->height;
          }

          // Invalidate OLD position
          compositor_invalidate_window(win);

          // Apply
          win->x = raw_x;
          win->y = raw_y;

          // Hard Constraints (Keep visible)
          if (win->y < 0)
            win->y = 0;
          if (win->y > screen_height - 24)
            win->y = screen_height - 24;

          if (win->x < -win->width + 20)
            win->x = -win->width + 20;
          if (win->x > screen_width - 20)
            win->x = screen_width - 20;

          // Invalidate NEW position
          compositor_invalidate_window(win);

          // Physics: Calculate Velocity (Delta per frame)
          // We can use the actual moved distance
          win->vel_x = (float)win->x - (float)win->prev_x;
          win->vel_y = (float)win->y - (float)win->prev_y;

          ui_dirty = 1;
        }
        return 1; // Handled
      }
    } else {
      if (win->is_dragging) {
        // Check for edge snap
        if (mx <= 40) {
          // LEFT SNAP
          print_serial("WIN: Left snap triggered\n");
          if (!win->is_snapped) {
            win->saved_x = win->x;
            win->saved_y = win->y;
            win->saved_w = win->width;
            win->saved_h = win->height;
          }
          win->is_snapped = 1;
          win->is_maximized = 0; // Explicitly clear maximized state
          win->anim_mode = 3;    // Snap
          win->vel_x = 0;        // Clear velocity for clean animation
          win->vel_y = 0;
          start_window_spring(win, win->x, win->y, win->width, win->height, 0,
                              0, screen_width / 2, screen_height,
                              SPRING_STIFF_K, SPRING_STIFF_D);
          win->needs_redraw = 1;
          compositor_invalidate_window(win);
          ui_dirty = 1;
        } else if (mx >= screen_width - 50) {
          // RIGHT SNAP
          print_serial("WIN: Right snap triggered\n");
          if (!win->is_snapped) {
            win->saved_x = win->x;
            win->saved_y = win->y;
            win->saved_w = win->width;
            win->saved_h = win->height;
          }
          win->is_snapped = 2;
          win->is_maximized = 0; // Explicitly clear maximized state
          win->anim_mode = 3;    // Snap
          win->vel_x = 0;        // Clear velocity
          win->vel_y = 0;
          start_window_spring(win, win->x, win->y, win->width, win->height,
                              screen_width / 2, 0, screen_width / 2,
                              screen_height, SPRING_STIFF_K, SPRING_STIFF_D);
          win->needs_redraw = 1;
          compositor_invalidate_window(win);
          ui_dirty = 1;
        } else if (my <= 30) {
          // TOP SNAP (MAXIMIZE)
          print_serial("WIN: Top snap (maximize) triggered\n");
          if (!win->is_snapped) {
            win->saved_x = win->x;
            win->saved_y = win->y;
            win->saved_w = win->width;
            win->saved_h = win->height;
          }
          win->is_snapped = 3;
          win->anim_mode = 3;    // Snap
          win->is_maximized = 1; // Treat as maximized
          win->vel_x = 0;        // Clear velocity
          win->vel_y = 0;
          start_window_spring(win, win->x, win->y, win->width, win->height, 0,
                              0, screen_width, screen_height, SPRING_STIFF_K,
                              SPRING_STIFF_D);
          win->needs_redraw = 1;
          compositor_invalidate_window(win);
          ui_dirty = 1;
        } else if (win->is_snapped) {
          // UNSNAP (Restore Size)
          win->is_maximized = 0;
          win->is_snapped = 0;
          win->anim_mode = 3; // Snap
          win->vel_x = 0;     // Clear velocity
          win->vel_y = 0;
          start_window_spring(win, win->x, win->y, win->width, win->height,
                              win->x, win->y, win->saved_w, win->saved_h,
                              SPRING_STIFF_K, SPRING_STIFF_D);
          ui_dirty = 1;
        }
      }
      if (win->is_dragging) {
        // Momentum will be handled by winmgr_tick_animations
        // via velocity check - don't set is_animating (that triggers ghost
        // rendering)
      }
      win->is_dragging = 0;
      win->is_resizing = 0;
      win->resize_edge = 0;
    }

    // Pass to app
    task_t *curr = get_current_task();
    if (win->owner_pid > 1 && (!curr || curr->id != win->owner_pid)) {
      msg_t msg;
      msg.type = MSG_MOUSE_CLICK;
      msg.d1 = mx - win->x;
      msg.d2 = my - win->y;
      msg.d3 = buttons;
      msg_send(win->owner_pid, &msg);
    } else if (win->on_mouse) {
      win->on_mouse(win, mx - win->x, my - win->y, buttons);
    }

    return 1;
  }

  return 0;
}

// Global handler to iterate windows
int winmgr_handle_mouse_global(int mx, int my, int buttons) {
  // Mission Control: intercept clicks to select a window
  extern int search_active;
  if (search_active) {
    extern int desktop_handle_search_mouse(int mx, int my, int buttons);
    if (desktop_handle_search_mouse(mx, my, buttons))
      return 1;
  }

  extern int mission_control_active;
  if (mission_control_active) {
    static int mc_last_btn = 0;
    int mc_down = (buttons & 1) && !(mc_last_btn & 1);
    mc_last_btn = buttons;
    if (!mc_down)
      return 1; // Absorb all mouse events

    // Calculate grid layout (must match compositor rendering)
    int open_count = 0;
    int open_indices[32];
    for (int i = 0; i < window_count && open_count < 32; i++) {
        window_t *win = &windows[window_z_order[i]];
        if (win->id != 0)
            open_indices[open_count++] = window_z_order[i];
    }
    if (open_count == 0) {
      mission_control_active = 0;
      extern int ui_dirty;
      ui_dirty = 1;
      compositor_invalidate_rect(0, 0, screen_width, screen_height);
      return 1;
    }

    int cols = 2;
    if (open_count > 2)
      cols = 3;
    if (open_count > 6)
      cols = 4;
    int rows = (open_count + cols - 1) / cols;

    int pad = 24;
    int top_margin = 50;
    int thumb_w = (screen_width - pad * (cols + 1)) / cols;
    int thumb_h = (screen_height - top_margin - pad * (rows + 1) - 60) / rows;
    if (thumb_h > 300)
      thumb_h = 300;

    for (int i = 0; i < open_count; i++) {
      int col = i % cols;
      int row = i / cols;
      int tx = pad + col * (thumb_w + pad);
      int ty = top_margin + pad + row * (thumb_h + pad + 20);

      if (mx >= tx && mx < tx + thumb_w && my >= ty && my < ty + thumb_h + 20) {
        // Found the clicked window
        window_t *win = &windows[open_indices[i]];
        if (win->is_minimized) {
          win->is_minimized = 0;
        }
        winmgr_bring_to_front(win);
        mission_control_active = 0;
        extern int ui_dirty;
        ui_dirty = 1;
        compositor_invalidate_rect(0, 0, screen_width, screen_height);
        return 1;
      }
    }

    // Clicked outside any thumbnail - close Mission Control
    mission_control_active = 0;
    extern int ui_dirty;
    ui_dirty = 1;
    compositor_invalidate_rect(0, 0, screen_width, screen_height);
    return 1;
  }

  // -2. Check System Menu (highest priority if active)
  extern int sysmenu_handle_mouse(int mx, int my, int buttons);
  if (sysmenu_handle_mouse(mx, my, buttons))
    return 1;

  // -1. Check Start Menu
  if (startmenu_handle_mouse(mx, my, buttons))
    return 1;

  // 0. Check Taskbar
  extern int taskbar_handle_mouse(int mx, int my, int buttons);
  if (taskbar_handle_mouse(mx, my, buttons))
    return 1;

  // 1. Check for Captured Windows (Dragging or Resizing)
  for (int i = 0; i < window_count; i++) {
    window_t *win = &windows[window_z_order[i]];
    if (win->id != 0 && win->fading_mode != 2 && !win->is_minimized &&
        (win->is_dragging || win->is_resizing)) {
      return window_handle_mouse(win, mx, my, buttons);
    }
  }

  // 2. Otherwise check top-most (last in array) first
  for (int i = window_count - 1; i >= 0; i--) {
    window_t *win = &windows[window_z_order[i]];
    if (win->id != 0 && win->fading_mode != 2 && !win->is_minimized) {
      if (window_handle_mouse(win, mx, my, buttons)) {
        return 1;
      }
    }
  }
  return 0;
}

#define SCROLL_WHEEL_DELTA 120

void winmgr_handle_scroll(int mx, int my, int scroll) {
  // Correct search order: Top-most (last in array) first
  for (int i = window_count - 1; i >= 0; i--) {
    window_t *win = &windows[window_z_order[i]];
    if (win->id == 0 || win->fading_mode == 2 || win->is_minimized)
      continue;
    if (mx >= win->x && mx < win->x + win->width && my >= win->y &&
        my < win->y + win->height) {

      int lines = (abs(scroll) * win->scroll_line_height) / SCROLL_WHEEL_DELTA;
      if (scroll < 0)
        lines = -lines;

      int new_pos = win->scroll_position - lines;
      if (new_pos < 0)
        new_pos = 0;
      if (new_pos > win->max_scroll)
        new_pos = win->max_scroll;

      int actual_delta = new_pos - win->scroll_position;
      if (actual_delta != 0) {
        win->scroll_position = new_pos;
        win->needs_redraw = 1;
      }

      // Normalize scroll to a simple direction (+1/-1) for app consumption
      int scroll_dir = (scroll > 0) ? 1 : -1;

      // Always send the scroll event so apps can implement their own scrolling
      task_t *curr = get_current_task();
      if (win->owner_pid > 1 && (!curr || curr->id != win->owner_pid)) {
        msg_t msg;
        msg.type = MSG_MOUSE_SCROLL;
        msg.d1 = mx - win->x;
        msg.d2 = my - win->y;
        msg.d3 = 0;
        msg.d4 = scroll_dir;
        msg_send(win->owner_pid, &msg);
      } else if (win->on_scroll) {
        win->on_scroll(win, scroll_dir);
      }

      return;
    }
  }
}

// Helper to safely close a window and clean up
void winmgr_close_window(window_t *win) {
  if (!win || win->id == 0 || (win->flags & WINDOW_FLAG_NO_CLOSE))
    return;

  // Check if already closing
  if (win->fading_mode == 2)
    return;

  // Start Fade Out
  win->fading_mode = 2;

  // Signal the owner process to exit if it's a threaded app (NOT the desktop)
  if (win->owner_pid != 0) {
    // If the owner is the desktop, don't tell it to quit!
    task_t *desktop = get_task_by_name("desktop");
    if (desktop && win->owner_pid != desktop->id) {
      msg_t quit_msg;
      quit_msg.type = MSG_QUIT;
      quit_msg.d1 = win->id;
      msg_send(win->owner_pid, &quit_msg);
    }
  }

  // macOS Genie close: fast pull, soft decelerating spring
  float close_k = 280.0f;
  float close_d = 30.0f;

  // Opacity fade-out with spring physics
  if (win->anim_opacity.active) {
    anim_retarget(&win->anim_opacity, 0.0f);
  } else {
    anim_start_spring(&win->anim_opacity, (float)win->opacity, 0.0f, close_k, close_d);
  }

  // IMMEDIATELY transfer focus if this was the active window
  if (win == active_window) {
    active_window = NULL;
    // Find next top-most valid window that isn't closing
    for (int i = window_count - 1; i >= 0; i--) {
      int idx = window_z_order[i];
      if (idx < 0) continue;
      window_t *next_active = &windows[idx];
      if (next_active->id != 0 && next_active != win &&
          next_active->fading_mode != 2 && !next_active->is_minimized) {
        winmgr_bring_to_front(next_active);
        break;
      }
    }
  }

  // Genie suck-down animation targeting the dock icon
  win->anim_mode = 2; // Minimize/Close

  int tx_orig = 20;
  int ty = screen_height;

  if (win->taskbar_x >= 0 && win->taskbar_y >= 0) {
      tx_orig = win->taskbar_x;
      ty = win->taskbar_y;
  } else if (win->launch_x >= 0 && win->launch_y >= 0) {
      tx_orig = win->launch_x;
      ty = win->launch_y;
  }

  // Set pinch direction: bottom pinches toward dock icon below, top if dock above
  win->pinch_top = (ty < win->y);

  // Target: funnel down to dock icon size (32x32) centered at dock position
  int target_x = tx_orig - 16;
  int target_y = ty - 16;
  int target_w = 32;
  int target_h = 32;

  if (win->is_animating) {
    anim_retarget(&win->anim_x, (float)target_x);
    anim_retarget(&win->anim_y, (float)target_y);
    anim_retarget(&win->anim_w, (float)target_w);
    anim_retarget(&win->anim_h, (float)target_h);
    anim_retarget(&win->anim_scale, 0.01f);
  } else {
    anim_start_spring(&win->anim_scale, win->scale, 0.01f, close_k, close_d);
    win->is_animating = 1;
    anim_start_spring(&win->anim_x, (float)win->x, (float)target_x, close_k, close_d);
    anim_start_spring(&win->anim_y, (float)win->y, (float)target_y, close_k, close_d);
    anim_start_spring(&win->anim_w, (float)win->width, (float)target_w, close_k, close_d);
    anim_start_spring(&win->anim_h, (float)win->height, (float)target_h, close_k, close_d);
  }

  // Kill any residual momentum so window doesn't fly off while fading
  win->vel_x = 0;
  win->vel_y = 0;
  // Let animation proceed
  win->is_animating = 1;

  // Don't kill ID yet - wait for animation to finish
}

void winmgr_close_active(void) {

  if (active_window) {
    winmgr_close_window(active_window);
    return;
  }

  // Fallback: search Z-order for top-most regular window
  for (int i = window_count - 1; i >= 0; i--) {
    window_t *win = &windows[window_z_order[i]];
    if (win->id != 0 && win->fading_mode != 2 &&
        !win->is_minimized) {
      winmgr_close_window(win);
      return;
    }
  }
}

void winmgr_bring_to_front(window_t *win) {
  int z_idx = -1;
  int win_idx = (int)(win - windows);

  // Find where this window is in the Z-order
  for (int i = 0; i < window_count; i++) {
    if (window_z_order[i] == win_idx) {
      z_idx = i;
      break;
    }
  }

  if (z_idx < 0)
    return;

  // If already at front, just update active_window
  if (z_idx == window_count - 1) {
    active_window = win;
    return;
  }

  // Move this index to the end (top) of the Z-order array
  int temp_idx = window_z_order[z_idx];
  for (int i = z_idx; i < window_count - 1; i++) {
    window_z_order[i] = window_z_order[i + 1];
  }
  window_z_order[window_count - 1] = temp_idx;

  active_window = win;
}

void winmgr_cycle_window(void) {
  if (window_count <= 1)
    return;

  // Move the current top window (index window_count-1) to the bottom (index 0)
  int top_idx = window_z_order[window_count - 1];

  // Shift others up
  for (int i = window_count - 1; i > 0; i--) {
    window_z_order[i] = window_z_order[i - 1];
  }
  window_z_order[0] = top_idx;

  // New top window is at window_count-1
  int new_top_idx = window_z_order[window_count - 1];
  window_t *new_top = &windows[new_top_idx];

  if (new_top->id != 0 && !new_top->is_minimized &&
      !(new_top->flags & WINDOW_FLAG_WIDGET)) {
    active_window = new_top;
    new_top->is_minimized = 0;
    winmgr_bring_to_front(new_top);
  }
}

void winmgr_handle_copy(void) {
  extern void print_serial(const char *);
  print_serial("KEYBOARD: Ctrl+C pressed.\n");
  if (active_window) {
    print_serial("WINMGR: Active window found.\n");
    if (active_window->on_copy) {
      print_serial("WINMGR: Dispatching to on_copy.\n");
      active_window->on_copy(active_window);
    } else {
      print_serial("WINMGR: Active window has no on_copy handler.\n");
    }
  } else {
    print_serial("WINMGR: No active window for copy.\n");
  }
}

void winmgr_handle_cut(void) {
  extern void print_serial(const char *);
  print_serial("KEYBOARD: Ctrl+X pressed.\n");
  if (active_window) {
    if (active_window->on_cut) {
      print_serial("WINMGR: Dispatching to on_cut.\n");
      active_window->on_cut(active_window);
    } else {
      print_serial("WINMGR: Active window has no on_cut handler.\n");
    }
  }
}

void winmgr_handle_paste(void) {
  extern void print_serial(const char *);
  print_serial("KEYBOARD: Ctrl+V pressed.\n");
  if (active_window) {
    if (active_window->on_paste) {
      print_serial("WINMGR: Dispatching to on_paste.\n");
      active_window->on_paste(active_window, clipboard_paste());
    } else {
      print_serial("WINMGR: Active window has no on_paste handler.\n");
    }
  }
}
