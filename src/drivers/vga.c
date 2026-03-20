// vga.c - Redirected to 16bpp Framebuffer
#include "vga.h"
#include "../drivers/ports.h"
#include "../kernel/screen.h"
#include <stdint.h>

// Defined in kernel.c
extern uint32_t *framebuffer;
extern void draw_char_32bpp(int x, int y, char c, uint32_t color);

// Globals for linking compatibility (unused)
uint32_t *back_buffer = 0;
uint32_t buffer_size = 0;

// Transparent color helper (no longer needed for 32->16 but kept for
// compatibility)
uint32_t color_32_to_32(uint32_t color) { return color; }

// LEGACY - Draws to global framebuffer
void vga_put_pixel_lfb_legacy(int x, int y, uint32_t color) {
  if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
    framebuffer[y * screen_width + x] = color;
  }
}

// NEW - Draws to a specified buffer
void vga_put_pixel_lfb(int x, int y, uint32_t color, uint32_t *buffer) {
  if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
    buffer[y * screen_width + x] = color;
  }
}

void vga_put_pixel(int x, int y, uint32_t color) {
  vga_put_pixel_lfb_legacy(x, y, color);
}

void vga_draw_rect(int x, int y, int w, int h, uint32_t color) {
  for (int cy = 0; cy < h; cy++) {
    int py = y + cy;
    if (py < 0 || py >= screen_height)
      continue;

    for (int cx = 0; cx < w; cx++) {
      int px = x + cx;
      if (px < 0 || px >= screen_width)
        continue;

      framebuffer[py * screen_width + px] = color;
    }
  }
}

// NEW - Draws rect to a specified buffer with custom dimensions
void vga_draw_rect_surface(int x, int y, int w, int h, uint32_t color,
                           uint32_t *buffer, int buf_w, int buf_h) {
  if (!buffer)
    return;

  for (int cy = 0; cy < h; cy++) {
    int py = y + cy;
    if (py < 0 || py >= buf_h)
      continue;

    // Row-level clipping
    int rx = x;
    int rw = w;
    if (rx < 0) {
      rw += rx;
      rx = 0;
    }
    if (rx + rw > buf_w) {
      rw = buf_w - rx;
    }
    if (rw <= 0)
      continue;

    uint32_t *line = &buffer[py * buf_w + rx];
    for (int cx = 0; cx < rw; cx++) {
      line[cx] = color;
    }
  }
}

void vga_draw_rect_lfb(int x, int y, int w, int h, uint32_t color,
                       uint32_t *buffer) {
  vga_draw_rect_surface(x, y, w, h, color, buffer, screen_width, screen_height);
}

void vga_draw_char(int x, int y, char c, uint32_t color) {
  draw_char_32bpp(x, y, c, color);
}

void vga_draw_char_lfb(int x, int y, char c, uint32_t color, uint32_t *buffer) {
  vga_draw_char_surface(x, y, c, color, buffer, screen_width, screen_height);
}

// NEW - Draws char to a specified buffer with custom dimensions
void vga_draw_char_surface(int x, int y, char c, uint32_t color,
                           uint32_t *buffer, int buf_w, int buf_h) {
  extern const uint8_t font8x8_basic[256][8];
  const uint8_t *glyph = font8x8_basic[(unsigned char)c];

  for (int cy = 0; cy < 8; cy++) {
    uint8_t row = glyph[cy];
    for (int cx = 0; cx < 8; cx++) {
      if (row & (1 << (7 - cx))) {
        int px = x + cx;
        int py = y + cy;
        if (px >= 0 && px < buf_w && py >= 0 && py < buf_h) {
          buffer[py * buf_w + px] = color;
        }
      }
    }
  }
}

// External serial print
extern void print_serial(const char *str);
static void print_char_serial(char c) {
  char s[2] = {c, 0};
  print_serial(s);
}

void vga_draw_string(int x, int y, const char *str, uint32_t color) {
  while (*str) {
    draw_char_32bpp(x, y, *str++, color);
    x += 8;
  }
}

