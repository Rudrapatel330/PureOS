#include "settings.h"
#include "../kernel/config.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"

extern void compositor_invalidate_rect(int x, int y, int w, int h);
extern int screen_width, screen_height;
extern window_t windows[];
extern int window_count;
extern void desktop_invalidate(void);
extern void *kmalloc(unsigned int size);
extern void kfree(void *ptr);
extern void print_serial(const char *s);
extern void k_itoa(int n, char *s);

#define SW 150
#define CX (SW + 15)
#define PA 0
#define PB 1
#define PT 2
#define PC 3
#define PD 4
#define PE 5
#define PW 6
#define NP 7

// Colors
#define C0 0xFF202020
#define C1 0xFF2D2D2D
#define C2 0xFF383838
#define CW 0xFFFFFFFF
#define CG 0xFF999999
#define CD 0xFF666666
#define CA 0xFF0078D4
#define CS 0xFF3D3D3D

typedef struct {
  int page, hover;
  char pw[32];
  int pwc, wp, th, di, filter, icon_int, bg_int;
  int dragging_icon, dragging_bg;
  int show_clock, show_calendar;
  int pinned[10];
  int num_pinned;
  int scroll_y, tz;
  int mx, my;
  int auto_hide_taskbar;
<<<<<<< HEAD
  int sel_res; // Selected resolution index
=======
  int show_sysmon;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
} sstate_t;

static void card(window_t *w, int x, int y, int cw, int h) {
  winmgr_fill_rect(w, x, y, cw, h, C1);
  winmgr_draw_rect(w, x, y, cw, h, CS);
}

static void draw_sb(window_t *w, sstate_t *s) {
  static const char *lb[] = {"Home", "Personal", "Accounts", "System", "About"};
  winmgr_fill_rect(w, 0, 0, SW, w->height, C1);
  winmgr_fill_rect(w, SW - 1, 0, 1, w->height, CS);
  // Profile
  winmgr_fill_rect(w, 15, 12, 28, 28, CA);
  winmgr_draw_text(w, 22, 18, "R", CW);
  winmgr_draw_text(w, 48, 15, "Rudra", CW);
  winmgr_draw_text(w, 48, 28, "local", CD);
  winmgr_fill_rect(w, 10, 48, SW - 20, 1, CS);
  for (int i = 0; i < NP; i++) {
    int y = 56 + i * 28;
    if (s->page == i) {
      winmgr_fill_rect(w, 4, y, SW - 8, 26, 0xFF555555); // Solid gray selection
      winmgr_fill_rect(w, 2, y + 6, 3, 14, CA);
    } else if (s->hover == i)
      winmgr_fill_rect(w, 4, y, SW - 8, 26, 0xFF353535); // Opaque blended hover
    winmgr_draw_text(w, 16, y + 7, lb[i], s->page == i ? CW : CG);
  }
}

static void pg_home(window_t *w, sstate_t *s) {
  int cw = w->width - SW - 30;
  (void)s;
  winmgr_draw_text(w, CX, 10, "Home", CW);
  // Hero
  card(w, CX, 32, cw, 55);
  winmgr_fill_rect(w, CX + 12, 40, 28, 20, CA);
  winmgr_draw_text(w, CX + 50, 38, "PURE-OS-PC", CW);
  winmgr_draw_text(w, CX + 50, 52, "PureOS v2.0", CG);
  // Cards
  int cy = 100;
  card(w, CX, cy, cw, 42);
  winmgr_fill_rect(w, CX + 10, cy + 8, 24, 24, CA);
  winmgr_draw_text(w, CX + 42, cy + 8, "Personalization", CW);
  winmgr_draw_text(w, CX + 42, cy + 22, "Wallpaper, themes", CG);
  winmgr_draw_text(w, CX + cw - 18, cy + 14, ">", CD);
  cy += 50;
  card(w, CX, cy, cw, 42);
  winmgr_fill_rect(w, CX + 10, cy + 8, 24, 24, 0xFF00AA44);
  winmgr_draw_text(w, CX + 42, cy + 8, "System", CW);
  winmgr_draw_text(w, CX + 42, cy + 22, "Display, sound", CG);
  winmgr_draw_text(w, CX + cw - 18, cy + 14, ">", CD);
  cy += 50;
  card(w, CX, cy, cw, 42);
  winmgr_fill_rect(w, CX + 10, cy + 8, 24, 24, 0xFFCC6600);
  winmgr_draw_text(w, CX + 42, cy + 8, "Accounts", CW);
  winmgr_draw_text(w, CX + 42, cy + 22, "Password, user", CG);
  winmgr_draw_text(w, CX + cw - 18, cy + 14, ">", CD);
}

static void pg_pers(window_t *w, sstate_t *s) {
  int cw = w->width - SW - 30;
  winmgr_draw_text(w, CX, 10, "Personalization", CW);
  // Wallpaper
  card(w, CX, 35, cw, 90);
  winmgr_draw_text(w, CX + 10, 42, "Background", CW);
  int tx = CX + 10, ty = 62, tw = 65, th = 40;
  for (int i = 0; i < 3; i++) {
    uint32_t bg[] = {0xFF445566, 0xFF1A1A2E, 0xFFCCDDEE};
    const char *nm[] = {"Image", "Dark", "Light"};
    uint32_t br = (s->wp == i) ? CA : CS;
    winmgr_fill_rect(w, tx, ty, tw, th, bg[i]);
    winmgr_draw_rect(w, tx - 1, ty - 1, tw + 2, th + 2, br);
    if (s->wp == i)
      winmgr_draw_rect(w, tx - 2, ty - 2, tw + 4, th + 4, br);
    winmgr_draw_text(w, tx + 12, ty + 14, nm[i], i == 2 ? 0xFF222222 : CW);
    tx += tw + 12;
  }
  // Theme
  card(w, CX, 140, cw, 65);
  winmgr_draw_text(w, CX + 10, 148, "Color mode", CW);
  int py = 170, pw = 85;
  winmgr_fill_rect(w, CX + 10, py, pw, 22, s->th == 0 ? CA : C2);
  winmgr_draw_text(w, CX + 28, py + 5, "Dark", CW);
  winmgr_fill_rect(w, CX + 10 + pw + 8, py, pw, 22, s->th == 1 ? CA : C2);
  winmgr_draw_text(w, CX + 10 + pw + 22, py + 5, "Light", CW);
  // Apply
  winmgr_fill_rect(w, CX, 220, 90, 26, CA);
  winmgr_draw_text(w, CX + 22, 226, "Apply", CW);
}

