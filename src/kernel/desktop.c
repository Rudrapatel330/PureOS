#include "desktop.h"
#include "../drivers/ports.h"
#include "../drivers/rtc.h"
#include "window.h"

extern uint32_t get_timer_ticks(void);
static rtc_time_t cached_time;
static uint32_t last_time_update = 0;
#include "../drivers/speaker.h"
#include "../drivers/vga.h"
#include "../ui/ctxmenu.h"
#include "compositor.h"
#include "config.h"
#include "string.h"
#include "theme.h"
#define STBI_NO_STDIO
#include "../drivers/rtc.h"
#include "../include/apps.h"
#include "heap.h"
#include "image.h"
#include "screen.h"
#include "window.h"

// Externs
extern int ui_dirty;
extern int screen_width;
extern int screen_height;
extern uint32_t *backbuffer;

#include "../include/apps.h"

// --- Global Search State ---
int search_active = 0;
static char search_input[64] = {0};
static int search_cursor = 0;
static int search_selection = 0;
static const app_entry_t *search_results[32];
static int search_result_count = 0;

static void desktop_search_invalidate(void) {
  int box_w = 400, box_h = 300;
  int box_x = (screen_width - box_w) / 2;
  int box_y = (screen_height - box_h) / 3;
  compositor_invalidate_rect(box_x - 10, box_y - 10, box_w + 20, box_h + 20);
}

static void search_update_results(void) {
  search_result_count = 0;
  if (search_input[0] == 0)
    return;

  for (int i = 0; i < installed_apps_count && search_result_count < 32; i++) {
    const char *name = installed_apps[i].name;
    int match = 1;
    for (int j = 0; search_input[j]; j++) {
      if (name[j] == 0) {
        match = 0;
        break;
      }
      char c1 = search_input[j];
      char c2 = name[j];
      if (c1 >= 'a' && c1 <= 'z')
        c1 -= 32;
      if (c2 >= 'a' && c2 <= 'z')
        c2 -= 32;
      if (c1 != c2) {
        match = 0;
        break;
      }
    }
    if (match) {
      search_results[search_result_count++] = &installed_apps[i];
    }
  }
  if (search_selection >= search_result_count)
    search_selection = 0;
}

void desktop_activate_search(void) {
  if (!search_active) {
    search_active = 1;
    search_input[0] = 0;
    search_cursor = 0;
    search_selection = 0;
    search_result_count = 0;
  }
  desktop_search_invalidate();
  ui_dirty = 1;
}

void desktop_handle_search_key(int key, char ascii) {
  if (!search_active)
    return;

  if (key == 0x01) { // Esc
    search_active = 0;
    desktop_search_invalidate();
    ui_dirty = 1;
    return;
  }
  if (key == 0x1C) { // Enter
    if (search_selection >= 0 && search_selection < search_result_count) {
      search_results[search_selection]->launch();
      search_active = 0;
      desktop_search_invalidate();
      ui_dirty = 1;
    }
    return;
  }
  if (key == 0x48) { // Up arrow
    if (search_selection > 0)
      search_selection--;
    desktop_search_invalidate();
    ui_dirty = 1;
    return;
  }
  if (key == 0x50) { // Down arrow
    if (search_selection < search_result_count - 1)
      search_selection++;
    desktop_search_invalidate();
    ui_dirty = 1;
    return;
  }
  if (key == 0x0E) { // Backspace
    if (search_cursor > 0) {
      search_input[--search_cursor] = 0;
      search_update_results();
      desktop_search_invalidate();
      ui_dirty = 1;
    }
    return;
  }
  if (ascii >= 32 && ascii < 127 && search_cursor < 63) {
    search_input[search_cursor++] = ascii;
    search_input[search_cursor] = 0;
    search_update_results();
    desktop_search_invalidate();
    ui_dirty = 1;
  }
}

int desktop_handle_search_mouse(int mx, int my, int buttons) {
  if (!search_active)
    return 0;

  int box_w = 400, box_h = 300;
  int box_x = (screen_width - box_w) / 2;
  int box_y = (screen_height - box_h) / 3;

  if (mx >= box_x && mx < box_x + box_w && my >= box_y && my < box_y + box_h) {
    // Check if clicked on a result
    int fy = box_y + 20, fh = 30;
    int ly = fy + fh + 10;
    for (int i = 0; i < search_result_count; i++) {
      if (my >= ly && my < ly + 20) {
        search_selection = i;
        if (buttons & 1) {
          search_results[i]->launch();
          search_active = 0;
          desktop_search_invalidate();
        }
        ui_dirty = 1;
        return 1;
      }
      ly += 20;
    }
    return 1;
  } else {
    if (buttons & 1) {
      search_active = 0;
      desktop_search_invalidate();
      ui_dirty = 1;
    }
    return 1;
  }
}

