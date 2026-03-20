#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct {
  int magic; // 0x105C0002
  char lock_password[32];
  int wallpaper_type; // 0 = BMP, 1 = dark gradient, 2 = light gradient, 3 = PNG
  int theme_mode;     // 0 = dark mode (default), 1 = light mode
  int screen_width;   // Typically 1024
  int screen_height;  // Typically 768
  int show_desktop_icons;       // 0 = hidden, 1 = visible (default)
  int icon_filter;              // 0 = None, 1 = Red, 2 = Green, 3 = Yellow
  int icon_filter_intensity;    // 0-255
  int icon_bg_filter_intensity; // 0-255
  int show_clock_widget;        // Digital clock on desktop
  int show_calendar_widget;     // Calendar on desktop
  int clock_x, clock_y, clock_w, clock_h;
  int calendar_x, calendar_y, calendar_w, calendar_h;
  int timezone_offset_m; // in minutes (e.g. +330 for IST)
  int num_pinned;
  int pinned[10];       // Array of APP_XXX IDs
  int taskbar_position; // 0 = Bottom, 1 = Left, 2 = Right
} os_config_t;

extern os_config_t global_config;

void config_init(void);
void config_load(void);
void config_save(void);

#endif
