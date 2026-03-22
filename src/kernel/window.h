#ifndef WINDOW_H
#define WINDOW_H

#include "anim.h"
#include <stdint.h>

typedef struct {
  int x, y, w, h;
} rect_t;

#define MAX_WINDOWS 32

// Window Flags
#define WINDOW_FLAG_NONE 0x00
#define WINDOW_FLAG_NO_TITLEBAR 0x01
#define WINDOW_FLAG_NO_BORDER 0x02
#define WINDOW_FLAG_NO_CONTROLS 0x04
#define WINDOW_FLAG_TRANSPARENT 0x08
#define WINDOW_FLAG_NON_INTERACTIVE 0x10
#define WINDOW_FLAG_FIXED_SIZE 0x20
#define WINDOW_FLAG_WIDGET 0x40
#define WINDOW_FLAG_NO_CLOSE 0x80

typedef struct {
  uint32_t title_color;
  uint32_t title_text_shadow;
  uint32_t border_light;
  uint32_t border_dark;
  uint32_t bg_color;
} window_style_t;

// Predefined Styles
#define STYLE_DEFAULT                                                          \
  (window_style_t){0x000080, 0x000000, 0xFFFFFF, 0x808080, 0xC0C0C0}
#define STYLE_DARK                                                             \
  (window_style_t){0x333333, 0x000000, 0x666666, 0x222222, 0x444444}
#define STYLE_TERMINAL                                                         \
  (window_style_t){0x000000, 0x000000, 0x00FF00, 0x004400, 0x000000}

typedef struct {
  int id;
  int owner_pid; // Process ID of the application that owns this window
  int x, y;
  int prev_x, prev_y; // For smart redraw
  int width, height;
  char title[32];
  uint32_t bg_color;
  window_style_t style;
  // Simple state
  int is_dragging;
  int drag_offset_x;
  int drag_offset_y;

  // Resize state
  int is_resizing;
  int resize_edge;

  // Phase 3: Window States
  int is_minimized;
  int is_maximized;
  int saved_x, saved_y, saved_w, saved_h;
  int is_snapped; // 1=left, 2=right, 3=top(maximized via snap)

  // Text Buffer
  char text_buffer[256];
  int cursor_pos;

  // Generic Callbacks
  int needs_redraw;
  void (*draw)(void *win);
  void (*on_mouse)(void *win, int mx, int my, int buttons);
  void (*on_key)(void *win, int key, char ascii);
  void (*on_scroll)(void *win, int direction);
  void (*on_copy)(void *win);
  void (*on_cut)(void *win);
  void (*on_paste)(void *win, const char *text);
  void (*on_close)(void *win);
  void *user_data; // Per-window app state (e.g. terminal instance)
  int app_type;    // 0=terminal, 1=calculator, 2=editor, 3=computer, 4=paint,
                   // 5=files

  // === Legacy/App-specific fields (migrating) ===
  // Editor State
  char **editor_lines;
  int editor_line_count;
  int editor_max_lines;
  int editor_scroll_y;
  int editor_scroll_x;
  int editor_cursor_row;
  int editor_cursor_col;
  int editor_syntax_mode;

  // Paint State
  uint32_t *paint_canvas;
  uint32_t paint_color;
  int paint_brush_size;
  int paint_prev_x;
  int paint_prev_y;

  // SysMon State
  int sysmon_history[60];
  uint32_t sysmon_last_tick;
  uint32_t sysmon_uptime;

  // File Manager / Computer State
  void *files; // For simplicity
  int file_count;
  int selected_file_idx;

  // Selection state
  int sel_start;
  int sel_end;

  // Animation State (New Engine)
  int is_animating;
  int anim_mode; // 1=Open/Restore, 2=Minimize
  animation_t anim_x;
  animation_t anim_y;
  animation_t anim_w;
  animation_t anim_h;

  // Phase 1: Compositor Support
  uint32_t *surface; // Off-screen buffer for this window
  int surface_w;     // Width of surface (usually == width)
  int surface_h;     // Height of surface (usually == height)
  uint8_t opacity;   // 0 to 255 (255 = opaque)

  // Opacity Animation
  animation_t anim_opacity;
  int fading_mode; // 0=None, 1=Fade In, 2=Fade Out

  // Phase 4: Scaling & Warp Support
  float scale;            // 1.0 = Normal size
  animation_t anim_scale; // For scale animations

  // Scroll state
  int scroll_position;
  int content_height;
  int visible_height;
  int max_scroll;
  int scroll_line_height;

  // Momentum
  float vel_x;
  float vel_y;

  int hover_btn; // 0=none, 1=min, 2=max, 3=close

  // Phase 3: Blur Support
  int blur_enabled;
  uint8_t blur_strength;

  // Legacy support (to be removed or adapted)
  int taskbar_x;
  int taskbar_y;
  int launch_x;
  int launch_y;
  int pinch_top;

  // Sub-window dirty rect tracking
  int sub_dirty_active;
  rect_t sub_dirty;

  // Virtual workspace support
  int workspace; // 0-3: which workspace this window belongs to
  int ws_hidden; // 1 = hidden because on a different workspace
  int exists;    // 1 = window slot is in use

  uint32_t flags;
  int always_on_top;
  int watchdog; // Frame-based counter to clear stuck animations

  // 64-slice mesh for high-fidelity warping
  float mesh_lx[64];
  float mesh_rx[64];
  float mesh_ty[64];
} window_t;