static void pg_acct(window_t *w, sstate_t *s) {
  int cw = w->width - SW - 30;
  winmgr_draw_text(w, CX, 10, "Accounts", CW);
  card(w, CX, 35, cw, 50);
  winmgr_fill_rect(w, CX + 10, 42, 32, 32, CA);
  winmgr_draw_text(w, CX + 20, 52, "R", CW);
  winmgr_draw_text(w, CX + 50, 42, "Rudra Patel", CW);
  winmgr_draw_text(w, CX + 50, 58, "Administrator", CG);
  // Password
  card(w, CX, 100, cw, 95);
  winmgr_draw_text(w, CX + 10, 108, "Lock Password", CW);
  int fx = CX + 10, fy = 130, fw = cw - 20;
  winmgr_fill_rect(w, fx, fy, fw, 22, 0xFF1A1A1A);
  winmgr_draw_rect(w, fx, fy, fw, 22, 0xFF555555);
  winmgr_fill_rect(w, fx, fy + 20, fw, 2, CA);
  winmgr_draw_text(w, fx + 4, fy + 5, s->pw, CW);
  winmgr_fill_rect(w, fx + 4 + s->pwc * 8, fy + 3, 1, 16, CW);
  winmgr_fill_rect(w, fx, 165, 100, 22, CA);
  winmgr_draw_text(w, fx + 14, 169, "Save", CW);
}

static void pg_sys(window_t *w) {
  int cw = w->width - SW - 30;
  winmgr_draw_text(w, CX, 10, "System", CW);
  card(w, CX, 35, cw, 50);
  winmgr_draw_text(w, CX + 10, 42, "Display", CW);
  winmgr_draw_text(w, CX + 10, 58, "1024x768 32bpp BGA", CG);
  card(w, CX, 95, cw, 42);
  winmgr_draw_text(w, CX + 10, 102, "Sound", CW);
  winmgr_draw_text(w, CX + 10, 118, "ES1370 AudioPCI", CG);
  card(w, CX, 147, cw, 42);
  winmgr_draw_text(w, CX + 10, 154, "Storage", CW);
  winmgr_draw_text(w, CX + 10, 170, "ATA Primary Master", CG);
}

static void pg_abt(window_t *w) {
  int cw = w->width - SW - 30;
  winmgr_draw_text(w, CX, 10, "About", CW);
  card(w, CX, 35, cw, 130);
  winmgr_fill_rect(w, CX + 10, 42, 44, 44, CA);
  winmgr_draw_text(w, CX + 16, 56, "Pure", CW);
  winmgr_draw_text(w, CX + 62, 44, "PureOS v2.0", CW);
  winmgr_draw_text(w, CX + 62, 60, "64-bit OS", CG);
  winmgr_fill_rect(w, CX + 10, 94, cw - 20, 1, CS);
  winmgr_draw_text(w, CX + 10, 104, "CPU: x86-64", CG);
  winmgr_draw_text(w, CX + 10, 120, "RAM: ~16 MB", CG);
  winmgr_draw_text(w, CX + 10, 136, "Arch: 64-bit", CG);
}

#define DRAW_HOVER(win, s, rx, ry, rw, rh, bg_col, hov_col) \
  do { \
    if ((s)->mx >= (rx) && (s)->mx < (rx) + (rw) && (s)->my >= (ry) && (s)->my < (ry) + (rh)) \
      winmgr_fill_rect((win), (rx), (ry), (rw), (rh), (hov_col)); \
    else \
      winmgr_fill_rect((win), (rx), (ry), (rw), (rh), (bg_col)); \
  } while(0)

<<<<<<< HEAD
=======
#define DRAW_ROUNDED_CARD(win, s, rx, ry, rw, rh, bg_col, hov_col, border_col) \
  do { \
    uint32_t fill_col = ((s)->mx >= (rx) && (s)->mx < (rx) + (rw) && (s)->my >= (ry) && (s)->my < (ry) + (rh)) ? (hov_col) : (bg_col); \
    winmgr_draw_rounded_rect_ex((win), (rx), (ry), (rw), (rh), fill_col, 1, (border_col), 8); \
  } while(0)

