#include "taskbar.h"
#include "../drivers/rtc.h"
#include "../drivers/vga.h"
#include "../kernel/config.h"
#include "../kernel/screen.h"
#include "../kernel/string.h"
#include "../kernel/window.h"
#include "startmenu.h"
#include "sysmenu.h"

extern void compositor_blur_rect(uint32_t *buffer, int x, int y, int w, int h, int radius);
extern uint32_t *backbuffer;

#define TASKBAR_HEIGHT 54
#define ICON_SIZE 36
#define START_BTN_W 48
#define BTN_W 44
#define BTN_GAP 4

// --- Magnification Math ---
static inline float kabs_f(float x) { return x < 0 ? -x : x; }
static float kcos(float x) {
  // Taylor series: 1 - x^2/2! + x^4/4! - x^6/6!
  float x2 = x * x;
  float x4 = x2 * x2;
  float x6 = x4 * x2;
  return 1.0f - (x2 / 2.0f) + (x4 / 24.0f) - (x6 / 720.0f);
}

rtc_time_t cached_time;
uint32_t last_rtc_tick = 0;
extern uint32_t get_timer_ticks(void);
extern int rect_intersect(rect_t a, rect_t b, rect_t *out);
extern window_t windows[];
extern int window_count;
extern void winmgr_bring_to_front(window_t *win);

static taskbar_state_t g_taskbar = {0};
static int taskbar_initialized = 0;

// Draw a stylized Eagle icon ("egal")
static void draw_eagle_icon(int x, int y, uint32_t *buffer) {
  uint32_t col = 0xFFFFFFFF; // White Eagle
  // Body/Head
  vga_draw_rect_lfb(x + 12, y + 4, 6, 8, col, buffer);
  // Beak
  vga_draw_rect_lfb(x + 18, y + 6, 4, 2, 0xFFFFD700, buffer); // Golden beak
  // Wings
  vga_draw_rect_lfb(x + 4, y + 10, 8, 4, col, buffer);   // Left wing top
  vga_draw_rect_lfb(x + 18, y + 10, 8, 4, col, buffer);  // Right wing top
  vga_draw_rect_lfb(x + 2, y + 14, 10, 4, col, buffer);  // Left wing mid
  vga_draw_rect_lfb(x + 18, y + 14, 10, 4, col, buffer); // Right wing mid
  // Tail
  vga_draw_rect_lfb(x + 10, y + 20, 10, 6, col, buffer);
}

// --- TOP MENU BAR ---
void menubar_draw(uint32_t *buffer, rect_t clip) {
  rect_t bar_r = {0, 0, screen_width, 24};
  rect_t overlap;
  if (!rect_intersect(clip, bar_r, &overlap))
    return;

  // 1. Sleek Translucent Top Bar (Forced Black)
  compositor_blur_rect(backbuffer, overlap.x, overlap.y, overlap.w, overlap.h, 6);
  vga_draw_rect_blend_lfb(overlap.x, overlap.y, overlap.w, overlap.h,
                          0xA0000000, buffer, 0); // Increased opacity fixed black
  vga_draw_rect_lfb(0, 23, screen_width, 1, 0xFF444444,
                    buffer); // Darker separator

  // 2. Left Menu Items (Fixed White)
  vga_draw_string_lfb(10, 8, "PureOS", 0xFFFFFFFF, buffer);
  vga_draw_string_lfb(70, 8, "File", 0xFFAAAAAA, buffer);
  vga_draw_string_lfb(115, 8, "Edit", 0xFFAAAAAA, buffer);
  vga_draw_string_lfb(160, 8, "View", 0xFFAAAAAA, buffer);

  // 3. Right Tray (Clock)
  uint32_t now = get_timer_ticks();
  if (now - last_rtc_tick > 100 || last_rtc_tick == 0) {
    rtc_read(&cached_time);
    last_rtc_tick = now;
  }

  char time_str[6];
  time_str[0] = (cached_time.hour / 10) % 10 + '0';
  time_str[1] = (cached_time.hour % 10) + '0';
  time_str[2] = ':';
  time_str[3] = (cached_time.minute / 10) % 10 + '0';
  time_str[4] = (cached_time.minute % 10) + '0';
  time_str[5] = 0;

  vga_draw_string_lfb(screen_width - 80, 8, time_str, 0xFFFFFFFF, buffer);

  // 4. Performance Overlay (draw before tray so tray icons aren't hidden)
  extern int current_fps;
  extern uint64_t compositor_cycles;
  extern uint32_t heap_get_used_bytes();
  extern uint32_t heap_get_total_bytes();

  uint32_t used_mb = heap_get_used_bytes() / (1024 * 1024);
  uint32_t total_mb = heap_get_total_bytes() / (1024 * 1024);
  uint32_t mcyc = (uint32_t)(compositor_cycles >> 20); // >> 20 is ~ / 1M

  char perf_str[64];
  snprintf(perf_str, sizeof(perf_str), "FPS: %d | Mem: %d/%d MB | Cmp: %u Mcyc",
           current_fps, used_mb, total_mb, mcyc);
  vga_draw_string_lfb(210, 8, perf_str, 0xFFFFFFFF, buffer);

  // 5. Tray icons (clickable area for system menu) — drawn last so visible
  int tray_x = screen_width - 180;
  // Wi-Fi icon
  vga_draw_string_lfb(tray_x, 8, "W", 0xFFAAAAAA, buffer);
  // Volume icon
  vga_draw_string_lfb(tray_x + 20, 8, "V", 0xFFAAAAAA, buffer);
  // Battery icon
  vga_draw_rect_lfb(tray_x + 40, 10, 14, 8, 0xFFAAAAAA, buffer);
  vga_draw_rect_lfb(tray_x + 54, 12, 2, 4, 0xFFAAAAAA, buffer);
  vga_draw_rect_lfb(tray_x + 42, 12, 10, 4, 0xFF66BB6A, buffer);
}