extern window_t *active_window;
extern window_t windows[MAX_WINDOWS];
extern int window_z_order[MAX_WINDOWS];
extern int window_count;

void winmgr_init();
window_t *winmgr_create_window(int x, int y, int w, int h, const char *title);
window_t *winmgr_get_window_by_app_type(int app_type);
void winmgr_invalidate_rect(window_t *win, int x, int y, int w, int h);
void winmgr_blit(window_t *dst, int dx, int dy, const uint32_t *src, int sw,
                 int sh, int sx, int sy, int w, int h);
void winmgr_blend_blit(window_t *dst, int dx, int dy, const uint32_t *src,
                       int sw, int sh, int sx, int sy, int w, int h,
                       uint8_t alpha);
void winmgr_fill_rect(window_t *win, int x, int y, int w, int h,
                      uint32_t color);
void winmgr_draw_rect(window_t *win, int x, int y, int w, int h,
                      uint32_t color);
void winmgr_draw_line(window_t *win, int x1, int y1, int x2, int y2,
                      uint32_t color);
void winmgr_draw_line_aa(window_t *win, int x1, int y1, int x2, int y2,
                         uint32_t color);
void winmgr_draw_arc_aa(window_t *win, int x, int y, int radius, int quadrant,
                        uint32_t color);
void winmgr_put_pixel(window_t *win, int x, int y, uint32_t color);
void winmgr_draw_text(window_t *win, int x, int y, const char *text,
                      uint32_t color);
void winmgr_render_all();
void winmgr_render_text_all();
void winmgr_render_window(window_t *win);

// Render all (flush updates)
void winmgr_flush_updates();
// Animation Helper
void start_window_anim(window_t *win, int sx, int sy, int sw, int sh, int ex,
                       int ey, int ew, int eh, float duration, int type);
void start_window_spring(window_t *win, int sx, int sy, int sw, int sh, int ex,
                         int ey, int ew, int eh, float stiffness,
                         float damping);
void draw_anim_ghost(window_t *win, rect_t clip);

void winmgr_draw_button(window_t *win, int x, int y, int w, int h,
                        const char *label);
void window_handle_key(int key, char c);
// Returns 1 if handled/consumed
int window_handle_mouse(window_t *win, int mx, int my, int buttons);
int winmgr_handle_mouse_global(int mx, int my, int buttons);
void winmgr_handle_scroll(int mx, int my, int scroll);
void winmgr_close_window(window_t *win);
void winmgr_close_active(void);
void winmgr_bring_to_front(window_t *win);
void winmgr_cycle_window(void);

extern window_t windows[MAX_WINDOWS];
extern int window_count;

#endif