void desktop_render_search(uint32_t *target, rect_t clip) {
  if (!search_active)
    return;

  int sw = screen_width, sh = screen_height;
  int box_w = 400, box_h = 300;
  int box_x = (sw - box_w) / 2;
  int box_y = (sh - box_h) / 3;

  // Background (semi-transparent dark glass)
  for (int y = 0; y < box_h; y++) {
    for (int x = 0; x < box_w; x++) {
      int gx = box_x + x, gy = box_y + y;
      if (gx < clip.x || gx >= clip.x + clip.w || gy < clip.y ||
          gy >= clip.y + clip.h)
        continue;
      if (gx < 0 || gx >= sw || gy < 0 || gy >= sh)
        continue;

      uint32_t bg = 0xEE2D2D2D;
      uint32_t dst = target[gy * sw + gx];
      uint8_t a = (bg >> 24) & 0xFF;
      uint32_t r =
          (((bg >> 16) & 0xFF) * a + ((dst >> 16) & 0xFF) * (255 - a)) >> 8;
      uint32_t g =
          (((bg >> 8) & 0xFF) * a + ((dst >> 8) & 0xFF) * (255 - a)) >> 8;
      uint32_t b = ((bg & 0xFF) * a + (dst & 0xFF) * (255 - a)) >> 8;
      target[gy * sw + gx] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
  }

  // Input field
  int fx = box_x + 20, fy = box_y + 20, fw = box_w - 40, fh = 30;
  vga_draw_rect_lfb(fx, fy, fw, fh, 0xFF444444, target);
  vga_draw_rect_lfb(fx, fy, fw, fh, 0xFF888888, target); // border

  // Draw input text
  int tx = fx + 5, ty = fy + 7;
  for (int i = 0; i < search_cursor; i++) {
    vga_draw_char_lfb(tx, ty, search_input[i], 0xFFFFFFFF, target);
    tx += 8;
  }
  // Cursor
  if ((get_timer_ticks() / 20) % 2 == 0) {
    vga_draw_rect_lfb(tx, ty, 2, 14, 0xFFFFFFFF, target);
  }

  // Results list
  int ly = fy + fh + 10;
  for (int i = 0; i < search_result_count; i++) {
    if (ly + 20 > box_y + box_h - 10)
      break;
    uint32_t col = (i == search_selection) ? 0xFF3B6EA5 : 0xFF000000;
    if (i == search_selection) {
      vga_draw_rect_lfb(box_x + 5, ly - 2, box_w - 10, 18, col, target);
    }
    vga_draw_string_lfb(box_x + 10, ly, search_results[i]->name,
                        (i == search_selection) ? 0xFFFFFFFF : 0xFFCCCCCC,
                        target);
    ly += 20;
  }
}

// App launch externs
extern void browser_init(void);

// Reference wallpaper data from ramfs.c (avoid duplicating 175KB)
extern const unsigned char wallpaper_bmp_data[];
extern const unsigned int wallpaper_bmp_size;
extern const unsigned char wallpaper_png_data[];
extern const unsigned int wallpaper_png_size;

#include "image.h"
uint32_t *desktop_buffer = 0;

// Externs from kernel
extern uint32_t *backbuffer;
extern int screen_width;
extern int screen_height;
extern int ui_dirty;

// Externs from window manager
extern window_t windows[];
extern int window_count;
extern window_t *active_window;

// Clock string (updated each frame)
char clock_str[16] = "00:00";

// Robust Reboot Implementation
extern void reboot_system(); // In kernel.c

void taskbar_reboot(void) {
  // This function should ideally not draw directly, but for now it's okay
  // as it's a terminal action.
  reboot_system();
}

void taskbar_shutdown(void) {
  vga_draw_rect(0, 0, screen_width, screen_height, 0xFF000000); // Opaque Black
  vga_draw_string(450, 380, "Status: Shutting Down...", 0xFFFFFFFF);

  for (volatile int i = 0; i < 10000000; i++)
    ;

  outw(0xB004, 0x2000);
  outw(0x604, 0x2000);
  outw(0x4004, 0x3400);
  outw(0x8900, 0x00);
  outl(0x7100, 0x00);

  __asm__ volatile("cli; hlt");
}

// Define App Types
typedef enum {
  APP_TERMINAL = 0,
  APP_CALCULATOR = 1,
  APP_EDITOR = 2,
  APP_COMPUTER = 3,
  APP_PAINT = 4,
  APP_FILEMGR = 5,
  APP_TASKMGR = 6,
  APP_BROWSER = 7,
  APP_VIDEOPLAYER = 8,
  APP_SETTINGS = 9,
  APP_PDFREADER = 10,
  APP_CAMERA = 11,
  APP_PHOTOS = 12,
  APP_MAIL = 13,
  APP_RECORDER = 14,
  APP_CHAT = 15,
  APP_PHONE = 16,
  APP_MUSIC = 17,
  APP_SYSMENU = 18
} app_type_t;

icon_t icons[MAX_ICONS];
int icon_count = 0;
static int hovered_icon = -1;

// PNG Icon Caching for desktop/taskbar
static const char *app_icon_paths[] = {
    [APP_TERMINAL] = "/TERMINAL.PNG",
    [APP_CALCULATOR] = "/CALCULAT.PNG",
    [APP_EDITOR] = "/NOTES.PNG",
    [APP_COMPUTER] = 0,
    [APP_PAINT] = "/PAINT.PNG",
    [APP_FILEMGR] = "/FILEEXPL.PNG",
    [APP_TASKMGR] = 0,
    [APP_BROWSER] = "/BROWSER.PNG",
    [APP_VIDEOPLAYER] = "/VIDEO.PNG",
    [APP_SETTINGS] = "/SETTINGS.PNG",
    [APP_PDFREADER] = 0,
    [APP_CAMERA] = 0,
    [APP_PHOTOS] = "/PHOTOS.PNG",
    [APP_MAIL] = "/MAIL.PNG",
    [APP_RECORDER] = "/RECORD.PNG",
    [APP_CHAT] = "/CHAT.PNG",
    [APP_PHONE] = "/MOBILE.PNG",
    [APP_MUSIC] = "/MUSIC.PNG",
    [APP_SYSMENU] = "/PEGION.PNG"
};
static uint32_t *app_icon_cache[32] = {0};
#include "../fs/fs.h"

static int draw_icon_png(int x, int y, int draw_w, int draw_h, int type, uint32_t *target) {
  if (type < 0 || type >= 32) return 0;
  if (!app_icon_cache[type] && app_icon_paths[type]) {
    file_entry_t *f = fs_find(app_icon_paths[type]);
    if (f) {
      extern void print_serial(const char *str);
      print_serial("draw_icon_png: found ");
      print_serial(app_icon_paths[type]);
      print_serial("\n");
      uint8_t *raw = (uint8_t *)kmalloc(f->size + 16);
      if (raw) {
        int rs = fs_read(app_icon_paths[type], raw);
        if (rs > 0) {
          int iw, ih, in;
          unsigned char *pixels = stbi_load_from_memory(raw, rs, &iw, &ih, &in, 4);
          if (pixels) {
            print_serial("draw_icon_png: decoded OK\n");
            uint32_t *cached = (uint32_t *)kmalloc(64 * 64 * 4);
            if (cached) {
              uint32_t *p32 = (uint32_t *)pixels;
              for (int py = 0; py < 64; py++) {
                int sy_start = (py * ih) / 64;
                int sy_end = ((py + 1) * ih) / 64;
                if (sy_end == sy_start) sy_end++;
                if (sy_end > ih) sy_end = ih;
                
                for (int px = 0; px < 64; px++) {
                  int sx_start = (px * iw) / 64;
                  int sx_end = ((px + 1) * iw) / 64;
                  if (sx_end == sx_start) sx_end++;
                  if (sx_end > iw) sx_end = iw;
                  
                  int r=0, g=0, b=0, a=0, count=0;
                  for (int sy = sy_start; sy < sy_end; sy++) {
                    for (int sx = sx_start; sx < sx_end; sx++) {
                       uint32_t p = p32[sy * iw + sx];
                       a += (p >> 24) & 0xFF;
                       r += p & 0xFF;
                       g += (p >> 8) & 0xFF;
                       b += (p >> 16) & 0xFF;
                       count++;
                    }
                  }
                  if (count > 0) {
                      a /= count; r /= count; g /= count; b /= count;
                  }
                  cached[py * 64 + px] = (a << 24) | (r << 16) | (g << 8) | b;
                }
              }
              app_icon_cache[type] = cached;
            }
            stbi_image_free(pixels);
          } else {
            print_serial("draw_icon_png: stbi_load_from_memory failed\n");
          }
        }
        kfree(raw);
      }
    }
  }

  if (app_icon_cache[type]) {
    uint32_t *data = app_icon_cache[type];
    for (int py = 0; py < draw_h; py++) {
      int dy = y + py;
      if (dy < 0 || dy >= screen_height) continue;
      for (int px = 0; px < draw_w; px++) {
        int dx = x + px;
        if (dx < 0 || dx >= screen_width) continue;
        
        int gx = (px * 64 * 256) / draw_w;
        int gy = (py * 64 * 256) / draw_h;
        int bx = gx >> 8;
        int by = gy >> 8;
        int u_frac = gx & 0xFF;
        int v_frac = gy & 0xFF;
        
        if (bx >= 63) bx = 62;
        if (by >= 63) by = 62;

        uint32_t c00 = data[by * 64 + bx];
        uint32_t c10 = data[by * 64 + bx + 1];
        uint32_t c01 = data[(by + 1) * 64 + bx];
        uint32_t c11 = data[(by + 1) * 64 + bx + 1];

        // Bilinear blend
        int u_inv = 256 - u_frac;
        int v_inv = 256 - v_frac;
        int w00 = (u_inv * v_inv) >> 8;
        int w10 = (u_frac * v_inv) >> 8;
        int w01 = (u_inv * v_frac) >> 8;
        int w11 = (u_frac * v_frac) >> 8;

        int aa = ((c00 >> 24) * w00 + (c10 >> 24) * w10 + (c01 >> 24) * w01 + (c11 >> 24) * w11) >> 8;
        int rr = (((c00 >> 16) & 0xFF) * w00 + ((c10 >> 16) & 0xFF) * w10 + ((c01 >> 16) & 0xFF) * w01 + ((c11 >> 16) & 0xFF) * w11) >> 8;
        int gg = (((c00 >> 8) & 0xFF) * w00 + ((c10 >> 8) & 0xFF) * w10 + ((c01 >> 8) & 0xFF) * w01 + ((c11 >> 8) & 0xFF) * w11) >> 8;
        int bb = ((c00 & 0xFF) * w00 + (c10 & 0xFF) * w10 + (c01 & 0xFF) * w01 + (c11 & 0xFF) * w11) >> 8;

        uint32_t p = (aa << 24) | (rr << 16) | (gg << 8) | bb;
        uint8_t a = aa;

        if (a == 0) continue;
        if (a == 255) {
          target[dy * screen_width + dx] = p;
        } else {
          uint32_t d = target[dy * screen_width + dx];
          uint32_t dr = (d >> 16) & 0xFF, dg = (d >> 8) & 0xFF, db = d & 0xFF;
          uint32_t out_r = ((rr - dr) * a >> 8) + dr;
          uint32_t out_g = ((gg - dg) * a >> 8) + dg;
          uint32_t out_b = ((bb - db) * a >> 8) + db;
          target[dy * screen_width + dx] = 0xFF000000 | (out_r << 16) | (out_g << 8) | out_b;
        }
      }
    }
    return 1;
  }
  return 0;
}


void desktop_add_icon(int x, int y, const char *label, int type,
                      uint32_t color) {
  if (icon_count >= MAX_ICONS)
    return;
  icons[icon_count].x = x;
  icons[icon_count].y = y;

  // Safe string copy
  int i = 0;
  while (label[i] && i < 31) {
    icons[icon_count].label[i] = label[i];
    i++;
  }
  icons[icon_count].label[i] = 0;

  icons[icon_count].type = type;
  icons[icon_count].color = color;

  // Init State
  icons[icon_count].selected = 0;
  icons[icon_count].is_dragging = 0;

  icon_count++;
}

// Max icons on desktop (Must match header)
// #define MAX_ICONS 32

void desktop_init() {
  icon_count = 0;
  desktop_add_icon(30, 30, "Computer", APP_COMPUTER, 0);
  desktop_add_icon(30, 140, "Terminal", APP_TERMINAL, 0);
  desktop_add_icon(30, 250, "Calculator", APP_CALCULATOR, 0);
  desktop_add_icon(30, 360, "Editor", APP_EDITOR, 0);
  desktop_add_icon(30, 470, "Paint", APP_PAINT, 0);
  desktop_add_icon(30, 580, "Files", APP_FILEMGR, 0);

  desktop_add_icon(140, 30, "Task Mgr", APP_TASKMGR, 0);
  desktop_add_icon(140, 140, "Browser", APP_BROWSER, 0);
  desktop_add_icon(140, 250, "Video", APP_VIDEOPLAYER, 0);
  desktop_add_icon(140, 360, "Settings", APP_SETTINGS, 0);
  desktop_add_icon(140, 470, "PDF Reader", APP_PDFREADER, 0);
  desktop_add_icon(140, 580, "Photos", APP_PHOTOS, 0);

  desktop_add_icon(250, 30, "Mail", APP_MAIL, 0);
  desktop_add_icon(250, 140, "Camera", APP_CAMERA, 0);
  desktop_add_icon(250, 250, "Recorder", APP_RECORDER, 0);
  desktop_add_icon(250, 360, "Chat", APP_CHAT, 0);
  desktop_add_icon(250, 470, "Phone", APP_PHONE, 0);

  // Initial draw to populate valid cache
  desktop_draw();
}

// Forward declarations
void draw_computer_icon(int x, int y, uint32_t *target);
void draw_terminal_icon(int x, int y, uint32_t *target);
void draw_calculator_icon(int x, int y, uint32_t *target);
void draw_editor_icon(int x, int y, uint32_t *target);

static void draw_rect_f(int x, int y, int w, int h, uint32_t color,
                        uint32_t *target) {
  extern os_config_t global_config;
  vga_draw_rect_lfb(x, y, w, h,
                    vga_apply_color_filter(color, global_config.icon_filter,
                                           global_config.icon_filter_intensity),
                    target);
}

void draw_icon(int x, int y, int draw_w, int draw_h, int type, uint32_t *target) {
  uint32_t W = 0xFFFFFFFF;
  uint32_t W90 = 0xFFE8E8E8;
  uint32_t W60 = 0xFFBBBBBB;

  if (type >= 0 && type < 32 && app_icon_paths[type]) {
    if (draw_icon_png(x, y, draw_w, draw_h, type, target)) return;
  }
  
  // Shift native OS icons to center them inside the requested box
  x += (draw_w - 40) / 2;
  y += (draw_h - 40) / 2;

  switch (type) {
  case APP_COMPUTER:
    // iMac style silver frame
    draw_rect_f(x + 3, y + 6, 34, 23, 0xFFC0C0C0, target);
    draw_rect_f(x + 5, y + 8, 30, 18, 0xFF111111, target);
    draw_rect_f(x + 6, y + 9, 28, 16, 0xFF0078D4, target);
    draw_rect_f(x + 6, y + 9, 14, 8, 0xFF0089F2, target);
    draw_rect_f(x + 15, y + 29, 10, 4, 0xFF808080, target);
    draw_rect_f(x + 10, y + 33, 20, 3, 0xFF999999, target);
    break;
  case APP_TERMINAL:
    // Dark glass terminal with prompt
    draw_rect_f(x + 3, y + 5, 34, 30, 0xFF2DB84D, target);
    draw_rect_f(x + 5, y + 7, 30, 26, 0xFF1A3A25, target);
    // Traffic light dots
    draw_rect_f(x + 8, y + 9, 3, 3, 0xFFef4444, target);
    draw_rect_f(x + 13, y + 9, 3, 3, 0xFFf59e0b, target);
    draw_rect_f(x + 18, y + 9, 3, 3, 0xFF10b981, target);
    // Arrow ">" shape
    draw_rect_f(x + 8, y + 16, 3, 3, W, target);
    draw_rect_f(x + 11, y + 19, 3, 3, W, target);
    draw_rect_f(x + 14, y + 22, 3, 3, W, target);
    draw_rect_f(x + 11, y + 25, 3, 3, W, target);
    draw_rect_f(x + 8, y + 28, 3, 3, W, target);
    // Underscore cursor
    draw_rect_f(x + 19, y + 28, 10, 3, W, target);
    break;
  case APP_CALCULATOR:
    // Cyan bg with calculator layout
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF00AEEF, target);
    // Display
    draw_rect_f(x + 7, y + 6, 26, 10, 0xFF003E5C, target);
    draw_rect_f(x + 25, y + 10, 6, 4, 0xFF00E676, target);
    // 3x3 button grid
    for (int r = 0; r < 3; r++)
      for (int c = 0; c < 3; c++)
        draw_rect_f(x + 7 + c * 9, y + 19 + r * 7, 7, 5, W, target);
    // Orange equals
    draw_rect_f(x + 27, y + 33, 8, 5, 0xFFFF9500, target);
    break;
  case APP_EDITOR:
    // Blue bg with document page
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF5B8BF5, target);
    // White page
    draw_rect_f(x + 9, y + 5, 24, 30, W, target);
    draw_rect_f(x + 6, y + 8, 3, 24, W, target);   // Left margin
    // Fold corner
    draw_rect_f(x + 28, y + 5, 5, 5, W90, target);
    // Code-like text lines
    draw_rect_f(x + 12, y + 11, 16, 3, 0xFF3B82F6, target);
    draw_rect_f(x + 12, y + 17, 18, 3, 0xFF64748B, target);
    draw_rect_f(x + 12, y + 23, 12, 3, 0xFF64748B, target);
    draw_rect_f(x + 12, y + 29, 16, 3, 0xFF64748B, target);
    break;
  case APP_PAINT:
    // Pink/red bg with canvas and brush
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFFFF6B81, target);
    // Canvas
    draw_rect_f(x + 6, y + 6, 20, 26, W, target);
    // Paint spots
    draw_rect_f(x + 9, y + 10, 6, 6, 0xFFFF3B30, target);
    draw_rect_f(x + 17, y + 16, 5, 5, 0xFF34C759, target);
    draw_rect_f(x + 10, y + 22, 8, 5, 0xFF007AFF, target);
    // Brush handle
    draw_rect_f(x + 26, y + 6, 4, 20, 0xFF8B5E3C, target);
    // Brush tip
    draw_rect_f(x + 25, y + 26, 6, 7, 0xFFFFCC02, target);
    break;
  case APP_FILEMGR:
    // Yellow bg with folder
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFFFFC107, target);
    // Folder tab
    draw_rect_f(x + 6, y + 10, 14, 5, 0xFFF5F5DC, target);
    // Folder body
    draw_rect_f(x + 5, y + 14, 30, 18, 0xFFF5F5DC, target);
    // Front flap
    draw_rect_f(x + 5, y + 18, 30, 14, W, target);
    draw_rect_f(x + 5, y + 18, 30, 2, 0xFFE8E8E0, target);
    break;
  case APP_TASKMGR:
    // Green bg with bar chart
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF43A047, target);
    // Three ascending bars
    draw_rect_f(x + 7, y + 24, 8, 12, W, target);
    draw_rect_f(x + 16, y + 16, 8, 20, W90, target);
    draw_rect_f(x + 25, y + 8, 8, 28, 0xFF00E676, target);
    break;
  case APP_BROWSER:
    // Teal bg with Safari compass
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF00B4D8, target);
    // Outer ring
    draw_rect_f(x + 6, y + 6, 28, 28, W, target);
    // Inner colored circle
    draw_rect_f(x + 8, y + 8, 24, 24, 0xFF009BBA, target);
    // Compass needle - red (NE)
    draw_rect_f(x + 20, y + 12, 3, 3, 0xFFFF3B30, target);
    draw_rect_f(x + 23, y + 9, 3, 3, 0xFFFF3B30, target);
    draw_rect_f(x + 22, y + 11, 3, 3, 0xFFFF3B30, target);
    draw_rect_f(x + 25, y + 8, 3, 3, 0xFFFF3B30, target);
    // Compass needle - white (SW)
    draw_rect_f(x + 14, y + 22, 3, 3, W, target);
    draw_rect_f(x + 17, y + 19, 3, 3, W, target);
    draw_rect_f(x + 12, y + 24, 3, 3, W, target);
    draw_rect_f(x + 15, y + 21, 3, 3, W, target);
    // Center dot
    draw_rect_f(x + 19, y + 19, 3, 3, W, target);
    break;
  case APP_VIDEOPLAYER:
    // Purple bg with play triangle
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFFAB47BC, target);
    // Play triangle
    draw_rect_f(x + 14, y + 8, 4, 24, W, target);
    draw_rect_f(x + 18, y + 11, 4, 18, W, target);
    draw_rect_f(x + 22, y + 14, 4, 12, W, target);
    draw_rect_f(x + 26, y + 17, 3, 6, W, target);
    break;
  case APP_SETTINGS:
    // Gray bg with gear icon
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF8E8E93, target);
    // Gear cross arms
    draw_rect_f(x + 17, y + 5, 7, 30, W90, target);   // Vertical
    draw_rect_f(x + 5, y + 15, 30, 7, W90, target);    // Horizontal
    // Diagonal teeth
    draw_rect_f(x + 9, y + 8, 7, 7, W90, target);
    draw_rect_f(x + 24, y + 8, 7, 7, W90, target);
    draw_rect_f(x + 9, y + 25, 7, 7, W90, target);
    draw_rect_f(x + 24, y + 25, 7, 7, W90, target);
    // Center circle
    draw_rect_f(x + 14, y + 13, 12, 12, 0xFF8E8E93, target);
    draw_rect_f(x + 15, y + 14, 10, 10, W60, target);
    // Inner dot
    draw_rect_f(x + 18, y + 17, 5, 5, 0xFF8E8E93, target);
    break;
  case APP_PDFREADER:
    // Dark red bg with document
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFFCC3333, target);
    // White page
    draw_rect_f(x + 9, y + 5, 22, 30, W, target);
    // Fold corner
    draw_rect_f(x + 26, y + 5, 5, 5, W90, target);
    // Red "PDF" label
    draw_rect_f(x + 12, y + 16, 16, 9, 0xFFFF3B30, target);
    // Text lines
    draw_rect_f(x + 12, y + 28, 16, 2, W60, target);
    draw_rect_f(x + 12, y + 32, 10, 2, W60, target);
    break;
  case APP_CAMERA:
    // Camera icon
    draw_rect_f(x + 5, y + 10, 30, 22, 0xFF333333, target);
    draw_rect_f(x + 12, y + 7, 16, 4, 0xFF333333, target);
    draw_rect_f(x + 14, y + 13, 12, 12, 0xFF111111, target);
    draw_rect_f(x + 16, y + 15, 8, 8, 0xFF0078D4, target);
    draw_rect_f(x + 22, y + 15, 2, 2, W, target);
    draw_rect_f(x + 7, y + 12, 4, 3, 0xFF666666, target);
    break;
  case APP_PHOTOS:
    // Colorful landscape
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF1E88E5, target);
    // Photo frame
    draw_rect_f(x + 5, y + 7, 30, 26, W, target);
    // Sky
    draw_rect_f(x + 6, y + 8, 28, 14, 0xFF87CEEB, target);
    // Sun
    draw_rect_f(x + 26, y + 10, 6, 6, 0xFFFFEB3B, target);
    // Mountains
    draw_rect_f(x + 6, y + 18, 16, 7, 0xFF4CAF50, target);
    draw_rect_f(x + 16, y + 15, 18, 10, 0xFF2E7D32, target);
    // Ground
    draw_rect_f(x + 6, y + 25, 28, 7, 0xFF388E3C, target);
    break;
  case APP_MAIL:
    // Blue bg with envelope
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF5C9CE5, target);
    // Envelope body
    draw_rect_f(x + 5, y + 11, 30, 20, W, target);
    // Envelope flap (V-shape)
    for (int i = 0; i < 15; i++) {
      draw_rect_f(x + 5 + i, y + 11 + i, 2, 1, W90, target);
      draw_rect_f(x + 33 - i, y + 11 + i, 2, 1, W90, target);
    }
    // Blue seal
    draw_rect_f(x + 17, y + 23, 6, 4, 0xFF007AFF, target);
    break;
  case APP_RECORDER:
    // Orange bg with microphone
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFFFF6600, target);
    // Mic head (rounded capsule)
    draw_rect_f(x + 13, y + 5, 14, 20, W, target);
    draw_rect_f(x + 15, y + 4, 10, 1, W, target);   // rounded top
    // Mic stand
    draw_rect_f(x + 18, y + 25, 4, 6, W90, target);
    // Base
    draw_rect_f(x + 12, y + 31, 16, 3, W90, target);
    // Red recording dot
    draw_rect_f(x + 28, y + 6, 6, 6, 0xFFFF3B30, target);
    break;
  case APP_CHAT:
    // Green bg with speech bubble
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF25D366, target);
    // Bubble body
    draw_rect_f(x + 5, y + 6, 30, 22, W, target);
    // Tail
    draw_rect_f(x + 8, y + 28, 8, 4, W, target);
    draw_rect_f(x + 8, y + 32, 4, 3, W, target);
    // Text lines
    draw_rect_f(x + 10, y + 12, 18, 3, 0xFF25D366, target);
    draw_rect_f(x + 10, y + 18, 22, 3, 0xFF25D366, target);
    draw_rect_f(x + 10, y + 24, 14, 3, 0xFF8FE8A8, target);
    break;
  case APP_PHONE:
    // Blue bg with phone handset
    draw_rect_f(x + 3, y + 3, 34, 34, 0xFF0078D4, target);
    // Earpiece
    draw_rect_f(x + 8, y + 8, 9, 10, W, target);
    draw_rect_f(x + 10, y + 10, 5, 6, 0xFF0078D4, target);   // Ear hole
    // Handle connecting
    draw_rect_f(x + 15, y + 16, 10, 5, W, target);
    draw_rect_f(x + 16, y + 20, 8, 4, W, target);
    // Mouthpiece
    draw_rect_f(x + 23, y + 22, 9, 10, W, target);
    draw_rect_f(x + 25, y + 24, 5, 6, 0xFF0078D4, target);   // Mouth hole
    break;
  } // end switch
} // end draw_icon