// Helper to map app IDs
static void launch_app_by_id(int id) {
  extern void terminal_init();
  extern void calculator_init();
  extern void editor_init();
  extern void explorer_init();
  extern void paint_init();
  extern void taskmgr_init();
  extern void settings_init();

  extern void videoplayer_init(const char *path);
  extern void browser_init(void);

  switch (id) {
  case 0:
    terminal_init();
    break;
  case 1:
    calculator_init();
    break;
  case 2:
    editor_init();
    break;
  case 3:
    explorer_init();
    break;
  case 4:
    paint_init();
    break;
  case 5:
    explorer_init();
    break;
  case 6:
    taskmgr_init();
    break;
  case 7:
    browser_init();
    break;
  case 8:
    videoplayer_init((void *)0);
    break;
  case 9:
    settings_init();
    break;
  case 10: {
    extern void pdfreader_init();
    pdfreader_init();
  } break;
  case 13: {
    extern void mail_app_init();
    mail_app_init();
  } break;
  case 11: {
    extern void camera_app_init();
    camera_app_init();
  } break;
  case 12: {
    extern void photos_init();
    photos_init();
  } break;
  case 14: {
    extern void recorder_init();
    recorder_init();
  } break;
  case 15: {
    extern void chat_init();
    chat_init();
  } break;
  case 16: {
    extern void phone_init();
    phone_init();
  } break;
  case 17: {
    extern void song_app_init();
    song_app_init();
  } break;
  }
}
// Local struct for searching
typedef struct {
  int app_id;
  int is_running;
  int is_pinned;
  window_t *win_ref;
} dock_search_t;

static dock_search_t dock_icons[MAX_DOCK_ICONS];
static int dock_icon_count = 0;

static void add_to_dock_icons(int aid, int pinned, int running, window_t *w) {
  if (dock_icon_count >= MAX_DOCK_ICONS)
    return;
  dock_icons[dock_icon_count].app_id = aid;
  dock_icons[dock_icon_count].is_pinned = pinned;
  dock_icons[dock_icon_count].is_running = running;
  dock_icons[dock_icon_count].win_ref = w;
  dock_icon_count++;
}

