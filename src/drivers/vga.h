#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void vga_init();
void vga_put_pixel(int x, int y, uint32_t color);
void vga_draw_rect(int x, int y, int w, int h, uint32_t color);
void vga_clear_screen(uint32_t color);
void vga_draw_char(int x, int y, char c, uint32_t color);
void vga_draw_string(int x, int y, const char *str, uint32_t color);
void vga_flip(); // Double Buffering

// New LFB-specific functions
// New LFB-specific functions
void vga_put_pixel_lfb(int x, int y, uint32_t color, uint32_t *buffer);
void vga_draw_rect_lfb(int x, int y, int w, int h, uint32_t color,
                       uint32_t *buffer);
void vga_draw_char_lfb(int x, int y, char c, uint32_t color, uint32_t *buffer);
void vga_draw_string_lfb(int x, int y, const char *str, uint32_t color,
                         uint32_t *buffer);

// Surface-aware functions for off-screen caching
void vga_draw_rect_surface(int x, int y, int w, int h, uint32_t color,
                           uint32_t *buffer, int buf_w, int buf_h);
void vga_draw_char_surface(int x, int y, char c, uint32_t color,
                           uint32_t *buffer, int buf_w, int buf_h);
void vga_draw_string_surface(int x, int y, const char *str, uint32_t color,
                             uint32_t *buffer, int buf_w, int buf_h);

// Bitmap Scaling
void vga_draw_scaled_bitmap(int dx, int dy, int dw, int dh, uint32_t *src_buf,
                            int sw, int sh, uint32_t *dest_buf, int dest_w,
                            int dest_h);

// Alpha blend / Rounded rects
void vga_draw_rect_blend_lfb_ex(int x, int y, int w, int h, uint32_t bg_color,
                                int border_thickness, uint32_t border_color,
                                uint32_t *buffer, int radius);
void vga_draw_rect_blend_lfb(int x, int y, int w, int h, uint32_t color,
                             uint32_t *buffer, int radius);

uint32_t vga_apply_color_filter(uint32_t color, int filter_type, int intensity);

// Optimization Helpers
void vga_restore_rect(int x, int y, int w, int h);
void vga_put_pixel_lfb_legacy(int x, int y, uint32_t color);

// Text / Debug
void vga_puts(int x, int y, const char *str, uint32_t color);
void vga_put_int(int num);

uint32_t color_32_to_16(uint32_t color);

extern uint32_t *back_buffer;
extern uint32_t buffer_size;

#endif