>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
static void settings_draw(void *w) {
  window_t *win = (window_t *)w;
  sstate_t *s = (sstate_t *)win->user_data;

  const theme_t *theme = theme_get();
 
  // Modern Theme Background
  winmgr_fill_rect(win, 0, 32, win->width, win->height - 32, theme->bg);

  // 1. SLEEK SIDEBAR (Premium Dark)
  winmgr_fill_rect(win, 0, 32, SW, win->height - 32, 0xFF051020);
  winmgr_fill_rect(win, SW - 1, 32, 1, win->height - 32, 0xFF1A1A2E);

  const char *tabs[] = {"Home", "Personal", "Taskbar", "Account",
                        "System", "About", "Widgets"};
  for (int i = 0; i < NP; i++) {
    int ty = 32 + i * 50;
    if (s->page == i) {
      winmgr_fill_rect(win, 0, ty, SW - 1, 50, theme->accent);
    }
    // Border between items
    winmgr_fill_rect(win, 0, ty + 49, SW - 1, 1, 0xFF1A1A2E);
    
    winmgr_draw_text(win, 20, ty + 18, tabs[i], 0xFFFFFFFF);
  }

  // 2. CONTENT AREA (CARDS)
  int cx = CX;
  int cy = 40 - s->scroll_y; // Apply scroll offset
  int card_w = win->width - cx - 20;

  if (s->page == PA) { // Home
    winmgr_draw_text(win, cx, cy, "General Settings", theme->fg);
    cy += 30;

    // Aesthetic Settings Card
<<<<<<< HEAD
    DRAW_HOVER(win, s, cx, cy, card_w, 45, theme->input_bg, 0xFF353545);
    winmgr_draw_rect(win, cx, cy, card_w, 45, theme->border);
=======
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 45, theme->input_bg, 0xFF353545, theme->border);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    winmgr_draw_text(win, cx + 15, cy + 15, "System Appearance", theme->fg);
    winmgr_draw_text(win, cx + card_w - 60, cy + 15, "Light >", theme->accent);

    cy += 60;
<<<<<<< HEAD
    DRAW_HOVER(win, s, cx, cy, card_w, 45, theme->input_bg, 0xFF353545);
    winmgr_draw_rect(win, cx, cy, card_w, 45, theme->border);
=======
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 45, theme->input_bg, 0xFF353545, theme->border);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    winmgr_draw_text(win, cx + 15, cy + 15, "Wallpapers", theme->fg);
  } else if (s->page == PT) { // Taskbar
    winmgr_draw_text(win, cx, cy, "Taskbar Behavior", theme->fg);
    cy += 30;

    // Auto-hide toggle
<<<<<<< HEAD
    DRAW_HOVER(win, s, cx, cy, card_w, 45, theme->input_bg, 0xFF353545);
    winmgr_draw_rect(win, cx, cy, card_w, 45, theme->border);
=======
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 45, theme->input_bg, 0xFF353545, theme->border);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    winmgr_draw_text(win, cx + 15, cy + 15, "Auto-hide Taskbar", theme->fg);
    winmgr_fill_rect(win, cx + card_w - 50, cy + 12, 40, 20,
                     s->auto_hide_taskbar ? CA : 0xFFCCCCCC);
    winmgr_fill_rect(win, cx + card_w - 50 + (s->auto_hide_taskbar ? 22 : 2), cy + 14, 16, 16,
                     0xFFFFFFFF);

    cy += 60;
    winmgr_draw_text(win, cx, cy, "Taskbar Apps", theme->fg);
    cy += 30;

    // List of apps to pin
    const char *app_names[] = {"Terminal", "Calculator", "Editor",   "Computer",
                               "Paint",    "Files",      "Task Mgr", "Browser",
                               "Video",    "Settings"};
    for (int i = 0; i < 10; i++) {
      DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 30, theme->input_bg, theme->input_bg, theme->border);
      winmgr_draw_text(win, cx + 15, cy + 8, app_names[i], theme->fg);

      // Is pinned?
      int is_pinned = 0;
      for (int j = 0; j < s->num_pinned; j++) {
        if (s->pinned[j] == i)
          is_pinned = 1;
      }

      // Draw toggle
      winmgr_fill_rect(win, cx + card_w - 50, cy + 5, 40, 20,
                       is_pinned ? CA : 0xFFCCCCCC);
      winmgr_fill_rect(win, cx + card_w - 50 + (is_pinned ? 22 : 2), cy + 7, 16,
                       16, 0xFFFFFFFF);
      cy += 35;
    }

    // Apply button
    DRAW_HOVER(win, s, cx, cy + 10, 100, 30, CA, 0xFF353545);
    winmgr_draw_text(win, cx + 30, cy + 18, "Apply", 0xFFFFFFFF);

  } else if (s->page == PC) { // Accounts
    winmgr_draw_text(win, cx, cy, "Accounts", theme->fg);
    cy += 30;

    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 60, theme->input_bg, theme->input_bg, theme->border);
    winmgr_fill_rect(win, cx + 15, cy + 15, 30, 30, CA);
    winmgr_draw_text(win, cx + 24, cy + 22, "R", 0xFFFFFFFF);
    winmgr_draw_text(win, cx + 55, cy + 15, "Rudra Patel", theme->fg);
    winmgr_draw_text(win, cx + 55, cy + 30, "Administrator", theme->fg_secondary);

    cy += 80;

    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 110, theme->input_bg, theme->input_bg, theme->border);
    winmgr_draw_text(win, cx + 15, cy + 15, "Lock Password", theme->fg);

    // Password textbox
    winmgr_fill_rect(win, cx + 15, cy + 40, card_w - 30, 25, 0xFFFFFFFF);
    winmgr_draw_rect(win, cx + 15, cy + 40, card_w - 30, 25, theme->border);

    // Draw masked dots for safety instead of cleartext
    char disp[32];
    for (int i = 0; i < s->pwc; i++)
      disp[i] = '*';
    disp[s->pwc] = 0;
    winmgr_draw_text(win, cx + 22, cy + 48, disp, 0xFF000000);

    // Blinking cursor
    extern uint32_t get_timer_ticks(void);
    if ((get_timer_ticks() / 20) % 2 == 0) {
      winmgr_fill_rect(win, cx + 22 + s->pwc * 8, cy + 45, 2, 16, 0xFF000000);
    }

    // Save button
    DRAW_HOVER(win, s, cx + 15, cy + 75, 80, 25, CA, 0xFF353545);
    winmgr_draw_text(win, cx + 38, cy + 82, "Save", 0xFFFFFFFF);

  } else if (s->page == PE) { // About
    winmgr_draw_text(win, cx + (card_w - 48) / 2, cy + 10, "PureOS",
                     theme->fg);
    winmgr_draw_text(win, cx + (card_w - 80) / 2, cy + 30, "Version 2.0",
                     theme->fg_secondary);

    cy += 70;
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 80, theme->input_bg, theme->input_bg, theme->border);
    winmgr_draw_text(win, cx + 15, cy + 15, "Processor: x86 CPU @ 2.4GHz",
                     theme->fg);
    winmgr_draw_text(win, cx + 15, cy + 35, "Memory: 512 MB DDR3", theme->fg);
    winmgr_draw_text(win, cx + 15, cy + 55, "Graphics: VBE Adapter",
                     theme->fg);
  } else if (s->page == PB) { // Personalization
    winmgr_draw_text(win, cx, cy, "Personalization", theme->fg);
    cy += 40;

    // Background Card
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 60, 0xFF0A1828, 0xFF0A1828, 0xFF1A2A3A);
    winmgr_draw_text(win, cx + 15, cy + 22, "Background", 0xFFFFFFFF);
    // Button pill
    winmgr_draw_rounded_rect_ex(win, cx + card_w - 110, cy + 16, 95, 28, 0xFF051020, 1, 0xFFFFFFFF, 14);
    winmgr_draw_text(win, cx + card_w - 95, cy + 22, "Image", 0xFFFFFFFF);
    cy += 75;

