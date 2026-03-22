#include "../ui/ctxmenu.h"
#include "../drivers/vga.h"
#include "../kernel/screen.h"

extern void compositor_invalidate_rect(int x, int y, int w, int h);

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

  compositor_invalidate_rect(context_menu.x - 2, context_menu.y - 2,
                               context_menu.width + 4, context_menu.height + 4);
}

void ctxmenu_hide(void) {
  if (context_menu.visible) {
    context_menu.visible = 0;
    compositor_invalidate_rect(context_menu.x - 2, context_menu.y - 2,
                                 context_menu.width + 4,
                                 context_menu.height + 4);
  }
}

void ctxmenu_draw_target(int mx, int my, uint32_t *target) {
  if (!context_menu.visible)
    return;

  int x = context_menu.x;
  int y = context_menu.y;
  int w = context_menu.width;
  int h = context_menu.height;

  // Modern White Rounded Menu with shadow borders
  vga_draw_rect_blend_lfb_ex(x, y, w, h, 0xFFFFFFFF, 1, 0xFFD0D0D0, target, 10);

  int cy = y + MENU_PAD;
  for (int i = 0; i < context_menu.item_count; i++) {
    if (!context_menu.items[i].label) {
      vga_draw_rect_lfb(x + 10, cy + 2, w - 20, 1, 0xFFE0E0E0, target);
      cy += SEP_H;
    } else {
      int hovered = (mx >= x && mx < x + w && my >= cy && my < cy + ITEM_H);
      if (hovered) {
        // Modern Blue Highlight
        vga_draw_rect_blend_lfb(x + 4, cy, w - 8, ITEM_H, 0xFF3B82F6, target, 4);
        vga_draw_string_lfb(x + 12, cy + 4, context_menu.items[i].label,
                            0xFFFFFFFF, target);
      } else {
        vga_draw_string_lfb(x + 12, cy + 4, context_menu.items[i].label,
                            0xFF333333, target);
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
