#ifndef VGA_GRAPHICS_H
#define VGA_GRAPHICS_H

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

void vga_init();
void vga_clear(unsigned char color);
void vga_putchar(int x, int y, char c, unsigned char color);
void vga_puts(int x, int y, const char *str, unsigned char color);
void vga_draw_box(int x, int y, int width, int height, unsigned char color);

#endif
