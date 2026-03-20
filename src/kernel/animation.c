#include "animation.h"
#include "../drivers/vga.h"
#include "../kernel/heap.h"
#include "window.h"

static Animation animations[MAX_ANIMATIONS];

void anim_init() {
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    animations[i].active = 0;
  }
}

// Ease Out Quad: Decelerating to zero velocity
static int ease_out_quad(int t, int total) {
  if (total == 0)
    return 1000;
  long t_scaled = (t * 1000) / total;
  long result = (t_scaled * (2000 - t_scaled)) / 1000;
  if (result > 1000)
    result = 1000;
  if (result < 0)
    result = 0;
  return (int)result;
}

static int ease_linear(int t, int total) {
  if (total == 0)
    return 1000;
  long result = (t * 1000) / total;
  if (result > 1000)
    result = 1000;
  return (int)result;
}

static int interpolate(int start, int end, int progress_scaled) {
  long diff = end - start;
  long delta = (diff * progress_scaled) / 1000;
  return start + (int)delta;
}

void animation_start_window_open(Window *win, int start_x, int start_y,
                                 int duration) {
  if (!win)
    return;

  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (!animations[i].active) {
      animations[i].active = 1;
      animations[i].type = ANIM_TYPE_WINDOW_OPEN;
      animations[i].target_win = win;

      extern uint32_t sys_ticks;
      animations[i].start_time = sys_ticks;
      animations[i].duration = duration;

      animations[i].start_x = start_x;
      animations[i].start_y = start_y;
      animations[i].start_w = 32;
      animations[i].start_h = 32;

      animations[i].end_x = win->x;
      animations[i].end_y = win->y;
      animations[i].end_w = win->width;
      animations[i].end_h = win->height;

      animations[i].curr_x = start_x;
      animations[i].curr_y = start_y;
      animations[i].curr_w = animations[i].start_w;
      animations[i].curr_h = animations[i].start_h;

      animations[i].easing = EASE_OUT_QUAD;
      return;
    }
  }
}

void animation_start_window_close(Window *win, int dest_x, int dest_y,
                                  int duration) {
  if (!win)
    return;

  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (!animations[i].active) {
      animations[i].active = 1;
      animations[i].type = ANIM_TYPE_WINDOW_CLOSE;
      animations[i].target_win = win;

      extern uint32_t sys_ticks;
      animations[i].start_time = sys_ticks;
      animations[i].duration = duration;

      animations[i].start_x = win->x;
      animations[i].start_y = win->y;
      animations[i].start_w = win->width;
      animations[i].start_h = win->height;

      animations[i].end_x = dest_x;
      animations[i].end_y = dest_y;
      animations[i].end_w = 32;
      animations[i].end_h = 32;

      animations[i].curr_x = animations[i].start_x;
      animations[i].curr_y = animations[i].start_y;
      animations[i].curr_w = animations[i].start_w;
      animations[i].curr_h = animations[i].start_h;

      animations[i].easing = EASE_OUT_QUAD;
      return;
    }
  }
}

int has_active_animations() {
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (animations[i].active)
      return 1;
  }
  return 0;
}

