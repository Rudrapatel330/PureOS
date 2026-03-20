#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>

#define TEXT_NORMAL     0
#define TEXT_INVERTED   1
#define TEXT_UNDERLINE  2
#define TEXT_BOLD       3

#define COLOR_BLACK     0x000000
#define COLOR_WHITE     0xFFFFFF
#define COLOR_RED       0xFF0000
#define COLOR_GREEN     0x00FF00
#define COLOR_BLUE      0x0000FF
#define COLOR_YELLOW    0xFFFF00
#define COLOR_CYAN      0x00FFFF
#define COLOR_MAGENTA   0xFF00FF
#define COLOR_GRAY      0x808080
#define COLOR_LGRAY     0xC0C0C0
#define COLOR_DGRAY     0x404040

void text_init(void);

void text_draw_char(int x, int y, char c, uint32_t color);
void text_draw_string(int x, int y, const char* str, uint32_t color);
void text_draw_string_with_bg(int x, int y, const char* str, uint32_t fg, uint32_t bg);

void text_draw_char_in_window(void* win, int x, int y, char c, uint32_t color);
void text_draw_string_in_window(void* win, int x, int y, const char* str, uint32_t color);

int text_string_width(const char* str);
int text_string_height(const char* str);
int text_char_width(void);
int text_char_height(void);

void text_draw_int(int x, int y, int value, uint32_t color);
void text_draw_hex(int x, int y, uint32_t value, uint32_t color);

#endif