<<<<<<< HEAD
    int tx = cx + 15, ty = cy + 40, tw = 65, th = 40;
    for (int i = 0; i < 3; i++) {
      uint32_t bg[] = {0xFF445566, 0xFF1A1A2E, 0xFFCCDDEE};
      const char *nm[] = {"Image", "Dark", "Light"};
      uint32_t br = (s->wp == i) ? CA : 0xFFCCCCCC;
      DRAW_HOVER(win, s, tx, ty, tw, th, bg[i], 0xFF353545);
      winmgr_draw_rect(win, tx - 1, ty - 1, tw + 2, th + 2, br);
      if (s->wp == i)
        winmgr_draw_rect(win, tx - 2, ty - 2, tw + 4, th + 4, br);
      winmgr_draw_text(win, tx + 12, ty + 14, nm[i],
                       i == 2 ? 0xFF222222 : 0xFFFFFFFF);
      tx += tw + 12;
    }
=======
    // Color Mode Card
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 60, 0xFF0A1828, 0xFF0A1828, 0xFF1A2A3A);
    winmgr_draw_text(win, cx + 15, cy + 22, "Color Mode", 0xFFFFFFFF);
    // Pill selection
    winmgr_draw_rounded_rect_ex(win, cx + card_w - 110, cy + 16, 95, 28, 0xFF051020, 1, 0xFFFFFFFF, 14);
    winmgr_draw_text(win, cx + card_w - 95, cy + 22, s->th == 0 ? "Dark" : "Light", 0xFFFFFFFF);
    winmgr_draw_text(win, cx + card_w - 30, cy + 22, "v", 0xFFFFFFFF);
    cy += 75;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

    // Desktop Icons Card
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 60, 0xFF0A1828, 0xFF0A1828, 0xFF1A2A3A);
    winmgr_draw_text(win, cx + 15, cy + 22, "Dsktop Icons", 0xFFFFFFFF);
    // Toggle switch (pill)
    uint32_t t_bg = s->di ? 0xFF5B7FFF : 0xFF313244;
    winmgr_draw_rounded_rect_ex(win, cx + card_w - 65, cy + 18, 50, 24, t_bg, 0, 0, 12);
    int knob_x = s->di ? (cx + card_w - 65) + 28 : (cx + card_w - 65) + 4;
    winmgr_draw_rounded_rect_ex(win, knob_x, cy + 22, 18, 16, 0xFFFFFFFF, 0, 0, 8);
    cy += 75;

    // Icon Filter Card
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 60, 0xFF0A1828, 0xFF0A1828, 0xFF1A2A3A);
    winmgr_draw_text(win, cx + 15, cy + 22, "Icon Filter", 0xFFFFFFFF);
    // Pill selection
    const char *fn[] = {"None", "Red", "Green", "Yellow"};
    winmgr_draw_rounded_rect_ex(win, cx + card_w - 110, cy + 16, 95, 28, 0xFF051020, 1, 0xFFFFFFFF, 14);
    winmgr_draw_text(win, cx + card_w - 95, cy + 22, fn[s->filter], 0xFFFFFFFF);
    winmgr_draw_text(win, cx + card_w - 30, cy + 22, "v", 0xFFFFFFFF);
    cy += 75;

<<<<<<< HEAD
    int py = cy + 38, pw = 85;
    DRAW_HOVER(win, s, cx + 15, py, pw, 22, s->th == 0 ? CA : theme->button, 0xFF353545);
    winmgr_draw_text(win, cx + 33, py + 5, "Dark",
                     s->th == 0 ? 0xFFFFFFFF : theme->fg);
    DRAW_HOVER(win, s, cx + 15 + pw + 8, py, pw, 22, s->th == 1 ? CA : theme->button, 0xFF353545);
    winmgr_draw_text(win, cx + 15 + pw + 30, py + 5, "Light",
                     s->th == 1 ? 0xFFFFFFFF : theme->fg);

    cy += 85;

    // Desktop Icons Toggle
    winmgr_fill_rect(win, cx, cy, card_w, 45, theme->input_bg);
    winmgr_draw_rect(win, cx, cy, card_w, 45, theme->border);
    winmgr_draw_text(win, cx + 15, cy + 15, "Desktop Icons", theme->fg);
    // Toggle switch
    winmgr_fill_rect(win, cx + card_w - 50, cy + 12, 40, 20,
                     s->di ? CA : 0xFFCCCCCC);
    winmgr_fill_rect(win, cx + card_w - 50 + (s->di ? 22 : 2), cy + 14, 16, 16,
                     0xFFFFFFFF);

    cy += 60;
    winmgr_fill_rect(win, cx, cy, card_w, 45, theme->input_bg);
    winmgr_draw_rect(win, cx, cy, card_w, 45, theme->border);
    winmgr_draw_text(win, cx + 15, cy + 15, "Icon Filter", theme->fg);

    int fx = cx + card_w - 180;
    const char *fn[] = {"None", "Red", "Grn", "Yel"};
    for (int i = 0; i < 4; i++) {
      uint32_t bg = (s->filter == i) ? CA : theme->button;
      DRAW_HOVER(win, s, fx + i * 42, cy + 12, 38, 22, bg, 0xFF353545);
      winmgr_draw_text(win, fx + i * 42 + 4, cy + 17, fn[i],
                       (s->filter == i) ? 0xFFFFFFFF : theme->fg);
    }

    cy += 60;

    // Intensity Controls
    winmgr_draw_text(win, cx + 15, cy, "Icon Intensity", theme->fg);
    char i_per[8];
    k_itoa((s->icon_int * 100) / 255, i_per);
    strcat(i_per, "%");
    winmgr_draw_text(win, cx + 185, cy + 15, i_per, theme->fg_secondary);

    winmgr_fill_rect(win, cx + 15, cy + 15, 120, 20, theme->button);
    winmgr_fill_rect(win, cx + 15, cy + 15, (s->icon_int * 120) / 255, 20, CA);
    winmgr_draw_text(win, cx + 145, cy + 15, "-", theme->fg);
    winmgr_draw_text(win, cx + 165, cy + 15, "+", theme->fg);
