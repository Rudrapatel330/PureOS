#ifndef SYSMENU_H
#define SYSMENU_H

#include <stdint.h>
#include "../kernel/window.h" // for rect_t

void sysmenu_toggle(void);
void sysmenu_draw(uint32_t *buffer, rect_t clip);
int sysmenu_handle_mouse(int mx, int my, int buttons);
int sysmenu_is_active(void);
int sysmenu_is_animating(void);
void sysmenu_tick_animation(float dt);

#endif
