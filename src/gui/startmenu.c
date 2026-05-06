#include "startmenu.h"
#include "../drivers/vga.h"
#include "../kernel/window.h"
#include "../fs/fs.h"
#include "../kernel/image.h"
#include "../kernel/heap.h"
#include "../kernel/anim.h"
#include "../kernel/screen.h"
#include "../kernel/string.h"

#define PADDING 60
#define SEARCH_H 44
#define ICON_SIZE 64
#define GRID_COLS 6
#define CELL_H 140

// Colors - Ultra-modern deep translucent palette
#define COL_BG 0xA012121A
#define COL_SEARCH 0x25FFFFFF
#define COL_TEXT 0xFFFFFFFF
#define COL_DIM 0xFFA0A0B0
#define COL_ACCENT 0xFF00A3FF
#define COL_DIVIDER 0x15FFFFFF

static int menu_active = 0;
static int menu_closing = 0;
static animation_t anim_p; // Progress: 0.0 to 1.0
static uint32_t *startmenu_surface = 0;
static uint32_t *blurred_bg_surface = 0;
static int blur_cache_valid = 0;
static int menu_w = 0, menu_h = 0;

typedef struct {
  const char *label;
  uint32_t icon_col;
  const char *icon_path;
  uint32_t *icon_data;
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
extern void chat_init();
extern void phone_init();
extern void recorder_init();

// --- Launch wrappers ---
<<<<<<< HEAD
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
static void launch_chat() {
  chat_init();
  menu_active = 0;
}
static void launch_phone() {
  phone_init();
  menu_active = 0;
}
static void launch_recorder() {
  recorder_init();
  menu_active = 0;
}
=======
static void launch_term() { terminal_init(); }
static void launch_editor() { editor_init(); }
static void launch_calc() { calculator_init(); }
static void launch_paint() { paint_init(); }
static void launch_files() { explorer_init(); }
static void launch_settings() { settings_init(); }
static void launch_taskmgr() { taskmgr_init(); }
static void launch_browser() { browser_init(); }
static void launch_video() { videoplayer_init(0); }
static void launch_lock() { lockscreen_show(); }
static void launch_pdfreader() { pdfreader_init(); }
static void launch_photos() { photos_init(); }
static void launch_mail() { mail_app_init(); }
static void launch_chat() { chat_init(); }
static void launch_phone() { phone_init(); }
static void launch_recorder() { recorder_init(); }
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

static menu_item_t pinned_items[] = {
<<<<<<< HEAD
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
    {"Mail", 0xFF00AADD, launch_mail},
    {"Chat", 0xFF25D366, launch_chat},
    {"Phone", 0xFF0078D4, launch_phone},
    {"Recorder", 0xFFFF6600, launch_recorder}};
#define PINNED_COUNT (sizeof(pinned_items) / sizeof(pinned_items[0]))

// All apps alphabetical
static menu_item_t all_apps_items[] = {
    {"Browser", 0xFF4FC3F7, launch_browser},
    {"Calculator", 0xFF00ADEF, launch_calc},
    {"Chat", 0xFF25D366, launch_chat},
    {"Editor", 0xFF5B8BF5, launch_editor},
    {"Explorer", 0xFFFFC107, launch_files},
    {"Lock", 0xFFE53935, launch_lock},
    {"Mail", 0xFF00AADD, launch_mail},
    {"Paint", 0xFFFF6B6B, launch_paint},
    {"PDF Reader", 0xFFCC3333, launch_pdfreader},
    {"Phone", 0xFF0078D4, launch_phone},
    {"Photos", 0xFFE91E63, launch_photos},
    {"Recorder", 0xFFFF6600, launch_recorder},
    {"Settings", 0xFF607D8B, launch_settings},
    {"Task Mgr", 0xFF66BB6A, launch_taskmgr},
    {"Terminal", 0xFF38B764, launch_term},
    {"Video", 0xFFE040FB, launch_video}};
#define ALL_APPS_COUNT (sizeof(all_apps_items) / sizeof(all_apps_items[0]))

static int startmenu_view_mode = 0;
static int hovered_item = -1; // Index of hovered item (-1 = none)

extern void print_serial(const char *);
=======
    {"Terminal", 0xFF38B764, 0, 0, launch_term},
    {"Editor", 0xFF5B8BF5, "/NOTES.PNG", 0, launch_editor},
    {"Calculator", 0xFF00ADEF, "/CALCULAT.PNG", 0, launch_calc},
    {"Paint", 0xFFFF6B6B, 0, 0, launch_paint},
    {"Explorer", 0xFFFFC107, "/FILEEXPL.PNG", 0, launch_files},
    {"Settings", 0xFF607D8B, "/SETTINGS.PNG", 0, launch_settings},
    {"Browser", 0xFF4FC3F7, "/BROWSER.PNG", 0, launch_browser},
    {"Video", 0xFFE040FB, "/VIDEO.PNG", 0, launch_video},
    {"Task Mgr", 0xFF66BB6A, 0, 0, launch_taskmgr},
    {"Lock", 0xFFE53935, 0, 0, launch_lock},
    {"PDF Reader", 0xFFCC3333, 0, 0, launch_pdfreader},
    {"Photos", 0xFFE91E63, "/PHOTOS.PNG", 0, launch_photos},
    {"Mail", 0xFF00AADD, "/MAIL.PNG", 0, launch_mail},
    {"Chat", 0xFF25D366, "/CHAT.PNG", 0, launch_chat},
    {"Phone", 0xFF0078D4, "/MOBILE.PNG", 0, launch_phone},
    {"Recorder", 0xFFFF4400, "/RECORD.PNG", 0, launch_recorder}};