=======
    // Intensity Sliders
    winmgr_draw_text(win, cx, cy, "Icon Intensity", theme->fg);
    cy += 25;
    winmgr_fill_rect(win, cx, cy, card_w, 6, 0xFF313244);
    winmgr_fill_rect(win, cx, cy, (s->icon_int * card_w) / 255, 6, theme->accent);
    winmgr_draw_rounded_rect_ex(win, cx + (s->icon_int * card_w) / 255 - 8, cy - 5, 16, 16, 0xFFFFFFFF, 0, 0, 8);
    cy += 35;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

    winmgr_draw_text(win, cx, cy, "BG Intensity", theme->fg);
    cy += 25;
    winmgr_fill_rect(win, cx, cy, card_w, 6, 0xFF313244);
    winmgr_fill_rect(win, cx, cy, (s->bg_int * card_w) / 255, 6, theme->accent);
    winmgr_draw_rounded_rect_ex(win, cx + (s->bg_int * card_w) / 255 - 8, cy - 5, 16, 16, 0xFFFFFFFF, 0, 0, 8);
    
    cy += 50;
<<<<<<< HEAD
    winmgr_draw_text(win, cx + 15, cy, "BG Intensity", theme->fg);
    char b_per[8];
    k_itoa((s->bg_int * 100) / 255, b_per);
    strcat(b_per, "%");
    winmgr_draw_text(win, cx + 185, cy + 15, b_per, theme->fg_secondary);

    winmgr_fill_rect(win, cx + 15, cy + 15, 120, 20, theme->button);
    winmgr_fill_rect(win, cx + 15, cy + 15, (s->bg_int * 120) / 255, 20, CA);
    winmgr_draw_text(win, cx + 145, cy + 15, "-", theme->fg);
    winmgr_draw_text(win, cx + 165, cy + 15, "+", theme->fg);

    cy += 60;

    // Apply button (maps exactly to mx >= CX && mx <= CX + 90 && my >= 220 &&
    // my <= 246)
    DRAW_HOVER(win, s, cx, cy + 5, 90, 26, CA, 0xFF353545);
    winmgr_draw_text(win, cx + 22, cy + 11, "Apply", 0xFFFFFFFF);
