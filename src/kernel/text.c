#include "text.h"
#include "../drivers/vga.h"
#include "screen.h"

extern const uint8_t font8x8_basic[256][8];
extern uint32_t *backbuffer;
extern int screen_width;
extern int screen_height;

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

void text_init(void) {}

static uint16_t rgb_to_16(uint32_t rgb) {
  uint8_t r = (rgb >> 16) & 0xFF;
  uint8_t g = (rgb >> 8) & 0xFF;
  uint8_t b = rgb & 0xFF;

  return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

void text_draw_char(int x, int y, char c, uint32_t color) {
  if (x < 0 || x + CHAR_WIDTH > screen_width || y < 0 ||
      y + CHAR_HEIGHT > screen_height) {
    return;
  }

  const uint8_t *glyph = font8x8_basic[(unsigned char)c];

  for (int cy = 0; cy < CHAR_HEIGHT; cy++) {
    uint8_t row = glyph[cy];
    int py = y + cy;

    for (int cx = 0; cx < CHAR_WIDTH; cx++) {
      if (row & (1 << (7 - cx))) {
        int px = x + cx;
        backbuffer[py * screen_width + px] = color;
      }
    }
  }
}

void text_draw_char_with_bg(int x, int y, char c, uint32_t fg, uint32_t bg) {
  if (x < 0 || x + CHAR_WIDTH > screen_width || y < 0 ||
      y + CHAR_HEIGHT > screen_height) {
    return;
  }

  const uint8_t *glyph = font8x8_basic[(unsigned char)c];

  for (int cy = 0; cy < CHAR_HEIGHT; cy++) {
    uint8_t row = glyph[cy];
    int py = y + cy;

    for (int cx = 0; cx < CHAR_WIDTH; cx++) {
      int px = x + cx;
      if (row & (1 << (7 - cx))) {
        backbuffer[py * screen_width + px] = fg;
      } else {
        backbuffer[py * screen_width + px] = bg;
      }
    }
  }
}

void text_draw_string(int x, int y, const char *str, uint32_t color) {
  int start_x = x;

  while (*str) {
    if (*str == '\n') {
      x = start_x;
      y += CHAR_HEIGHT;
    } else {
      text_draw_char(x, y, *str, color);
      x += CHAR_WIDTH;
    }
    str++;
  }
}

void text_draw_string_with_bg(int x, int y, const char *str, uint32_t fg,
                              uint32_t bg) {
  int start_x = x;

  while (*str) {
    if (*str == '\n') {
      x = start_x;
      y += CHAR_HEIGHT;
    } else {
      text_draw_char_with_bg(x, y, *str, fg, bg);
      x += CHAR_WIDTH;
    }
    str++;
  }
}

void text_draw_char_in_window(void *win_ptr, int rel_x, int rel_y, char c,
                              uint32_t color) {
  int screen_x = ((int *)win_ptr)[1] + rel_x;
  int screen_y = ((int *)win_ptr)[2] + rel_y;
  text_draw_char(screen_x, screen_y, c, color);
}

void text_draw_string_in_window(void *win_ptr, int rel_x, int rel_y,
                                const char *str, uint32_t color) {
  int screen_x = ((int *)win_ptr)[1] + rel_x;
  int screen_y = ((int *)win_ptr)[2] + rel_y;
  text_draw_string(screen_x, screen_y, str, color);
}

int text_string_width(const char *str) {
  int len = 0;
  while (*str) {
    if (*str != '\n')
      len++;
    str++;
  }
  return len * CHAR_WIDTH;
}

int text_string_height(const char *str) {
  int lines = 1;
  while (*str) {
    if (*str == '\n')
      lines++;
    str++;
  }
  return lines * CHAR_HEIGHT;
}

int text_char_width(void) { return CHAR_WIDTH; }
int text_char_height(void) { return CHAR_HEIGHT; }

void text_draw_int(int x, int y, int value, uint32_t color) {
  char buf[16];
  int i = 0;

  if (value == 0) {
    buf[0] = '0';
    buf[1] = 0;
    text_draw_string(x, y, buf, color);
    return;
  }

  if (value < 0) {
    buf[i++] = '-';
    value = -value;
  }

  char temp[16];
  int j = 0;
  while (value > 0) {
    temp[j++] = '0' + (value % 10);
    value /= 10;
  }

  while (j > 0) {
    buf[i++] = temp[--j];
  }
  buf[i] = 0;

  text_draw_string(x, y, buf, color);
}

void text_draw_hex(int x, int y, uint32_t value, uint32_t color) {
  char buf[11] = "0x00000000";
  const char hex[] = "0123456789ABCDEF";

  buf[2] = hex[(value >> 28) & 0xF];
  buf[3] = hex[(value >> 24) & 0xF];
  buf[4] = hex[(value >> 20) & 0xF];
  buf[5] = hex[(value >> 16) & 0xF];
  buf[6] = hex[(value >> 12) & 0xF];
  buf[7] = hex[(value >> 8) & 0xF];
  buf[8] = hex[(value >> 4) & 0xF];
  buf[9] = hex[value & 0xF];

  text_draw_string(x, y, buf, color);
}
