#ifndef TASKBAR_H
#define TASKBAR_H

#include "../kernel/window.h" // For rect_t
#include <stdint.h>

#define MAX_DOCK_ICONS 32

typedef struct {
  float x, y;
  float y_offset; // For bounce animations (horizontal)
  float x_offset; // For bounce animations (vertical)
  int w, h;
  float scale;
  float target_scale;
  float target_x;
  float target_y;
  int app_id;
  int is_running;
  int is_pinned;
  window_t *win_ref;
  int animating;
} dock_icon_state_t;

typedef struct {
  dock_icon_state_t icons[MAX_DOCK_ICONS];
  int icon_count;
  int hovered_index;
  int mouse_x;
  int mouse_y;
  int dock_x, dock_y, dock_w, dock_h;
  float anim_dock_x, anim_dock_y, anim_dock_w, anim_dock_h;
  int edit_mode;
  int drag_active;
} taskbar_state_t;

void taskbar_draw(uint32_t *buffer, rect_t clip);
void menubar_draw(uint32_t *buffer, rect_t clip);
int taskbar_handle_mouse(int mx, int my, int buttons);
void taskbar_tick_animations(float dt);
int taskbar_is_animating(void);

#endif
