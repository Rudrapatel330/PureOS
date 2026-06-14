#include "settings.h"
#include "../kernel/config.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"
#include "../kernel/ui_layout.h"
#include "../kernel/image.h"
#include "../fs/fs.h"

extern void compositor_invalidate_rect(int x, int y, int w, int h);
extern int screen_width, screen_height;
extern window_t windows[];
extern int window_count;
extern void desktop_invalidate(void);
#include <stddef.h>

static int get_sidebar_width(void) {
    int fs = ui_get_font_scale();
    // Use a long tab name as a benchmark for sidebar width
    return ui_measure_text_width("\xF0\x9F\x8E\xA8 Personalization", fs) + 60;
}
static int get_cx(void) {
    return get_sidebar_width() + 30;
}
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern void print_serial(const char *s);
extern void k_itoa(int n, char *s);



#define PB 0
#define PWALL 1
#define PT 2
#define PC 3
#define PD 4
#define PW 5
#define PF 6
#define NP 7
#define PA 98
#define PE 99

// Colors
#define C0 0xFF202020
#define C1 0xFF2D2D2D
#define C2 0xFF383838
#define CW 0xFFFFFFFF
#define CG 0xFF999999
#define CD 0xFF666666
#define CA 0xFF0078D4
#define CS 0xFF3D3D3D

#define DRAW_HOVER(win, s, rx, ry, rw, rh, bg_col, hov_col) \
  do { \
    if ((s)->mx >= (rx) && (s)->mx < (rx) + (rw) && (s)->my >= (ry) && (s)->my < (ry) + (rh)) \
      winmgr_fill_rect((win), (rx), (ry), (rw), (rh), (hov_col)); \
    else \
      winmgr_fill_rect((win), (rx), (ry), (rw), (rh), (bg_col)); \
  } while(0)

#define DRAW_ROUNDED_CARD(win, s, rx, ry, rw, rh, bg_col, hov_col, border_col) \
  do { \
    uint32_t fill_col = ((s)->mx >= (rx) && (s)->mx < (rx) + (rw) && (s)->my >= (ry) && (s)->my < (ry) + (rh)) ? (hov_col) : (bg_col); \
    winmgr_draw_rounded_rect_ex((win), (rx), (ry), (rw), (rh), fill_col, 1, (border_col), 8); \
  } while(0)

#define DRAW_ROUNDED_HOVER(win, s, rx, ry, rw, rh, bg_col, hov_col, radius) \
  do { \
    uint32_t fill_col = ((s)->mx >= (rx) && (s)->mx < (rx) + (rw) && (s)->my >= (ry) && (s)->my < (ry) + (rh)) ? (hov_col) : (bg_col); \
    winmgr_draw_rounded_rect_ex((win), (rx), (ry), (rw), (rh), fill_col, 0, 0, (radius)); \
  } while(0)

typedef struct {
  int page, hover;
  char pw[32];
  int pwc, wp, th, di, filter, icon_int, bg_int;
  int dragging_icon, dragging_bg;
  int show_clock, show_calendar;
  int pinned[32];
  int num_pinned;
  int scroll_y, tz;
  int mx, my;
  int auto_hide_taskbar;
  int show_sysmon;
  int sel_res;
  int font_size, font_style;
  int picker_open;
  int dropdown_open; // 0 = none, 1 = Color Mode, 2 = Icon Filter
  animation_t anim_toggles[64];
  animation_t flash_toggles[64];
  animation_t page_anim;
} sstate_t;

static uint32_t *wp_thumbs[4] = {0};
#define TH_W 480
#define TH_H 300

static void load_wp_thumbs(int incremental) {
  int fs = ui_get_font_scale();
  static int loading = 0;
  if (loading) return;
  loading = 1;

  fs_mkdir("/THUMBS");

  for (int i = 0; i < 4; i++) {
    if (wp_thumbs[i]) continue;

    char cache_path[32];
    char idx[4];
    k_itoa(i + 1, idx);
    
    // 1. TRY PRE-GENERATED THUMB (from build script, in root)
    strcpy(cache_path, "/W");
    strcat(cache_path, idx);
    strcat(cache_path, "THUMB.RAW");

    file_entry_t *cf = fs_find(cache_path);
    if (!cf) {
        // 2. TRY SYSTEM CACHE (in /THUMBS/)
        strcpy(cache_path, "/THUMBS/WALL");
        strcat(cache_path, idx);
        strcat(cache_path, ".RAW");
        cf = fs_find(cache_path);
    }

    if (cf && cf->size == TH_W * TH_H * 4) {
      uint32_t *cached = (uint32_t *)kmalloc(TH_W * TH_H * 4);
      if (cached) {
        if (fs_read(cache_path, (uint8_t *)cached) > 0) {
          wp_thumbs[i] = cached;
          print_serial("SETTINGS: Instant Load from CACHE: ");
          print_serial(cache_path);
          print_serial("\n");
          if (incremental) { loading = 0; return; }
          continue;
        }
        kfree(cached);
      }
    }

    // Cache miss... (unchanged)
    char path[32];
    strcpy(path, "/WALL");
    strcat(path, idx);
    strcat(path, ".JPG");

    file_entry_t *f = fs_find(path);
    if (!f) {
      strcpy(path, "/wall");
      strcat(path, idx);
      strcat(path, ".jpg");
      f = fs_find(path);
    }
    
    if (f) {
      uint8_t *raw = (uint8_t *)kmalloc(f->size + 16);
      if (raw) {
        int rs = fs_read(path, raw);
        if (rs > 0) {
          int iw, ih, in;
          unsigned char *pixels = stbi_load_from_memory(raw, rs, &iw, &ih, &in, 4);
          if (pixels) {
            uint32_t *cached = (uint32_t *)kmalloc(TH_W * TH_H * 4);
            if (cached) {
              uint32_t *p32 = (uint32_t *)pixels;
              for (int py = 0; py < TH_H; py++) {
                for (int px = 0; px < TH_W; px++) {
                  int gx = (px * iw << 8) / TH_W;
                  int gy = (py * ih << 8) / TH_H;
                  int sx = gx >> 8;
                  int sy = gy >> 8;
                  int fx = gx & 0xFF;
                  int fy = gy & 0xFF;
                  if (sx >= iw - 1) sx = iw - 2;
                  if (sy >= ih - 1) sy = ih - 2;
                  uint32_t c00 = p32[sy * iw + sx];
                  uint32_t c10 = p32[sy * iw + sx + 1];
                  uint32_t c01 = p32[(sy + 1) * iw + sx];
                  uint32_t c11 = p32[(sy + 1) * iw + sx + 1];
                  int w00 = (256 - fx) * (256 - fy), w10 = fx * (256 - fy), w01 = (256 - fx) * fy, w11 = fx * fy;
                  uint8_t r = ((c00 & 0xFF) * w00 + (c10 & 0xFF) * w10 + (c01 & 0xFF) * w01 + (c11 & 0xFF) * w11) >> 16;
                  uint8_t g = (((c00 >> 8) & 0xFF) * w00 + ((c10 >> 8) & 0xFF) * w10 + ((c01 >> 8) & 0xFF) * w01 + ((c11 >> 8) & 0xFF) * w11) >> 16;
                  uint8_t b = (((c00 >> 16) & 0xFF) * w00 + ((c10 >> 16) & 0xFF) * w10 + ((c01 >> 16) & 0xFF) * w01 + ((c11 >> 16) & 0xFF) * w11) >> 16;
                  uint8_t a = (((c00 >> 24) & 0xFF) * w00 + ((c10 >> 24) & 0xFF) * w10 + ((c01 >> 24) & 0xFF) * w01 + ((c11 >> 24) & 0xFF) * w11) >> 16;
                  cached[py * TH_W + px] = (a << 24) | (r << 16) | (g << 8) | b;
                }
              }
              wp_thumbs[i] = cached;
              fs_write(cache_path, (uint8_t *)cached, TH_W * TH_H * 4);
            }
            stbi_image_free(pixels);
          }
        }
        kfree(raw);
      }
    }
    if (incremental) { loading = 0; return; }
  }
  loading = 0;
}

