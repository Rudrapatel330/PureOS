#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include "window.h"
#include "config.h"

// Font scaling constants (base is 8px)
#define FONT_SCALE_NORMAL 16
#define FONT_SCALE_LARGE 20
#define FONT_SCALE_XL 24

// Get the current font scale in pixels
int ui_get_font_scale(void);

// Measure the width of a single line of text
int ui_measure_text_width(const char *text, int font_scale);

// Measure the total height required for text within a given width (includes wrapping)
int ui_measure_text_height(const char *text, int max_width, int font_scale);

// Draw wrapped text into a window's surface
void ui_draw_text_wrapped(window_t *win, int x, int y, int max_width, const char *text, uint32_t color, int font_scale);

// Draw scaled text (single line)
void ui_draw_text_scaled(window_t *win, int x, int y, const char *text, uint32_t color, int font_scale);

#endif
