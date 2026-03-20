#ifndef FILEMGR_H
#define FILEMGR_H

#include "../kernel/window.h"

void filemgr_init();
void filemgr_draw(window_t *win);
void filemgr_on_mouse(window_t *win, int mx, int my, int buttons);
void filemgr_on_key(window_t *win, int key, char c);

#endif
