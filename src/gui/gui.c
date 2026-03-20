#include "gui.h"
#include "../drivers/vbe.h"
#include "font.h"

extern u32* framebuffer;
extern int screen_width;
extern int screen_height;
extern int screen_bpp;

void gui_init() {
    // Nothing special for now, just ensure VBE was init
}

void put_pixel(int x, int y, u32 color) {
    if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) return;
    
    if (screen_bpp == 24) {
        // BGR Format for 24-bit
        u8* vmem = (u8*)framebuffer;
        int offset = (y * screen_width + x) * 3;
        vmem[offset]     = (color) & 0xFF;       // Blue
        vmem[offset + 1] = (color >> 8) & 0xFF;  // Green
        vmem[offset + 2] = (color >> 16) & 0xFF; // Red
    } else if (screen_bpp == 32) {
        framebuffer[y * screen_width + x] = color;
    } else if (screen_bpp == 8) {
        u8* vga_mem = (u8*)framebuffer;
        vga_mem[y * screen_width + x] = (u8)color;
    }
}

void fill_screen(u32 color) {
    int x, y;
    
    if (screen_bpp == 24) {
        u8 r = (color >> 16) & 0xFF;
        u8 g = (color >> 8) & 0xFF;
        u8 b = color & 0xFF;
        
        u8* vmem = (u8*)framebuffer;
        int len = screen_width * screen_height;
        for (int i = 0; i < len; i++) {
            vmem[i*3]     = b;
            vmem[i*3 + 1] = g;
            vmem[i*3 + 2] = r;
        }
    } else if (screen_bpp == 32) {
        for (y = 0; y < screen_height; y++) {
            for (x = 0; x < screen_width; x++) {
                framebuffer[y * screen_width + x] = color;
            }
        }
    } else if (screen_bpp == 8) {
        // VGA 13h
        u8* vga_mem = (u8*)framebuffer;
        u8 c = (u8)(color & 0xFF);
        for (y = 0; y < screen_height; y++) {
            for (x = 0; x < screen_width; x++) {
                vga_mem[y * screen_width + x] = c;
            }
        }
    }
}

void draw_rect(int x, int y, int w, int h, u32 color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            put_pixel(i, j, color);
        }
    }
}

void draw_char(int x, int y, char c, u32 color) {
    if (c < 0 || c > 127) return;
    
    u8* bitmap = font8x8_basic[(int)c];
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            // Bit 7 is leftmost
            if (bitmap[row] & (1 << (7-col))) {
                put_pixel(x + col, y + row, color);
            }
        }
    }
}

void draw_string(int x, int y, char* str, u32 color) {
    int cur_x = x;
    while (*str) {
        draw_char(cur_x, y, *str, color);
        cur_x += 8;
        str++;
    }
}
