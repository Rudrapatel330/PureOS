#include "startmenu.h"
#include "../drivers/vga.h"
#include "../kernel/anim.h"
#include "../kernel/screen.h"
#include "../kernel/string.h"

#define MENU_W 460
#define MENU_H 540
#define PADDING 20
#define SEARCH_H 36
#define ICON_SIZE 32
#define GRID_COLS 5
#define CELL_W ((MENU_W - (PADDING * 2)) / GRID_COLS)
#define CELL_H 75

// Colors - Modern dark frosted glass palette
#define COL_BG 0xFF1E1E2E
#define COL_BG2 0xFF252535
#define COL_SEARCH 0xFF2A2A3C
#define COL_HOVER 0xFF333348
#define COL_ACCENT 0xFF0078D4
#define COL_TEXT 0xFFE0E0E0
#define COL_DIM 0xFF888898
#define COL_DIVIDER 0xFF333344
#define COL_FOOTER 0xFF1A1A28

static int menu_active = 0;
static int menu_closing = 0;
static animation_t anim_y;
static int menu_x = 0;
static uint32_t *startmenu_surface = 0;

typedef struct {
  const char *label;
  uint32_t icon_col;
  void (*launch)();
} menu_item_t;

extern void terminal_init();
extern void editor_init();
extern void calculator_init();
extern void paint_init();
extern void explorer_init();
extern void settings_init();
extern void taskmgr_init();
extern void browser_init();
extern void videoplayer_init(const char *path);
extern void lockscreen_show();
extern void pdfreader_init();
extern void photos_init();
extern void mail_app_init();

// --- Launch wrappers ---
static void launch_term() {
  terminal_init();
  menu_active = 0;
}
static void launch_editor() {
  editor_init();
  menu_active = 0;
}
static void launch_calc() {
  calculator_init();
  menu_active = 0;
}
static void launch_paint() {
  paint_init();
  menu_active = 0;
}
static void launch_files() {
  explorer_init();
  menu_active = 0;
}
static void launch_settings() {
  settings_init();
  menu_active = 0;
}
static void launch_taskmgr() {
  taskmgr_init();
  menu_active = 0;
}
static void launch_browser() {
  browser_init();
  menu_active = 0;
}
static void launch_video() {
  videoplayer_init(0);
  menu_active = 0;
}
static void launch_lock() {
  menu_active = 0;
  lockscreen_show();
}
static void launch_pdfreader() {
  pdfreader_init();
  menu_active = 0;
}
static void launch_photos() {
  photos_init();
  menu_active = 0;
}
static void launch_mail() {
  mail_app_init();
  menu_active = 0;
}

// Pinned apps (shown in grid)
static menu_item_t pinned_items[] = {
    {"Terminal", 0xFF38B764, launch_term},
    {"Editor", 0xFF5B8BF5, launch_editor},
    {"Calculator", 0xFF00ADEF, launch_calc},
    {"Paint", 0xFFFF6B6B, launch_paint},
    {"Explorer", 0xFFFFC107, launch_files},
    {"Settings", 0xFF607D8B, launch_settings},
    {"Browser", 0xFF4FC3F7, launch_browser},
    {"Video", 0xFFE040FB, launch_video},
    {"Task Mgr", 0xFF66BB6A, launch_taskmgr},
    {"Lock", 0xFFE53935, launch_lock},
    {"PDF Reader", 0xFFCC3333, launch_pdfreader},
    {"Photos", 0xFFE91E63, launch_photos},
    {"Mail", 0xFF00AADD, launch_mail}};
#define PINNED_COUNT (sizeof(pinned_items) / sizeof(pinned_items[0]))

// All apps alphabetical
static menu_item_t all_apps_items[] = {
    {"Browser", 0xFF4FC3F7, launch_browser},
    {"Calculator", 0xFF00ADEF, launch_calc},
    {"Editor", 0xFF5B8BF5, launch_editor},
    {"Explorer", 0xFFFFC107, launch_files},
    {"Lock", 0xFFE53935, launch_lock},
    {"Mail", 0xFF00AADD, launch_mail},
    {"Paint", 0xFFFF6B6B, launch_paint},
    {"PDF Reader", 0xFFCC3333, launch_pdfreader},
    {"Photos", 0xFFE91E63, launch_photos},
    {"Settings", 0xFF607D8B, launch_settings},
    {"Task Mgr", 0xFF66BB6A, launch_taskmgr},
    {"Terminal", 0xFF38B764, launch_term},
    {"Video", 0xFFE040FB, launch_video}};
