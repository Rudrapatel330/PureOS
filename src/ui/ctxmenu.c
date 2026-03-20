#include "../ui/ctxmenu.h"
#include "../drivers/vga.h"
#include "../kernel/screen.h"

extern uint32_t *backbuffer;
extern int screen_width, screen_height;

ctxmenu_t context_menu = {0};

#define ITEM_H 20
#define SEP_H 6
#define MENU_PAD 4
#define MENU_W 130

void ctxmenu_show(int x, int y, ctxmenu_item_t *items, int count) {
  context_menu.visible = 1;
  context_menu.x = x;
  context_menu.y = y;
  context_menu.item_count =
      count > CTXMENU_MAX_ITEMS ? CTXMENU_MAX_ITEMS : count;
  context_menu.width = MENU_W;

  int h = MENU_PAD * 2;
  for (int i = 0; i < context_menu.item_count; i++) {
    context_menu.items[i] = items[i];
    h += items[i].label ? ITEM_H : SEP_H;
  }
  context_menu.height = h;

  if (x + context_menu.width > screen_width)
    context_menu.x = screen_width - context_menu.width;
  if (y + context_menu.height > screen_height)
    context_menu.y = screen_height - context_menu.height;
}

void ctxmenu_hide(void) { context_menu.visible = 0; }

void ctxmenu_draw_target(int mx, int my, uint32_t *target) {
  if (!context_menu.visible)
    return;

  int x = context_menu.x;
  int y = context_menu.y;
  int w = context_menu.width;
  int h = context_menu.height;

  vga_draw_rect_lfb(x, y, w, h, 0xFFF0F0F0, target);
  vga_draw_rect_lfb(x, y, w, 1, 0xFFFFFFFF, target);
  vga_draw_rect_lfb(x, y, 1, h, 0xFFFFFFFF, target);
  vga_draw_rect_lfb(x + w - 1, y, 1, h, 0xFF555555, target);
  vga_draw_rect_lfb(x, y + h - 1, w, 1, 0xFF555555, target);

  int cy = y + MENU_PAD;
  for (int i = 0; i < context_menu.item_count; i++) {
    if (!context_menu.items[i].label) {
      vga_draw_rect_lfb(x + 4, cy + 2, w - 8, 1, 0xFF888888, target);
      cy += SEP_H;
    } else {
      int hovered = (mx >= x && mx < x + w && my >= cy && my < cy + ITEM_H);
      if (hovered) {
        vga_draw_rect_lfb(x + 2, cy, w - 4, ITEM_H, 0xFF0055CC, target);
        vga_draw_string_lfb(x + 8, cy + 4, context_menu.items[i].label,
                            0xFFFFFFFF, target);
      } else {
        vga_draw_string_lfb(x + 8, cy + 4, context_menu.items[i].label,
                            0xFF000000, target);
      }
      cy += ITEM_H;
    }
  }
}

void ctxmenu_draw(int mx, int my) { ctxmenu_draw_target(mx, my, backbuffer); }

int ctxmenu_click(int mx, int my) {
  if (!context_menu.visible)
    return 0;

  int x = context_menu.x;
  int y = context_menu.y;

  if (mx < x || mx >= x + context_menu.width || my < y ||
      my >= y + context_menu.height) {
    ctxmenu_hide();
    return 1;
  }

  int cy = y + MENU_PAD;
  for (int i = 0; i < context_menu.item_count; i++) {
    int ih = context_menu.items[i].label ? ITEM_H : SEP_H;
    if (my >= cy && my < cy + ih && context_menu.items[i].label) {
      if (context_menu.items[i].action) {
        context_menu.items[i].action();
      }
      ctxmenu_hide();
      return 1;
    }
    cy += ih;
  }
  return 1;
}