uint32_t blend_color(uint32_t c1, uint32_t c2, int step, int total) {
  int b1 = c1 & 0xFF;
  int b2 = c2 & 0xFF;
  int b = b1 + (b2 - b1) * step / total;

  int g1 = (c1 >> 8) & 0xFF;
  int g2 = (c2 >> 8) & 0xFF;
  int g = g1 + (g2 - g1) * step / total;

  return (0 << 16) | (g << 8) | b;
}

// Helper: convert RGB to 32-bit ARGB
static inline uint32_t wp_argb32(uint8_t r, uint8_t g, uint8_t b) {
  return 0xFF000000 | (r << 16) | (g << 8) | b;
}

// Cached Desktop Buffer Optimization
// Made global for compositor access

static inline int cubic_interp(int p0, int p1, int p2, int p3, int t) {
  int t2 = (t * t) >> 8;
  int t3 = (t2 * t) >> 8;

  int w0 = -t3 + 2 * t2 - t;
  int w1 = 3 * t3 - 5 * t2 + 512;
  int w2 = -3 * t3 + 4 * t2 + t;
  int w3 = t3 - t2;

  int res = (p0 * w0 + p1 * w1 + p2 * w2 + p3 * w3) >> 9;
  if (res < 0)
    res = 0;
  if (res > 255)
    res = 255;
  return res;
}