// --- Magnification Logic ---
static void taskbar_update_magnification(int mx, int my) {
  extern os_config_t global_config;
  int is_vertical = (global_config.taskbar_position == 1 ||
                     global_config.taskbar_position == 2);

  float max_scale = 1.6f;
  int min_size = 64;

  if (!is_vertical) {
    // 1. Calculate perfectly continuous fractional index (u)
    float u = -1.0f;
    if (g_taskbar.hovered_index != -1) {
      for (int i = 0; i < g_taskbar.icon_count; i++) {
        float sw = 64.0f * g_taskbar.icons[i].scale;
        float left = g_taskbar.icons[i].x - (sw - 64.0f) / 2.0f;
        if (mx >= left && mx < left + sw) {
          float offset = (mx - (left + sw / 2.0f)) / sw;
          u = (float)i + offset;
          break;
        }
      }
    }
    // 2. Apply target scales using Cosine curve based on index distance
    for (int i = 0; i < g_taskbar.icon_count; i++) {
      if (u < 0.0f) {
        g_taskbar.icons[i].target_scale = 1.0f;
      } else {
        float dist = kabs_f((float)i - u);
        float radius = 2.5f;
        if (dist < radius) {
          float angle = (dist / radius) * 1.57079f;
          g_taskbar.icons[i].target_scale = 1.0f + (max_scale - 1.0f) * kcos(angle);
        } else {
          g_taskbar.icons[i].target_scale = 1.0f;
        }
      }
    }

    int total_w = 0;
    for (int i = 0; i < g_taskbar.icon_count; i++) {
      total_w += (int)(min_size * g_taskbar.icons[i].target_scale);
    }

    float start_x = (float)(screen_width - (total_w + 128)) / 2.0f + 16.0f;
    float cur_x = start_x;
    for (int i = 0; i < g_taskbar.icon_count; i++) {
      float sw = (float)min_size * g_taskbar.icons[i].target_scale;
      g_taskbar.icons[i].target_x = cur_x + (sw - (float)min_size) / 2.0f;
      g_taskbar.icons[i].target_y = 0.0f;
      cur_x += sw;
    }
  } else {
    // 1. Calculate perfectly continuous fractional index (u)
    float u = -1.0f;
    if (g_taskbar.hovered_index != -1) {
      for (int i = 0; i < g_taskbar.icon_count; i++) {
        float sh = 64.0f * g_taskbar.icons[i].scale;
        float top = g_taskbar.icons[i].y - (sh - 64.0f) / 2.0f;
        if (my >= top && my < top + sh) {
          float offset = (my - (top + sh / 2.0f)) / sh;
          u = (float)i + offset;
          break;
        }
      }
    }
    // 2. Apply target scales using Cosine curve based on index distance
    for (int i = 0; i < g_taskbar.icon_count; i++) {
      if (u < 0.0f) {
        g_taskbar.icons[i].target_scale = 1.0f;
      } else {
        float dist = kabs_f((float)i - u);
        float radius = 2.5f;
        if (dist < radius) {
          float angle = (dist / radius) * 1.57079f;
          g_taskbar.icons[i].target_scale = 1.0f + (max_scale - 1.0f) * kcos(angle);
        } else {
          g_taskbar.icons[i].target_scale = 1.0f;
        }
      }
    }

    int total_w = 0;
    for (int i = 0; i < g_taskbar.icon_count; i++) {
      total_w += (int)(min_size * g_taskbar.icons[i].target_scale);
    }

    float start_y = (float)(screen_height - (total_w + 128)) / 2.0f + 16.0f;
    float cur_y = start_y;
    for (int i = 0; i < g_taskbar.icon_count; i++) {
      float sw = (float)min_size * g_taskbar.icons[i].target_scale;
      g_taskbar.icons[i].target_y = cur_y + (sw - (float)min_size) / 2.0f;
      g_taskbar.icons[i].target_x = 0.0f;
      cur_y += sw;
    }
  }
}

int taskbar_is_animating(void) {
  extern os_config_t global_config;
  int is_vertical = (global_config.taskbar_position == 1 ||
                     global_config.taskbar_position == 2);

  if (kabs_f(g_taskbar.dock_x - g_taskbar.anim_dock_x) > 0.5f ||
      kabs_f(g_taskbar.dock_y - g_taskbar.anim_dock_y) > 0.5f ||
      kabs_f(g_taskbar.dock_w - g_taskbar.anim_dock_w) > 0.5f ||
      kabs_f(g_taskbar.dock_h - g_taskbar.anim_dock_h) > 0.5f) {
    return 1;
  }

  for (int i = 0; i < g_taskbar.icon_count; i++) {
    dock_icon_state_t *icon = &g_taskbar.icons[i];
    float target_y =
        (!is_vertical && i == g_taskbar.hovered_index ? -15.0f : 0.0f);
    float target_x_off = 0.0f;
    if (is_vertical && i == g_taskbar.hovered_index) {
      target_x_off = (global_config.taskbar_position == 1) ? 15.0f : -15.0f;
    }

    if (kabs_f(icon->target_scale - icon->scale) > 0.005f ||
        kabs_f(icon->target_x - icon->x) > 0.2f ||
        kabs_f(icon->target_y - icon->y) > 0.2f ||
        kabs_f(target_y - icon->y_offset) > 0.2f ||
        kabs_f(target_x_off - icon->x_offset) > 0.2f) {
      return 1;
    }
  }
  return 0;
}

