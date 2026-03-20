#ifndef STARTMENU_H
#define STARTMENU_H

#include <stdint.h>

#include "../kernel/window.h" // for rect_t
void startmenu_show(int x, int y);
void startmenu_draw(uint32_t *buffer, rect_t clip);
int startmenu_handle_mouse(int mx, int my, int buttons);
int startmenu_is_active();
int startmenu_get_x();
int startmenu_get_y();
int startmenu_get_w();
int startmenu_get_h();

#endif