#define PINNED_COUNT (sizeof(pinned_items) / sizeof(pinned_items[0]))

static int hovered_item = -1;

>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
extern void compositor_invalidate_rect(int x, int y, int w, int h);
extern int screen_width, screen_height;
extern void compositor_blur_rect(int x, int y, int w, int h, int radius);
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
  (void)x; (void)y;
  extern int ui_dirty;
  
  menu_w = screen_width;
  menu_h = screen_height - 24 - 62; // Cover area between bars

  if (!startmenu_surface) {
    startmenu_surface = (uint32_t *)kmalloc(menu_w * menu_h * 4);
  }
  if (!blurred_bg_surface) {
    blurred_bg_surface = (uint32_t *)kmalloc(menu_w * menu_h * 4);
  }

  if (!menu_active && !menu_closing) {
    menu_active = 1;
    menu_closing = 0;
    blur_cache_valid = 0; // Request new capture
    startmenu_render_to_cache();
    
    // Animate fade-in matching the closing animation
    anim_start(&anim_p, 0.0f, 1.0f, 0.15f, EASE_OUT_CUBIC);
  } else {
    menu_closing = 1;
    float cur_p = anim_p.active ? anim_p.current_val : 1.0f;
    anim_start(&anim_p, cur_p, 0.0f, 0.15f, EASE_OUT_CUBIC);
    anim_p.on_complete = menu_close_complete;
  }

  ui_dirty = 1;
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
}

int startmenu_is_active() { return menu_active || menu_closing; }
int startmenu_is_animating() { return anim_p.active; }

void startmenu_tick_animation(float dt) {
  if (anim_p.active) {
    anim_tick(&anim_p, dt);
    compositor_invalidate_rect(0, 24, menu_w, menu_h);
    extern int ui_dirty;
    ui_dirty = 1;
  }
}