static void draw_thumb_scaled(window_t *win, int x, int y, int w, int h, int index, int blur) {
  if (index < 0 || index >= 4 || !wp_thumbs[index]) {
    winmgr_fill_rect(win, x, y, w, h, 0xFF111111);
    return;
  }
  uint32_t *data = wp_thumbs[index];
  uint32_t *target = win->surface;

  // Bilinear interpolation for drawing thumbnails smoothly
  for (int py = 0; py < h; py++) {
    int dy = y + py;
    if (dy < 32 || dy >= win->height) continue;
    int gy = (py * TH_H << 8) / h;
    int sy = gy >> 8;
    int fy = gy & 0xFF;

    for (int px = 0; px < w; px++) {
      int dx = x + px;
      if (dx < 0 || dx >= win->width) continue;
      int gx = (px * TH_W << 8) / w;
      int sx = gx >> 8;
      int fx = gx & 0xFF;

      if (sx >= TH_W - 1) sx = TH_W - 2;
      if (sy >= TH_H - 1) sy = TH_H - 2;

      uint32_t c00 = data[sy * TH_W + sx];
      uint32_t c10 = data[sy * TH_W + sx + 1];
      uint32_t c01 = data[(sy + 1) * TH_W + sx];
      uint32_t c11 = data[(sy + 1) * TH_W + sx + 1];

      int w00 = (256 - fx) * (256 - fy);
      int w10 = fx * (256 - fy);
      int w01 = (256 - fx) * fy;
      int w11 = fx * fy;

      uint32_t r = (((c00 >> 16) & 0xFF) * w00 + ((c10 >> 16) & 0xFF) * w10 + ((c01 >> 16) & 0xFF) * w01 + ((c11 >> 16) & 0xFF) * w11) >> 16;
      uint32_t g = (((c00 >> 8) & 0xFF) * w00 + ((c10 >> 8) & 0xFF) * w10 + ((c01 >> 8) & 0xFF) * w01 + ((c11 >> 8) & 0xFF) * w11) >> 16;
      uint32_t b = ((c00 & 0xFF) * w00 + (c10 & 0xFF) * w10 + (c01 & 0xFF) * w01 + (c11 & 0xFF) * w11) >> 16;

      uint32_t p = 0xFF000000 | (r << 16) | (g << 8) | b;
      if (blur) {
        p = (0xFF000000) | ((r / 2) << 16) | ((g / 2) << 8) | (b / 2);
      }
      target[dy * win->width + dx] = p;
    }
  }
}


extern void winmgr_draw_char_scaled(window_t *win, int x, int y, char c, uint32_t color, int scale);

static void draw_text_spaced(window_t *win, int x, int y, const char *text, uint32_t color, int scale, int spacing) {
    if (!win || !text) return;
    int cur_x = x;
    while (*text) {
        winmgr_draw_char_scaled(win, cur_x, y, *text, color, scale);
        extern int font_get_width_16(unsigned char c);
        if (scale >= 10) cur_x += (font_get_width_16(*text) * scale) / 16 + (spacing > 1 ? 1 : spacing);
        else cur_x += scale + (spacing > 1 ? 1 : spacing);
        text++;
    }
}

static void draw_animated_toggle(window_t *win, int x, int y, int state, animation_t *anim, animation_t *flash) {
    if (!anim->active && anim->current_val != (float)state) {
        if (anim->duration == 0.0f && anim->current_val == 0.0f && anim->start_val == 0.0f) {
            anim_init_val(anim, (float)state);
            anim->duration = 1.0f; // mark initialized
            if (flash) anim_init_val(flash, 0.0f);
        } else {
            anim_start_spring(anim, anim->current_val, (float)state, 400.0f, 30.0f);
            if (flash) anim_start_spring(flash, 1.0f, 0.0f, 150.0f, 15.0f);
        }
    }
    
    anim_tick(anim, 0.04f);
    if (anim->active) {
        win->needs_redraw = 1;
        compositor_invalidate_rect(win->x, win->y, win->width, win->height);
    }
    
    if (flash) {
        anim_tick(flash, 0.04f);
        if (flash->active) {
            win->needs_redraw = 1;
            compositor_invalidate_rect(win->x, win->y, win->width, win->height);
        }
    }

    float p = anim->current_val;
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    
    uint32_t bg_off = 0xFF333333;
    uint32_t bg_on = 0xFF3B82F6;
    
    int r = ((bg_off >> 16) & 0xFF) + (int)((((bg_on >> 16) & 0xFF) - ((bg_off >> 16) & 0xFF)) * p);
    int g = ((bg_off >> 8) & 0xFF) + (int)((((bg_on >> 8) & 0xFF) - ((bg_off >> 8) & 0xFF)) * p);
    int b = (bg_off & 0xFF) + (int)(((bg_on & 0xFF) - (bg_off & 0xFF)) * p);
    
    float flash_val = flash ? flash->current_val : 0.0f;
    if (flash_val > 0.0f) {
        r = (int)(r + (255 - r) * flash_val * 0.4f);
        g = (int)(g + (255 - g) * flash_val * 0.4f);
        b = (int)(b + (255 - b) * flash_val * 0.4f);
        if (r > 255) r = 255;
        if (g > 255) g = 255;
        if (b > 255) b = 255;
    }
    
    uint32_t current_bg = 0xFF000000 | (r << 16) | (g << 8) | b;
    
    winmgr_draw_rounded_rect_ex(win, x, y, 55, 30, current_bg, 0, 0, 15);
    
    int knob_x = x + 4 + (int)(24.0f * p);
    winmgr_draw_rounded_rect_ex(win, knob_x, y + 4, 22, 22, 0xFFFFFFFF, 0, 0, 11);
}