void taskbar_tick_animations(float dt) {
  extern os_config_t global_config;
  int is_vertical = (global_config.taskbar_position == 1 ||
                     global_config.taskbar_position == 2);
  float speed = 30.0f;
  int animating = 0;

  float ddx = g_taskbar.dock_x - g_taskbar.anim_dock_x;
  float ddy = g_taskbar.dock_y - g_taskbar.anim_dock_y;
  float ddw = g_taskbar.dock_w - g_taskbar.anim_dock_w;
  float ddh = g_taskbar.dock_h - g_taskbar.anim_dock_h;

  if (kabs_f(ddx) > 0.5f || kabs_f(ddy) > 0.5f || kabs_f(ddw) > 0.5f ||
      kabs_f(ddh) > 0.5f) {
    g_taskbar.anim_dock_x += ddx * speed * dt;
    g_taskbar.anim_dock_y += ddy * speed * dt;
    g_taskbar.anim_dock_w += ddw * speed * dt;
    g_taskbar.anim_dock_h += ddh * speed * dt;
    animating = 1;
  } else {
    g_taskbar.anim_dock_x = g_taskbar.dock_x;
    g_taskbar.anim_dock_y = g_taskbar.dock_y;
    g_taskbar.anim_dock_w = g_taskbar.dock_w;
    g_taskbar.anim_dock_h = g_taskbar.dock_h;
  }

  for (int i = 0; i < g_taskbar.icon_count; i++) {
    dock_icon_state_t *icon = &g_taskbar.icons[i];

    // Scale animation
    float ds = icon->target_scale - icon->scale;
    if (kabs_f(ds) > 0.001f) {
      icon->scale += ds * speed * dt;
      animating = 1;
    } else {
      icon->scale = icon->target_scale;
    }

    // Position animation
    float dx = icon->target_x - icon->x;
    if (kabs_f(dx) > 0.1f) {
      icon->x += dx * speed * dt;
      animating = 1;
    } else {
      icon->x = icon->target_x;
    }

    float dy2 = icon->target_y - icon->y;
    if (kabs_f(dy2) > 0.1f) {
      icon->y += dy2 * speed * dt;
      animating = 1;
    } else {
      icon->y = icon->target_y;
    }

    // Y Bounce (Hop) animation
    float target_y = 0.0f;
    if (!is_vertical && g_taskbar.hovered_index != -1) {
      target_y = -25.0f * (icon->scale - 1.0f);
    }
    float dy = target_y - icon->y_offset;
    if (kabs_f(dy) > 0.1f) {
      icon->y_offset += dy * speed * dt;
      animating = 1;
    } else {
      icon->y_offset = target_y;
    }

    // X Bounce animation
    float target_x_off = 0.0f;
    if (is_vertical && g_taskbar.hovered_index != -1) {
      float dir = (global_config.taskbar_position == 1) ? 1.0f : -1.0f;
      target_x_off = dir * 25.0f * (icon->scale - 1.0f);
    }
    float dx_off = target_x_off - icon->x_offset;
    if (kabs_f(dx_off) > 0.1f) {
      icon->x_offset += dx_off * speed * dt;
      animating = 1;
    } else {
      icon->x_offset = target_x_off;
    }
  }

  if (animating) {
    extern int screen_width, screen_height;
    extern void compositor_invalidate_rect(int x, int y, int w, int h);
    if (g_taskbar.edit_mode) {
      compositor_invalidate_rect(0, 0, screen_width, screen_height);
    } else if (!is_vertical) {
      compositor_invalidate_rect(0, screen_height - 150, screen_width, 150);
    } else if (global_config.taskbar_position == 1) {
      compositor_invalidate_rect(0, 0, 150, screen_height);
    } else {
      compositor_invalidate_rect(screen_width - 150, 0, 150, screen_height);
    }
  }
}

