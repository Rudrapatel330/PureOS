#include "ttf_font.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/window.h"
#include <math.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STBTT_assert(x)
#define STBTT_ifloor(x) ((int)floor(x))
#define STBTT_iceil(x) ((int)ceil(x))
#define STBTT_sqrt(x) sqrt(x)
#define STBTT_pow(x,y) pow(x,y)
#define STBTT_fmod(x,y) fmod(x,y)
#define STBTT_cos(x) cos(x)
#define STBTT_sin(x) sin(x)
#define STBTT_acos(x) acos(x)
#define STBTT_asin(x) asin(x)
#define STBTT_fabs(x) fabs(x)
#define STBTT_malloc(x,u) kmalloc(x)
#define STBTT_free(x,u) kfree(x)

#include "../lib/stb_truetype.h"

static ttf_font_t fonts[TTF_MAX_FONTS];
static stbtt_fontinfo font_infos[TTF_MAX_FONTS];
static int default_font_slot = -1;

// Symbols from the embedded font in fonts.asm
extern uint8_t _binary_browser_font_otf_start[];
extern uint8_t _binary_browser_font_otf_end[];
extern uint8_t _binary_browser_font_otf_size[];

void ttf_init(void) {
  for (int i = 0; i < TTF_MAX_FONTS; i++) {
    fonts[i].loaded = 0;
    fonts[i].data = 0;
    memset(&font_infos[i], 0, sizeof(stbtt_fontinfo));
  }
}

int ttf_load_font(const uint8_t *data, int data_len, int font_slot) {
  if (font_slot < 0 || font_slot >= TTF_MAX_FONTS) return -1;
  if (!data || data_len < 4) return -1;

  if (fonts[font_slot].loaded && fonts[font_slot].data) {
    kfree(fonts[font_slot].data);
  }

  fonts[font_slot].data = (uint8_t *)kmalloc(data_len);
  if (!fonts[font_slot].data) return -1;
  memcpy(fonts[font_slot].data, data, data_len);
  fonts[font_slot].data_len = data_len;

  int offset = stbtt_GetFontOffsetForIndex(fonts[font_slot].data, 0);
  if (offset < 0) {
    kfree(fonts[font_slot].data);
    fonts[font_slot].data = 0;
    return -1;
  }

  if (!stbtt_InitFont(&font_infos[font_slot], fonts[font_slot].data, offset)) {
    kfree(fonts[font_slot].data);
    fonts[font_slot].data = 0;
    return -1;
  }

  fonts[font_slot].loaded = 1;
  fonts[font_slot].font_index = font_slot;
  fonts[font_slot].scale = 1.0f;

  int ascent, descent, line_gap;
  stbtt_GetFontVMetrics(&font_infos[font_slot], &ascent, &descent, &line_gap);
  fonts[font_slot].ascent = ascent;
  fonts[font_slot].descent = descent;
  fonts[font_slot].line_gap = line_gap;

  return 0;
}

int ttf_load_browser_font(void) {
  uint32_t font_size = (uint32_t)(uintptr_t)_binary_browser_font_otf_size;
  if (font_size == 0) return -1;
  int slot = 0;
  int ret = ttf_load_font(_binary_browser_font_otf_start, font_size, slot);
  if (ret == 0) {
    default_font_slot = slot;
    ttf_set_size(slot, 14.0f);
  }
  return ret;
}

int ttf_get_default_font(void) {
  return default_font_slot;
}

void ttf_set_size(int font_slot, float size) {
  if (font_slot < 0 || font_slot >= TTF_MAX_FONTS || !fonts[font_slot].loaded)
    return;
  fonts[font_slot].scale = stbtt_ScaleForPixelHeight(&font_infos[font_slot], size);
}

int ttf_text_width(int font_slot, const char *text) {
  if (font_slot < 0 || font_slot >= TTF_MAX_FONTS || !fonts[font_slot].loaded || !text)
    return 0;

  int total_w = 0;
  while (*text) {
    int advance, lsb;
    stbtt_GetGlyphHMetrics(&font_infos[font_slot],
      stbtt_FindGlyphIndex(&font_infos[font_slot], *text), &advance, &lsb);
    total_w += (int)(advance * fonts[font_slot].scale);
    text++;
  }
  return total_w;
}

int ttf_font_height(int font_slot) {
  if (font_slot < 0 || font_slot >= TTF_MAX_FONTS || !fonts[font_slot].loaded)
    return 0;
  return (int)((fonts[font_slot].ascent - fonts[font_slot].descent) * fonts[font_slot].scale);
}

void ttf_render_text(int font_slot, const char *text, int x, int y,
                     uint32_t *buffer, int buf_w, int buf_h, uint32_t color) {
  if (font_slot < 0 || font_slot >= TTF_MAX_FONTS || !fonts[font_slot].loaded || !text || !buffer)
    return;

  int r = (color >> 16) & 0xFF;
  int g = (color >> 8) & 0xFF;
  int b = color & 0xFF;
  int cur_x = x;

  while (*text) {
    unsigned char c = *text;
    int glyph = stbtt_FindGlyphIndex(&font_infos[font_slot], c);
    if (glyph == 0) { text++; continue; }

    int advance, lsb;
    stbtt_GetGlyphHMetrics(&font_infos[font_slot], glyph, &advance, &lsb);

    int gw, gh, gox, goy;
    unsigned char *bitmap = stbtt_GetGlyphBitmap(&font_infos[font_slot],
      fonts[font_slot].scale, fonts[font_slot].scale, glyph, &gw, &gh, &gox, &goy);

    if (bitmap) {
      int draw_y = y + (int)(fonts[font_slot].ascent * fonts[font_slot].scale) + goy;
      for (int py = 0; py < gh; py++) {
        for (int px = 0; px < gw; px++) {
          int sx = cur_x + gox + px;
          int sy = draw_y + py;
          if (sx >= 0 && sx < buf_w && sy >= 0 && sy < buf_h) {
            unsigned char alpha = bitmap[py * gw + px];
            if (alpha > 0) {
              uint32_t dst = buffer[sy * buf_w + sx];
              int dr = (dst >> 16) & 0xFF;
              int dg = (dst >> 8) & 0xFF;
              int db = dst & 0xFF;
              int nr = (r * alpha + dr * (255 - alpha)) / 255;
              int ng = (g * alpha + dg * (255 - alpha)) / 255;
              int nb = (b * alpha + db * (255 - alpha)) / 255;
              buffer[sy * buf_w + sx] = (0xFF << 24) | (nr << 16) | (ng << 8) | nb;
            }
          }
        }
      }
      stbtt_FreeBitmap(bitmap, 0);
    }

    cur_x += (int)(advance * fonts[font_slot].scale);
    text++;
  }
}

void ttf_draw_text(void *win_ptr, int font_slot, int x, int y,
                   const char *text, uint32_t color) {
  if (font_slot < 0 || font_slot >= TTF_MAX_FONTS || !fonts[font_slot].loaded || !text || !win_ptr)
    return;

  window_t *w = (window_t *)win_ptr;

  int buf_w = w->surface_w > 0 ? w->surface_w : w->width;
  int buf_h = w->surface_h > 0 ? w->surface_h : w->height;

  ttf_render_text(font_slot, text, x, y, w->surface, buf_w, buf_h, color);
}

int ttf_measure_text(const char *text, int font_size) {
  if (!text) return 0;
  if (default_font_slot < 0) return strlen(text) * 8;
  ttf_set_size(default_font_slot, (float)font_size);
  return ttf_text_width(default_font_slot, text);
}