static void settings_draw(void *w) {
  int fs = ui_get_font_scale();
  window_t *win = (window_t *)w;
  sstate_t *s = (sstate_t *)win->user_data;

  const theme_t *theme = theme_get();
 
  // Modern Theme Background
  winmgr_fill_rect(win, 0, 32, win->width, win->height - 32, theme->bg);

  // 1. SLEEK SIDEBAR (Premium Dark)
  winmgr_fill_rect(win, 0, 32, get_sidebar_width(), win->height - 32, 0xFF141414);
  winmgr_fill_rect(win, get_sidebar_width() - 1, 32, 1, win->height - 32, 0xFF222222);

  const char *tabs[] = {"\xF0\x9F\x8E\xA8 Personalization", "\xF0\x9F\x96\xBC Wallpaper", 
                        "\xF0\x9F\x93\x8C Taskbar", "\xF0\x9F\x91\xA4 Account", 
                        "\xF0\x9F\x92\xBB System", "\xF0\x9F\xA7\xA9 Widgets", 
                        "\xF0\x9F\x94\xA4 Fonts & Style"};
  for (int i = 0; i < NP; i++) {
    int item_h = fs + 24; int ty = 45 + i * ((fs + 24) + 10);
    if (s->page == i) {
      winmgr_draw_rounded_rect_ex(win, 15, ty, get_sidebar_width() - 30, (fs + 24), 0xFF3B82F6, 0, 0, 8);
      winmgr_draw_text(win, 30, ty + ((fs + 24) - fs) / 2, tabs[i], 0xFFFFFFFF);
    } else {
      winmgr_draw_text(win, 30, ty + ((fs + 24) - fs) / 2, tabs[i], 0xFF999999);
    }
  }

  // 2. CONTENT AREA (CARDS)
  anim_tick(&s->page_anim, 0.04f);
  if (s->page_anim.active) {
      win->needs_redraw = 1;
      compositor_invalidate_rect(win->x, win->y, win->width, win->height);
  }
  int cx = get_cx();
  int cy = 50 - s->scroll_y + (int)s->page_anim.current_val; // Apply scroll and animation offset
  int card_w = win->width - cx - 20;

  if (s->page == PA) { // Home
    winmgr_draw_text(win, cx, cy, "General Settings", theme->fg);
    cy += 30;

    // Aesthetic Settings Card
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 2 + 10), theme->input_bg, 0xFF353545, theme->border);
    winmgr_draw_text(win, cx + 15, cy + (fs * 0.5), "System Appearance", theme->fg);
    winmgr_draw_text(win, cx + card_w - 60, cy + (fs * 0.5), "Light >", theme->accent);

    cy += 60;
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 2 + 10), theme->input_bg, 0xFF353545, theme->border);
    winmgr_draw_text(win, cx + 15, cy + (fs * 0.5), "Wallpapers", theme->fg);
  } else if (s->page == PWALL) { // Wallpaper
    draw_text_spaced(win, cx, cy, "Wallpaper", 0xFFFFFFFF, 32, 4);
    cy += 80;

    int wp_card_h = 320;
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, wp_card_h, 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    draw_text_spaced(win, cx + 20, cy + 20, "Choose a background", 0xFFFFFFFF, 18, 1);
    
    int thumb_w = (card_w - 60) / 2;
    int thumb_h = 90;
    int tw_x = cx + 20;
    int tw_y = cy + 60;
    
    load_wp_thumbs(1); // Background load
    
    for(int i=0; i<4; i++) {
       int tx = tw_x + (i % 2) * (thumb_w + 20);
       int ty = tw_y + (i / 2) * (thumb_h + 20);
       
       if (s->wp == i) {
           winmgr_draw_rounded_rect_ex(win, tx - 4, ty - 4, thumb_w + 8, thumb_h + 8, 0xFF3B82F6, 0, 0, 10);
       }
       
       if (!wp_thumbs[i]) {
           winmgr_fill_rect(win, tx, ty, thumb_w, thumb_h, 0xFF222222);
           draw_text_spaced(win, tx + thumb_w/2 - 30, ty + thumb_h/2 - 8, "Loading", 0xFFFFFFFF, 8, 1);
       } else {
           draw_thumb_scaled(win, tx, ty, thumb_w, thumb_h, i, 0);
       }
    }

  } else if (s->page == PT) { // Taskbar
    draw_text_spaced(win, cx, cy, "Taskbar Behavior", 0xFFFFFFFF, 32, 4);
    cy += 80;

    // First Card: Auto-hide
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 6 + 40), 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    draw_text_spaced(win, cx + 20, cy + 20, "Auto-hide Taskbar", 0xFFFFFFFF, 18, 1);
    draw_text_spaced(win, cx + 20, cy + 20 + fs + 20, "Automatically hide the taskbar when not in use", 0xFF888888, 14, 1);
    
    draw_animated_toggle(win, cx + card_w - 75, cy + 25, s->auto_hide_taskbar, &s->anim_toggles[0], &s->flash_toggles[0]);

    cy += (fs * 6 + 40) + 40;
    
    // Second Card: Pinned Apps
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 6 + 40) + 18 * 50, 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    draw_text_spaced(win, cx + 20, cy + 20, "Pinned Apps", 0xFFFFFFFF, 18, 1);
    draw_text_spaced(win, cx + 20, cy + 20 + fs + 20, "Select which apps appear on the dock", 0xFF888888, 14, 1);
    
    int list_y = cy + 80;
    const char *app_names[] = {"Terminal", "Calculator", "Editor",   "Computer",
                               "Paint",    "Files",      "Task Mgr", "Browser",
                               "Video",    "Settings",   "PDF Reader", "Camera",
                               "Photos",   "Mail",       "Recorder", "Chat", 
                               "Phone",    "Music"};
    for (int i = 0; i < 18; i++) {
      draw_text_spaced(win, cx + 20, list_y + 10, app_names[i], 0xFFFFFFFF, 16, 1);

      int is_pinned = 0;
      for (int j = 0; j < s->num_pinned; j++) {
        if (s->pinned[j] == i) is_pinned = 1;
      }

      draw_animated_toggle(win, cx + card_w - 75, list_y + 5, is_pinned, &s->anim_toggles[20 + i], &s->flash_toggles[20 + i]);
      
      list_y += 50;
    }

  } else if (s->page == PC) { // Accounts
    draw_text_spaced(win, cx, cy, "Account Settings", 0xFFFFFFFF, 32, 4);
    cy += 80;

    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 4 + 30), 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    // Large circle with R
    winmgr_draw_rounded_rect_ex(win, cx + 20, cy + 10, 60, 60, 0xFF3B82F6, 0, 0, 30);
    draw_text_spaced(win, cx + 43, cy + 40, "R", 0xFFFFFFFF, 24, 2);
    
    draw_text_spaced(win, cx + 100, cy + 30, "Rudra Patel", 0xFFFFFFFF, 18, 1);
    draw_text_spaced(win, cx + 100, cy + 65, "Administrator", 0xFF888888, 14, 1);

    cy += 200;
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 10 + 60), 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    draw_text_spaced(win, cx + 20, cy + 20, "Lock Password", 0xFFFFFFFF, 18, 1);

    // Password textbox
    winmgr_draw_rounded_rect_ex(win, cx + 20, cy + 60, card_w - 120, (fs + 24), 0xFF121212, 1, 0xFF333333, 8);
    // Draw masked dots for safety
    char disp[32];
    for (int i = 0; i < s->pwc; i++) disp[i] = '*';
    disp[s->pwc] = 0;
    draw_text_spaced(win, cx + 30, cy + 74, disp, 0xFFFFFFFF, 8, 1);

    // Blinking cursor
    extern uint32_t get_timer_ticks(void);
    if ((get_timer_ticks() / 20) % 2 == 0) {
      winmgr_fill_rect(win, cx + 30 + s->pwc * 9, cy + 70, 2, 20, 0xFFFFFFFF);
    }

    // Save button
    DRAW_ROUNDED_HOVER(win, s, cx + card_w - 85, cy + 60, 65, (fs + 24), 0xFF3B82F6, 0xFF5B9BF5, 8);
    draw_text_spaced(win, cx + card_w - 70, cy + 74, "Save", 0xFFFFFFFF, 8, 1);

  } else if (s->page == PE) { // About
    draw_text_spaced(win, cx + (card_w - 100) / 2, cy + 10, "PureOS", 0xFFFFFFFF, 32, 4);
    draw_text_spaced(win, cx + (card_w - 120) / 2, cy + 60, "Version 2.0", 0xFF888888, 14, 1);

    cy += 100;
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 6 + 40), theme->input_bg, theme->input_bg, theme->border);
    draw_text_spaced(win, cx + 15, cy + 15, "Processor: x86 CPU @ 2.4GHz", 0xFFFFFFFF, 12, 1);
    draw_text_spaced(win, cx + 15, cy + 15 + fs + 12, "Memory: 512 MB DDR3", 0xFFFFFFFF, 12, 1);
    draw_text_spaced(win, cx + 15, cy + 15 + (fs + 12)*2, "Graphics: VBE Adapter", 0xFFFFFFFF, 12, 1);
  } else if (s->page == PB) { // Personalization
    draw_text_spaced(win, cx, cy, "Personalization", 0xFFFFFFFF, 32, 4);
    cy += 80;

    // First Card: Color Mode
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 6 + 40), 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    draw_text_spaced(win, cx + 20, cy + 20, "Color Mode", 0xFFFFFFFF, 18, 1);
    draw_text_spaced(win, cx + 20, cy + 20 + fs + 20, "Choose your default system theme", 0xFF888888, 14, 1);
    
    // Dropdown button (Color Mode)
    int drp_y = cy + 20;
    int drp_h = 35;
    winmgr_draw_rounded_rect_ex(win, cx + card_w - 180, drp_y, 160, drp_h, 0xFF121212, 1, 0xFF333333, 8);
    draw_text_spaced(win, cx + card_w - 170, drp_y + (drp_h - 14)/2 + 2, s->th == 0 ? "Dark Mode" : "Light Mode", 0xFFFFFFFF, 14, 1);
    draw_text_spaced(win, cx + card_w - 40, drp_y + (drp_h - 14)/2 + 2, "v", 0xFFFFFFFF, 14, 1);
    
    cy += (fs * 6 + 40) + 40; // Next card

    // Second Card: Desktop Icons & Icon Filter
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, (fs * 14 + 100), 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    
    // Row 1: Desktop Icons
    draw_text_spaced(win, cx + 20, cy + 20, "Desktop Icons", 0xFFFFFFFF, 18, 1);
    draw_text_spaced(win, cx + 20, cy + 20 + fs + 20, "Show or hide icons on the desktop", 0xFF888888, 14, 1);
    
    // Toggle Switch
    draw_animated_toggle(win, cx + card_w - 75, cy + 32, s->di, &s->anim_toggles[11], &s->flash_toggles[11]);
    
    // Row 2: Icon Filter
    draw_text_spaced(win, cx + 20, cy + 120, "Icon Filter", 0xFFFFFFFF, 18, 1);
    draw_text_spaced(win, cx + 20, cy + 160, "Apply a color tint to desktop icons", 0xFF888888, 14, 1);
    
    // Dropdown button (Icon Filter)
    const char *fn[] = {"None", "Red", "Green", "Yellow"};
    int if_y = cy + 105;
    int if_h = 35;
    winmgr_draw_rounded_rect_ex(win, cx + card_w - 180, if_y, 160, if_h, 0xFF121212, 1, 0xFF333333, 8);
    draw_text_spaced(win, cx + card_w - 170, if_y + (if_h - 14)/2 + 2, fn[s->filter], 0xFFFFFFFF, 14, 1);
    draw_text_spaced(win, cx + card_w - 40, if_y + (if_h - 14)/2 + 2, "v", 0xFFFFFFFF, 14, 1);

  } else if (s->page == PD) { // System
    draw_text_spaced(win, cx, cy, "System", 0xFFFFFFFF, 32, 4);
    cy += 80;

    // Card 1: Display Resolution
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 100, 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    draw_text_spaced(win, cx + 25, cy + 25, "Display Resolution", 0xFFFFFFFF, 20, 1);
    char cur_res[32];
    k_itoa(screen_width, cur_res);
    strcat(cur_res, "x");
    char hs[8];
    k_itoa(screen_height, hs);
    strcat(cur_res, hs);
    char res_info[64] = "Current: ";
    strcat(res_info, cur_res);
    draw_text_spaced(win, cx + 25, cy + 60, res_info, 0xFF888888, 14, 1);

    // Dropdown button (Resolution)
    const char *res_names[] = {"800x600", "1024x768", "1280x720", "1280x800", "1440x900", "1600x900", "1920x1000", "1920x1040", "1920x1080"};
    int dr_y = cy + 32;
    int dr_h = 35;
    winmgr_draw_rounded_rect_ex(win, cx + card_w - 200, dr_y, 180, dr_h, 0xFF121212, 1, 0xFF333333, 8);
    draw_text_spaced(win, cx + card_w - 190, dr_y + (dr_h - 14)/2 + 2, res_names[s->sel_res], 0xFFFFFFFF, 14, 1);
    draw_text_spaced(win, cx + card_w - 40, dr_y + (dr_h - 14)/2 + 2, "v", 0xFFFFFFFF, 14, 1);

    cy += 140;

    // Card 2: About PureOS
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 150, 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    draw_text_spaced(win, cx + 25, cy + 25, "About PureOS", 0xFFFFFFFF, 20, 1);
    draw_text_spaced(win, cx + 25, cy + 60, "Version 2.0 Web Replica", 0xFF888888, 14, 1);
    draw_text_spaced(win, cx + 25, cy + 85, "Processor: x86 CPU @ 2.4GHz", 0xFF888888, 14, 1);
    draw_text_spaced(win, cx + 25, cy + 110, "Memory: 512 MB DDR3", 0xFF888888, 14, 1);

  } else if (s->page == PW) { // Widgets
    draw_text_spaced(win, cx, cy, "Desktop Widgets", 0xFFFFFFFF, 32, 4);
    cy += 80;

    // Single large card for all widgets
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 180, 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    
    // Digital Clock row
    draw_text_spaced(win, cx + 25, cy + 30, "Digital Clock", 0xFFFFFFFF, 18, 1);
    draw_animated_toggle(win, cx + card_w - 75, cy + 25, s->show_clock, &s->anim_toggles[12], &s->flash_toggles[12]);

    // Month Calendar row
    draw_text_spaced(win, cx + 25, cy + 80, "Month Calendar", 0xFFFFFFFF, 18, 1);
    draw_animated_toggle(win, cx + card_w - 75, cy + 75, s->show_calendar, &s->anim_toggles[13], &s->flash_toggles[13]);

    // System Monitor row
    draw_text_spaced(win, cx + 25, cy + 130, "System Monitor", 0xFFFFFFFF, 18, 1);
    draw_animated_toggle(win, cx + card_w - 75, cy + 125, s->show_sysmon, &s->anim_toggles[14], &s->flash_toggles[14]);

    // Legacy widgets code removed for clean UI
  } else if (s->page == PF) { // Fonts & Style
    draw_text_spaced(win, cx, cy, "Fonts & Style", 0xFFFFFFFF, 32, 4);
    cy += 80;

    // Single card for font size
    DRAW_ROUNDED_CARD(win, s, cx, cy, card_w, 160, 0xFF1C1C1C, 0xFF1C1C1C, 0xFF2A2A2A);
    draw_text_spaced(win, cx + 25, cy + 25, "Font Size", 0xFFFFFFFF, 20, 1);
    draw_text_spaced(win, cx + 25, cy + 60, "Adjust the system-wide text size", 0xFF888888, 14, 1);

    // Large Dropdown for font size
    const char *sz_names[] = {"Normal (8px)", "Large (10px)", "Extra Large (12px)"};
    int fdr_y = cy + 95;
    int fdr_h = 45;
    winmgr_draw_rounded_rect_ex(win, cx + 25, fdr_y, card_w - 50, fdr_h, 0xFF121212, 1, 0xFF333333, 8);
    draw_text_spaced(win, cx + 40, fdr_y + (fdr_h - 14)/2 + 2, sz_names[s->font_size], 0xFFFFFFFF, 14, 1);
    draw_text_spaced(win, cx + card_w - 60, fdr_y + (fdr_h - 14)/2 + 2, "v", 0xFFFFFFFF, 14, 1);
  }

  // --- PICKER OVERLAY (Full Window Carousel) ---
  if (s->picker_open) {
    load_wp_thumbs(1); // Background load one
    // Fill whole app window
    winmgr_fill_rect(win, 0, 32, win->width, win->height - 32, 0xFF050A15); 
    
    int cx = win->width / 2;
    int cy = 32 + (win->height - 32) / 2 - 20;
    
    // Full width carousel
    int main_w = 340, main_h = 200;
    int side_w = 260, side_h = 160;
    
    // 1. Draw Side Images (at the edges of the window)
    int prev = (s->wp + 3) % 4;
    int next = (s->wp + 1) % 4;
    
    // Left preview (peek from edge)
    draw_thumb_scaled(win, -60, cy - side_h / 2, side_w, side_h, prev, 1);
    // Right preview (peek from edge)
    draw_thumb_scaled(win, win->width - side_w + 60, cy - side_h / 2, side_w, side_h, next, 1);
    
    // 2. Draw Main Image (Center)
    winmgr_draw_rect(win, cx - main_w / 2 - 2, cy - main_h / 2 - 2, main_w + 4, main_h + 4, theme->accent);
    if (!wp_thumbs[s->wp]) {
       winmgr_draw_text(win, cx - 40, cy - 8, "Loading...", 0xFFFFFFFF);
    } else {
       draw_thumb_scaled(win, cx - main_w / 2, cy - main_h / 2, main_w, main_h, s->wp, 0);
    }
    
    // 3. Arrow Navigation (At the very edges of the app window)
    // Left Arrow
    int lax = 45, lay = cy;
    DRAW_ROUNDED_HOVER(win, s, lax - 25, lay - 25, 50, 50, 0xAA222222, 0xDD444444, 25);
    winmgr_draw_rounded_rect_ex(win, lax - 25, lay - 25, 50, 50, 0x00000000, 2, 0xFFFFFFFF, 25);
    winmgr_draw_text(win, lax - 6, lay - 8, "<", 0xFFFFFFFF);
    
    // Right Arrow
    int rax = win->width - 45, ray = cy;
    DRAW_ROUNDED_HOVER(win, s, rax - 25, ray - 25, 50, 50, 0xAA222222, 0xDD444444, 25);
    winmgr_draw_rounded_rect_ex(win, rax - 25, ray - 25, 50, 50, 0x00000000, 2, 0xFFFFFFFF, 25);
    winmgr_draw_text(win, rax - 6, lay - 8, ">", 0xFFFFFFFF);
    
    // 4. "Apply" Button
    int bx = cx - 90, by = win->height - 100;
    DRAW_ROUNDED_HOVER(win, s, bx, by, 180, 50, 0xFF5B8BF5, 0xFF7B9BF5, 25);
    winmgr_draw_text(win, bx + 58, by + 16, "Apply", 0xFFFFFFFF);
    
    // 5. Close Button
    DRAW_HOVER(win, s, win->width - 45, 45, 30, 30, 0x00000000, 0xFFFF0000);
    winmgr_draw_text(win, win->width - 35, 53, "X", 0xFFFFFFFF);
  }

  // --- DROPDOWN OVERLAYS ---
  if (s->dropdown_open) {
    int cx = get_cx();
    int cy = 50 - s->scroll_y + 80; // Content start
    int card_w = win->width - cx - 20;

    if (s->dropdown_open == 1) { // Color Mode
        int d_x = cx + card_w - 180;
        int d_y = cy + 20 + 35 + 2; 
        int item_h = 45;
        winmgr_draw_rounded_rect_ex(win, d_x, d_y, 160, item_h * 2 + 4, 0xFF1C1C1C, 1, 0xFF444444, 8);
        
        DRAW_HOVER(win, s, d_x + 2, d_y + 2, 156, item_h, 0xFF1C1C1C, 0xFF3B82F6);
        draw_text_spaced(win, d_x + 10, d_y + 2 + (item_h - 14)/2 + 2, "Dark Mode", 0xFFFFFFFF, 14, 1);
        DRAW_HOVER(win, s, d_x + 2, d_y + 2 + item_h, 156, item_h, 0xFF1C1C1C, 0xFF3B82F6);
        draw_text_spaced(win, d_x + 10, d_y + 2 + item_h + (item_h - 14)/2 + 2, "Light Mode", 0xFFFFFFFF, 14, 1);
        
    } else if (s->dropdown_open == 2) { // Icon Filter
        int d_x = cx + card_w - 180;
        int d_y = cy + (fs * 6 + 40) + 40 + 105 + 35 + 2; 
        int item_h = 45;
        winmgr_draw_rounded_rect_ex(win, d_x, d_y, 160, item_h * 4 + 4, 0xFF1C1C1C, 1, 0xFF444444, 8);
        const char *fn[] = {"None", "Red", "Green", "Yellow"};
        for (int i=0; i<4; i++) {
            DRAW_HOVER(win, s, d_x + 2, d_y + 2 + i * item_h, 156, item_h, 0xFF1C1C1C, 0xFF3B82F6);
            draw_text_spaced(win, d_x + 10, d_y + 2 + i * item_h + (item_h - 14)/2 + 2, fn[i], 0xFFFFFFFF, 14, 1);
        }
    } else if (s->dropdown_open == 3) { // System Resolution
        int d_x = cx + card_w - 200;
        int d_y = cy + 32 + 35 + 2; 
        int item_h = 45;
        winmgr_draw_rounded_rect_ex(win, d_x, d_y, 180, item_h * 9 + 4, 0xFF1C1C1C, 1, 0xFF444444, 8);
        const char *res_names[] = {"800x600", "1024x768", "1280x720", "1280x800", "1440x900", "1600x900", "1920x1000", "1920x1040", "1920x1080"};
        for (int i=0; i<9; i++) {
            DRAW_HOVER(win, s, d_x + 2, d_y + 2 + i * item_h, 176, item_h, 0xFF1C1C1C, 0xFF3B82F6);
            draw_text_spaced(win, d_x + 10, d_y + 2 + i * item_h + (item_h - 14)/2 + 2, res_names[i], 0xFFFFFFFF, 14, 1);
        }
    } else if (s->dropdown_open == 4) { // Font Size
        int d_x = cx + 25;
        int d_y = cy + 95 + 45 + 2; 
        int item_h = 45;
        winmgr_draw_rounded_rect_ex(win, d_x, d_y, card_w - 50, item_h * 3 + 4, 0xFF1C1C1C, 1, 0xFF444444, 8);
        const char *sz_names[] = {"Normal (8px)", "Large (10px)", "Extra Large (12px)"};
        for (int i=0; i<3; i++) {
            DRAW_HOVER(win, s, d_x + 2, d_y + 2 + i * item_h, card_w - 54, item_h, 0xFF1C1C1C, 0xFF3B82F6);
            draw_text_spaced(win, d_x + 10, d_y + 2 + i * item_h + (item_h - 14)/2 + 2, sz_names[i], 0xFFFFFFFF, 14, 1);
        }
    }
  }
}