// --- FLOATING DOCK ---
void taskbar_draw(uint32_t *buffer, rect_t clip) {
  extern os_config_t global_config;
  int is_vertical = (global_config.taskbar_position == 1 ||
                     global_config.taskbar_position == 2);

  // 1. Gather all icons to show
  dock_icon_count = 0;

  // Add pinned apps first
  int num = global_config.num_pinned;
  if (num < 0)
    num = 0;
  if (num > 32)
    num = 32;
  for (int i = 0; i < num; i++) {
    int aid = global_config.pinned[i];
    int is_running = 0;
    window_t *win_ref = (void *)0;

    // Check if running
    for (int j = 0; j < window_count; j++) {
      window_t *win = &windows[window_z_order[j]];
      if (win->id != 0 && win->app_type == aid && win->fading_mode != 2) {
        is_running = 1;
        win_ref = win;
        break;
      }
    }
    add_to_dock_icons(aid, 1, is_running, win_ref);
  }

  // Add open but unpinned apps
  for (int i = 0; i < window_count; i++) {
    window_t *win = &windows[window_z_order[i]];
    if (win->id == 0 || win->fading_mode == 2 || (win->flags & WINDOW_FLAG_WIDGET))
      continue;

    int already_pinned = 0;
    for (int j = 0; j < global_config.num_pinned; j++) {
      if (win->app_type == global_config.pinned[j]) {
        already_pinned = 1;
        break;
      }
    }

    if (!already_pinned) {
      // Also check if already in dock_icons (from another window)
      int already_in_dock = 0;
      for (int j = 0; j < dock_icon_count; j++) {
        if (dock_icons[j].app_id == win->app_type) {
          already_in_dock = 1;
          break;
        }
      }

      if (!already_in_dock) {
        add_to_dock_icons(win->app_type, 0, 1, win);
      }
    }
  }

  // Update state with icons found - Robust mapping by app_id
  int old_icon_count = g_taskbar.icon_count;
  g_taskbar.icon_count = dock_icon_count;

  for (int i = 0; i < dock_icon_count; i++) {
    int aid = dock_icons[i].app_id;

    // Find existing state for this app_id
    int found = 0;
    for (int j = 0; j < old_icon_count; j++) {
      if (g_taskbar.icons[j].app_id == aid) {
        // Move existing state to new position i
        if (i != j) {
          dock_icon_state_t tmp = g_taskbar.icons[i];
          g_taskbar.icons[i] = g_taskbar.icons[j];
          g_taskbar.icons[j] = tmp;
        }
        found = 1;
        break;
      }
    }

    // Sync transient info
    g_taskbar.icons[i].app_id = aid;
    g_taskbar.icons[i].is_pinned = dock_icons[i].is_pinned;
    g_taskbar.icons[i].is_running = dock_icons[i].is_running;
    g_taskbar.icons[i].win_ref = dock_icons[i].win_ref;

    if (!found) {
      // Initialize new icon
      g_taskbar.icons[i].scale = 1.0f;
      g_taskbar.icons[i].target_scale = 1.0f;
      g_taskbar.icons[i].y_offset = 0.0f;
      g_taskbar.icons[i].x_offset = 0.0f;
      if (!is_vertical) {
        g_taskbar.icons[i].x =
            (old_icon_count > 0) ? g_taskbar.icons[old_icon_count - 1].x + 64.0f
                                 : (screen_width / 2.0f);
        g_taskbar.icons[i].y = 0;
      } else {
        g_taskbar.icons[i].x = 0;
        g_taskbar.icons[i].y =
            (old_icon_count > 0) ? g_taskbar.icons[old_icon_count - 1].y + 64.0f
                                 : (screen_height / 2.0f);
      }
    }
  }

  // Always recalculate targets to handle app launches/closures correctly
  static int last_known_count = 0;
  if (dock_icon_count != last_known_count || !taskbar_initialized) {
    taskbar_update_magnification(g_taskbar.mouse_x, g_taskbar.mouse_y);
    if (!taskbar_initialized) {
      for (int i = 0; i < g_taskbar.icon_count; i++) {
        g_taskbar.icons[i].x = g_taskbar.icons[i].target_x;
        g_taskbar.icons[i].y = g_taskbar.icons[i].target_y;
      }
      taskbar_initialized = 1;
    }
    last_known_count = dock_icon_count;
  }

  // Calculate dock size
  float total_icon_w = 0.0f;
  for (int i = 0; i < g_taskbar.icon_count; i++)
    total_icon_w += (64.0f * g_taskbar.icons[i].scale);

  int dock_w, dock_h, dock_x, dock_y;
  if (!is_vertical) {
    dock_h = 74;
    dock_w = 128 + (int)total_icon_w;
    dock_x = (screen_width - dock_w) / 2;
    dock_y = screen_height - dock_h - 10;
  } else {
    dock_w = 74;
    dock_h = 128 + (int)total_icon_w;
    dock_y = (screen_height - dock_h) / 2;
    dock_x =
        (global_config.taskbar_position == 1) ? 8 : (screen_width - dock_w - 8);
  }

  extern window_t *active_window;
  int is_max = (active_window && active_window->id != 0 && active_window->is_maximized);
  
  if (global_config.auto_hide_taskbar || is_max) {
      int is_hovering = 0;
      if (!is_vertical) {
          if (g_taskbar.mouse_y >= screen_height - 12 || 
              (g_taskbar.mouse_y >= g_taskbar.anim_dock_y && g_taskbar.mouse_x >= g_taskbar.anim_dock_x && g_taskbar.mouse_x <= g_taskbar.anim_dock_x + g_taskbar.anim_dock_w)) {
              is_hovering = 1;
          }
          if (!is_hovering) dock_y = screen_height + 15;
      } else {
         if (global_config.taskbar_position == 1) { // Left
            if (g_taskbar.mouse_x <= 12 ||
                (g_taskbar.mouse_x <= g_taskbar.anim_dock_x + g_taskbar.anim_dock_w && g_taskbar.mouse_y >= g_taskbar.anim_dock_y && g_taskbar.mouse_y <= g_taskbar.anim_dock_y + g_taskbar.anim_dock_h)) {
                is_hovering = 1;
            }
            if (!is_hovering) dock_x = -dock_w - 15;
         } else { // Right
            if (g_taskbar.mouse_x >= screen_width - 12 ||
                (g_taskbar.mouse_x >= g_taskbar.anim_dock_x && g_taskbar.mouse_y >= g_taskbar.anim_dock_y && g_taskbar.mouse_y <= g_taskbar.anim_dock_y + g_taskbar.anim_dock_h)) {
                is_hovering = 1;
            }
            if (!is_hovering) dock_x = screen_width + 15;
         }
      }
  }

  g_taskbar.dock_x = dock_x;
  g_taskbar.dock_y = dock_y;
  g_taskbar.dock_w = dock_w;
  g_taskbar.dock_h = dock_h;

  if (!taskbar_initialized || g_taskbar.anim_dock_w == 0) {
    g_taskbar.anim_dock_x = dock_x;
    g_taskbar.anim_dock_y = dock_y;
    g_taskbar.anim_dock_w = dock_w;
    g_taskbar.anim_dock_h = dock_h;
  }

  int bd_x = (int)g_taskbar.anim_dock_x;
  int bd_y = (int)g_taskbar.anim_dock_y;
  int bd_w = (int)g_taskbar.anim_dock_w;
  int bd_h = (int)g_taskbar.anim_dock_h;

  rect_t dock_r = {bd_x, bd_y, bd_w, bd_h};
  rect_t overlap;
  if (!rect_intersect(clip, dock_r, &overlap))
    return;

  // 2. Glass Background (Fixed Translucent Black) - Rounded corners
  compositor_blur_rect(backbuffer, bd_x, bd_y, bd_w, bd_h, 0); // Disable blur to avoid ghost corners
  vga_draw_rect_blend_lfb_ex(bd_x, bd_y, bd_w, bd_h, 0xA0000000, 0, 0,
                             buffer, 16);

  // 3. SysMenu Icon (FileExplorer PNG) & Separator
  if (!is_vertical) {
    int sep_x = bd_x + 32 + (int)total_icon_w;
    int sys_x = bd_x + 48 + (int)total_icon_w;
    vga_draw_rect_lfb(sep_x, bd_y + 10, 1, bd_h - 20, 0x50FFFFFF, buffer);
    draw_icon(sys_x, bd_y + 5, 64, 64, 18, buffer); // APP_SYSMENU is 18
  } else {
    draw_icon(bd_x + 5, bd_y + 48 + (int)total_icon_w, 64, 64, 18, buffer); // Bottom of vertical dock
    vga_draw_rect_lfb(bd_x + 10, bd_y + 32 + (int)total_icon_w, bd_w - 20, 1, 0x50FFFFFF, buffer);
  }

  // 5. App Icons
  extern window_t *active_window;
  extern void draw_icon(int x, int y, int w, int h, int type, uint32_t *target);

  for (int i = 0; i < g_taskbar.icon_count; i++) {
    dock_icon_state_t *icon = &g_taskbar.icons[i];
    int x = (int)icon->x;
    int y = (int)icon->y;

    // Magnified size logic
    int sq_size = (int)(56.0f * icon->scale);
    int sq_x, sq_y;
    if (!is_vertical) {
      sq_x = x + (64 - sq_size) / 2;
      sq_y = (int)(bd_y + (bd_h - sq_size) / 2.0f + icon->y_offset);
    } else {
      sq_x = (int)(bd_x + (bd_w - sq_size) / 2.0f + icon->x_offset);
      sq_y = y + (64 - sq_size) / 2;
    }
    extern os_config_t global_config;

    // Draw selection highlight if in edit mode
    if (g_taskbar.edit_mode) {
      vga_draw_rect_blend_lfb_ex(sq_x, sq_y, sq_size, sq_size, 0x80FFAA55, 0, 0,
                                 buffer, sq_size / 2);
    }

    // Draw app icon natively scaled with bilinear filtering
    draw_icon(sq_x, sq_y, sq_size, sq_size, icon->app_id, buffer);

    // Running indicator dot
    if (icon->is_running) {
      int is_active = (icon->win_ref && icon->win_ref == active_window);
      uint32_t dot_color = is_active ? 0xFFFFFFFF : 0xFF888888;
      if (!is_vertical) {
        vga_draw_rect_lfb(x + 19, bd_y + bd_h - 6, 5, 3, dot_color, buffer);
      } else {
        if (global_config.taskbar_position == 1) { // Left
          vga_draw_rect_lfb(bd_x + 6, y + 19, 3, 5, dot_color, buffer);
        } else { // Right
          vga_draw_rect_lfb(bd_x + bd_w - 9, y + 19, 3, 5, dot_color, buffer);
        }
      }

      if (icon->win_ref) {
        icon->win_ref->taskbar_x = (!is_vertical) ? x + (sq_size/2) : bd_x + (bd_w/2);
        icon->win_ref->taskbar_y = (!is_vertical) ? bd_y + (bd_h/2) : y + (sq_size/2);
      }
    }
  }

}