=======
    DRAW_HOVER(win, s, cx, cy, 100, 32, CA, 0xFF353545);
    winmgr_draw_text(win, cx + 25, cy + 10, "Apply", 0xFFFFFFFF);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

  } else if (s->page == PD) { // System
    winmgr_draw_text(win, cx, cy, "System", theme->fg);
    cy += 30;

<<<<<<< HEAD
=======
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 50, theme->input_bg, theme->input_bg, theme->border);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    winmgr_draw_text(win, cx + 15, cy + 12, "Display", theme->fg);
    winmgr_draw_text(win, cx + 15, cy + 28, "Active Resolution", theme->fg_secondary);

    cy += 60;
    // Resolution List
    winmgr_draw_text(win, cx, cy, "Screen Resolution", theme->fg);
    cy += 30;

    const char *res_names[] = {"800x600", "1024x768", "1280x720", "1280x800",
                               "1440x900", "1600x900", "1920x1080"};
    int res_w[] = {800, 1024, 1280, 1280, 1440, 1600, 1920};
    int res_h[] = {600, 768, 720, 800, 900, 900, 1080};

    for (int i = 0; i < 7; i++) {
      int item_y = cy + i * 35;
      if (item_y + 30 > win->height + s->scroll_y) break;

      uint32_t bg = (s->sel_res == i) ? theme->accent : theme->input_bg;
      DRAW_HOVER(win, s, cx, item_y, card_w, 30, bg, 0xFF353545);
      winmgr_draw_rect(win, cx, item_y, card_w, 30, theme->border);
      winmgr_draw_text(win, cx + 15, item_y + 8, res_names[i], 
                       (s->sel_res == i) ? 0xFFFFFFFF : theme->fg);
      
      // Mark current resolution
      if (screen_width == res_w[i] && screen_height == res_h[i]) {
        winmgr_draw_text(win, cx + card_w - 80, item_y + 8, "(Current)", theme->fg_secondary);
      }
    }

    cy += 7 * 35 + 10;
    // Apply Resolution Button
    DRAW_HOVER(win, s, cx, cy, 140, 30, theme->accent, 0xFF353545);
    winmgr_draw_text(win, cx + 25, cy + 8, "Apply Resolution", 0xFFFFFFFF);

    cy += 60;
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 50, 0xFFFAFAFA, 0xFFFAFAFA, 0xFFE0E0E0);
    winmgr_draw_text(win, cx + 15, cy + 12, "Sound", 0xFF000000);
    winmgr_draw_text(win, cx + 15, cy + 28, "ES1370 AudioPCI", 0xFF666666);

    cy += 60;
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 50, 0xFFFAFAFA, 0xFFFAFAFA, 0xFFE0E0E0);
    winmgr_draw_text(win, cx + 15, cy + 12, "Storage", 0xFF000000);
    winmgr_draw_text(win, cx + 15, cy + 28, "ATA Primary Master", 0xFF666666);
  } else if (s->page == PW) { // Widgets
    winmgr_draw_text(win, cx, cy, "Desktop Widgets", theme->fg);
    cy += 30;

    // Clock Widget
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 45, theme->input_bg, theme->input_bg, theme->border);
    winmgr_draw_text(win, cx + 15, cy + 15, "Digital Clock", theme->fg);
    winmgr_fill_rect(win, cx + card_w - 50, cy + 12, 40, 20,
                     s->show_clock ? CA : 0xFFCCCCCC);
    winmgr_fill_rect(win, cx + card_w - 50 + (s->show_clock ? 22 : 2), cy + 14,
                     16, 16, 0xFFFFFFFF);

    cy += 60;
    // Calendar Widget
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 45, theme->input_bg, theme->input_bg, theme->border);
    winmgr_draw_text(win, cx + 15, cy + 15, "Month Calendar", theme->fg);
    winmgr_fill_rect(win, cx + card_w - 50, cy + 12, 40, 20,
                     s->show_calendar ? CA : 0xFFCCCCCC);
    winmgr_fill_rect(win, cx + card_w - 50 + (s->show_calendar ? 22 : 2),
                     cy + 14, 16, 16, 0xFFFFFFFF);
    
    cy += 60;
    // SysMon Widget
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 45, theme->input_bg, theme->input_bg, theme->border);
    winmgr_draw_text(win, cx + 15, cy + 15, "System Monitor", theme->fg);
    winmgr_fill_rect(win, cx + card_w - 50, cy + 12, 40, 20,
                     s->show_sysmon ? CA : 0xFFCCCCCC);
    winmgr_fill_rect(win, cx + card_w - 50 + (s->show_sysmon ? 22 : 2),
                     cy + 14, 16, 16, 0xFFFFFFFF);

    cy += 50;
    // Timezone Offset
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 45, 0xFFFAFAFA, 0xFFFAFAFA, 0xFFE0E0E0);
    winmgr_draw_text(win, cx + 15, cy + 15, "Timezone Offset", 0xFF333333);

    char tz_str[16];
    int h = s->tz / 60;
    int m = (s->tz >= 0 ? s->tz : -s->tz) % 60;
    strcpy(tz_str, "UTC");
    if (s->tz >= 0)
      strcat(tz_str, "+");
    else
      strcat(tz_str, "-");
    char hs[4];
    k_itoa(h >= 0 ? h : -h, hs);
    strcat(tz_str, hs);
    strcat(tz_str, ":");
    if (m < 10)
      strcat(tz_str, "0");
    char ms[4];
    k_itoa(m, ms);
    strcat(tz_str, ms);

    winmgr_draw_text(win, cx + card_w - 140, cy + 15, tz_str, 0xFF666666);
    DRAW_HOVER(win, s, cx + card_w - 50, cy + 10, 20, 25, 0xFFE0E0E0, 0xFF353545);
    winmgr_draw_text(win, cx + card_w - 44, cy + 16, "-", 0xFF333333);
    DRAW_HOVER(win, s, cx + card_w - 25, cy + 10, 20, 25, 0xFFE0E0E0, 0xFF353545);
    winmgr_draw_text(win, cx + card_w - 18, cy + 16, "+", 0xFF333333);

    cy += 60;
    // Reset Positions Button
    DRAW_HOVER(win, s, cx, cy, 140, 26, 0xFFE0E0E0, 0xFF353545);
    winmgr_draw_rect(win, cx, cy, 140, 26, 0xFFCCCCCC);
    winmgr_draw_text(win, cx + 15, cy + 6, "Reset Positions", 0xFF333333);

    cy += 50;
    // Apply button
    DRAW_HOVER(win, s, cx, cy + 5, 90, 26, CA, 0xFF353545);
    winmgr_draw_text(win, cx + 22, cy + 11, "Apply", 0xFFFFFFFF);
  }
}

static void apply_config(sstate_t *s) {
  strcpy(global_config.lock_password, s->pw);
  global_config.wallpaper_type = s->wp;
  global_config.theme_mode = s->th;
  global_config.show_desktop_icons = s->di;
  global_config.icon_filter = s->filter;
  global_config.icon_filter_intensity = s->icon_int;
  global_config.icon_bg_filter_intensity = s->bg_int;
  global_config.show_clock_widget = s->show_clock;
  global_config.show_calendar_widget = s->show_calendar;
  global_config.show_sysmon_widget = s->show_sysmon;
  global_config.timezone_offset_m = s->tz;
  global_config.auto_hide_taskbar = s->auto_hide_taskbar;
<<<<<<< HEAD
=======

  extern void winmgr_toggle_sysmon(int);
  winmgr_toggle_sysmon(s->show_sysmon);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
 
  theme_set_mode(s->th);

  global_config.num_pinned = s->num_pinned;
  for (int i = 0; i < s->num_pinned; i++) {
    global_config.pinned[i] = s->pinned[i];
  }

  config_save();
  for (int i = 0; i < window_count; i++) {
    if (windows[i].id != 0) {
      windows[i].needs_redraw = 1;
    }
  }
  desktop_invalidate();
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
  print_serial("SETTINGS: Configuration applied (theme updated).\n");
}