static void apply_config(sstate_t *s) {
  int fs = ui_get_font_scale();
  
  int res_w[] = {800, 1024, 1280, 1280, 1440, 1600, 1920, 1920, 1920};
  int res_h[] = {600, 768, 720, 800, 900, 900, 1000, 1040, 1080};
  extern int screen_width, screen_height;
  if (screen_width != res_w[s->sel_res] || screen_height != res_h[s->sel_res]) {
      extern void screen_set_resolution(int width, int height);
      screen_set_resolution(res_w[s->sel_res], res_h[s->sel_res]);
  }

  strcpy(global_config.lock_password, s->pw);
  global_config.wallpaper_index = s->wp; // Save index
  global_config.wallpaper_type = 3;      // Force JPG/PNG
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
  global_config.font_size = s->font_size;
  global_config.font_style = s->font_style;

  extern void winmgr_toggle_sysmon(int);
  winmgr_toggle_sysmon(s->show_sysmon);
 
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
  print_serial("SETTINGS: Calling desktop_invalidate...\n");
  desktop_invalidate();
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
  print_serial("SETTINGS: Configuration applied (theme updated).\n");
}

static void settings_on_mouse(void *w, int mx, int my, int buttons) {
  int fs = ui_get_font_scale();
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

    win->needs_redraw = 1;
  }

  // 1. Sidebar handling (Premium Dark Sidebar)
  if (click && rx < get_sidebar_width() && ry >= 32) {
    int item_h = fs + 24;
    int spacing = 10;
    int nh = (ry - 45) / (item_h + spacing);
    if (ry < 45) nh = 0;
    if (nh >= 0 && nh < NP) {
      if (s->page != nh) {
        s->page = nh;
        s->scroll_y = 0;
        s->picker_open = 0; // Close picker when switching pages
        s->dropdown_open = 0;
        anim_init_val(&s->page_anim, 150.0f);
        anim_start_spring(&s->page_anim, 150.0f, 0.0f, 300.0f, 25.0f);
        win->needs_redraw = 1;
      }
      return;
    }
  }

  // Handle dragging release
  if (!(buttons & 1)) {
    s->dragging_icon = 0;
    s->dragging_bg = 0;
  }

  // DROPDOWN LOGIC (High priority overlay after picker)
  if (s->dropdown_open) {
      if (click) {
          int cw = win->width - get_cx() - 20;
          int cy = 50 + 80; // Content start
          int ry_scroll = ry + s->scroll_y;
          if (s->dropdown_open == 1) { // Color Mode
              int d_x = get_cx() + cw - 180;
              int d_y = cy + 20 + 35 + 2;
              int item_h = 45;
              if (rx >= d_x && rx <= d_x + 160 && ry_scroll >= d_y && ry_scroll <= d_y + item_h * 2 + 4) {
                  int opt = (ry_scroll - d_y - 2) / item_h;
                  if (opt >= 0 && opt < 2) s->th = (opt == 0 ? 0 : 1);
                  apply_config(s);
              }
              s->dropdown_open = 0;
              win->needs_redraw = 1;
              return;
          } else if (s->dropdown_open == 2) { // Icon Filter
              int d_x = get_cx() + cw - 180;
              int d_y = cy + (fs * 6 + 40) + 40 + 105 + 35 + 2; 
              int item_h = 45;
              if (rx >= d_x && rx <= d_x + 160 && ry_scroll >= d_y && ry_scroll <= d_y + item_h * 4 + 4) {
                  int opt = (ry_scroll - d_y - 2) / item_h;
                  if (opt >= 0 && opt < 4) s->filter = opt;
                  apply_config(s);
              }
              s->dropdown_open = 0;
              win->needs_redraw = 1;
              return;
          } else if (s->dropdown_open == 3) { // System Resolution
              int d_x = get_cx() + cw - 200;
              int d_y = cy + 32 + 35 + 2;
              int item_h = 45;
              if (rx >= d_x && rx <= d_x + 180 && ry_scroll >= d_y && ry_scroll <= d_y + item_h * 9 + 4) {
                  int opt = (ry_scroll - d_y - 2) / item_h;
                  if (opt >= 0 && opt < 9) {
                      s->sel_res = opt;
                      apply_config(s);
                  }
              }
              s->dropdown_open = 0;
              win->needs_redraw = 1;
              return;
          } else if (s->dropdown_open == 4) { // Font Size
              int d_x = get_cx() + 25;
              int d_y = cy + 95 + 45 + 2;
              int item_h = 45;
              if (rx >= d_x && rx <= d_x + cw - 50 && ry_scroll >= d_y && ry_scroll <= d_y + item_h * 3 + 4) {
                  int opt = (ry_scroll - d_y - 2) / item_h;
                  if (opt >= 0 && opt < 3) {
                      s->font_size = opt;
                      global_config.font_size = opt;
                      apply_config(s);
                  }
              }
              s->dropdown_open = 0;
              win->needs_redraw = 1;
              return;
          }
      }
      if (!click) return;
  }

  // PICKER LOGIC (Highest priority overlay)
  if (s->picker_open) {
    int cx = win->width / 2;
    int cy = 32 + (win->height - 32) / 2 - 20;

    if (click) {
      // Left Arrow (lax = 45, lay = cy)
      if (rx >= 45 - 25 && rx <= 45 + 25 && ry >= cy - 25 && ry <= cy + 25) {
        s->wp = (s->wp + 3) % 4;
        win->needs_redraw = 1;
      }
      // Right Arrow (rax = win->width - 45, ray = cy)
      if (rx >= win->width - 45 - 25 && rx <= win->width - 45 + 25 && ry >= cy - 25 && ry <= cy + 25) {
        s->wp = (s->wp + 1) % 4;
        win->needs_redraw = 1;
      }
      // Apply Button (bx = cx - 90, by = win->height - 100, 180x50)
      if (rx >= cx - 90 && rx <= cx + 90 && ry >= win->height - 100 && ry <= win->height - 50) {
        apply_config(s);
        s->picker_open = 0;
        win->needs_redraw = 1;
      }
      // Close Button (win->width - 45, 45, 30x30)
      if (rx >= win->width - 45 && rx <= win->width - 15 && ry >= 45 && ry <= 75) {
        s->picker_open = 0;
        win->needs_redraw = 1;
      }
    }
    return; // Don't let clicks fall through
  }

  int cw = win->width - get_cx() - 20;

  // Personalization Sliders (Needs rx/ry before scroll compensation)
  if (s->page == PB) {
    if (s->dragging_icon) {
      int val = (rx - get_cx()) * 255 / cw;
      if (val < 0) val = 0;
      if (val > 255) val = 255;
      s->icon_int = val; win->needs_redraw = 1;
    } else if (s->dragging_bg) {
      int val = (rx - get_cx()) * 255 / cw;
      if (val < 0) val = 0;
      if (val > 255) val = 255;
      s->bg_int = val; win->needs_redraw = 1;
    }
  }

  if (!click && !s->dragging_icon && !s->dragging_bg) return;

  // 2. Content area compensation (Scroll)
  int content_cx = get_cx();
  int ry_scroll = ry + s->scroll_y - (int)s->page_anim.current_val;

  // Home Page (PA)
  if (s->page == PA) {
    int cy = 50 + 30; // Card 1 offset
    if (click && rx >= content_cx && rx <= content_cx + cw && ry_scroll >= cy && ry_scroll <= cy + (fs * 2 + 10)) {
      s->th = !s->th; apply_config(s); win->needs_redraw = 1;
    }
    cy += 60; // Card 2 offset
    if (click && rx >= content_cx && rx <= content_cx + cw && ry_scroll >= cy && ry_scroll <= cy + (fs * 2 + 10)) {
      s->page = PWALL; s->scroll_y = 0;
      anim_init_val(&s->page_anim, 150.0f);
      anim_start_spring(&s->page_anim, 150.0f, 0.0f, 300.0f, 25.0f);
      win->needs_redraw = 1;
    }
  }

  // Wallpaper Page (PWALL)
  if (s->page == PWALL) {
    int cy = 50 + 80;
    int thumb_w = (cw - 60) / 2;
    int thumb_h = 90;
    int tw_x = content_cx + 20;
    int tw_y = cy + 60;
    for (int i = 0; i < 4; i++) {
      int tx = tw_x + (i % 2) * (thumb_w + 20);
      int ty = tw_y + (i / 2) * (thumb_h + 20);
      if (click && rx >= tx && rx <= tx + thumb_w && ry_scroll >= ty && ry_scroll <= ty + thumb_h) {
        s->wp = i;
        s->picker_open = 1;
        win->needs_redraw = 1;
      }
    }
  }

  // Personalization Page (PB)
  if (s->page == PB) {
    int cy = 50 + 80;
    // First Card: Color Mode
    if (click && rx >= content_cx + cw - 180 && rx <= content_cx + cw - 20 && ry_scroll >= cy + 20 && ry_scroll <= cy + 55) {
      s->dropdown_open = 1; win->needs_redraw = 1;
    }
    cy += (fs * 6 + 40) + 40;
    // Second Card: Desktop Icons
    if (click && rx >= content_cx + cw - 75 && rx <= content_cx + cw - 20 && ry_scroll >= cy + 32 && ry_scroll <= cy + 62) {
      s->di = !s->di; apply_config(s); win->needs_redraw = 1;
    }
    // Second Card: Icon Filter
    if (click && rx >= content_cx + cw - 180 && rx <= content_cx + cw - 20 && ry_scroll >= cy + 105 && ry_scroll <= cy + 140) {
      s->dropdown_open = 2; win->needs_redraw = 1;
    }
  }

  // System settings (PD)
  if (s->page == PD) {
    int cy = 50 + 80;
    if (click && rx >= content_cx + cw - 200 && rx <= content_cx + cw - 20 && ry_scroll >= cy + 32 && ry_scroll <= cy + 67) {
      s->dropdown_open = 3; win->needs_redraw = 1;
    }
  }

  // Widgets settings (PW)
  if (s->page == PW) {
    int cy = 50 + 80;
    if (click && rx >= content_cx + cw - 75 && rx <= content_cx + cw - 20) {
       if (ry_scroll >= cy + 25 && ry_scroll <= cy + 55) { s->show_clock = !s->show_clock; apply_config(s); win->needs_redraw = 1; }
       if (ry_scroll >= cy + 75 && ry_scroll <= cy + 105) { s->show_calendar = !s->show_calendar; apply_config(s); win->needs_redraw = 1; }
       if (ry_scroll >= cy + 125 && ry_scroll <= cy + 155) { s->show_sysmon = !s->show_sysmon; apply_config(s); win->needs_redraw = 1; }
    }
  }

  // Fonts & Style settings (PF)
  if (s->page == PF) {
    int cy = 50 + 80;
    if (click && rx >= content_cx + 25 && rx <= content_cx + cw - 25 && ry_scroll >= cy + 95 && ry_scroll <= cy + 140) {
      s->dropdown_open = 4; win->needs_redraw = 1;
    }
  }

  // Taskbar settings (PT)
  if (s->page == PT) {
    int cy = 50 + 80;
    if (click && rx >= content_cx + cw - 75 && rx <= content_cx + cw - 20 && ry_scroll >= cy + 25 && ry_scroll <= cy + 55) {
      s->auto_hide_taskbar = !s->auto_hide_taskbar; apply_config(s); win->needs_redraw = 1;
    }
    cy += (fs * 6 + 40) + 40;
    int list_y = cy + 80;
    for (int i = 0; i < 18; i++) {
      if (click && rx >= content_cx + cw - 75 && rx <= content_cx + cw - 20 && ry_scroll >= list_y + 5 && ry_scroll <= list_y + 35) {
        int is_pinned = 0, pin_idx = -1;
        for (int j = 0; j < s->num_pinned; j++) {
          if (s->pinned[j] == i) { is_pinned = 1; pin_idx = j; break; }
        }
        if (is_pinned) {
          for (int j = pin_idx; j < s->num_pinned - 1; j++) s->pinned[j] = s->pinned[j + 1];
          s->num_pinned--;
        } else if (s->num_pinned < 32) {
          s->pinned[s->num_pinned++] = i;
        }
        apply_config(s); win->needs_redraw = 1;
      }
      list_y += 50;
    }
  }

  // Account settings (PC)
  if (s->page == PC) {
    int cy = 50 + 80;
    cy += 200; // Match settings_draw's jump to password card
    if (click && rx >= content_cx + cw - 85 && rx <= content_cx + cw - 20 && ry_scroll >= cy + 60 && ry_scroll <= cy + 100) {
      apply_config(s); win->needs_redraw = 1;
    }
  }

  // Logic for PD, PW, PF is now handled in the modern sections above
}

