#include "config.h"
#include "string.h"

os_config_t global_config;

void config_init(void) {
  global_config.magic = 0x105C0002;
  // Set default values first in case file load fails
  strcpy(global_config.lock_password, "123456");
  global_config.wallpaper_type = 3; // 3 = PNG
  global_config.theme_mode = 0;     // 0 = dark (default)
  global_config.screen_width = 1600;
  global_config.screen_height = 900;
  global_config.show_desktop_icons = 1; // 1 = visible (default)
  global_config.num_pinned = 6;
  global_config.pinned[0] = 5;  // APP_FILEMGR
  global_config.pinned[1] = 0;  // APP_TERMINAL
  global_config.pinned[2] = 2;  // APP_EDITOR
  global_config.pinned[3] = 13; // APP_MAIL
  global_config.pinned[4] = 7;  // APP_BROWSER
  global_config.pinned[5] = 9;  // APP_SETTINGS
  global_config.show_clock_widget = 0;
  global_config.show_calendar_widget = 0;
  global_config.clock_x = 1320;
  global_config.clock_y = 60;
  global_config.clock_w = 200;
  global_config.clock_h = 200;
  global_config.calendar_x = 1310;
  global_config.calendar_y = 280;
  global_config.calendar_w = 220;
  global_config.calendar_h = 240;
  global_config.timezone_offset_m = 0; // Default to UTC
  global_config.taskbar_position = 0;  // 0 = bottom
}

#include "../fs/fat.h"

void config_load(void) {
  extern void print_serial(const char *);
  config_init(); // Set defaults first
  print_serial("CONFIG: Loading from disk (C:/os.cfg)...\n");

  uint8_t buffer[sizeof(os_config_t) + 64];
  int bytes = fat_read_file("/os.cfg", buffer);

  // Accept old config files that are smaller than current struct
  // (backward compatibility when new fields are added)
  int min_size = 48; // Size up to screen_height (the original core fields)
  if (bytes >= min_size) {
    // Copy whatever we have
    int copy_size =
        bytes < (int)sizeof(os_config_t) ? bytes : (int)sizeof(os_config_t);

    // Save current defaults in case we need to partial-copy or magic fails
    os_config_t temp_cfg = global_config;
    memcpy(&temp_cfg, buffer, copy_size);

    if (temp_cfg.magic == 0x105C0002) {
      global_config = temp_cfg;
      print_serial("CONFIG: Loaded successfully. Magic: 0x");
      char mstr[16];
      extern void k_itoa_hex(uint32_t, char *);
      k_itoa_hex(global_config.magic, mstr);
      print_serial(mstr);
      print_serial(" Password: ");
      print_serial(global_config.lock_password);
      print_serial("\n");
      // FORCE PNG for this version to ensure the user sees the new wallpaper
      global_config.wallpaper_type = 3;
    } else {
      print_serial("CONFIG: Incompatible version (Magic mismatch). Resetting "
                   "to defaults.\n");
      // Keep defaults from config_init() which was called above
    }
  } else {
    print_serial("CONFIG: File not found or invalid size. Using defaults.\n");
    // Keep defaults
  }
}

void config_save(void) {
  extern void print_serial(const char *);
  extern void k_itoa(int, char *);
  print_serial("CONFIG: Saving to disk (C:/os.cfg). Password: ");
  print_serial(global_config.lock_password);
  print_serial("\n");

  int result =
      fat_write_file("/os.cfg", (uint8_t *)&global_config, sizeof(os_config_t));
  if (result == 1) {
    print_serial("CONFIG: Saved successfully.\n");
  } else {
    print_serial("CONFIG: Save FAILED! (fat_write_file returned ");
    char buf[16];
    k_itoa(result, buf);
    print_serial(buf);
    print_serial(")\n");
  }
}