#define ALL_APPS_COUNT (sizeof(all_apps_items) / sizeof(all_apps_items[0]))

static int startmenu_view_mode = 0;

extern void print_serial(const char *);
extern void compositor_invalidate_rect(int x, int y, int w, int h);
extern int screen_width, screen_height;
extern void *kmalloc(uint32_t size);
static void startmenu_render_to_cache();

static void menu_close_complete(void *data) {
  (void)data;
  menu_active = 0;
  menu_closing = 0;
  extern int ui_dirty;
  ui_dirty = 1;
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
}

void startmenu_show(int x, int y) {
  (void)x;
  (void)y;
  extern int ui_dirty;
  float y_closed = (float)screen_height;
  float y_open = (float)(screen_height - 62 - MENU_H - 10);

  if (!startmenu_surface) {
    startmenu_surface = (uint32_t *)kmalloc(MENU_W * MENU_H * 4);
  }

  if (!menu_active && !menu_closing) {
    menu_active = 1;
    menu_closing = 0;
    menu_x = (screen_width - MENU_W) / 2;
    startmenu_render_to_cache();
    anim_start(&anim_y, y_closed, y_open, 0.4f, EASE_IN_OUT_CUBIC);
  } else {
    menu_closing = 1;
    float cur_y = anim_y.active ? anim_y.current_val : y_open;
    anim_start(&anim_y, cur_y, y_closed, 0.35f, EASE_IN_OUT_CUBIC);
    anim_y.on_complete = menu_close_complete;
  }

  ui_dirty = 1;
  int iy = (int)y_open;
  int ih = (int)(screen_height - y_open);
  compositor_invalidate_rect(menu_x - 5, iy - 5, MENU_W + 10, ih + 10);
}

int startmenu_is_active() { return menu_active || menu_closing; }
int startmenu_is_animating() { return anim_y.active; }

void startmenu_tick_animation(float dt) {
  if (anim_y.active) {
    int old_y = (int)anim_y.current_val;
    anim_tick(&anim_y, dt);
    int new_y = (int)anim_y.current_val;
    int start_y = (old_y < new_y) ? old_y : new_y;
    int end_y = (old_y > new_y) ? old_y : new_y;
    int h = (end_y - start_y) + MENU_H;
    compositor_invalidate_rect(menu_x - 2, start_y - 2, MENU_W + 4, h + 4);
    extern int ui_dirty;
    ui_dirty = 1;
  }
}