static inline uint32_t bicubic_interpolate(const uint8_t *pixels, int bw,
                                           int bh, int row_stride, int bx,
                                           int by, int dx, int dy) {
  int p[4][3];

  int ys[4] = {by + 1, by, by - 1, by - 2};
  int xs[4] = {bx - 1, bx, bx + 1, bx + 2};

  for (int i = 0; i < 4; i++) {
    if (xs[i] < 0)
      xs[i] = 0;
    if (xs[i] >= bw)
      xs[i] = bw - 1;
  }

  for (int i = 0; i < 4; i++) {
    int y = ys[i];
    if (y < 0)
      y = 0;
    if (y >= bh)
      y = bh - 1;

    int r_off = y * row_stride;
    const uint8_t *p0 = pixels + r_off + xs[0] * 3;
    const uint8_t *p1 = pixels + r_off + xs[1] * 3;
    const uint8_t *p2 = pixels + r_off + xs[2] * 3;
    const uint8_t *p3 = pixels + r_off + xs[3] * 3;

    p[i][0] = cubic_interp(p0[0], p1[0], p2[0], p3[0], dx);
    p[i][1] = cubic_interp(p0[1], p1[1], p2[1], p3[1], dx);
    p[i][2] = cubic_interp(p0[2], p1[2], p2[2], p3[2], dx);
  }

  int b = cubic_interp(p[0][0], p[1][0], p[2][0], p[3][0], dy);
  int g = cubic_interp(p[0][1], p[1][1], p[2][1], p[3][1], dy);
  int r = cubic_interp(p[0][2], p[1][2], p[2][2], p[3][2], dy);

  return wp_argb32(r, g, b);
}

static int cache_valid = 0;
void desktop_invalidate() { cache_valid = 0; }

static int desktop_last_w = 0, desktop_last_h = 0;

void desktop_draw() {
  if (desktop_buffer && (desktop_last_w != screen_width || desktop_last_h != screen_height)) {
    kfree(desktop_buffer);
    desktop_buffer = 0;
  }

  if (!desktop_buffer) {
    desktop_buffer = (uint32_t *)kmalloc(screen_width * screen_height * 4);
    desktop_last_w = screen_width;
    desktop_last_h = screen_height;
    if (!desktop_buffer)
      return;
  }

  if (desktop_buffer && !cache_valid) {
    print_serial("DESKTOP: cache_valid is 0, re-rendering wallpaper...\n");
    uint32_t *target = desktop_buffer;
    extern os_config_t global_config;
    int valid = 0;

    // Choose wallpaper based on index
    char wall_path[32];
    char idx_str[4];
    k_itoa(global_config.wallpaper_index + 1, idx_str);
    
    // Try primary path (uppercase, rooted)
    strcpy(wall_path, "/WALL");
    strcat(wall_path, idx_str);
    strcat(wall_path, ".JPG");

    file_entry_t *entry = fs_find(wall_path);
    if (!entry) {
        // Try lowercase
        strcpy(wall_path, "/wall");
        strcat(wall_path, idx_str);
        strcat(wall_path, ".jpg");
        entry = fs_find(wall_path);
    }
    
    if (!entry) entry = fs_find("/wallpaper.png");
    if (!entry) entry = fs_find("/1desktop.png");

    if (entry) {
      print_serial("DESKTOP: Found wallpaper file: ");
      print_serial(entry->name);
      print_serial("\n");
      int bw, bh, channels;
      unsigned char *pixels = NULL;
      unsigned char *file_buf = (unsigned char *)kmalloc(entry->size);
      
      if (file_buf) {
        fs_read(entry->name, file_buf);
        pixels = stbi_load_from_memory(file_buf, entry->size, &bw, &bh, &channels, 4);
        kfree(file_buf);
      }

      if (pixels) {
        print_serial("DESKTOP: Wallpaper Loaded from Disk Successfully\n");
        valid = 1;
        for (int sy = 0; sy < screen_height; sy++) {
          for (int sx = 0; sx < screen_width; sx++) {
            int gx = (int)(((uint64_t)sx * (uint64_t)bw << 8) / screen_width);
            int gy = (int)(((uint64_t)sy * (uint64_t)bh << 8) / screen_height);

            int bx = gx >> 8;
            int by = gy >> 8;

            if (bx >= bw - 1)
              bx = bw - 2;
            if (by >= bh - 1)
              by = bh - 2;
            if (bx < 0)
              bx = 0;
            if (by < 0)
              by = 0;

            int dx = gx & 0xFF;
            int dy = gy & 0xFF;

            const uint8_t *p00 = pixels + (by * bw + bx) * 4;
            const uint8_t *p10 = p00 + 4;
            const uint8_t *p01 = pixels + ((by + 1) * bw + bx) * 4;
            const uint8_t *p11 = p01 + 4;

            int w00 = (256 - dx) * (256 - dy);
            int w10 = dx * (256 - dy);
            int w01 = (256 - dx) * dy;
            int w11 = dx * dy;

            uint8_t r =
                (p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11) >>
                16;
            uint8_t g =
                (p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11) >>
                16;
            uint8_t b =
                (p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11) >>
                16;

            target[sy * screen_width + sx] = wp_argb32(r, g, b);
          }
        }
        stbi_image_free(pixels);
      } else {
        print_serial("DESKTOP: PNG Wallpaper Load FAILED!\n");
      }
    } else if (global_config.wallpaper_type == 0) {
      const uint8_t *bmp = wallpaper_bmp_data;
      valid = (bmp[0] == 'B' && bmp[1] == 'M');

      if (valid) {
        uint32_t poff =
            bmp[10] | (bmp[11] << 8) | (bmp[12] << 16) | (bmp[13] << 24);
        int bw = bmp[18] | (bmp[19] << 8) | (bmp[20] << 16) | (bmp[21] << 24);
        int bh = bmp[22] | (bmp[23] << 8) | (bmp[24] << 16) | (bmp[25] << 24);
        int bpp = bmp[28] | (bmp[29] << 8);

        if (bpp == 24 && bw > 0 && bh > 0) {
          int row_stride = ((bw * 3 + 3) & ~3);
          const uint8_t *pixels = bmp + poff;

          for (int sy = 0; sy < screen_height; sy++) {
            for (int sx = 0; sx < screen_width; sx++) {
              int gx = (int)(((uint64_t)sx * (uint64_t)bw << 8) / screen_width);
              int gy = (int)(((uint64_t)sy * (uint64_t)bh << 8) / screen_height);

              int bx = gx >> 8;
              int by = bh - 1 - (gy >> 8);

              if (bx >= bw - 1)
                bx = bw - 2;
              if (by <= 0)
                by = 1;
              if (bx < 0)
                bx = 0;
              if (by >= bh)
                by = bh - 1;

              int dx = gx & 0xFF;
              int dy = gy & 0xFF;

              const uint8_t *p00 = pixels + by * row_stride + bx * 3;
              const uint8_t *p10 = p00 + 3;
              const uint8_t *p01 = pixels + (by - 1) * row_stride + bx * 3;
              const uint8_t *p11 = p01 + 3;

              int w00 = (256 - dx) * (256 - dy);
              int w10 = dx * (256 - dy);
              int w01 = (256 - dx) * dy;
              int w11 = dx * dy;

              uint8_t b =
                  (p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11) >>
                  16;
              uint8_t g =
                  (p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11) >>
                  16;
              uint8_t r =
                  (p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11) >>
                  16;

              target[sy * screen_width + sx] = wp_argb32(r, g, b);
            }
          }
        } else {
          valid = 0;
        }
      }
    }

    if (!valid) {
      for (int y = 0; y < screen_height; y++) {
        uint32_t col;
        if (global_config.wallpaper_type == 2) {
          col = blend_color(0xFFE0E5EC, 0xFFFFFFFF, y, screen_height) |
                0xFF000000;
        } else {
          col = blend_color(0xFF0078D7, 0xFF004080, y, screen_height) |
                0xFF000000;
        }
        for (int x = 0; x < screen_width; x++) {
          target[y * screen_width + x] = col;
        }
      }
    }
    cache_valid = 1;
    extern void compositor_update_blurred_bg(void);
    compositor_update_blurred_bg();
  }

  if (desktop_buffer && cache_valid) {
    memcpy(backbuffer, desktop_buffer, screen_width * screen_height * 4);
  }
}

