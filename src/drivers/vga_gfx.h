// vga_gfx.h
#ifndef VGA_GFX_H
#define VGA_GFX_H

#include "../kernel/types.h"

void gfx_putpixel(int x, int y, uint8_t color);
uint8_t gfx_getpixel(int x, int y);
void gfx_fill(uint8_t color);
void gfx_draw_rect(int x, int y, int w, int h, uint8_t color);
void gfx_test_pattern();

// Cursor functions
void gfx_save_cursor(int x, int y);
void gfx_restore_cursor(int x, int y);
void gfx_draw_cursor(int x, int y);

#endif
