#ifndef THEME_H
#define THEME_H

#include <stdint.h>

// Named color slots for the UI theme
typedef struct {
  uint32_t bg;                // Window/app background
  uint32_t fg;                // Primary text color
  uint32_t fg_secondary;      // Secondary/dimmed text
  uint32_t accent;            // Accent color (buttons, highlights)
  uint32_t titlebar;          // Window titlebar background
  uint32_t titlebar_text;     // Window titlebar text
  uint32_t titlebar_inactive; // Inactive window titlebar
  uint32_t button;            // Button background
  uint32_t button_hover;      // Button hover state
  uint32_t button_text;       // Button text
  uint32_t border;            // Window/element borders
  uint32_t taskbar_bg;        // Taskbar background
  uint32_t taskbar_text;      // Taskbar text
  uint32_t menu_bg;           // Start menu / context menu bg
  uint32_t menu_hover;        // Menu item hover
  uint32_t input_bg;          // Text input background
  uint32_t input_border;      // Text input border
  uint32_t scrollbar;         // Scrollbar track
  uint32_t scrollbar_thumb;   // Scrollbar thumb
} theme_t;

// Theme modes
#define THEME_DARK 0
#define THEME_LIGHT 1

// Get current theme
const theme_t *theme_get(void);

// Set theme mode (THEME_DARK or THEME_LIGHT)
void theme_set_mode(int mode);

// Get current mode
int theme_get_mode(void);

// Initialize theme system
void theme_init(void);

#endif
