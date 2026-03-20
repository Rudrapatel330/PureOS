#include "vga_graphics.h"
#include "font.h"

static unsigned char *vram = (unsigned char*)0xA0000;

// Need to declare external font array from font.c
extern unsigned char font8x8_basic[128][8];

void vga_clear(unsigned char color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        vram[i] = color;
    }
}

void vga_putchar(int x, int y, char c, unsigned char color) {
    if (c < 0 || c >= 128) return;  // Only handle ASCII
    
    // Use the font array from font.c
    const unsigned char *glyph = font8x8_basic[(int)c];
    
    for (int dy = 0; dy < CHAR_HEIGHT; dy++) {
        unsigned char row = glyph[dy];
        for (int dx = 0; dx < CHAR_WIDTH; dx++) {
            // Font bits are usually 1=set. 
            // Checking bit 7-dx means left-to-right.
            if (row & (1 << (7 - dx))) { 
                int px = x + dx;
                int py = y + dy;
                // Clip
                if (px < SCREEN_WIDTH && py < SCREEN_HEIGHT) {
                    vram[py * SCREEN_WIDTH + px] = color;
                }
            }
        }
    }
}

void vga_puts(int x, int y, const char *str, unsigned char color) {
    int cursor_x = x;
    int cursor_y = y;
    while (*str) {
        if (*str == '\n') {
            cursor_y += CHAR_HEIGHT;
            cursor_x = x;
        } else {
            vga_putchar(cursor_x, cursor_y, *str, color);
            cursor_x += CHAR_WIDTH;
        }
        str++;
    }
}

void vga_draw_box(int x, int y, int width, int height, unsigned char color) {
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px < SCREEN_WIDTH && py < SCREEN_HEIGHT) {
                vram[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
}
