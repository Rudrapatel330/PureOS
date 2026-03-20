#ifndef DESKTOP_H
#define DESKTOP_H

#include "window.h" // For rect_t
#include <stdint.h>

// Initialize desktop (load wallpaper etc)
void desktop_init();

// Draw full desktop (or init buffer)
void desktop_draw();

// Icon Structure
typedef struct {
  int x, y;
  char label[32];
  int type; // 0=Terminal, 1=Calc, 2=Editor, 3=Computer, 4=Paint, 5=FileMgr,
            // 6=TaskMgr
  uint32_t color;

  // Interaction State
  int selected;
  int is_dragging;
} icon_t;

// Max icons on desktop
#define MAX_ICONS 32

// Global Icon List
extern icon_t icons[MAX_ICONS];
extern int icon_count;

// Management
void desktop_add_icon(int x, int y, const char *label, int type,
                      uint32_t color);

void desktop_render_icons(uint32_t *target, rect_t clip);

// Draw dynamic overlays (clock, window tabs, drag shadows)
void desktop_draw_overlay();

// Exposed buffer for Compositor
extern uint32_t *desktop_buffer;

// Global Search Bar
extern int search_active;
void desktop_activate_search(void);
void desktop_render_search(uint32_t *target, rect_t clip);
void desktop_handle_search_key(int key, char ascii);
int desktop_handle_search_mouse(int mx, int my, int buttons);

#endif