void animation_update(uint32_t current_time) {
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (animations[i].active) {
      Animation *anim = &animations[i];
      uint32_t elapsed = current_time - anim->start_time;

      if (elapsed >= anim->duration) {
        anim->curr_x = anim->end_x;
        anim->curr_y = anim->end_y;
        anim->curr_w = anim->end_w;
        anim->curr_h = anim->end_h;

        anim->active = 0;

        if (anim->type == ANIM_TYPE_WINDOW_CLOSE && anim->target_win) {
          anim->target_win->is_visible = 0;
          anim->target_win->is_valid = 0;
          extern Window *active_window;
          if (active_window == anim->target_win) {
            active_window = NULL;
          }
        } else if (anim->type == ANIM_TYPE_WINDOW_MINIMIZE &&
                   anim->target_win) {
          anim->target_win->is_visible = 0;
          anim->target_win->is_minimized = 1;
        } else if (anim->type == ANIM_TYPE_WINDOW_RESTORE && anim->target_win) {
          anim->target_win->is_visible = 1;
          anim->target_win->is_minimized = 0;
        } else if (anim->type == ANIM_TYPE_WINDOW_BOUNDS && anim->target_win) {
          // just generic bounds animation finished (like maximize)
        } else if (anim->type == ANIM_TYPE_WINDOW_MINIMIZE &&
                   anim->target_win) {
          anim->target_win->is_visible = 0;
          anim->target_win->is_minimized = 1;
        } else if (anim->type == ANIM_TYPE_WINDOW_RESTORE && anim->target_win) {
          anim->target_win->is_visible = 1;
          anim->target_win->is_minimized = 0;
        } else if (anim->type == ANIM_TYPE_WINDOW_BOUNDS && anim->target_win) {
          // just generic bounds animation finished (like maximize)
        }
      } else {
        int progress_scaled = 0;
        if (anim->easing == EASE_OUT_QUAD) {
          progress_scaled = ease_out_quad(elapsed, anim->duration);
        } else {
          progress_scaled = ease_linear(elapsed, anim->duration);
        }

        anim->curr_x = interpolate(anim->start_x, anim->end_x, progress_scaled);
        anim->curr_y = interpolate(anim->start_y, anim->end_y, progress_scaled);
        anim->curr_w = interpolate(anim->start_w, anim->end_w, progress_scaled);
        anim->curr_h = interpolate(anim->start_h, anim->end_h, progress_scaled);
      }
    }
  }
}

void animation_start_window_minimize(Window *win, int dest_x, int dest_y,
                                     int duration) {
  if (!win)
    return;
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (!animations[i].active) {
      animations[i].active = 1;
      animations[i].type = ANIM_TYPE_WINDOW_MINIMIZE;
      animations[i].target_win = win;

      extern uint32_t sys_ticks;
      animations[i].start_time = sys_ticks;
      animations[i].duration = duration;

      animations[i].start_x = win->x;
      animations[i].start_y = win->y;
      animations[i].start_w = win->width;
      animations[i].start_h = win->height;

      animations[i].end_x = dest_x;
      animations[i].end_y = dest_y;
      animations[i].end_w = 32;
      animations[i].end_h = 32;

      animations[i].curr_x = animations[i].start_x;
      animations[i].curr_y = animations[i].start_y;
      animations[i].curr_w = animations[i].start_w;
      animations[i].curr_h = animations[i].start_h;

      animations[i].easing = EASE_OUT_QUAD;
      return;
    }
  }
}

void animation_start_window_restore(Window *win, int start_x, int start_y,
                                    int duration) {
  if (!win)
    return;
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (!animations[i].active) {
      animations[i].active = 1;
      animations[i].type = ANIM_TYPE_WINDOW_RESTORE;
      animations[i].target_win = win;

      extern uint32_t sys_ticks;
      animations[i].start_time = sys_ticks;
      animations[i].duration = duration;

      animations[i].start_x = start_x;
      animations[i].start_y = start_y;
      animations[i].start_w = 32;
      animations[i].start_h = 32;

      animations[i].end_x = win->x;
      animations[i].end_y = win->y;
      animations[i].end_w = win->width;
      animations[i].end_h = win->height;

      animations[i].curr_x = start_x;
      animations[i].curr_y = start_y;
      animations[i].curr_w = animations[i].start_w;
      animations[i].curr_h = animations[i].start_h;

      animations[i].easing = EASE_OUT_QUAD;
      return;
    }
  }
}

int is_window_animating(Window *win) {
  if (!win)
    return 0;
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (animations[i].active && animations[i].target_win == win) {
      return 1;
    }
  }
  return 0;
}