static void draw_app_icon(int x, int y, int idx, uint32_t *buffer) {
  menu_item_t *item = &pinned_items[idx];
  
  // Try loading icon if path exists and not yet loaded
  if (item->icon_path && !item->icon_data) {
    file_entry_t *f = fs_find(item->icon_path);
    if (f) {
      extern void print_serial(const char *str);
      print_serial("startmenu: found ");
      print_serial(item->icon_path);
      print_serial("\n");
      uint8_t *raw = (uint8_t *)kmalloc(f->size + 16);
      if (raw) {
        int rs = fs_read(item->icon_path, raw);
        if (rs > 0) {
          int iw, ih, in;
          unsigned char *pixels = stbi_load_from_memory(raw, rs, &iw, &ih, &in, 4);
          if (pixels) {
            // Resize to ICON_SIZE x ICON_SIZE and convert to ARGB
            uint32_t *resized = (uint32_t *)kmalloc(ICON_SIZE * ICON_SIZE * 4);
            if (resized) {
              uint32_t *p32 = (uint32_t *)pixels;
              for (int py = 0; py < ICON_SIZE; py++) {
                int sy = (py * ih) / ICON_SIZE;
                for (int px = 0; px < ICON_SIZE; px++) {
                  int sx = (px * iw) / ICON_SIZE;
                  uint32_t p = p32[sy * iw + sx];
                  // RGBA -> ARGB
                  uint8_t r = p & 0xFF;
                  uint8_t g = (p >> 8) & 0xFF;
                  uint8_t b = (p >> 16) & 0xFF;
                  uint8_t a = (p >> 24) & 0xFF;
                  resized[py * ICON_SIZE + px] = (a << 24) | (r << 16) | (g << 8) | b;
                }
              }
              item->icon_data = resized;
            }
            stbi_image_free(pixels);
          }
        }
        kfree(raw);
      }
    }
<<<<<<< HEAD
  } else if (strcmp(name, "Chat") == 0) {
    // Speech bubble
    vga_draw_rect_surface(x + 4, y + 6, 24, 16, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 6, y + 22, 6, 4, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    // Text lines
    vga_draw_rect_surface(x + 8, y + 10, 12, 2, 0xFF25D366, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 8, y + 15, 16, 2, 0xFF25D366, buffer, MENU_W,
                          MENU_H);
  } else if (strcmp(name, "Phone") == 0) {
    // Phone handset
    vga_draw_rect_surface(x + 8, y + 6, 6, 8, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 12, y + 12, 8, 4, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 18, y + 14, 6, 8, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
  } else if (strcmp(name, "Recorder") == 0) {
    // Microphone
    vga_draw_rect_surface(x + 12, y + 6, 8, 14, 0xFFFFFFFF, buffer, MENU_W,
                          MENU_H);
    // Stand
    vga_draw_rect_surface(x + 15, y + 20, 2, 4, 0xFFCCCCCC, buffer, MENU_W,
                          MENU_H);
    vga_draw_rect_surface(x + 10, y + 24, 12, 2, 0xFFCCCCCC, buffer, MENU_W,
                          MENU_H);
    // Red dot
    vga_draw_rect_surface(x + 22, y + 6, 4, 4, 0xFFFF0000, buffer, MENU_W,
                          MENU_H);
=======
    // If loading failed, set icon_path to NULL so we don't retry every frame
    if (!item->icon_data) item->icon_path = 0;
  }

  if (item->icon_data) {
    // Draw cached image icon
    for (int py = 0; py < ICON_SIZE; py++) {
      if (y + py < 0 || y + py >= menu_h) continue;
      for (int px = 0; px < ICON_SIZE; px++) {
        if (x + px < 0 || x + px >= menu_w) continue;
        uint32_t p = item->icon_data[py * ICON_SIZE + px];
        uint8_t a = (p >> 24) & 0xFF;
        if (a == 0) continue;
        if (a == 255) {
          buffer[(y + py) * menu_w + (x + px)] = p;
        } else {
          // Blend with background
          uint32_t d = buffer[(y + py) * menu_w + (x + px)];
          uint32_t sr = (p >> 16) & 0xFF;
          uint32_t sg = (p >> 8) & 0xFF;
          uint32_t sb = p & 0xFF;
          uint32_t dr = (d >> 16) & 0xFF;
          uint32_t dg = (d >> 8) & 0xFF;
          uint32_t db = d & 0xFF;
          uint32_t r = ((sr - dr) * a >> 8) + dr;
          uint32_t g = ((sg - dg) * a >> 8) + dg;
          uint32_t b = ((sb - db) * a >> 8) + db;
          buffer[(y + py) * menu_w + (x + px)] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
    }
  } else {
    // Fallback: Original vector-ish icon
    uint32_t col = item->icon_col;
    int r = 14;
    for (int py = 0; py < ICON_SIZE; py++) {
      for (int px = 0; px < ICON_SIZE; px++) {
        int draw = 1;
        if (px < r && py < r) {
          int dx = r - px - 1, dy = r - py - 1;
          if (dx * dx + dy * dy >= r * r) draw = 0;
        } else if (px >= ICON_SIZE - r && py < r) {
          int dx = px - (ICON_SIZE - r), dy = r - py - 1;
          if (dx * dx + dy * dy >= r * r) draw = 0;
        } else if (px < r && py >= ICON_SIZE - r) {
          int dx = r - px - 1, dy = py - (ICON_SIZE - r);
          if (dx * dx + dy * dy >= r * r) draw = 0;
        } else if (px >= ICON_SIZE - r && py >= ICON_SIZE - r) {
          int dx = px - (ICON_SIZE - r), dy = py - (ICON_SIZE - r);
          if (dx * dx + dy * dy >= r * r) draw = 0;
        }
        if (draw) buffer[(y + py) * menu_w + (x + px)] = col;
      }
    }
    // Simplified symbols
    int cx = x + ICON_SIZE / 2;
    int cy = y + ICON_SIZE / 2;
    const char *name = item->label;
    if (strcmp(name, "Terminal") == 0) vga_draw_string_surface(cx - 8, cy - 4, ">_", 0xFFFFFFFF, buffer, menu_w, menu_h);
    else if (strcmp(name, "Explorer") == 0) vga_draw_rect_surface(cx - 14, cy - 8, 28, 20, 0xFFFFF176, buffer, menu_w, menu_h);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  }
}

static void startmenu_render_to_cache() {
  if (!startmenu_surface) return;
  uint32_t *buffer = startmenu_surface;
  for (int i = 0; i < menu_w * menu_h; i++) buffer[i] = 0x00000000;

  // Render icons and text to surface
  int sy = 60;
  int sw = 460;
  int sx = (menu_w - sw) / 2;
  vga_draw_rect_surface(sx, sy, sw, SEARCH_H, COL_SEARCH, buffer, menu_w, menu_h);
  vga_draw_string_surface(sx + 16, sy + 15, "Search apps...", 0xFFAAAAAA, buffer, menu_w, menu_h);

  int cell_w = (menu_w - PADDING * 2) / GRID_COLS;
  int grid_y = sy + SEARCH_H + 60;
  for (int i = 0; i < (int)PINNED_COUNT; i++) {
    int col = i % GRID_COLS;
    int row = i / GRID_COLS;
    int ix = PADDING + (col * cell_w) + (cell_w - ICON_SIZE) / 2;
    int iy = grid_y + row * CELL_H;
    if (iy + ICON_SIZE > menu_h) continue;

    draw_app_icon(ix, iy, i, buffer);

    int lbl_len = 0;
    const char *lbl = pinned_items[i].label;
    while (lbl[lbl_len]) lbl_len++;
    int lbl_x = PADDING + col * cell_w + (cell_w - lbl_len * 8) / 2;
    vga_draw_string_surface(lbl_x, iy + ICON_SIZE + 12, lbl, COL_TEXT, buffer, menu_w, menu_h);
  }

  int footer_y = menu_h - 60;
  vga_draw_rect_surface(PADDING, footer_y, menu_w - PADDING * 2, 1, COL_DIVIDER, buffer, menu_w, menu_h);
  vga_draw_string_surface(PADDING + 10, footer_y + 20, "Rudra Patel", COL_TEXT, buffer, menu_w, menu_h);
  vga_draw_string_surface(menu_w - PADDING - 80, footer_y + 20, "Shut Down", COL_ACCENT, buffer, menu_w, menu_h);
}

static void startmenu_fast_blur(uint32_t *surface, int w, int h) {
  int dw = w / 4;
  int dh = h / 4;
  if (dw <= 0 || dh <= 0 || dw > 480 || dh > 270) return;
  
  static uint32_t down_buf[480 * 270];
  static uint32_t blur_buf[480 * 270];
  
  // 1. Box Downsample 4x
  for (int y = 0; y < dh; y++) {
    for (int x = 0; x < dw; x++) {
      int r = 0, g = 0, b = 0;
      for (int dy = 0; dy < 4; dy++) {
        uint32_t *row = &surface[(y * 4 + dy) * w];
        for (int dx = 0; dx < 4; dx++) {
          uint32_t p = row[x * 4 + dx];
          r += (p >> 16) & 0xFF;
          g += (p >> 8) & 0xFF;
          b += p & 0xFF;
        }
      }
      down_buf[y * dw + x] = 0xFF000000 | ((r/16) << 16) | ((g/16) << 8) | (b/16);
    }
  }
  
  // 2. Horizontal Blur (radius 3)
  for (int y = 0; y < dh; y++) {
    for (int x = 0; x < dw; x++) {
      int r = 0, g = 0, b = 0, count = 0;
      for (int dx = -3; dx <= 3; dx++) {
        int nx = x + dx;
        if (nx >= 0 && nx < dw) {
          uint32_t p = down_buf[y * dw + nx];
          r += (p >> 16) & 0xFF;
          g += (p >> 8) & 0xFF;
          b += p & 0xFF;
          count++;
        }
      }
      blur_buf[y * dw + x] = 0xFF000000 | ((r/count) << 16) | ((g/count) << 8) | (b/count);
    }
  }
  
  // 3. Vertical Blur
  for (int y = 0; y < dh; y++) {
    for (int x = 0; x < dw; x++) {
      int r = 0, g = 0, b = 0, count = 0;
      for (int dy = -3; dy <= 3; dy++) {
        int ny = y + dy;
        if (ny >= 0 && ny < dh) {
          uint32_t p = blur_buf[ny * dw + x];
          r += (p >> 16) & 0xFF;
          g += (p >> 8) & 0xFF;
          b += p & 0xFF;
          count++;
        }
      }
      down_buf[y * dw + x] = 0xFF000000 | ((r/count) << 16) | ((g/count) << 8) | (b/count);
    }
  }
  
  // 4. Upsample nearest neighbor back to surface
  for (int y = 0; y < h; y++) {
    int sy = y / 4;
    for (int x = 0; x < w; x++) {
      int sx = x / 4;
      surface[y * w + x] = down_buf[sy * dw + sx];
    }
  }
}

void startmenu_draw(uint32_t *buffer, rect_t clip) {
  if (!startmenu_is_active() || !startmenu_surface) return;

  float p = anim_p.current_val;
  int start_y_abs = 24;
  int menu_y_offset = (int)((1.0f - p) * 50.0f); // Icons slide up 50px

  // 1. Capture and Blur Background (ONCE per activation for performance)
  if (!blur_cache_valid && blurred_bg_surface) {
    // Copy unblurred screen state to our surface
    for (int y = 0; y < menu_h; y++) {
      memcpy(&blurred_bg_surface[y * menu_w], &buffer[(start_y_abs + y) * screen_width], menu_w * 4);
    }
    // Perform ultra-fast isolated downsample blur!
    // This entirely prevents backbuffer corruption and neon tearing glitches.
    startmenu_fast_blur(blurred_bg_surface, menu_w, menu_h);
    blur_cache_valid = 1;
  }

  // 2. Draw Blurred Background with Increasing Opacity
  if (blurred_bg_surface && blur_cache_valid) {
    uint8_t blur_opacity = (uint8_t)(p * 255.0f);
    for (int y = 0; y < menu_h; y++) {
      int dst_y = start_y_abs + y;
      if (dst_y < clip.y || dst_y >= clip.y + clip.h) continue;
      
      uint32_t *src_row = &blurred_bg_surface[y * menu_w];
      uint32_t *dst_row = &buffer[dst_y * screen_width];
      
      for (int x = 0; x < menu_w; x++) {
        if (x < clip.x || x >= clip.x + clip.w) continue;
        uint32_t bg_px = src_row[x];
        uint32_t dst_px = dst_row[x];
        
        // Blend blurred bg over current bg based on animation progress
        uint32_t r = (((bg_px >> 16) & 0xFF) * blur_opacity + ((dst_px >> 16) & 0xFF) * (255 - blur_opacity)) >> 8;
        uint32_t g = (((bg_px >> 8) & 0xFF) * blur_opacity + ((dst_px >> 8) & 0xFF) * (255 - blur_opacity)) >> 8;
        uint32_t b = ((bg_px & 0xFF) * blur_opacity + (dst_px & 0xFF) * (255 - blur_opacity)) >> 8;
        dst_row[x] = 0xFF000000 | (r << 16) | (g << 8) | b;
      }
    }
  }

  // 2. Translucent Overlay (Fading in)
  uint32_t overlay_alpha = (uint32_t)(p * 160.0f); // Max 160/255 opacity
  uint32_t overlay_col = (overlay_alpha << 24) | (COL_BG & 0x00FFFFFF);
  
  for (int y = start_y_abs; y < start_y_abs + menu_h; y++) {
    if (y < clip.y || y >= clip.y + clip.h) continue;
    uint32_t *row = &buffer[y * screen_width];
    for (int x = 0; x < screen_width; x++) {
      if (x < clip.x || x >= clip.x + clip.w) continue;
      // Simple alpha blend
      uint32_t dst = row[x];
      uint32_t r = (((overlay_col >> 16) & 0xFF) * overlay_alpha + ((dst >> 16) & 0xFF) * (255 - overlay_alpha)) >> 8;
      uint32_t g = (((overlay_col >> 8) & 0xFF) * overlay_alpha + ((dst >> 8) & 0xFF) * (255 - overlay_alpha)) >> 8;
      uint32_t b = ((overlay_col & 0xFF) * overlay_alpha + (dst & 0xFF) * (255 - overlay_alpha)) >> 8;
      row[x] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
  }

  // 3. Foreground (Icons) - Sliding Up & Fading In
  uint8_t icon_alpha = (uint8_t)(p * 255.0f);
  if (icon_alpha == 0) return;

  for (int y = 0; y < menu_h; y++) {
    int dst_y = start_y_abs + y - menu_y_offset; // Apply slide offset
    if (dst_y < start_y_abs || dst_y >= start_y_abs + menu_h) continue;
    if (dst_y < clip.y || dst_y >= clip.y + clip.h) continue;

    uint32_t *src_row = &startmenu_surface[y * menu_w];
    uint32_t *dst_row = &buffer[dst_y * screen_width];

    for (int x = 0; x < menu_w; x++) {
      if (x < clip.x || x >= clip.x + clip.w) continue;
      uint32_t spx = src_row[x];
      uint32_t sa = (spx >> 24) & 0xFF;
      if (sa == 0) continue;

      // Combine surface alpha with global fade-in
      uint32_t final_a = (sa * icon_alpha) >> 8;
      uint32_t dpx = dst_row[x];
      uint32_t r = (((spx >> 16) & 0xFF) * final_a + ((dpx >> 16) & 0xFF) * (255 - final_a)) >> 8;
      uint32_t g = (((spx >> 8) & 0xFF) * final_a + ((dpx >> 8) & 0xFF) * (255 - final_a)) >> 8;
      uint32_t b = ((spx & 0xFF) * final_a + (dpx & 0xFF) * (255 - final_a)) >> 8;
      dst_row[x] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
  }

  // Draw hover overlay on hovered item
  if (hovered_item >= 0) {
    int hx = 0, hy_start = 0, hw = 0, hh = 0;
    int search_bottom = PADDING + SEARCH_H;
    int head_y_h = search_bottom + 20;

    if (startmenu_view_mode == 0) {
      // Pinned grid
      int grid_y_h = head_y_h + 24;
      int col = hovered_item % GRID_COLS;
      int row = hovered_item / GRID_COLS;
      hx = PADDING + col * CELL_W;
      hy_start = grid_y_h + row * CELL_H;
      hw = CELL_W;
      hh = CELL_H;
    } else {
      // All apps list - compute position by iterating
      int list_y_h = head_y_h + 30;
      char cur_l = '\0';
      for (int i = 0; i <= hovered_item && i < (int)ALL_APPS_COUNT; i++) {
        char first = all_apps_items[i].label[0];
        if (first != cur_l && cur_l != '\0') list_y_h += 8;
        if (first != cur_l) cur_l = first;
        if (i == hovered_item) {
          hx = PADDING;
          hy_start = list_y_h;
          hw = MENU_W - PADDING * 2;
          hh = 40;
          break;
        }
        list_y_h += 40;
      }
    }

    // Draw transparent gray hover overlay
    if (hw > 0 && hh > 0) {
      for (int oy = 0; oy < hh; oy++) {
        int py = cy + hy_start + oy;
        if (py < clip.y || py >= clip.y + clip.h) continue;
        for (int ox = 0; ox < hw; ox++) {
          int px_x = menu_x + hx + ox;
          if (px_x < 0 || px_x >= screen_width || py < 0 || py >= screen_height) continue;
          uint32_t dst = buffer[py * screen_width + px_x];
          // 25% white blend
          uint32_t r = (((dst >> 16) & 0xFF) * 192 + 255 * 64) >> 8;
          uint32_t g = (((dst >> 8) & 0xFF) * 192 + 255 * 64) >> 8;
          uint32_t b = ((dst & 0xFF) * 192 + 255 * 64) >> 8;
          buffer[py * screen_width + px_x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
    }
  }
}

int startmenu_handle_mouse(int mx, int my, int buttons) {
  static int sm_last_btn = 0;
  int down_edge = (buttons & 1) && !(sm_last_btn & 1);
  sm_last_btn = buttons;
  if (!startmenu_is_active() || menu_closing) return 0;

<<<<<<< HEAD
  int cy = (int)anim_y.current_val;
  if (mx < menu_x || mx >= menu_x + MENU_W || my < cy || my >= cy + MENU_H) {
    if (hovered_item != -1) {
      hovered_item = -1;
      compositor_invalidate_rect(menu_x, cy, MENU_W, MENU_H);
    }
    if (down_edge) {
      startmenu_show(menu_x, screen_height);
      return 1;
    }
    return 0;
  }

  // Convert to local menu coordinates for hover
  int lx = mx - menu_x;
  int ly = my - cy;
  int search_bottom_h = PADDING + SEARCH_H;
  int head_y_h = search_bottom_h + 20;

  // Compute hover index on every mouse move
  int new_hover = -1;
  if (startmenu_view_mode == 0) {
    int grid_y_h = head_y_h + 24;
    for (int i = 0; i < (int)PINNED_COUNT; i++) {
      int col = i % GRID_COLS;
      int row = i / GRID_COLS;
      int ix = PADDING + col * CELL_W;
      int iy = grid_y_h + row * CELL_H;
      if (lx >= ix && lx < ix + CELL_W && ly >= iy && ly < iy + CELL_H) {
        new_hover = i;
        break;
      }
    }
  } else {
    int list_y_h = head_y_h + 30;
    char cur_l = '\0';
    for (int i = 0; i < (int)ALL_APPS_COUNT; i++) {
      char first = all_apps_items[i].label[0];
      if (first != cur_l && cur_l != '\0') list_y_h += 8;
      if (first != cur_l) cur_l = first;
      if (ly >= list_y_h && ly < list_y_h + 40 && lx >= PADDING && lx < MENU_W - PADDING) {
        new_hover = i;
        break;
      }
      list_y_h += 40;
    }
  }
  if (new_hover != hovered_item) {
    hovered_item = new_hover;
    compositor_invalidate_rect(menu_x, cy, MENU_W, MENU_H);
  }

  if (!down_edge)
    return 1;

  // Convert to local menu coordinates
  // Conver to local menu coordinates - ALREADY DONE ABOVE
  lx = mx - menu_x;
  ly = my - cy;

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
=======
  int sy = 24;
  if (my < sy || my >= sy + menu_h) {
    if (down_edge) startmenu_show(0, 0);
    return 0;
  }

  int lx = mx;
  int ly = my - sy;

  int cell_w = (menu_w - PADDING * 2) / GRID_COLS;
  int grid_y = 60 + SEARCH_H + 60;
  int new_hover = -1;
  for (int i = 0; i < (int)PINNED_COUNT; i++) {
    int col = i % GRID_COLS;
    int row = i / GRID_COLS;
    int ix = PADDING + col * cell_w;
    int iy = grid_y + row * CELL_H;
    if (lx >= ix && lx < ix + cell_w && ly >= iy && ly < iy + CELL_H) {
      new_hover = i;
      break;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    }
  }

  if (new_hover != hovered_item) {
    hovered_item = new_hover;
    compositor_invalidate_rect(0, sy, menu_w, menu_h);
  }

  if (down_edge && hovered_item >= 0) {
    if (pinned_items[hovered_item].launch) {
      pinned_items[hovered_item].launch();
      startmenu_show(0, 0);
    }
  }
  return 1;
}

int startmenu_get_x() { return 0; }
int startmenu_get_y() { return 24; }
int startmenu_get_w() { return menu_w; }
int startmenu_get_h() { return menu_h; }