// NEW - Draws string to a specified buffer
void vga_draw_string_lfb(int x, int y, const char *str, uint32_t color,
                         uint32_t *buffer) {
  vga_draw_string_surface(x, y, str, color, buffer, screen_width,
                          screen_height);
}

void vga_draw_string_surface(int x, int y, const char *str, uint32_t color,
                             uint32_t *buffer, int buf_w, int buf_h) {
  if (!str || !buffer)
    return;

  if (x >= buf_w || y >= buf_h) {
    return;
  }

  while (*str) {
    vga_draw_char_surface(x, y, *str++, color, buffer, buf_w, buf_h);
    x += 8;
  }
}

void vga_draw_scaled_bitmap(int dx, int dy, int dw, int dh, uint32_t *src_buf,
                            int sw, int sh, uint32_t *dest_buf, int dest_w,
                            int dest_h) {
  if (!src_buf || !dest_buf || dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0)
    return;

  // Fixed-point scaling (16.16) for speed and to avoid floats in kernel
  int step_x = (sw << 16) / dw;
  int step_y = (sh << 16) / dh;

  for (int y = 0; y < dh; y++) {
    int cur_dy = dy + y;
    if (cur_dy < 0 || cur_dy >= dest_h)
      continue;

    int src_y = (y * step_y) >> 16;
    if (src_y >= sh)
      src_y = sh - 1;

    uint32_t *dest_row = &dest_buf[cur_dy * dest_w];
    uint32_t *src_row = &src_buf[src_y * sw];

    for (int x = 0; x < dw; x++) {
      int cur_dx = dx + x;
      if (cur_dx < 0 || cur_dx >= dest_w)
        continue;

      int src_x = (x * step_x) >> 16;
      if (src_x >= sw)
        src_x = sw - 1;

      dest_row[cur_dx] = src_row[src_x];
    }
  }
}

void vga_init(void) {
  // Nothing needed for VESA LFB mode (handled by kernel/bootloader)
}

// void vga_flip() - Implemented in kernel.c for double buffering

void vga_restore_rect(int x, int y, int w, int h) {
  // No backbuffer to restore from
}

void vga_clear_screen(uint32_t color) {
  vga_draw_rect(0, 0, screen_width, screen_height, color);
}

// Debug output
void vga_puts(int x, int y, const char *str, uint32_t color) {
  vga_draw_string(x > 0 ? x : 0, y > 0 ? y : 0, str, color);
}

void vga_put_int(int num) {
  // Implementation not critical for now
}

static int int_sqrt(int n) {
  if (n <= 0)
    return 0;
  int x = n;
  int y = (x + 1) / 2;
  while (y < x) {
    x = y;
    y = (x + n / x) / 2;
  }
  return x;
}

