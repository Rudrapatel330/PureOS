#ifndef TTF_FONT_H
#define TTF_FONT_H

#include <stdint.h>

#define TTF_MAX_FONTS 4
#define TTF_GLYPH_CACHE_SIZE 256

typedef struct {
  uint8_t *data;
  int data_len;
  int font_index; // stbtt_fontinfo index
  float scale;
  int ascent, descent, line_gap;
  int loaded;
} ttf_font_t;

// Initialize the TTF font system
void ttf_init(void);

// Load a TTF font from memory
int ttf_load_font(const uint8_t *data, int data_len, int font_slot);

// Load the embedded browser font (NotoSans-Regular.otf)
int ttf_load_browser_font(void);

// Get the default font slot (-1 if not loaded)
int ttf_get_default_font(void);

// Set font size (in pixels)
void ttf_set_size(int font_slot, float size);

// Get the width of a string in pixels
int ttf_text_width(int font_slot, const char *text);

// Get the height of a font (ascent + descent)
int ttf_font_height(int font_slot);

// Render text to an RGBA buffer
void ttf_render_text(int font_slot, const char *text, int x, int y,
                     uint32_t *buffer, int buf_w, int buf_h, uint32_t color);

// Render text with anti-aliasing to a window surface
void ttf_draw_text(void *win, int font_slot, int x, int y,
                   const char *text, uint32_t color);

// Measure text using default browser font
int ttf_measure_text(const char *text, int font_size);

#endif