static void settings_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  sstate_t *s = (sstate_t *)win->user_data;
  if (!s) return;

  // Wheel UP (direction > 0) -> scroll viewport UP -> decrease scroll_y
  s->scroll_y -= direction * 20;

  if (s->scroll_y < 0) s->scroll_y = 0;
  if (s->scroll_y > 800) s->scroll_y = 800;

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
  int fs = ui_get_font_scale();
  window_t *win = winmgr_create_window(-1, -1, 800, 600, "Settings");
  if (!win)
    return;
  
  // No longer blocking load here. It happens in settings_draw incrementally.

  sstate_t *st = (sstate_t *)kmalloc(sizeof(sstate_t));
  st->page = PA;
  st->hover = -1;
  st->scroll_y = 0;
  strcpy(st->pw, global_config.lock_password);
  st->pwc = strlen(st->pw);
  st->wp = global_config.wallpaper_index;
  st->th = global_config.theme_mode;
  st->di = global_config.show_desktop_icons;
  st->filter = global_config.icon_filter;
  st->icon_int = global_config.icon_filter_intensity;
  st->show_clock = global_config.show_clock_widget;
  st->show_calendar = global_config.show_calendar_widget;
  st->tz = global_config.timezone_offset_m;
  st->bg_int = global_config.icon_bg_filter_intensity;
  st->auto_hide_taskbar = global_config.auto_hide_taskbar;
  st->show_sysmon = global_config.show_sysmon_widget;
  st->dragging_icon = 0;
  st->dragging_bg = 0;
  st->picker_open = 0;
  st->num_pinned = global_config.num_pinned;
  for (int i = 0; i < global_config.num_pinned; i++) {
    st->pinned[i] = global_config.pinned[i];
  }

  for (int i = 0; i < 64; i++) {
      anim_init(&st->anim_toggles[i]);
      anim_init_val(&st->flash_toggles[i], 0.0f);
  }
  anim_init(&st->page_anim);
  anim_init_val(&st->page_anim, 150.0f);
  anim_start_spring(&st->page_anim, 150.0f, 0.0f, 300.0f, 25.0f);

  // Initialize selected resolution to current
  st->sel_res = 1; // Default 1024x768
  int res_w[] = {800, 1024, 1280, 1280, 1440, 1600, 1920, 1920, 1920};
  int res_h[] = {600, 768, 720, 800, 900, 900, 1000, 1040, 1080};
  for (int i = 0; i < 9; i++) {
    if (screen_width == res_w[i] && screen_height == res_h[i]) {
      st->sel_res = i;
      break;
    }
  }

  st->font_size = global_config.font_size;
  st->font_style = global_config.font_style;

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