static void settings_on_mouse(void *w, int mx, int my, int buttons) {
  window_t *win = (window_t *)w;
  sstate_t *s = (sstate_t *)win->user_data;
  if (!s) return;

  int rx = mx;
  int ry = my;
  static int lb = 0;
  int click = (buttons & 1) && !(lb & 1);
  lb = buttons;

  if (s->mx != rx || s->my != ry) {
    s->mx = rx;
    s->my = ry;
<<<<<<< HEAD
    win->needs_redraw = 1;
  }

  if (click) {
    print_serial("SETTINGS: Click at ");
    char b[12];
    k_itoa(mx, b);
    print_serial(b);
    print_serial(",");
    k_itoa(my, b);
    print_serial(b);
    print_serial("\n");
  }

  // 1. Sidebar handling (BEFORE scroll compensation)
  int nh = (rx < SW && ry >= 40) ? (ry - 40) / 36 : -1;
  if (nh >= NP)
    nh = -1;
  if (nh != s->hover) {
    s->hover = nh;
=======
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    win->needs_redraw = 1;
  }

  // 1. Sidebar handling (Premium Dark Sidebar)
  if (click && rx < SW && ry >= 32) {
    int nh = (ry - 32) / 50;
    if (nh >= 0 && nh < NP) {
      s->page = nh;
      s->scroll_y = 0;
      win->needs_redraw = 1;
      return;
    }
  }

  // Handle dragging release
  if (!(buttons & 1)) {
    s->dragging_icon = 0;
    s->dragging_bg = 0;
  }

  int cw = win->width - CX - 20;

  // Personalization Sliders (Needs rx/ry before scroll compensation)
  if (s->page == PB) {
    int cy_base = 40;
    int slider_y = cy_base + 40 + 75*4 - s->scroll_y;
    if (s->dragging_icon) {
      int val = (rx - CX) * 255 / cw;
      if (val < 0) val = 0; if (val > 255) val = 255;
      s->icon_int = val; win->needs_redraw = 1;
    } else if (s->dragging_bg) {
      int val = (rx - CX) * 255 / cw;
      if (val < 0) val = 0; if (val > 255) val = 255;
      s->bg_int = val; win->needs_redraw = 1;
    }
  }

  if (!click && !s->dragging_icon && !s->dragging_bg) return;

  // 2. Content area compensation (Scroll)
  ry += s->scroll_y;

  // Personalization Page (PB)
  if (s->page == PB) {
    int cy = 40 + 40; // Header + Header offset
    
    // Background button pill
    if (click && rx >= CX + cw - 110 && rx <= CX + cw - 15 && ry >= cy + 16 && ry <= cy + 44) {
      s->wp = (s->wp + 1) % 3; win->needs_redraw = 1;
    }
    cy += 75;

    // Color Mode button pill
    if (click && rx >= CX + cw - 110 && rx <= CX + cw - 15 && ry >= cy + 16 && ry <= cy + 44) {
      s->th = !s->th; win->needs_redraw = 1;
    }
    cy += 75;

    // Desktop Icons Toggle pill
    if (click && rx >= CX + cw - 65 && rx <= CX + cw - 15 && ry >= cy + 18 && ry <= cy + 42) {
      s->di = !s->di; win->needs_redraw = 1;
    }
    cy += 75;

    // Icon Filter button pill
    if (click && rx >= CX + cw - 110 && rx <= CX + cw - 15 && ry >= cy + 16 && ry <= cy + 44) {
      s->filter = (s->filter + 1) % 4; win->needs_redraw = 1;
    }
    cy += 75;

    // Intensity Sliders
    // Icon Intensity
    if (click && rx >= CX && rx <= CX + cw && ry >= cy + 25 && ry <= cy + 31) {
      s->dragging_icon = 1;
    }
    cy += 25 + 35; // Slider 1 height + spacing

    // BG Intensity
    if (click && rx >= CX && rx <= CX + cw && ry >= cy + 25 && ry <= cy + 31) {
      s->dragging_bg = 1;
    }
    cy += 25 + 50; // Slider 2 height + spacing

    // Apply button
    if (click && rx >= CX && rx <= CX + 100 && ry >= cy && ry <= cy + 32) {
      apply_config(s); win->needs_redraw = 1;
    }
  }

  // Taskbar settings (PT)
  if (s->page == PT) {
<<<<<<< HEAD
    int cy = 70;
    int card_w = win->width - CX - 20;

    // Auto-hide
    if (click && mx >= CX + card_w - 50 && mx <= CX + card_w - 10 &&
        ry >= cy + 12 && ry <= cy + 32) {
      s->auto_hide_taskbar = !s->auto_hide_taskbar;
      win->needs_redraw = 1;
    }
    cy += 60;
    cy += 30; // "Taskbar Apps" header
=======
    int cy = 40 + 30; // Header + Offset
    // Auto-hide
    if (click && rx >= CX + cw - 50 && rx <= CX + cw - 10 && ry >= cy + 12 && ry <= cy + 32) {
      s->auto_hide_taskbar = !s->auto_hide_taskbar;
      win->needs_redraw = 1;
    }
    cy += 60 + 30; // "Taskbar Apps" header offset
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

    for (int i = 0; i < 10; i++) {
      if (click && rx >= CX + cw - 50 && rx <= CX + cw - 10 && ry >= cy + 5 && ry <= cy + 25) {
        // Toggle pin state
        int is_pinned = 0;
        int pin_idx = -1;
        for (int j = 0; j < s->num_pinned; j++) {
          if (s->pinned[j] == i) {
            is_pinned = 1;
            pin_idx = j;
            break;
          }
        }
        if (is_pinned) {
          // Remove
          for (int j = pin_idx; j < s->num_pinned - 1; j++) {
            s->pinned[j] = s->pinned[j + 1];
          }
          s->num_pinned--;
        } else {
          if (s->num_pinned < 10) {
            s->pinned[s->num_pinned++] = i;
          }
        }
        win->needs_redraw = 1;
<<<<<<< HEAD
      } // Removed break, because doing an action shouldn't skip further tests without 'return'
=======
      }
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      cy += 35;
    }
    
    // Apply button
    if (click && rx >= CX && rx <= CX + 100 && ry >= cy + 10 && ry <= cy + 40) {
      apply_config(s);
      win->needs_redraw = 1;
    }
  }

  // Account settings (PC)
  if (s->page == PC) {
    int cy = 40 + 30 + 80; // Past header and profile card
    // Save button
    if (click && rx >= CX + 15 && rx <= CX + 95 && ry >= cy + 75 && ry <= cy + 100) {
      strcpy(global_config.lock_password, s->pw);
      config_save();
      win->needs_redraw = 1;
    }
  }