// Draw stylized app icons
static void draw_app_icon(int x, int y, const char *name, uint32_t col,
                          uint32_t *buffer) {
  // Rounded colored square background
  vga_draw_rect_surface(x, y, 32, 32, col, buffer, MENU_W, MENU_H);
  // Inner rounded highlight (top 2px lighter)
  uint32_t hi = 0x30FFFFFF;
  vga_draw_rect_surface(
      x + 2, y + 1, 28, 2,
      (col & 0xFF000000) |
          ((((col >> 16) & 0xFF) + 30 > 255 ? 255 : (((col >> 16) & 0xFF) + 30))
           << 16) |
          ((((col >> 8) & 0xFF) + 30 > 255 ? 255 : (((col >> 8) & 0xFF) + 30))
           << 8) |
          (((col & 0xFF) + 30 > 255 ? 255 : ((col & 0xFF) + 30))),
      buffer, MENU_W, MENU_H);
  (void)hi;

  if (strcmp(name, "Terminal") == 0) {
    vga_draw_string_surface(x + 6, y + 8, ">_", 0xFFFFFFFF, buffer, MENU_W,
                            MENU_H);
  } else if (strcmp(name, "Explorer") == 0) {
    vga_draw_rect_surface(x + 4, y + 12, 24, 16, 0xFFFFF176, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 4, y + 8, 10, 4, 0xFFFFF176, buffer, MENU_W,
                          MENU_H);
  } else if (strcmp(name, "Calculator") == 0) {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        vga_draw_rect_surface(x + 6 + j * 8, y + 6 + i * 8, 5, 5, 0xFFFFFFFF,
                              buffer, MENU_W, MENU_H);
  } else if (strcmp(name, "Editor") == 0) {
    vga_draw_rect_surface(x + 6, y + 8, 20, 2, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 6, y + 14, 16, 2, 0xFFCCCCCC, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 6, y + 20, 18, 2, 0xFFCCCCCC, buffer, MENU_W,
                          MENU_H);
  } else if (strcmp(name, "Settings") == 0) {
    vga_draw_rect_surface(x + 10, y + 10, 12, 12, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 13, y + 13, 6, 6, col, buffer, MENU_W, MENU_H);
  } else if (strcmp(name, "Lock") == 0) {
    vga_draw_rect_surface(x + 13, y + 10, 6, 6, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 14, y + 16, 4, 6, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
  } else if (strcmp(name, "Paint") == 0) {
    vga_draw_rect_surface(x + 8, y + 10, 4, 4, 0xFFFF0000, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 16, y + 6, 4, 4, 0xFF00FF00, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 22, y + 14, 4, 4, 0xFF0088FF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 12, y + 18, 8, 8, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
  } else if (strcmp(name, "Browser") == 0) {
    vga_draw_rect_surface(x + 8, y + 8, 16, 16, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 10, y + 12, 12, 4, col, buffer, MENU_W, MENU_H);
    vga_draw_rect_surface(x + 15, y + 8, 2, 16, col, buffer, MENU_W, MENU_H);
  } else if (strcmp(name, "Video") == 0) {
    // Play triangle
    vga_draw_rect_surface(x + 10, y + 8, 3, 16, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 13, y + 10, 3, 12, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 16, y + 12, 3, 8, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 19, y + 14, 3, 4, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
  } else if (strcmp(name, "Task Mgr") == 0) {
    // Bar chart
    vga_draw_rect_surface(x + 7, y + 18, 5, 10, 0xFF00CC44, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 14, y + 12, 5, 16, 0xFF00AA33, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 21, y + 8, 5, 20, 0xFF44FF66, buffer, MENU_W,
                          MENU_H);
  } else if (strcmp(name, "Photos") == 0) {
    // Landscape hint
    vga_draw_rect_surface(x + 6, y + 6, 20, 14, 0xFF00BCD4, buffer, MENU_W,
                          MENU_H); // Sky
    vga_draw_rect_surface(x + 20, y + 8, 4, 4, 0xFFFFEB3B, buffer, MENU_W,
                          MENU_H); // Sun
    vga_draw_rect_surface(x + 6, y + 14, 10, 6, 0xFF4CAF50, buffer, MENU_W,
                          MENU_H); // Grass/Hill
  } else if (strcmp(name, "Mail") == 0) {
    // Envelope icon
    vga_draw_rect_surface(x + 6, y + 10, 20, 12, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    for (int i = 0; i < 10; i++) {
      vga_draw_rect_surface(x + 6 + i, y + 10 + i, 2, 1, 0xFFCCCCCC, buffer,
                            MENU_W, MENU_H);
      vga_draw_rect_surface(x + 24 - i, y + 10 + i, 2, 1, 0xFFCCCCCC, buffer,
                            MENU_W, MENU_H);
    }
  }
}

static void startmenu_render_to_cache() {
  if (!startmenu_surface)
    return;

  uint32_t *buffer = startmenu_surface;
  int radius = 12;

  // Clear to transparent
  for (int i = 0; i < MENU_W * MENU_H; i++)
    buffer[i] = 0x00000000;

  // Draw rounded rect body with dark glass background
  for (int y = 0; y < MENU_H; y++) {
    for (int x = 0; x < MENU_W; x++) {
      int draw = 1;
      // Rounded corners
      if (x < radius && y < radius) {
        int dx = radius - x - 1, dy = radius - y - 1;
        if (dx * dx + dy * dy >= radius * radius)
          draw = 0;
      } else if (x >= MENU_W - radius && y < radius) {
        int dx = x - (MENU_W - radius), dy = radius - y - 1;
        if (dx * dx + dy * dy >= radius * radius)
          draw = 0;
      } else if (x < radius && y >= MENU_H - radius) {
        int dx = radius - x - 1, dy = y - (MENU_H - radius);
        if (dx * dx + dy * dy >= radius * radius)
          draw = 0;
      } else if (x >= MENU_W - radius && y >= MENU_H - radius) {
        int dx = x - (MENU_W - radius), dy = y - (MENU_H - radius);
        if (dx * dx + dy * dy >= radius * radius)
          draw = 0;
      }
      if (draw)
        buffer[y * MENU_W + x] = COL_BG;
    }
  }

  // === SEARCH BAR ===
  int sy = PADDING;
  int sx = PADDING;
  int sw = MENU_W - PADDING * 2;
  vga_draw_rect_surface(sx, sy, sw, SEARCH_H, COL_SEARCH, buffer, MENU_W,
                        MENU_H);
  // Search icon (magnifying glass hint)
  vga_draw_string_surface(sx + 12, sy + 12, "Search apps...", COL_DIM, buffer,
                          MENU_W, MENU_H);
  // Bottom accent line
  vga_draw_rect_surface(sx + 4, sy + SEARCH_H - 2, sw - 8, 2, COL_ACCENT,
                        buffer, MENU_W, MENU_H);

  if (startmenu_view_mode == 0) {
    // ====== PINNED VIEW ======
    int head_y = sy + SEARCH_H + 20;
    vga_draw_string_surface(PADDING, head_y, "Pinned", COL_TEXT, buffer, MENU_W,
                            MENU_H);

    // "All apps >" link
    vga_draw_string_surface(MENU_W - PADDING - 80, head_y, "All apps >",
                            COL_ACCENT, buffer, MENU_W, MENU_H);

    // Pinned Grid
    int grid_y = head_y + 24;
    for (int i = 0; i < (int)PINNED_COUNT; i++) {
      int col = i % GRID_COLS;
      int row = i / GRID_COLS;
      int ix = PADDING + (col * CELL_W) + (CELL_W - ICON_SIZE) / 2;
      int iy = grid_y + row * CELL_H;

      draw_app_icon(ix, iy, pinned_items[i].label, pinned_items[i].icon_col,
                    buffer);

      // Center label below icon
      int lbl_len = 0;
      const char *lbl = pinned_items[i].label;
      while (lbl[lbl_len])
        lbl_len++;
      int lbl_x = PADDING + col * CELL_W + (CELL_W - lbl_len * 8) / 2;
      vga_draw_string_surface(lbl_x, iy + 38, lbl, COL_DIM, buffer, MENU_W,
                              MENU_H);
    }

    // === RECOMMENDED ===
    int rows_used = ((int)PINNED_COUNT + GRID_COLS - 1) / GRID_COLS;
    int rec_y = grid_y + rows_used * CELL_H + 10;
    // Divider
    vga_draw_rect_surface(PADDING, rec_y, MENU_W - PADDING * 2, 1, COL_DIVIDER,
                          buffer, MENU_W, MENU_H);
    rec_y += 12;
    vga_draw_string_surface(PADDING, rec_y, "Recommended", COL_TEXT, buffer,
                            MENU_W, MENU_H);
    rec_y += 24;
    vga_draw_string_surface(PADDING + 4, rec_y, "Recent files will appear here",
                            COL_DIM, buffer, MENU_W, MENU_H);

  } else {
    // ====== ALL APPS VIEW ======
    int head_y = sy + SEARCH_H + 20;
    vga_draw_string_surface(PADDING, head_y, "< Back", COL_ACCENT, buffer,
                            MENU_W, MENU_H);
    vga_draw_string_surface(MENU_W - PADDING - 64, head_y, "All Apps", COL_TEXT,
                            buffer, MENU_W, MENU_H);

    int list_y = head_y + 30;
    char cur_letter = '\0';

    for (int i = 0; i < (int)ALL_APPS_COUNT; i++) {
      char first = all_apps_items[i].label[0];
      if (first != cur_letter && cur_letter != '\0')
        list_y += 8; // Group spacing

      if (first != cur_letter) {
        cur_letter = first;
        char ls[2] = {cur_letter, 0};
        vga_draw_string_surface(PADDING + 8, list_y + 8, ls, COL_ACCENT, buffer,
                                MENU_W, MENU_H);
      }

      // Icon + label for this app
      draw_app_icon(PADDING + 36, list_y, all_apps_items[i].label,
                    all_apps_items[i].icon_col, buffer);
      vga_draw_string_surface(PADDING + 76, list_y + 10,
                              all_apps_items[i].label, COL_TEXT, buffer, MENU_W,
                              MENU_H);

      list_y += 40;
    }
  }

  // === FOOTER BAR ===
  int footer_y = MENU_H - 52;
  // Divider line
  vga_draw_rect_surface(PADDING, footer_y, MENU_W - PADDING * 2, 1, COL_DIVIDER,
                        buffer, MENU_W, MENU_H);
  footer_y += 8;

  // Footer background (keep the rounded corners intact)
  for (int fy = footer_y; fy < MENU_H; fy++) {
    for (int fx = 0; fx < MENU_W; fx++) {
      if (buffer[fy * MENU_W + fx] != 0x00000000)
        buffer[fy * MENU_W + fx] = COL_FOOTER;
    }
  }

  // User avatar
  vga_draw_rect_surface(PADDING + 4, footer_y + 6, 28, 28, COL_ACCENT, buffer,
                        MENU_W, MENU_H);
  vga_draw_string_surface(PADDING + 12, footer_y + 14, "R", 0xFFFFFFFF, buffer,
                          MENU_W, MENU_H);
  vga_draw_string_surface(PADDING + 40, footer_y + 14, "Rudra Patel", COL_TEXT,
                          buffer, MENU_W, MENU_H);

  // Power icon (right side)
  vga_draw_rect_surface(MENU_W - PADDING - 28, footer_y + 10, 24, 24,
                        0xFF444455, buffer, MENU_W, MENU_H);
  vga_draw_string_surface(MENU_W - PADDING - 22, footer_y + 16, "O", 0xFFFF4444,
                          buffer, MENU_W, MENU_H);
}

void startmenu_draw(uint32_t *buffer, rect_t clip) {
  if (!startmenu_is_active() || !startmenu_surface)
    return;

  int cy = (int)anim_y.current_val;
  int start_y = (cy > clip.y) ? cy : clip.y;
  int end_y =
      (cy + MENU_H < clip.y + clip.h) ? (cy + MENU_H) : (clip.y + clip.h);

  if (start_y >= end_y)
    return;

  for (int y = start_y; y < end_y; y++) {
    int src_y = y - cy;
    if (src_y < 0 || src_y >= MENU_H)
      continue;

    int dst_offset = y * screen_width + menu_x;
    int src_offset = src_y * MENU_W;

    // Alpha-aware blit for rounded corners
    for (int x = 0; x < MENU_W; x++) {
      uint32_t px = startmenu_surface[src_offset + x];
      if ((px >> 24) > 0)
        buffer[dst_offset + x] = px;
    }
  }
}

int startmenu_handle_mouse(int mx, int my, int buttons) {
  static int sm_last_btn = 0;
  int down_edge = (buttons & 1) && !(sm_last_btn & 1);
  sm_last_btn = buttons;
  if (!startmenu_is_active() || menu_closing)
    return 0;

  int cy = (int)anim_y.current_val;
  if (mx < menu_x || mx >= menu_x + MENU_W || my < cy || my >= cy + MENU_H) {
    if (down_edge) {
      startmenu_show(menu_x, screen_height);
      return 1;
    }
    return 0;
  }

  if (!down_edge)
    return 1;

  // Convert to local menu coordinates
  int lx = mx - menu_x;
  int ly = my - cy;

  int search_bottom = PADDING + SEARCH_H;
  int head_y = search_bottom + 20;

  if (startmenu_view_mode == 0) {
    // --- PINNED VIEW ---

    // "All apps >" click
    if (lx >= MENU_W - PADDING - 80 && lx <= MENU_W - PADDING &&
        ly >= head_y - 4 && ly <= head_y + 14) {
      startmenu_view_mode = 1;
      startmenu_render_to_cache();
      compositor_invalidate_rect(menu_x, cy, MENU_W, MENU_H);
      return 1;
    }

    // Hit test pinned grid
    int grid_y = head_y + 24;
    for (int i = 0; i < (int)PINNED_COUNT; i++) {
      int col = i % GRID_COLS;
      int row = i / GRID_COLS;
      int ix = PADDING + col * CELL_W;
      int iy = grid_y + row * CELL_H;

      // Click anywhere in the cell
      if (lx >= ix && lx < ix + CELL_W && ly >= iy && ly < iy + CELL_H) {
        if (pinned_items[i].launch) {
          pinned_items[i].launch();
          startmenu_show(0, 0);
        }
        return 1;
      }
    }
  } else {
    // --- ALL APPS VIEW ---

    // "< Back" click
    if (lx >= PADDING && lx <= PADDING + 60 && ly >= head_y - 4 &&
        ly <= head_y + 14) {
      startmenu_view_mode = 0;
      startmenu_render_to_cache();
      compositor_invalidate_rect(menu_x, cy, MENU_W, MENU_H);
      return 1;
    }

    // App list hit testing
    int list_y = head_y + 30;
    char cur_letter = '\0';
    for (int i = 0; i < (int)ALL_APPS_COUNT; i++) {
      char first = all_apps_items[i].label[0];
      if (first != cur_letter && cur_letter != '\0')
        list_y += 8;
      if (first != cur_letter)
        cur_letter = first;

      if (ly >= list_y && ly < list_y + 40 && lx >= PADDING &&
          lx <= MENU_W - PADDING) {
        if (all_apps_items[i].launch) {
          all_apps_items[i].launch();
          startmenu_show(0, 0);
        }
        return 1;
      }
      list_y += 40;
    }
  }

  return 1;
}

int startmenu_get_x() { return menu_x; }
int startmenu_get_y() { return (int)anim_y.current_val; }
int startmenu_get_w() { return MENU_W; }
int startmenu_get_h() { return MENU_H; }
