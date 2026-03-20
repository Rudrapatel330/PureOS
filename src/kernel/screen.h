#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

// Screen constants
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900
#define SCREEN_BPP 32

// External variables
extern int screen_width;
extern int screen_height;
extern int current_cursor_type; // 0 = default, 1 = resize

// Function prototypes for screen operations
void vga_flip(void);
void vga_flip_rect(int x, int y, int w, int h);
void vga_draw_rect(int x, int y, int w, int h, uint32_t color);
void vga_draw_string(int x, int y, const char *str, uint32_t color);

#endif