// Render dynamic icons to target (Backbuffer)
// Helper to draw string to buffer with grayscale antialiasing
static void draw_string_to_trg(uint32_t *buf, int x, int y, const char *str,
                               uint32_t color) {
  extern uint8_t font_get_aa_pixel(unsigned char c, int x, int y);
  
  uint32_t s_r = (color >> 16) & 0xFF;
  uint32_t s_g = (color >> 8) & 0xFF;
  uint32_t s_b = color & 0xFF;
  
  while (*str) {
    for (int cy = -1; cy < 9; cy++) {
      for (int cx = -1; cx < 9; cx++) {
        int px = x + cx;
        int py = y + cy;
        if (px < 0 || px >= screen_width || py < 0 || py >= screen_height)
          continue;
        
        uint8_t alpha = font_get_aa_pixel((unsigned char)*str, cx, cy);
        if (alpha <= 4) continue;
        
        if (alpha >= 250) {
          buf[py * screen_width + px] = 0xFF000000 | (s_r << 16) | (s_g << 8) | s_b;
        } else {
          uint32_t dst = buf[py * screen_width + px];
          uint32_t d_r = (dst >> 16) & 0xFF;
          uint32_t d_g = (dst >> 8) & 0xFF;
          uint32_t d_b = dst & 0xFF;
          uint32_t r = d_r + (((int)s_r - (int)d_r) * alpha >> 8);
          uint32_t g = d_g + (((int)s_g - (int)d_g) * alpha >> 8);
          uint32_t b = d_b + (((int)s_b - (int)d_b) * alpha >> 8);
          buf[py * screen_width + px] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
    }
    x += 8;
    str++;
  }
}


// Helper to draw pixel to buffer
static void put_pixel_trg(uint32_t *buf, int x, int y, uint32_t c) {
  if (x < 0 || x >= screen_width || y < 0 || y >= screen_height)
    return;
  buf[y * screen_width + x] = c;
}

// Helper to draw rect to buffer
static void draw_rect_trg(uint32_t *buf, int x, int y, int w, int h,
                          uint32_t c) {
  for (int iy = 0; iy < h; iy++) {
    for (int ix = 0; ix < w; ix++) {
      int px = x + ix;
      int py = y + iy;
      if (px >= 0 && px < screen_width && py >= 0 && py < screen_height)
        buf[py * screen_width + px] = c;
    }
  }
}

void desktop_render_icons(uint32_t *target, rect_t clip) {
  if (!target)
    return;

  // Check global config for desktop icons visibility
  extern os_config_t global_config;
  if (!global_config.show_desktop_icons)
    return;

  // Icons
  for (int i = 0; i < icon_count; i++) {
    // Icon hit box for clipping (approx 80x100)
    rect_t icon_r = {icons[i].x - 8, icons[i].y - 8, 80, 100};
    rect_t overlap;
    if (!rect_intersect(clip, icon_r, &overlap))
      continue;

    // Selection Highlight — solid gray like Windows
    if (icons[i].selected) {
      draw_rect_trg(target, icons[i].x - 8, icons[i].y - 8, 80, 96, 0xFF555555);
    }
    // Hover Highlight — transparent gray overlay
    else if (i == hovered_icon) {
      // Blend transparent gray over the background
      for (int hy = 0; hy < 96; hy++) {
        for (int hx = 0; hx < 80; hx++) {
          int px = icons[i].x - 8 + hx;
          int py = icons[i].y - 8 + hy;
          if (px >= 0 && px < screen_width && py >= 0 && py < screen_height) {
            uint32_t dst = target[py * screen_width + px];
            // Blend 25% white over existing pixel
            uint32_t r = (((dst >> 16) & 0xFF) * 192 + 255 * 64) >> 8;
            uint32_t g = (((dst >> 8) & 0xFF) * 192 + 255 * 64) >> 8;
            uint32_t b = ((dst & 0xFF) * 192 + 255 * 64) >> 8;
            target[py * screen_width + px] = 0xFF000000 | (r << 16) | (g << 8) | b;
          }
        }
      }
    }

    // Draw app icon natively without bounding box background
    draw_icon(icons[i].x, icons[i].y, 64, 64, icons[i].type, target);

    uint32_t text_col = icons[i].selected ? 0xFFFFFFFF : theme_get()->fg;
    int text_len = 0;
    for (int c = 0; icons[i].label[c]; c++) text_len += 8;
    int text_x = icons[i].x + 32 - (text_len / 2); // Center around 64px width
    draw_string_to_trg(target, text_x, icons[i].y + 70, icons[i].label,
                       text_col);
  }
}

// --- DESKTOP WIDGETS ---

static void put_pixel_blend(uint32_t *target, int x, int y, uint32_t color,
                            rect_t clip) {
  if (x < clip.x || x >= clip.x + clip.w || y < clip.y || y >= clip.y + clip.h)
    return;
  if (x < 0 || x >= screen_width || y < 0 || y >= screen_height)
    return;
  uint32_t alpha = (color >> 24) & 0xFF;
  if (alpha == 255) {
    target[y * screen_width + x] = color;
  } else if (alpha > 0) {
    uint32_t dst = target[y * screen_width + x];
    uint32_t r = (((color >> 16) & 0xFF) * alpha +
                  ((dst >> 16) & 0xFF) * (255 - alpha)) >>
                 8;
    uint32_t g =
        (((color >> 8) & 0xFF) * alpha + ((dst >> 8) & 0xFF) * (255 - alpha)) >>
        8;
    uint32_t b = ((color & 0xFF) * alpha + (dst & 0xFF) * (255 - alpha)) >> 8;
    target[y * screen_width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
  }
}

static void draw_rounded_rect_trg(uint32_t *target, int x, int y, int w, int h,
                                  int r, uint32_t color, rect_t clip) {
  int x1 = (x > clip.x) ? x : clip.x;
  int y1 = (y > clip.y) ? y : clip.y;
  int x2 = (x + w < clip.x + clip.w) ? x + w : clip.x + clip.w;
  int y2 = (y + h < clip.y + clip.h) ? y + h : clip.y + clip.h;

  for (int dy = y1; dy < y2; dy++) {
    uint32_t *row = &target[dy * screen_width];
    int cy = dy - y;
    for (int dx = x1; dx < x2; dx++) {
      int cx = dx - x;

      int is_corner = 0;
      if (cx < r && cy < r) { // TL
        int ddx = r - cx - 1, ddy = r - cy - 1;
        if (ddx * ddx + ddy * ddy >= r * r)
          is_corner = 1;
      } else if (cx >= w - r && cy < r) { // TR
        int ddx = cx - (w - r), ddy = r - cy - 1;
        if (ddx * ddx + ddy * ddy >= r * r)
          is_corner = 1;
      } else if (cx < r && cy >= h - r) { // BL
        int ddx = r - cx - 1, ddy = cy - (h - r);
        if (ddx * ddx + ddy * ddy >= r * r)
          is_corner = 1;
      } else if (cx >= w - r && cy >= h - r) { // BR
        int ddx = cx - (w - r), ddy = cy - (h - r);
        if (ddx * ddx + ddy * ddy >= r * r)
          is_corner = 1;
      }

      if (!is_corner) {
        uint32_t alpha = (color >> 24) & 0xFF;
        if (alpha == 255) {
          row[dx] = color;
        } else {
          uint32_t dst = row[dx];
          uint32_t br = (((color >> 16) & 0xFF) * alpha +
                         ((dst >> 16) & 0xFF) * (255 - alpha)) >>
                        8;
          uint32_t bg = (((color >> 8) & 0xFF) * alpha +
                         ((dst >> 8) & 0xFF) * (255 - alpha)) >>
                        8;
          uint32_t bb =
              ((color & 0xFF) * alpha + (dst & 0xFF) * (255 - alpha)) >> 8;
          row[dx] = 0xFF000000 | (br << 16) | (bg << 8) | bb;
        }
      }
    }
  }
}

static void draw_string_large(uint32_t *target, int x, int y, const char *str,
                              uint32_t color, int scale, rect_t clip) {
  extern uint8_t font_get_aa_pixel(unsigned char c, int x, int y);
  extern uint8_t font_get_aa_pixel_16(unsigned char c, int x, int y);
  
  uint32_t s_r = (color >> 16) & 0xFF;
  uint32_t s_g = (color >> 8) & 0xFF;
  uint32_t s_b = color & 0xFF;
  
  // Total output size per glyph
  int out_size = 8 * scale;
  // Use 16x16 source for scale >= 2 (output >= 16px), 8x8 AA for scale 1
  int use_16 = (scale >= 2);
  
  while (*str) {
    for (int cy = 0; cy < out_size; cy++) {
      for (int cx = 0; cx < out_size; cx++) {
        int gx = x + cx;
        int gy = y + cy;
        
        if (gx < clip.x || gx >= clip.x + clip.w ||
            gy < clip.y || gy >= clip.y + clip.h)
          continue;
        if (gx < 0 || gx >= screen_width || gy < 0 || gy >= screen_height)
          continue;
        
        int alpha;
        
        if (use_16) {
          // Map [0..out_size) -> [0..16) with 8.8 fixed point
          int src_x_fp = (cx * (16 << 8)) / out_size;
          int src_y_fp = (cy * (16 << 8)) / out_size;
          if (src_x_fp > (15 << 8)) src_x_fp = 15 << 8;
          if (src_y_fp > (15 << 8)) src_y_fp = 15 << 8;
          
          int sx = src_x_fp >> 8;
          int sy = src_y_fp >> 8;
          int fx = src_x_fp & 0xFF;
          int fy = src_y_fp & 0xFF;
          
          uint8_t a00 = font_get_aa_pixel_16((unsigned char)*str, sx,     sy);
          uint8_t a10 = font_get_aa_pixel_16((unsigned char)*str, sx + 1, sy);
          uint8_t a01 = font_get_aa_pixel_16((unsigned char)*str, sx,     sy + 1);
          uint8_t a11 = font_get_aa_pixel_16((unsigned char)*str, sx + 1, sy + 1);
          
          int top    = a00 + ((a10 - a00) * fx >> 8);
          int bottom = a01 + ((a11 - a01) * fx >> 8);
          alpha = top + ((bottom - top) * fy >> 8);
        } else {
          // Map [0..out_size) -> [-1..9) for 10x10 AA cache
          int src_x_fp = (cx * (10 << 8)) / out_size - (1 << 8);
          int src_y_fp = (cy * (10 << 8)) / out_size - (1 << 8);
          if (src_x_fp < -(1 << 8)) src_x_fp = -(1 << 8);
          if (src_y_fp < -(1 << 8)) src_y_fp = -(1 << 8);
          if (src_x_fp > (8 << 8))  src_x_fp = (8 << 8);
          if (src_y_fp > (8 << 8))  src_y_fp = (8 << 8);
          
          int sx = src_x_fp >> 8;
          int sy = src_y_fp >> 8;
          int fx = src_x_fp & 0xFF;
          int fy = src_y_fp & 0xFF;
          
          uint8_t a00 = font_get_aa_pixel((unsigned char)*str, sx,     sy);
          uint8_t a10 = font_get_aa_pixel((unsigned char)*str, sx + 1, sy);
          uint8_t a01 = font_get_aa_pixel((unsigned char)*str, sx,     sy + 1);
          uint8_t a11 = font_get_aa_pixel((unsigned char)*str, sx + 1, sy + 1);
          
          int top    = a00 + ((a10 - a00) * fx >> 8);
          int bottom = a01 + ((a11 - a01) * fx >> 8);
          alpha = top + ((bottom - top) * fy >> 8);
        }
        
        if (alpha <= 4) continue;
        if (alpha > 255) alpha = 255;
        
        if (alpha >= 250) {
          target[gy * screen_width + gx] = 0xFF000000 | (s_r << 16) | (s_g << 8) | s_b;
        } else {
          uint32_t dst = target[gy * screen_width + gx];
          uint32_t d_r = (dst >> 16) & 0xFF;
          uint32_t d_g = (dst >> 8) & 0xFF;
          uint32_t d_b = dst & 0xFF;
          uint32_t r = d_r + (((int)s_r - (int)d_r) * alpha >> 8);
          uint32_t g = d_g + (((int)s_g - (int)d_g) * alpha >> 8);
          uint32_t b = d_b + (((int)s_b - (int)d_b) * alpha >> 8);
          target[gy * screen_width + gx] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
    }
    x += 8 * scale;
    str++;
  }

}


// Widget State
static int dragging_widget = 0; // 1=Clock, 2=Calendar
static int resizing_widget = 0; // 1=Clock, 2=Calendar
static int widget_off_x = 0;
static int widget_off_y = 0;

void draw_clock_widget(uint32_t *target, int x, int y, int w, int h,
                       rect_t clip) {
  rtc_time_t now = cached_time;

  const theme_t *theme = theme_get();
 
  // Auto-font-scale based on width
  int font_scale = w / 50;
  if (font_scale < 1)
    font_scale = 1;
  if (font_scale > 8)
    font_scale = 8;

  // Background card (Glassmorphism based on theme)
  uint32_t widget_bg = (theme->menu_bg & 0x00FFFFFF) | 0x80000000;
  draw_rounded_rect_trg(target, x, y, w, h, 20, widget_bg, clip);

  // Border if active
  if (dragging_widget == 1 || resizing_widget == 1) {
    // Draw thin border
    for (int i = 0; i < w; i++) {
      put_pixel_blend(target, x + i, y, 0xFFFFFFFF, clip);
      put_pixel_blend(target, x + i, y + h - 1, 0xFFFFFFFF, clip);
    }
    for (int i = 0; i < h; i++) {
      put_pixel_blend(target, x, y + i, 0xFFFFFFFF, clip);
      put_pixel_blend(target, x + w - 1, y + i, 0xFFFFFFFF, clip);
    }
  }
  // Resize handle
  draw_rounded_rect_trg(target, x + w - 10, y + h - 10, 10, 10, 5, 0xFFAAAAAA,
                        clip);

  char time_str[16];
  char h_str[4], m_str[4];
  k_itoa(now.hour, h_str);
  if (now.hour < 10) {
    time_str[0] = '0';
    time_str[1] = h_str[0];
    time_str[2] = 0;
  } else
    strcpy(time_str, h_str);
  strcat(time_str, ":");
  k_itoa(now.minute, m_str);
  if (now.minute < 10)
    strcat(time_str, "0");
  strcat(time_str, m_str);

  // Center time
  int time_len = strlen(time_str) * 8 * font_scale;
  int tx = x + (w - time_len) / 2;
  int ty = y + (h / 2) - (8 * font_scale / 2) - 5;
  draw_string_large(target, tx, ty, time_str, theme->fg, font_scale, clip);

  char date_str[32];
  char d[4];
  k_itoa(now.day, d);
  const char *ms[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  strcpy(date_str, "Today, ");
  strcat(date_str, ms[(now.month - 1) % 12]);
  strcat(date_str, " ");
  strcat(date_str, d);

  int date_len = strlen(date_str) * 8;
  int dx = x + (w - date_len) / 2;
  int dy = ty + (8 * font_scale) + 5;
  if (dy < y + h - 15)
    draw_string_large(target, dx, dy, date_str, theme->fg_secondary, 1, clip);
}

void draw_calendar_widget(uint32_t *target, int x, int y, int w, int h,
                          rect_t clip) {
  rtc_time_t now = cached_time;

  const theme_t *theme = theme_get();
 
  // Card (Glassmorphism based on theme)
  uint32_t widget_bg = (theme->menu_bg & 0x00FFFFFF) | 0xCC000000;
  draw_rounded_rect_trg(target, x, y, w, h, 20, widget_bg, clip);

  // Border if active
  if (dragging_widget == 2 || resizing_widget == 2) {
    for (int i = 0; i < w; i++) {
      put_pixel_blend(target, x + i, y, 0xFFFFFFFF, clip);
      put_pixel_blend(target, x + i, y + h - 1, 0xFFFFFFFF, clip);
    }
    for (int i = 0; i < h; i++) {
      put_pixel_blend(target, x, y + i, 0xFFFFFFFF, clip);
      put_pixel_blend(target, x + w - 1, y + i, 0xFFFFFFFF, clip);
    }
  }
  // Resize handle
  draw_rounded_rect_trg(target, x + w - 10, y + h - 10, 10, 10, 5, 0xFF555555,
                        clip);

  const char *months[] = {"January",   "February", "March",    "April",
                          "May",       "June",     "July",     "August",
                          "September", "October",  "November", "December"};
  char head[32];
  strcpy(head, months[(now.month - 1) % 12]);
  strcat(head, " ");
  char yr[8];
  k_itoa(2000 + now.year, yr);
  strcat(head, yr);

  int font_head = (w > 250) ? 2 : 1;
  draw_string_large(target, x + 10, y + 10, head, theme->fg, font_head, clip);
  draw_string_large(target, x + 10, y + 15 + font_head * 15, "S M T W T F S",
                    theme->fg_secondary, 1, clip);

  int grid_w = (w - 20) / 7;
  int grid_h = (h - 60) / 6;
  if (grid_w < 15)
    grid_w = 15;
  if (grid_h < 15)
    grid_h = 15;

  int day_count = 31;
  if (now.month == 2)
    day_count = 28;
  else if (now.month == 4 || now.month == 6 || now.month == 9 ||
           now.month == 11)
    day_count = 30;

  int cur_d = 1;
  int grid_y = y + 35 + font_head * 15;
  for (int row = 0; row < 6; row++) {
    for (int col = 0; col < 7; col++) {
      if (cur_d > day_count)
        break;
      int dx = x + 10 + col * grid_w;
      int dy = grid_y + row * grid_h;
      if (dy > y + h - 20)
        break;

      char ds[4];
      k_itoa(cur_d, ds);
      if (cur_d == now.day) {
        draw_rounded_rect_trg(target, dx - 2, dy - 2, grid_w - 2, grid_h - 2, 8,
                              0xFF0078D4, clip);
        draw_string_large(target, dx + (cur_d < 10 ? 4 : 0), dy, ds, 0xFFFFFFFF,
                          1, clip);
      } else {
        draw_string_large(target, dx + (cur_d < 10 ? 4 : 0), dy, ds, 0xFFBBBBBB,
                          1, clip);
      }
      cur_d++;
    }
  }
}

void desktop_render_widgets(uint32_t *target, rect_t clip) {
  extern os_config_t global_config;
  if (!global_config.show_clock_widget && !global_config.show_calendar_widget)
    return;

  uint32_t ticks = get_timer_ticks();
  if (ticks - last_time_update > 10 || last_time_update == 0) {
    rtc_read(&cached_time);
    last_time_update = ticks;
  }

  if (global_config.show_clock_widget) {
    int wx = global_config.clock_x, wy = global_config.clock_y;
    int ww = global_config.clock_w, wh = global_config.clock_h;
    if (!(clip.x >= wx + ww || clip.x + clip.w <= wx || clip.y >= wy + wh ||
          clip.y + clip.h <= wy)) {
      draw_clock_widget(target, wx, wy, ww, wh, clip);
    }
  }
  if (global_config.show_calendar_widget) {
    int wx = global_config.calendar_x, wy = global_config.calendar_y;
    int ww = global_config.calendar_w, wh = global_config.calendar_h;
    if (!(clip.x >= wx + ww || clip.x + clip.w <= wx || clip.y >= wy + wh ||
          clip.y + clip.h <= wy)) {
      draw_calendar_widget(target, wx, wy, ww, wh, clip);
    }
  }
}

int start_menu_open = 0;

// Helper to draw a mini 16x16 app icon at (x, y)
static void draw_mini_icon(int x, int y, int type, uint32_t *target) {
  switch (type) {
  case 0: // Terminal: Monitor with green prompt
    draw_rect_f(x + 2, y + 2, 12, 10, 0xFF000000, target);
    draw_rect_f(x + 3, y + 3, 10, 8, 0xFF00FF00, target);
    draw_rect_f(x + 1, y + 1, 14, 12, 0xFF808080, target);
    break;
  case 5: // Files: Folder icon
    draw_rect_f(x + 2, y + 4, 12, 10, 0xFFC0C0C0, target);
    draw_rect_f(x + 2, y + 2, 6, 3, 0xFFC0C0C0, target);
    break;
  case 1: // Calculator: Grid
    draw_rect_f(x + 3, y + 3, 10, 10, 0xFFFFFFFF, target);
    for (int i = 0; i < 3; i++) {
      draw_rect_f(x + 4, y + 4 + i * 3, 8, 2, 0xFF000000, target);
    }
    break;
  case 10: // PDF Reader: Red mini doc
    draw_rect_f(x + 3, y + 2, 10, 12, 0xFFF0F0F0, target);
    draw_rect_f(x + 3, y + 6, 10, 4, 0xFFCC3333, target);
    break;
  default: // Generic App
    draw_rect_f(x + 2, y + 2, 12, 12, 0xFF0000FF, target);
    break;
  }
}

// desktop_draw_overlay removed as it's now integrated into compositor.c

extern void terminal_init();
extern void calculator_init();
extern void editor_init();
extern void paint_init();
extern void explorer_init();
extern void taskmgr_init();
extern void videoplayer_init(const char *path);
extern void settings_init();
extern void pdfreader_init();
extern void camera_app_init();
extern window_t *winmgr_create_window(int, int, int, int, const char *);

// Context menu actions
static void action_new_terminal(void) { terminal_init(); }
static void action_new_editor(void) { editor_init(); }
static void action_refresh(void) {
  extern int ui_dirty;
  ui_dirty = 1;
}

static void action_about(void) {
  window_t *win = winmgr_create_window(-1, -1, 400, 200, "About");
  const char *about = "PureOS v2.0\nBy You!\n32-bit Protected Mode";
  int i = 0;
  while (about[i] && i < 255) {
    win->text_buffer[i] = about[i];
    i++;
  }
  win->text_buffer[i] = 0;
  win->cursor_pos = -1;
}

// Drag State
static int drag_icon = -1;
static int drag_off_x = 0;
static int drag_off_y = 0;
static int was_dragged = 0;

// Double Click State
extern unsigned int get_timer_ticks(void);
static unsigned int last_click_tick = 0;
static int last_click_icon = -1;

// Debug helper
extern void print_serial(const char *str);
static void debug_print_int(int val) {
  if (val < 0) {
    print_serial("-");
    val = -val;
  }
  if (val == 0) {
    print_serial("0");
    return;
  }
  char buf[12];
  int top = 0;
  while (val > 0) {
    buf[top++] = '0' + (val % 10);
    val /= 10;
  }
  while (top > 0) {
    char c[2] = {buf[--top], 0};
    print_serial(c);
  }
}

void desktop_mouse_up(int mx, int my) {
  if (drag_icon != -1) {
    // If we didn't drag, treat as click
    if (!was_dragged) {
      // SIMPLE CONSECUTIVE CLICK: If same icon clicked twice in a row, launch
      // it No timing check - immune to Bochs PIT frequency issues
      if (last_click_icon != -1 && drag_icon == last_click_icon) {
        // DOUBLE CLICK DETECTED - Launch app!
        // print_serial("DOUBLE CLICK LAUNCH!\n");
        if (!start_menu_open) {
          extern int next_anim_origin_x;
          extern int next_anim_origin_y;
          // Set origin to icon center (Icons are roughly 40x60)
          next_anim_origin_x = icons[drag_icon].x + 20;
          next_anim_origin_y = icons[drag_icon].y + 30;

          switch (icons[drag_icon].type) {
          case APP_TERMINAL:
            terminal_init();
            break;
          case APP_CALCULATOR:
            calculator_init();
            break;
          case APP_EDITOR:
            editor_init();
            break;
          case APP_PAINT:
            paint_init();
            break;
          case APP_FILEMGR:
            explorer_init();
            break;
          case APP_TASKMGR:
            taskmgr_init();
            break;
          case APP_COMPUTER:
            explorer_init();
            break;
          case APP_BROWSER:
            browser_init();
            break;
          case APP_VIDEOPLAYER:
            videoplayer_init(NULL);
            break;
          case APP_SETTINGS:
            settings_init();
            break;
          case APP_PDFREADER:
            pdfreader_init();
            break;
          case APP_CAMERA:
            camera_app_init();
            break;
          case APP_PHOTOS: {
            extern void photos_init();
            photos_init();
          } break;
          case APP_MAIL: {
            extern void mail_app_init();
            mail_app_init();
          } break;
          case APP_RECORDER: {
            extern void recorder_init();
            recorder_init();
          } break;
          case APP_CHAT: {
            extern void chat_init();
            chat_init();
          } break;
          case APP_PHONE: {
            extern void phone_init();
            phone_init();
          } break;
          default:
            break;
          }
        }
        // Reset after launch
        last_click_icon = -1;
      } else {
        // First click on this icon - store for next click
        last_click_icon = drag_icon;
      }
    } else {
      // Was a drag - clear state
      last_click_icon = -1;
    }

    // Reset drag state
    drag_icon = -1;
    was_dragged = 0;
  }

  if (dragging_widget || resizing_widget) {
    dragging_widget = 0;
    resizing_widget = 0;
    config_save(); // Save the final position/size
    extern int ui_dirty;
    ui_dirty = 1;
  }
}

void desktop_mouse_move(int mx, int my) {
  // Track hovered icon
  int new_hover = -1;
  extern os_config_t global_config;
  if (global_config.show_desktop_icons) {
    for (int i = 0; i < icon_count; i++) {
      if (mx >= icons[i].x - 8 && mx < icons[i].x + 72 &&
          my >= icons[i].y - 8 && my < icons[i].y + 92) {
        new_hover = i;
        break;
      }
    }
  }
  if (new_hover != hovered_icon) {
    // Invalidate old hover area
    if (hovered_icon >= 0 && hovered_icon < icon_count) {
      compositor_invalidate_rect(icons[hovered_icon].x - 8, icons[hovered_icon].y - 8, 80, 100);
    }
    hovered_icon = new_hover;
    // Invalidate new hover area
    if (hovered_icon >= 0 && hovered_icon < icon_count) {
      compositor_invalidate_rect(icons[hovered_icon].x - 8, icons[hovered_icon].y - 8, 80, 100);
    }
    extern int ui_dirty;
    ui_dirty = 1;
  }

  if (drag_icon != -1) {
    // Threshold to consider it a drag (30 pixels) - higher to prevent Bochs
    // jitter
    if (abs(mx - (icons[drag_icon].x + drag_off_x)) > 30 ||
        abs(my - (icons[drag_icon].y + drag_off_y)) > 30) {
      was_dragged = 1;
    }

    if (was_dragged) {
      int old_x = icons[drag_icon].x;
      int old_y = icons[drag_icon].y;

      icons[drag_icon].x = mx - drag_off_x;
      icons[drag_icon].y = my - drag_off_y;

      // Bounds Check
      if (icons[drag_icon].x < 0)
        icons[drag_icon].x = 0;
      if (icons[drag_icon].y < 0)
        icons[drag_icon].y = 0;
      if (icons[drag_icon].x > screen_width - 32)
        icons[drag_icon].x = screen_width - 32;
      if (icons[drag_icon].y > screen_height - 60)
        icons[drag_icon].y = screen_height - 60;

      // Invalidate OLD position (restores wallpaper)
      compositor_invalidate_rect(old_x - 8, old_y - 8, 80, 100);

      // Invalidate NEW position (draws icon)
      compositor_invalidate_rect(icons[drag_icon].x - 8, icons[drag_icon].y - 8,
                                 80, 100);

      extern int ui_dirty;
      ui_dirty = 1;
    }
  }

  // Handle Resize Cursor Type
  extern int current_cursor_type;
  int prev_cursor = current_cursor_type;
  current_cursor_type = 0; // Default

  if (resizing_widget) {
    current_cursor_type = 1; // Resize
  } else {
    // Check if hovering over handles
    if (global_config.show_clock_widget) {
      int wx = global_config.clock_x, wy = global_config.clock_y;
      int ww = global_config.clock_w, wh = global_config.clock_h;
      if (mx >= wx + ww - 15 && mx < wx + ww && my >= wy + wh - 15 &&
          my < wy + wh) {
        current_cursor_type = 1;
      }
    }
    if (global_config.show_calendar_widget && current_cursor_type == 0) {
      int wx = global_config.calendar_x, wy = global_config.calendar_y;
      int ww = global_config.calendar_w, wh = global_config.calendar_h;
      if (mx >= wx + ww - 15 && mx < wx + ww && my >= wy + wh - 15 &&
          my < wy + wh) {
        current_cursor_type = 1;
      }
    }
  }

  if (current_cursor_type != prev_cursor) {
    extern int ui_dirty;
    ui_dirty = 1;
    // Invalidate old and new cursor positions to force redraw
    compositor_invalidate_rect(mx - 16, my - 16, 32, 32);
  }

  if (dragging_widget) {
    int prev_x, prev_y, prev_w, prev_h;
    if (dragging_widget == 1) {
      prev_x = global_config.clock_x;
      prev_y = global_config.clock_y;
      prev_w = global_config.clock_w;
      prev_h = global_config.clock_h;
      global_config.clock_x = mx - widget_off_x;
      global_config.clock_y = my - widget_off_y;
    } else {
      prev_x = global_config.calendar_x;
      prev_y = global_config.calendar_y;
      prev_w = global_config.calendar_w;
      prev_h = global_config.calendar_h;
      global_config.calendar_x = mx - widget_off_x;
      global_config.calendar_y = my - widget_off_y;
    }
    compositor_invalidate_rect(prev_x - 10, prev_y - 10, prev_w + 20,
                               prev_h + 20);
    if (dragging_widget == 1)
      compositor_invalidate_rect(
          global_config.clock_x - 10, global_config.clock_y - 10,
          global_config.clock_w + 20, global_config.clock_h + 20);
    else
      compositor_invalidate_rect(
          global_config.calendar_x - 10, global_config.calendar_y - 10,
          global_config.calendar_w + 20, global_config.calendar_h + 20);
    extern int ui_dirty;
    ui_dirty = 1;
  }

  if (resizing_widget) {
    int prev_x, prev_y, prev_w, prev_h;
    if (resizing_widget == 1) {
      prev_x = global_config.clock_x;
      prev_y = global_config.clock_y;
      prev_w = global_config.clock_w;
      prev_h = global_config.clock_h;
      global_config.clock_w = mx - global_config.clock_x;
      global_config.clock_h = my - global_config.clock_y;
      if (global_config.clock_w < 100)
        global_config.clock_w = 100;
      if (global_config.clock_h < 100)
        global_config.clock_h = 100;
    } else {
      prev_x = global_config.calendar_x;
      prev_y = global_config.calendar_y;
      prev_w = global_config.calendar_w;
      prev_h = global_config.calendar_h;
      global_config.calendar_w = mx - global_config.calendar_x;
      global_config.calendar_h = my - global_config.calendar_y;
      if (global_config.calendar_w < 150)
        global_config.calendar_w = 150;
      if (global_config.calendar_h < 150)
        global_config.calendar_h = 150;
    }
    compositor_invalidate_rect(prev_x - 10, prev_y - 10, prev_w + 20,
                               prev_h + 20);
    if (resizing_widget == 1)
      compositor_invalidate_rect(
          global_config.clock_x - 10, global_config.clock_y - 10,
          global_config.clock_w + 20, global_config.clock_h + 20);
    else
      compositor_invalidate_rect(
          global_config.calendar_x - 10, global_config.calendar_y - 10,
          global_config.calendar_w + 20, global_config.calendar_h + 20);
    extern int ui_dirty;
    ui_dirty = 1;
  }

  extern ctxmenu_t context_menu;
  if (context_menu.visible) {
    if (mx >= context_menu.x && mx < context_menu.x + context_menu.width &&
        my >= context_menu.y && my < context_menu.y + context_menu.height) {
      compositor_invalidate_rect(context_menu.x, context_menu.y,
                                 context_menu.width, context_menu.height);
      extern int ui_dirty;
      ui_dirty = 1;
    }
  }
}

void desktop_click(int mx, int my, int buttons) {
  // Right-click on desktop -> show context menu
  if (buttons & 2) {
    // ... (Context Menu Logic) ...
    ctxmenu_item_t items[] = {{"New Terminal", action_new_terminal},
                              {"New Editor", action_new_editor},
                              {0, 0},
                              {"Refresh", action_refresh},
                              {"About PureOS", action_about},
                              {0, 0},
                              {"Restart", taskbar_reboot},
                              {"Shut Down", taskbar_shutdown}};
    ctxmenu_show(mx, my, items, 8);
    extern int ui_dirty;
    ui_dirty = 1;
    return;
  }

  // Start Menu interaction is handled by startmenu_handle_mouse()

  // Check Icons (only if desktop icons are visible)
  int clicked_icon = -1;
  extern os_config_t global_config;
  if (global_config.show_desktop_icons) {
    for (int i = 0; i < icon_count; i++) {
      if (mx >= icons[i].x - 8 && mx < icons[i].x + 72 && my >= icons[i].y - 8 &&
          my < icons[i].y + 92) {
        clicked_icon = i;
        break;
      }
    }
  }

  // Selection & Drag Logic
  int selection_changed = 0;
  if (clicked_icon != -1) {
    beep();
    // Initiate Drag
    drag_icon = clicked_icon;
    drag_off_x = mx - icons[clicked_icon].x;
    drag_off_y = my - icons[clicked_icon].y;
    was_dragged = 0;

    // Selection
    for (int i = 0; i < icon_count; i++) {
      if (icons[i].selected != (i == clicked_icon)) {
        icons[i].selected = (i == clicked_icon);
        selection_changed = 1;
        // Invalidate icon area to redraw selection state
        compositor_invalidate_rect(icons[i].x - 8, icons[i].y - 8, 80, 100);
      }
    }

    // REMOVED: Immediate Launch. Now handled in Mouse Up if !was_dragged.

    if (selection_changed) {
      extern int ui_dirty;
      ui_dirty = 1;
    }
    return;
  }

  // Clicked Empty Space -> Deselect All + Reset double-click state
  last_click_icon = -1;

  for (int i = 0; i < icon_count; i++) {
    if (icons[i].selected) {
      icons[i].selected = 0;
      selection_changed = 1;
      // Invalidate to redraw unselected state
      compositor_invalidate_rect(icons[i].x - 5, icons[i].y - 5, 55, 70);
    }
  }

  if (selection_changed) {
    extern int ui_dirty;
    ui_dirty = 1;
  }

  // Check Widgets (Clock)
  if (global_config.show_clock_widget) {
    int wx = global_config.clock_x, wy = global_config.clock_y;
    int ww = global_config.clock_w, wh = global_config.clock_h;
    if (mx >= wx && mx < wx + ww && my >= wy && my < wy + wh) {
      if (mx >= wx + ww - 15 && my >= wy + wh - 15) {
        resizing_widget = 1;
      } else {
        dragging_widget = 1;
        widget_off_x = mx - wx;
        widget_off_y = my - wy;
      }
      extern int ui_dirty;
      ui_dirty = 1;
      return;
    }
  }

  // Check Widgets (Calendar)
  if (global_config.show_calendar_widget) {
    int wx = global_config.calendar_x, wy = global_config.calendar_y;
    int ww = global_config.calendar_w, wh = global_config.calendar_h;
    if (mx >= wx && mx < wx + ww && my >= wy && my < wy + wh) {
      if (mx >= wx + ww - 15 && my >= wy + wh - 15) {
        resizing_widget = 2;
      } else {
        dragging_widget = 2;
        widget_off_x = mx - wx;
        widget_off_y = my - wy;
      }
      extern int ui_dirty;
      ui_dirty = 1;
      return;
    }
  }

  // Taskbar tab clicks and Start button clicks are handled by
  // taskbar_handle_mouse() and startmenu_handle_mouse()
}

void draw_computer_icon(int x, int y, uint32_t *target) {
  // Monitor body (dark frame)
  vga_draw_rect_lfb(x + 3, y + 7, 34, 22, 0xFF333333, target);
  // Screen (cyan desktop)
  vga_draw_rect_lfb(x + 5, y + 9, 30, 18, 0xFF0088AA, target);
  // Screen highlight
  vga_draw_rect_lfb(x + 5, y + 9, 30, 1, 0xFF00BBDD, target);
  // Power LED
  vga_draw_rect_lfb(x + 18, y + 27, 4, 2, 0xFF00FF00, target);
  // Stand
  vga_draw_rect_lfb(x + 16, y + 29, 8, 3, 0xFF555555, target);
  // Base
  vga_draw_rect_lfb(x + 10, y + 32, 20, 3, 0xFF666666, target);
  vga_draw_rect_lfb(x + 10, y + 32, 20, 1, 0xFF888888, target);
  // Screen reflection
  vga_draw_rect_lfb(x + 6, y + 10, 8, 6, 0xFF00AACC, target);
}

void draw_terminal_icon(int x, int y, uint32_t *target) {
  // Black terminal background
  vga_draw_rect_lfb(x + 3, y + 6, 34, 28, 0xFF111111, target);
  // Top bar (dark gray)
  vga_draw_rect_lfb(x + 3, y + 6, 34, 3, 0xFF444444, target);
  // Border
  vga_draw_rect_lfb(x + 3, y + 6, 34, 1, 0xFF666666, target);
  vga_draw_rect_lfb(x + 3, y + 6, 1, 28, 0xFF666666, target);
  // Prompt '>_'
  vga_draw_rect_lfb(x + 6, y + 13, 4, 2, 0xFF00FF00, target);
  vga_draw_rect_lfb(x + 6, y + 15, 2, 2, 0xFF00FF00, target);
  // Text after prompt
  vga_draw_rect_lfb(x + 12, y + 13, 10, 2, 0xFFCCCCCC, target);
  // Cursor (blinking look)
  vga_draw_rect_lfb(x + 24, y + 13, 2, 4, 0xFF00FF00, target);
  // Second line
  vga_draw_rect_lfb(x + 6, y + 21, 20, 2, 0xFF00AA00, target);
}

void draw_calculator_icon(int x, int y, uint32_t *target) {
  // Body
  vga_draw_rect_lfb(x + 6, y + 2, 28, 36, 0xFFCCCCCC, target);
  vga_draw_rect_lfb(x + 6, y + 2, 28, 1, 0xFFEEEEEE, target);
  vga_draw_rect_lfb(x + 6, y + 2, 1, 36, 0xFFEEEEEE, target);
  // Display
  vga_draw_rect_lfb(x + 8, y + 5, 24, 9, 0xFF112211, target);
  vga_draw_rect_lfb(x + 20, y + 7, 10, 5, 0xFF44FF44, target);
  // Buttons grid (3x4)
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 3; c++) {
      uint32_t btn_col = 0xFF888888;
      if (r == 3 && c == 2)
        btn_col = 0xFF4488FF; // = button blue
      if (r == 0)
        btn_col = 0xFFBB6644; // Top row orange
      vga_draw_rect_lfb(x + 8 + c * 8, y + 17 + r * 5, 6, 3, btn_col, target);
    }
  }
}

void draw_editor_icon(int x, int y, uint32_t *target) {
  // Page background
  vga_draw_rect_lfb(x + 6, y + 3, 28, 34, 0xFFFFFFFF, target);
  // Page shadow (right/bottom)
  vga_draw_rect_lfb(x + 34, y + 5, 1, 33, 0xFF888888, target);
  vga_draw_rect_lfb(x + 8, y + 37, 27, 1, 0xFF888888, target);
  // Folded corner
  vga_draw_rect_lfb(x + 28, y + 3, 6, 6, 0xFFCCCCCC, target);
  vga_draw_rect_lfb(x + 28, y + 9, 6, 1, 0xFFAAAAAA, target);
  vga_draw_rect_lfb(x + 28, y + 3, 1, 6, 0xFFAAAAAA, target);
  // Text lines (varying lengths)
  vga_draw_rect_lfb(x + 9, y + 8, 16, 2, 0xFF000000, target);
  vga_draw_rect_lfb(x + 9, y + 13, 20, 2, 0xFF444444, target);
  vga_draw_rect_lfb(x + 9, y + 18, 12, 2, 0xFF444444, target);
  vga_draw_rect_lfb(x + 9, y + 23, 18, 2, 0xFF444444, target);
  vga_draw_rect_lfb(x + 9, y + 28, 14, 2, 0xFF444444, target);
}
