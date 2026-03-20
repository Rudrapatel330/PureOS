#include "theme.h"
#include "config.h"

extern void print_serial(const char *str);

static int current_mode = THEME_DARK;

// --- Dark Theme (Deep grays + blue accent) ---
static const theme_t dark_theme = {
    .bg = 0xFF1E1E2E,                // Deep charcoal
    .fg = 0xFFCDD6F4,                // Lavender white
    .fg_secondary = 0xFF6C7086,      // Muted gray
    .accent = 0xFF89B4FA,            // Soft blue
    .titlebar = 0xFF181825,          // Near black
    .titlebar_text = 0xFFCDD6F4,     // Lavender white
    .titlebar_inactive = 0xFF313244, // Dark gray
    .button = 0xFF45475A,            // Medium gray
    .button_hover = 0xFF585B70,      // Lighter gray
    .button_text = 0xFFCDD6F4,       // Lavender white
    .border = 0xFF45475A,            // Medium gray
    .taskbar_bg = 0xCC11111B,        // Semi-transparent very dark
    .taskbar_text = 0xFFCDD6F4,      // Lavender white
    .menu_bg = 0xF0181825,           // Near-opaque dark
    .menu_hover = 0xFF313244,        // Highlighted row
    .input_bg = 0xFF1E1E2E,          // Match window bg
    .input_border = 0xFF45475A,      // Medium gray
    .scrollbar = 0xFF313244,         // Dark gray
    .scrollbar_thumb = 0xFF585B70,   // Medium gray
};

// --- Light Theme (White bg + slate text + teal accent) ---
static const theme_t light_theme = {
    .bg = 0xFFF5F5F5,                // Off-white
    .fg = 0xFF1E1E2E,                // Dark text
    .fg_secondary = 0xFF6C7086,      // Muted text
    .accent = 0xFF0EA5E9,            // Sky blue
    .titlebar = 0xFFE2E8F0,          // Light slate
    .titlebar_text = 0xFF1E293B,     // Dark slate
    .titlebar_inactive = 0xFFF1F5F9, // Very light
    .button = 0xFFE2E8F0,            // Light slate
    .button_hover = 0xFFCBD5E1,      // Darker slate
    .button_text = 0xFF1E293B,       // Dark slate
    .border = 0xFFCBD5E1,            // Slate border
    .taskbar_bg = 0xE6F8FAFC,        // Light semi-transparent
    .taskbar_text = 0xFF1E293B,      // Dark slate
    .menu_bg = 0xF0FFFFFF,           // White
    .menu_hover = 0xFFE2E8F0,        // Light slate
    .input_bg = 0xFFFFFFFF,          // Pure white
    .input_border = 0xFFCBD5E1,      // Slate
    .scrollbar = 0xFFE2E8F0,         // Light
    .scrollbar_thumb = 0xFFCBD5E1,   // Slate
};

const theme_t *theme_get(void) {
  if (current_mode == THEME_LIGHT)
    return &light_theme;
  return &dark_theme;
}

void theme_set_mode(int mode) {
  current_mode = mode;
  global_config.theme_mode = mode;
  print_serial("THEME: Mode changed to ");
  print_serial(mode == THEME_LIGHT ? "light" : "dark");
  print_serial("\n");
}

int theme_get_mode(void) { return current_mode; }

void theme_init(void) {
  current_mode = global_config.theme_mode;
  print_serial("THEME: Initialized (");
  print_serial(current_mode == THEME_LIGHT ? "light" : "dark");
  print_serial(" mode)\n");
}
