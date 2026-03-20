#ifndef CTXMENU_H
#define CTXMENU_H

#include <stdint.h>

#define CTXMENU_MAX_ITEMS 8

typedef struct {
  const char *label;
  void (*action)(void);
} ctxmenu_item_t;

typedef struct {
  int visible;
  int x, y;
  int width, height;
  ctxmenu_item_t items[CTXMENU_MAX_ITEMS];
  int item_count;
} ctxmenu_t;

extern ctxmenu_t context_menu;

void ctxmenu_show(int x, int y, ctxmenu_item_t *items, int count);
void ctxmenu_hide(void);
void ctxmenu_draw(int mouse_x, int mouse_y);
void ctxmenu_draw_target(int mouse_x, int mouse_y, uint32_t *target);
int ctxmenu_click(int mx, int my);

#endif
