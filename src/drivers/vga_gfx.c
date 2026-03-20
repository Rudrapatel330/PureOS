// vga_gfx.c
#include "vga_gfx.h"

// VGA Mode 13h Framebuffer
#define VGA_MEM ((uint8_t*)0xA0000)
#define SCREEN_W 320
#define SCREEN_H 200

// Cursor buffers
#define CURSOR_W 12
#define CURSOR_H 19
uint8_t cursor_bg[CURSOR_W * CURSOR_H];

void gfx_putpixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        VGA_MEM[y * SCREEN_W + x] = color;
    }
}

uint8_t gfx_getpixel(int x, int y) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        return VGA_MEM[y * SCREEN_W + x];
    }
    return 0;
}

void gfx_fill(uint8_t color) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        VGA_MEM[i] = color;
    }
}

void gfx_draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            gfx_putpixel(i, j, color);
        }
    }
}

void gfx_test_pattern() {
    gfx_fill(1); // Blue
    gfx_draw_rect(20, 20, 100, 50, 40); // Red
    gfx_draw_rect(150, 20, 100, 50, 48); // Green
    for (int i = 0; i < 256; i++) {
        gfx_draw_rect((i % 32) * 10, 100 + (i / 32) * 10, 10, 10, i);
    }
}

void gfx_save_cursor(int x, int y) {
    int i = 0;
    for (int dy = 0; dy < CURSOR_H; dy++) {
        for (int dx = 0; dx < CURSOR_W; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < SCREEN_W && py >= 0 && py < SCREEN_H) {
                cursor_bg[i++] = VGA_MEM[py * SCREEN_W + px];
            } else {
                cursor_bg[i++] = 0; // Black or safe default
            }
        }
    }
}

void gfx_restore_cursor(int x, int y) {
    int i = 0;
    for (int dy = 0; dy < CURSOR_H; dy++) {
        for (int dx = 0; dx < CURSOR_W; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < SCREEN_W && py >= 0 && py < SCREEN_H) {
                 VGA_MEM[py * SCREEN_W + px] = cursor_bg[i];
            }
            i++;
        }
    }
}

void gfx_draw_cursor(int x, int y) {
    // Standard arrow cursor implementation
    // 1 = White, 0 = Transparent (skip)
    // Using simplistic shape for 12x19
    static const uint8_t arrow[] = {
        1,0,0,0,0,0,0,0,0,0,0,0,
        1,1,0,0,0,0,0,0,0,0,0,0,
        1,1,1,0,0,0,0,0,0,0,0,0,
        1,1,1,1,0,0,0,0,0,0,0,0,
        1,1,1,1,1,0,0,0,0,0,0,0,
        1,1,1,1,1,1,0,0,0,0,0,0,
        1,1,1,1,1,1,1,0,0,0,0,0,
        1,1,1,1,1,1,1,1,0,0,0,0,
        1,1,1,1,1,1,1,1,1,0,0,0,
        1,1,1,1,1,1,1,1,1,1,0,0,
        1,1,1,1,1,1,0,0,0,0,0,0,
        1,1,0,0,1,1,0,0,0,0,0,0,
        1,0,0,0,1,1,1,0,0,0,0,0,
        0,0,0,0,0,1,1,1,0,0,0,0,
        0,0,0,0,0,1,1,1,0,0,0,0,
        0,0,0,0,0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,1,1,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0
    };
    
    for (int dy = 0; dy < 19; dy++) {
        for (int dx = 0; dx < 12; dx++) {
            if (arrow[dy * 12 + dx]) {
                gfx_putpixel(x + dx, y + dy, 15); // White
            }
        }
    }
}
