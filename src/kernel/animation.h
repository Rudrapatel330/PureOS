#ifndef ANIMATION_H
#define ANIMATION_H

#include "window.h" // For Window pointer
#include <stdint.h>

#include "anim.h" // For Easing types (already defined as macros)

typedef enum {
  ANIM_TYPE_NONE = 0,
  ANIM_TYPE_WINDOW_OPEN,
  ANIM_TYPE_WINDOW_CLOSE,
  ANIM_TYPE_WINDOW_MINIMIZE,
  ANIM_TYPE_WINDOW_RESTORE,
  ANIM_TYPE_WINDOW_BOUNDS
} AnimType;

// Core Animation Data Structure
typedef struct {
  int active;
  AnimType type;
  Window *target_win;

  // Time tracking
  uint32_t start_time;
  uint32_t duration;

  // Properties (Start -> End)
  int start_x, start_y, start_w, start_h;
  int end_x, end_y, end_w, end_h;

  // Current interpolated properties
  int curr_x, curr_y, curr_w, curr_h;

  // No more snapshots for scaling animation to save memory and avoid lag

  int easing;
} Animation;

#define MAX_ANIMATIONS 5

// API
void animation_init();
void animation_start_window_open(Window *win, int start_x, int start_y,
                                 int duration);
void animation_start_window_close(Window *win, int dest_x, int dest_y,
                                  int duration);
void animation_start_window_minimize(Window *win, int dest_x, int dest_y,
                                     int duration);
void animation_start_window_restore(Window *win, int start_x, int start_y,
                                    int duration);
void animation_start_bounds_transition(Window *win, int start_x, int start_y,
                                       int start_w, int start_h, int duration);
void animation_update(uint32_t current_time);
int has_active_animations();
int is_window_animating(Window *win);

#endif