void vga_draw_rect_blend_lfb_ex(int x, int y, int w, int h, uint32_t bg_color,
                                int border_thickness, uint32_t border_color,
                                uint32_t *buffer, int radius) {
  uint32_t bg_a = (bg_color >> 24) & 0xFF;
  uint32_t bd_a = (border_color >> 24) & 0xFF;

  if (bg_a == 0 && bd_a == 0)
    return;

  uint32_t bg_rb = bg_color & 0xFF00FF;
  uint32_t bg_g = bg_color & 0x00FF00;

  uint32_t bd_rb = border_color & 0xFF00FF;
  uint32_t bd_g = border_color & 0x00FF00;

  for (int cy = 0; cy < h; cy++) {
    int py = y + cy;
    if (py < 0 || py >= screen_height)
      continue;
    uint32_t *line = &buffer[py * screen_width];
    for (int cx = 0; cx < w; cx++) {
      int px = x + cx;
      if (px < 0 || px >= screen_width)
        continue;

      int cur_a = 0;
      uint32_t target_rb = 0, target_g = 0;

      if (radius > 0) {
        int dx = 0, dy = 0;
        if (cx < radius)
          dx = radius * 2 - (cx * 2 + 1);
        else if (cx >= w - radius)
          dx = (cx * 2 + 1) - (w - radius) * 2;

        if (cy < radius)
          dy = radius * 2 - (cy * 2 + 1);
        else if (cy >= h - radius)
          dy = (cy * 2 + 1) - (h - radius) * 2;

        int dist_x2 = 0;
        if (dx > 0 && dy > 0) {
          dist_x2 = int_sqrt(dx * dx + dy * dy);
        } else {
          dist_x2 = dx > dy ? dx : dy;
        }

        int shape_dist_x2 = dist_x2 - (radius * 2);

        if (shape_dist_x2 >= 2)
          continue;

        if (border_thickness > 0 &&
            shape_dist_x2 >= -(border_thickness * 2) - 1) {
          cur_a = bd_a;
          target_rb = bd_rb;
          target_g = bd_g;
        } else {
          cur_a = bg_a;
          target_rb = bg_rb;
          target_g = bg_g;
        }

        if (shape_dist_x2 > -2) {
          int coverage = 255 - (shape_dist_x2 + 2) * 255 / 4;
          if (coverage < 0)
            coverage = 0;
          if (coverage > 255)
            coverage = 255;
          cur_a = (cur_a * coverage) >> 8;
        }
      } else {
        int dist_to_l = cx;
        int dist_to_r = w - 1 - cx;
        int dist_to_t = cy;
        int dist_to_b = h - 1 - cy;
        int min_x = dist_to_l < dist_to_r ? dist_to_l : dist_to_r;
        int min_y = dist_to_t < dist_to_b ? dist_to_t : dist_to_b;
        int min_dist = min_x < min_y ? min_x : min_y;

        if (min_dist < border_thickness) {
          cur_a = bd_a;
          target_rb = bd_rb;
          target_g = bd_g;
        } else {
          cur_a = bg_a;
          target_rb = bg_rb;
          target_g = bg_g;
        }
      }

      if (cur_a == 0)
        continue;

      uint32_t dst = line[px];
      if (cur_a == 255) {
        line[px] = 0xFF000000 | target_rb | target_g;
      } else {
        uint32_t d_rb = dst & 0xFF00FF;
        uint32_t d_g = dst & 0x00FF00;
        uint32_t rb = (((target_rb - d_rb) * cur_a) >> 8) + d_rb;
        uint32_t g = (((target_g - d_g) * cur_a) >> 8) + d_g;
        line[px] = 0xFF000000 | (rb & 0xFF00FF) | (g & 0x00FF00);
      }
    }
  }
}

void vga_draw_rect_blend_lfb(int x, int y, int w, int h, uint32_t color,
                             uint32_t *buffer, int radius) {
  vga_draw_rect_blend_lfb_ex(x, y, w, h, color, 0, 0, buffer, radius);
}

uint32_t vga_apply_color_filter(uint32_t color, int filter, int intensity) {
  if (filter == 0 || intensity <= 0)
    return color; // None or 0 intensity

  if (intensity > 255)
    intensity = 255;

  uint8_t a = (color >> 24) & 0xFF;
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;

  uint8_t target_r = r, target_g = g, target_b = b;

  if (filter == 1) { // Red
    target_r = (r + 255) / 2;
    target_g = g / 2;
    target_b = b / 2;
  } else if (filter == 2) { // Green
    target_r = r / 2;
    target_g = (g + 255) / 2;
    target_b = b / 2;
  } else if (filter == 3) { // Yellow
    target_r = (r + 255) / 2;
    target_g = (g + 255) / 2;
    target_b = b / 4;
  }

  // Linear interpolation: res = original + (target - original) * intensity /
  // 255
  r = (uint8_t)(r + ((int)(target_r - r) * intensity) / 255);
  g = (uint8_t)(g + ((int)(target_g - g) * intensity) / 255);
  b = (uint8_t)(b + ((int)(target_b - b) * intensity) / 255);

  return (a << 24) | (r << 16) | (g << 8) | b;
}