int taskbar_handle_mouse(int mx, int my, int buttons) {
  extern os_config_t global_config;
  int is_vertical = (global_config.taskbar_position == 1 ||
                     global_config.taskbar_position == 2);

  int in_dock =
      (mx >= g_taskbar.dock_x && mx <= g_taskbar.dock_x + g_taskbar.dock_w &&
       my >= g_taskbar.dock_y && my <= g_taskbar.dock_y + g_taskbar.dock_h);

  extern int screen_width, screen_height;
  int near_edge = 0;
  extern window_t *active_window;
  int is_max = (active_window && active_window->id != 0 && active_window->is_maximized);

  if (global_config.auto_hide_taskbar || is_max) {
      if (is_vertical && global_config.taskbar_position == 1 && mx <= 12) near_edge = 1;
      else if (is_vertical && global_config.taskbar_position == 2 && mx >= screen_width - 12) near_edge = 1;
      else if (!is_vertical && my >= screen_height - 12) near_edge = 1;
  }

  int hovered = -1;
  if (in_dock) {
    for (int i = 0; i < g_taskbar.icon_count; i++) {
      if (!is_vertical) {
        float sw = 64.0f * g_taskbar.icons[i].scale;
        float left = g_taskbar.icons[i].x - (sw - 64.0f) / 2.0f;
        if (mx >= left && mx < left + sw) {
          hovered = i;
          break;
        }
      } else {
        float sh = 64.0f * g_taskbar.icons[i].scale;
        float top = g_taskbar.icons[i].y - (sh - 64.0f) / 2.0f;
        if (my >= top && my < top + sh) {
          hovered = i;
          break;
        }
      }
    }
  }

  int was_near_edge = 0;
  if (global_config.auto_hide_taskbar || is_max) {
      if (is_vertical && global_config.taskbar_position == 1 && g_taskbar.mouse_x <= 12) was_near_edge = 1;
      else if (is_vertical && global_config.taskbar_position == 2 && g_taskbar.mouse_x >= screen_width - 12) was_near_edge = 1;
      else if (!is_vertical && g_taskbar.mouse_y >= screen_height - 12) was_near_edge = 1;
  }

  int force_dirty = (near_edge != was_near_edge);

  if (hovered != g_taskbar.hovered_index || force_dirty ||
      (in_dock && (mx != g_taskbar.mouse_x || my != g_taskbar.mouse_y))) {
    g_taskbar.hovered_index = hovered;
    g_taskbar.mouse_x = mx;
    g_taskbar.mouse_y = my;
    taskbar_update_magnification(mx, my);
    if (force_dirty) {
        extern int ui_dirty;
        ui_dirty = 1;
    }
  } else {
      g_taskbar.mouse_x = mx;
      g_taskbar.mouse_y = my;
  }

  static int tb_last_btn = 0;
  int down_edge = (buttons & 1) && !(tb_last_btn & 1);
  int right_down_edge = (buttons & 2) && !(tb_last_btn & 2);
  int left_up = !(buttons & 1) && (tb_last_btn & 1);
  tb_last_btn = buttons;

  if (right_down_edge && in_dock) {
    g_taskbar.edit_mode = !g_taskbar.edit_mode;
    if (!g_taskbar.edit_mode)
      g_taskbar.drag_active = 0;
    extern int screen_width, screen_height;
    extern void compositor_invalidate_rect(int x, int y, int w, int h);
    compositor_invalidate_rect(0, 0, screen_width, screen_height);
    return 1;
  }

  if (g_taskbar.edit_mode) {
    if (down_edge && in_dock) {
      g_taskbar.drag_active = 1;
    }
    if (left_up) {
      g_taskbar.drag_active = 0;
    }
    if (g_taskbar.drag_active) {
      int new_pos = global_config.taskbar_position;
      extern int screen_width, screen_height;

      if (mx < screen_width / 3)
        new_pos = 1; // Left
      else if (mx > (screen_width * 2) / 3)
        new_pos = 2; // Right
      else if (my > (screen_height * 2) / 3)
        new_pos = 0; // Bottom

      if (new_pos != global_config.taskbar_position) {
        global_config.taskbar_position = new_pos;
        extern void config_save(void);
        config_save();
        taskbar_initialized = 0; // Force immediate re-layout
      }
      return 1;
    }
  }

  if (!down_edge)
    return in_dock || (my < 24);

  // Top Menu Click
  if (my < 24) {
    if (mx < 60)
      startmenu_show(10, 24);
    else if (mx >= screen_width - 180 && mx < screen_width - 60) {
      extern void print_serial(const char *);
      print_serial("[TASKBAR] Tray clicked, calling sysmenu_toggle\n");
      sysmenu_toggle();
    }
    return 1;
  }

  if (!in_dock)
    return 0;

  if (!is_vertical) {
    float total_icon_w = 0.0f;
    for (int i = 0; i < g_taskbar.icon_count; i++)
      total_icon_w += (64.0f * g_taskbar.icons[i].scale);

    int sys_x = g_taskbar.dock_x + 48 + (int)total_icon_w;
    if (mx >= sys_x && mx < sys_x + 64) {
      startmenu_show(sys_x, g_taskbar.dock_y);
      return 1;
    }
  } else {
    if (my >= g_taskbar.dock_y && my < g_taskbar.dock_y + 55) {
      startmenu_show(g_taskbar.dock_x, g_taskbar.dock_y);
      return 1;
    }
  }

  extern int ctrl_pressed;
  for (int i = 0; i < g_taskbar.icon_count; i++) {
    int hit = 0;
    if (!is_vertical) {
      float sw = 64.0f * g_taskbar.icons[i].scale;
      float left = g_taskbar.icons[i].x - (sw - 64.0f) / 2.0f;
      if (mx >= left && mx < left + sw) hit = 1;
    } else {
      float sh = 64.0f * g_taskbar.icons[i].scale;
      float top = g_taskbar.icons[i].y - (sh - 64.0f) / 2.0f;
      if (my >= top && my < top + sh) hit = 1;
    }

    if (hit) {
      if (g_taskbar.icons[i].is_running && g_taskbar.icons[i].win_ref && !ctrl_pressed) {
        window_t *win = g_taskbar.icons[i].win_ref;
        if (win->is_minimized) {
          win->anim_mode = 3; // Restore (Warp)
          float restore_k = 280.0f;
          float restore_d = 30.0f;
          anim_start_spring(&win->anim_scale, 0.01f, 1.0f, restore_k, restore_d);

          // Set pinch direction based on dock position relative to window
          win->pinch_top = (g_taskbar.dock_y < win->y);

          win->is_animating = 1;
          if (!is_vertical) {
            anim_start_spring(&win->anim_x, (float)win->taskbar_x, (float)win->x, restore_k, restore_d);
            anim_start_spring(&win->anim_y, (float)g_taskbar.dock_y, (float)win->y, restore_k, restore_d);
            anim_start_spring(&win->anim_w, 32.0f, (float)win->width, restore_k, restore_d);
            anim_start_spring(&win->anim_h, 32.0f, (float)win->height, restore_k, restore_d);
          } else {
            anim_start_spring(&win->anim_x, (float)g_taskbar.dock_x, (float)win->x, restore_k, restore_d);
            anim_start_spring(&win->anim_y, g_taskbar.icons[i].y, (float)win->y, restore_k, restore_d);
            anim_start_spring(&win->anim_w, 32.0f, (float)win->width, restore_k, restore_d);
            anim_start_spring(&win->anim_h, 32.0f, (float)win->height, restore_k, restore_d);
          }
          win->is_minimized = 0;
        }
        winmgr_bring_to_front(win);
      } else {
        extern int next_anim_origin_x;
        extern int next_anim_origin_y;
        if (!is_vertical) {
          next_anim_origin_x = (int)(g_taskbar.icons[i].x + 32.0f * g_taskbar.icons[i].scale);
          next_anim_origin_y = g_taskbar.dock_y + 20;
        } else {
          next_anim_origin_x = g_taskbar.dock_x + 20;
          next_anim_origin_y = (int)(g_taskbar.icons[i].y + 32.0f * g_taskbar.icons[i].scale);
        }
        launch_app_by_id(g_taskbar.icons[i].app_id);
      }
      return 1;
    }
  }
  return 1;
}