<<<<<<< HEAD
  // System settings (Resolution)
  if (s->page == PD) {
    int res_cy = 40 + 30 + 60 + 30; // Matches settings_draw for PD
    int card_w = win->width - CX - 20;

    for (int i = 0; i < 7; i++) {
        int item_y = res_cy + i * 35;
        if (click && rx >= CX && rx <= CX + card_w && ry >= item_y && ry <= item_y + 30) {
            s->sel_res = i;
            win->needs_redraw = 1;
        }
    }

    int apply_res_y = res_cy + 7 * 35 + 10;
    if (click && rx >= CX && rx <= CX + 140 && ry >= apply_res_y && ry <= apply_res_y + 30) {
        int res_w[] = {800, 1024, 1280, 1280, 1440, 1600, 1920};
        int res_h[] = {600, 768, 720, 800, 900, 900, 1080};
        extern void screen_set_resolution(int, int);
        screen_set_resolution(res_w[s->sel_res], res_h[s->sel_res]);
        win->needs_redraw = 1;
=======
  // Widget settings (PW)
  if (s->page == PW) {
    int cy = 40 + 30;
    // Clock Toggle
    if (click && rx >= CX + cw - 50 && rx <= CX + cw - 10 && ry >= cy + 12 && ry <= cy + 32) {
      s->show_clock = !s->show_clock;
      win->needs_redraw = 1;
    }
    cy += 60;
    // Calendar Toggle
    if (click && rx >= CX + cw - 50 && rx <= CX + cw - 10 && ry >= cy + 12 && ry <= cy + 32) {
      s->show_calendar = !s->show_calendar;
      win->needs_redraw = 1;
    }
    cy += 60;
    // SysMon Toggle
    if (click && rx >= CX + cw - 50 && rx <= CX + cw - 10 && ry >= cy + 12 && ry <= cy + 32) {
      s->show_sysmon = !s->show_sysmon;
      win->needs_redraw = 1;
    }
    cy += 50;
    // Timezone Offset Buttons
    if (click && ry >= cy + 10 && ry <= cy + 35) {
      if (rx >= CX + cw - 50 && rx <= CX + cw - 30) {
        s->tz -= 30;
        win->needs_redraw = 1;
      }
      if (rx >= CX + cw - 25 && rx <= CX + cw - 5) {
        s->tz += 30;
        win->needs_redraw = 1;
      }
    }
    cy += 60;
    // Reset Positions
    if (click && rx >= CX && rx <= CX + 140 && ry >= cy && ry <= cy + 26) {
      global_config.clock_x = 744;
      global_config.clock_y = 60;
      global_config.clock_w = 200;
      global_config.clock_h = 200;
      global_config.calendar_x = 734;
      global_config.calendar_y = 280;
      global_config.calendar_w = 220;
      global_config.calendar_h = 240;
      apply_config(s);
      win->needs_redraw = 1;
    }
    cy += 50;
    // Apply button
    if (click && rx >= CX && rx <= CX + 100 && ry >= cy + 5 && ry <= cy + 31) {
      apply_config(s);
      win->needs_redraw = 1;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    }
  }
}

static void settings_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  sstate_t *s = (sstate_t *)win->user_data;
  if (!s)
    return;

  // Wheel UP (direction > 0) -> scroll viewport UP -> decrease scroll_y
  s->scroll_y -= direction * 20;

  if (s->scroll_y < 0)
    s->scroll_y = 0;

  // Add a reasonable bound
  if (s->scroll_y > 800)
    s->scroll_y = 800;

  win->needs_redraw = 1;
  extern int ui_dirty;
  ui_dirty = 1;
}

static void settings_on_key(void *w, int key, char ascii) {
  window_t *win = (window_t *)w;
  sstate_t *s = (sstate_t *)win->user_data;
  (void)key;
  if (s->page != PC)
    return;
  if (ascii == '\b') {
    if (s->pwc > 0) {
      s->pw[--s->pwc] = 0;
      win->needs_redraw = 1;
    }
  } else if (ascii >= ' ' && ascii <= '~' && s->pwc < 31) {
    s->pw[s->pwc++] = ascii;
    s->pw[s->pwc] = 0;
    win->needs_redraw = 1;
  }
}

static void settings_on_close(void *w) {
  window_t *win = (window_t *)w;
  if (win->user_data) {
    kfree(win->user_data);
    win->user_data = 0;
  }
}

void settings_init() {
  window_t *win = winmgr_create_window(-1, -1, 800, 600, "Settings");
  if (!win)
    return;
  sstate_t *st = (sstate_t *)kmalloc(sizeof(sstate_t));
  st->page = PA;
  st->hover = -1;
  st->scroll_y = 0;
  strcpy(st->pw, global_config.lock_password);
  st->pwc = strlen(st->pw);
  st->wp = global_config.wallpaper_type;
  st->th = global_config.theme_mode;
  st->di = global_config.show_desktop_icons;
  st->filter = global_config.icon_filter;
  st->icon_int = global_config.icon_filter_intensity;
  st->di = global_config.show_desktop_icons;
  st->show_clock = global_config.show_clock_widget;
  st->show_calendar = global_config.show_calendar_widget;
  st->tz = global_config.timezone_offset_m;
  st->filter = global_config.icon_filter;
  st->bg_int = global_config.icon_bg_filter_intensity;
  st->auto_hide_taskbar = global_config.auto_hide_taskbar;
<<<<<<< HEAD
=======
  st->show_sysmon = global_config.show_sysmon_widget;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  st->dragging_icon = 0;
  st->dragging_bg = 0;
  st->num_pinned = global_config.num_pinned;
  for (int i = 0; i < global_config.num_pinned; i++) {
    st->pinned[i] = global_config.pinned[i];
  }

  // Initialize selected resolution to current
  st->sel_res = 1; // Default 1024x768
  int res_w[] = {800, 1024, 1280, 1280, 1440, 1600, 1920};
  int res_h[] = {600, 768, 720, 800, 900, 900, 1080};
  for (int i = 0; i < 7; i++) {
    if (screen_width == res_w[i] && screen_height == res_h[i]) {
      st->sel_res = i;
      break;
    }
  }

  win->user_data = st;

  win->draw = settings_draw;
  win->on_mouse = settings_on_mouse;
  win->on_key = settings_on_key;
  win->on_scroll = settings_on_scroll;
  win->on_close = settings_on_close;
  win->app_type = 9;

  win->bg_color = 0xFFFFFFFF;
  win->blur_enabled = 0; // Opaque
  win->style = (window_style_t){0xFFF0F0F0, 0, 0xFFD0D0D0, 0xFFF0F0F0, 0xFFFFFFFF};
}
