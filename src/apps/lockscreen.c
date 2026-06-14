#include "../kernel/ui_layout.h"
/*
 * lockscreen.c - PureOS Lock Screen (32-bit ARGB optimized)
 * Password: 123456
 */
#include "../kernel/hal/gfx_device.h"
#include "../drivers/keyboard.h"
#include "../fs/fs.h"
#include "../kernel/config.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#define STBI_NO_STDIO
#include "../kernel/image.h"
#include "../kernel/theme.h"
#include <stdint.h>

/* External globals from kernel.c */
extern uint32_t *real_lfb;
extern uint32_t *backbuffer;
extern int screen_width;
extern int screen_height;
int lockscreen_active = 0;

extern os_config_t global_config;

extern const unsigned char wallpaper_bmp_data[];
extern const unsigned int wallpaper_bmp_size;
extern const unsigned char wallpaper_png_data[];
extern const unsigned int wallpaper_png_size;

/* Serial debug */
extern void print_serial(const char *s);
extern void sleep(int ticks);
extern void compositor_blur_rect(uint32_t *buffer, int x, int y, int w, int h, int radius);

/* ======================== COLOR HELPERS ======================== */

static uint32_t ls_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) |
         (uint32_t)b;
}

static void ls_from_argb(uint32_t c, uint8_t *a, uint8_t *r, uint8_t *g,
                         uint8_t *b) {
  *a = (c >> 24) & 0xFF;
  *r = (c >> 16) & 0xFF;
  *g = (c >> 8) & 0xFF;
  *b = c & 0xFF;
}

/* ======================== DRAWING HELPERS ======================== */

static void ls_fill_rect(uint32_t *buf, int x, int y, int w, int h,
                         uint32_t color) {
  if (!buf)
    return;
  for (int j = 0; j < h; j++) {
    int py = y + j;
    if (py < 0 || py >= screen_height)
      continue;
    for (int i = 0; i < w; i++) {
      int px = x + i;
      if (px < 0 || px >= screen_width)
        continue;
      buf[py * screen_width + px] = color;
    }
  }
}

static void ls_draw_rect(uint32_t *buf, int x, int y, int w, int h,
                         uint32_t color) {
  if (!buf)
    return;
  for (int i = 0; i < w; i++) {
    if (x + i >= 0 && x + i < screen_width) {
      if (y >= 0 && y < screen_height)
        buf[y * screen_width + x + i] = color;
      if (y + h - 1 >= 0 && y + h - 1 < screen_height)
        buf[(y + h - 1) * screen_width + x + i] = color;
    }
  }
  for (int j = 0; j < h; j++) {
    if (y + j >= 0 && y + j < screen_height) {
      if (x >= 0 && x < screen_width)
        buf[(y + j) * screen_width + x] = color;
      if (x + w - 1 >= 0 && x + w - 1 < screen_width)
        buf[(y + j) * screen_width + x + w - 1] = color;
    }
  }
}

static void ls_frosted_rect(uint32_t *buf, int x, int y, int w, int h,
                            uint8_t tr, uint8_t tg, uint8_t tb, int alpha) {
  if (!buf)
    return;
  for (int j = 0; j < h; j++) {
    int py = y + j;
    if (py < 0 || py >= screen_height)
      continue;
    for (int i = 0; i < w; i++) {
      int px = x + i;
      if (px < 0 || px >= screen_width)
        continue;
      uint32_t bg = buf[py * screen_width + px];
      uint8_t ba, br, bgg, bb;
      ls_from_argb(bg, &ba, &br, &bgg, &bb);

      uint32_t nr = (tr * alpha + br * (255 - alpha)) >> 8;
      uint32_t ng = (tg * alpha + bgg * (255 - alpha)) >> 8;
      uint32_t nb = (tb * alpha + bb * (255 - alpha)) >> 8;

      buf[py * screen_width + px] = 0xFF000000 | (nr << 16) | (ng << 8) | nb;
    }
  }
}

static void ls_draw_text(uint32_t *buf, int x, int y, const char *text,
                         uint32_t color, int scale) {
  if (!buf)
    return;
  extern const uint8_t font8x8_basic[256][8];
  while (*text) {
    uint8_t c = (uint8_t)*text;
    for (int row = 0; row < 8; row++) {
      uint8_t bits = font8x8_basic[c][row];
      for (int col = 0; col < 8; col++) {
        if (bits & (0x80 >> col)) {
          for (int sy = 0; sy < scale; sy++) {
            for (int sx = 0; sx < scale; sx++) {
              int px = x + col * scale + sx;
              int py = y + row * scale + sy;
              if (px >= 0 && px < screen_width && py >= 0 && py < screen_height)
                buf[py * screen_width + px] = color;
            }
          }
        }
      }
    }
    x += 8 * scale;
    text++;
  }
}

static void ls_fill_circle(uint32_t *buf, int cx, int cy, int r,
                           uint32_t color) {
  if (!buf)
    return;
  for (int dy = -r; dy <= r; dy++) {
    for (int dx = -r; dx <= r; dx++) {
      int d2 = dx * dx + dy * dy;
      if (d2 <= r * r) {
        int px = cx + dx;
        int py = cy + dy;
        if (px >= 0 && px < screen_width && py >= 0 && py < screen_height) {
          uint32_t final_col = color;
          // Simple AA for the circle edge
          if (d2 > (r - 1) * (r - 1)) {
            uint32_t bg = buf[py * screen_width + px];
            uint8_t ba, br, bgg, bb;
            ls_from_argb(bg, &ba, &br, &bgg, &bb);
            uint8_t tr = (color >> 16) & 0xFF;
            uint8_t tg = (color >> 8) & 0xFF;
            uint8_t tb = color & 0xFF;
            int alpha = (r * r - d2) * 255 / (2 * r - 1);
            if (alpha < 0) alpha = 0;
            if (alpha > 255) alpha = 255;
            uint32_t nr = (tr * alpha + br * (255 - alpha)) >> 8;
            uint32_t ng = (tg * alpha + bgg * (255 - alpha)) >> 8;
            uint32_t nb = (tb * alpha + bb * (255 - alpha)) >> 8;
            final_col = 0xFF000000 | (nr << 16) | (ng << 8) | nb;
          }
          buf[py * screen_width + px] = final_col;
        }
      }
    }
  }
}

static void ls_draw_rounded_frosted_rect(uint32_t *buf, int x, int y, int w, int h, int r, uint8_t tr, uint8_t tg, uint8_t tb, int alpha) {
  if (!buf) return;
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int px = x + i;
      int py = y + j;
      if (px < 0 || px >= screen_width || py < 0 || py >= screen_height) continue;

      // High-quality sub-pixel coverage (4x4 sampling)
      int coverage = 0;
      for (int sy = 0; sy < 4; sy++) {
        for (int sx = 0; sx < 4; sx++) {
          float fx = (float)i + (sx + 0.5f) / 4.0f;
          float fy = (float)j + (sy + 0.5f) / 4.0f;
          
          int in_rect = 1;
          if (fx < r && fy < r) { // TL
            float dx = (float)r - fx, dy = (float)r - fy;
            if (dx*dx + dy*dy > (float)r*r) in_rect = 0;
          } else if (fx >= (float)w - r && fy < r) { // TR
            float dx = fx - ((float)w - r), dy = (float)r - fy;
            if (dx*dx + dy*dy > (float)r*r) in_rect = 0;
          } else if (fx < r && fy >= (float)h - r) { // BL
            float dx = (float)r - fx, dy = fy - ((float)h - r);
            if (dx*dx + dy*dy > (float)r*r) in_rect = 0;
          } else if (fx >= (float)w - r && fy >= (float)h - r) { // BR
            float dx = fx - ((float)w - r), dy = fy - ((float)h - r);
            if (dx*dx + dy*dy > (float)r*r) in_rect = 0;
          }
          if (in_rect) coverage++;
        }
      }

      if (coverage == 0) continue;
      int cur_alpha = (alpha * coverage) >> 4;

      uint32_t bg = buf[py * screen_width + px];
      uint8_t ba, br, bgg, bb;
      ls_from_argb(bg, &ba, &br, &bgg, &bb);

      uint32_t nr = (tr * cur_alpha + br * (255 - cur_alpha)) >> 8;
      uint32_t ng = (tg * cur_alpha + bgg * (255 - cur_alpha)) >> 8;
      uint32_t nb = (tb * cur_alpha + bb * (255 - cur_alpha)) >> 8;

      buf[py * screen_width + px] = 0xFF000000 | (nr << 16) | (ng << 8) | nb;
    }
  }
}

static void ls_draw_rounded_rect_outline(uint32_t *buf, int x, int y, int w, int h, int r, uint32_t color) {
  if (!buf) return;
  uint8_t tr = (color >> 16) & 0xFF;
  uint8_t tg = (color >> 8) & 0xFF;
  uint8_t tb = color & 0xFF;

  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int px = x + i;
      int py = y + j;
      if (px < 0 || px >= screen_width || py < 0 || py >= screen_height) continue;

      int coverage = 0;
      float inner_r = (float)r - 1.5f;
      float outer_r = (float)r + 0.5f;

      for (int sy = 0; sy < 4; sy++) {
        for (int sx = 0; sx < 4; sx++) {
          float fx = (float)i + (sx + 0.5f) / 4.0f;
          float fy = (float)j + (sy + 0.5f) / 4.0f;
          
          int is_edge = 0;
          if (fx < r && fy < r) { // TL
            float dx = (float)r - fx, dy = (float)r - fy;
            float d2 = dx*dx + dy*dy;
            if (d2 <= outer_r*outer_r && d2 > inner_r*inner_r) is_edge = 1;
          } else if (fx >= (float)w - r && fy < r) { // TR
            float dx = fx - ((float)w - r), dy = (float)r - fy;
            float d2 = dx*dx + dy*dy;
            if (d2 <= outer_r*outer_r && d2 > inner_r*inner_r) is_edge = 1;
          } else if (fx < r && fy >= (float)h - r) { // BL
            float dx = (float)r - fx, dy = fy - ((float)h - r);
            float d2 = dx*dx + dy*dy;
            if (d2 <= outer_r*outer_r && d2 > inner_r*inner_r) is_edge = 1;
          } else if (fx >= (float)w - r && fy >= (float)h - r) { // BR
            float dx = fx - ((float)w - r), dy = fy - ((float)h - r);
            float d2 = dx*dx + dy*dy;
            if (d2 <= outer_r*outer_r && d2 > inner_r*inner_r) is_edge = 1;
          } else {
            if (fx < 1.0f || fx >= (float)w - 1.0f || fy < 1.0f || fy >= (float)h - 1.0f) is_edge = 1;
          }
          if (is_edge) coverage++;
        }
      }

      if (coverage > 0) {
        uint32_t bg = buf[py * screen_width + px];
        uint8_t ba, br, bgg, bb;
        ls_from_argb(bg, &ba, &br, &bgg, &bb);
        int alpha = (coverage * 255) >> 4;
        uint32_t nr = (tr * alpha + br * (255 - alpha)) >> 8;
        uint32_t ng = (tg * alpha + bgg * (255 - alpha)) >> 8;
        uint32_t nb = (tb * alpha + bb * (255 - alpha)) >> 8;
        buf[py * screen_width + px] = 0xFF000000 | (nr << 16) | (ng << 8) | nb;
      }
    }
  }
}

/* ======================== LIQUID GLASS CARD ======================== */

static void ls_draw_liquid_glass_card(uint32_t *buf, int x, int y, int w, int h, int r) {
  if (!buf) return;

  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int px = x + i;
      int py = y + j;
      if (px < 0 || px >= screen_width || py < 0 || py >= screen_height) continue;

      int coverage = 0;

      for (int sy = 0; sy < 4; sy++) {
        for (int sx = 0; sx < 4; sx++) {
          float fx = (float)i + (sx + 0.5f) / 4.0f;
          float fy = (float)j + (sy + 0.5f) / 4.0f;
          int in = 1;

          if (fx < r && fy < r) {
            float dx = (float)r - fx, dy = (float)r - fy;
            if (dx*dx + dy*dy > (float)(r*r)) in = 0;
          } else if (fx >= (float)w - r && fy < r) {
            float dx = fx - ((float)w - r), dy = (float)r - fy;
            if (dx*dx + dy*dy > (float)(r*r)) in = 0;
          } else if (fx < r && fy >= (float)h - r) {
            float dx = (float)r - fx, dy = fy - ((float)h - r);
            if (dx*dx + dy*dy > (float)(r*r)) in = 0;
          } else if (fx >= (float)w - r && fy >= (float)h - r) {
            float dx = fx - ((float)w - r), dy = fy - ((float)h - r);
            if (dx*dx + dy*dy > (float)(r*r)) in = 0;
          }
          if (in) coverage++;
        }
      }
      if (coverage == 0) continue;

      // Base frost alpha: exactly 28% opacity white (71/255) for modern iOS flat glass
      int cur_alpha = (71 * coverage) >> 4;

      uint32_t bg = buf[py * screen_width + px];
      uint8_t ba, br, bgg, bb;
      ls_from_argb(bg, &ba, &br, &bgg, &bb);

      // Vibrancy / Color Saturation Boost (iOS style)
      // Increase saturation to mimic light gathering through thick frosted glass
      int avg = ((int)br + (int)bgg + (int)bb) / 3;
      int vr = avg + ((int)br - avg) * 16 / 10;
      int vg = avg + ((int)bgg - avg) * 16 / 10;
      int vb = avg + ((int)bb - avg) * 16 / 10;
      
      // Slight brightness bump (+15)
      vr += 15; vg += 15; vb += 15;

      // Clamp Vibrancy
      if (vr > 255) vr = 255; else if (vr < 0) vr = 0;
      if (vg > 255) vg = 255; else if (vg < 0) vg = 0;
      if (vb > 255) vb = 255; else if (vb < 0) vb = 0;

      // Flat Frost Tint Overlay (Perfectly uniform white)
      uint32_t nr = (255 * cur_alpha + vr * (255 - cur_alpha)) >> 8;
      uint32_t ng = (255 * cur_alpha + vg * (255 - cur_alpha)) >> 8;
      uint32_t nb = (255 * cur_alpha + vb * (255 - cur_alpha)) >> 8;

      buf[py * screen_width + px] = 0xFF000000 | (nr << 16) | (ng << 8) | nb;
    }
  }
}

/* ======================== BLUR & RENDER ======================== */

static void ls_fast_blur(uint32_t *buf) {
  if (!buf)
    return;
  // Simple darkening to simulate blur/focus shift
  for (int i = 0; i < screen_width * screen_height; i++) {
    uint8_t a, r, g, b;
    ls_from_argb(buf[i], &a, &r, &g, &b);
    buf[i] = (a << 24) | ((r >> 1) << 16) | ((g >> 1) << 8) | (b >> 1);
  }
}

static void ls_render_wallpaper(uint32_t *buf) {
  if (!buf)
    return;

  print_serial("LS: Attempting to load wallpaper...\n");

  // Try multiple possible paths for the wallpaper
  // Note: bulk_upload_icons.py uploads to the root of the data disk
  const char *paths[] = {"/WALL4.JPG", "WALL4.JPG", "/WALLPAPER/WALL4.JPG", "/wallpaper/wall4.jpg", "wall4.jpg"};
  file_entry_t *fe = NULL;
  for (int i = 0; i < 5; i++) {
    fe = fs_find(paths[i]);
    if (fe) {
      print_serial("LS: Found wallpaper at ");
      print_serial(paths[i]);
      print_serial("\n");
      break;
    }
  }
  
  if (fe) {
    print_serial("LS: Allocating buffer for raw data...\n");
    uint8_t *raw = (uint8_t *)kmalloc(fe->size);
    if (raw) {
      if (fs_read(fe->name, raw) > 0) {
        print_serial("LS: Decoding image data...\n");
        int bw, bh, channels;
        unsigned char *pixels = stbi_load_from_memory(raw, fe->size, &bw, &bh, &channels, 4);
        if (pixels) {
          print_serial("LS: Image decoded successfully. Rendering...\n");
          for (int sy = 0; sy < screen_height; sy++) {
            for (int sx = 0; sx < screen_width; sx++) {
              int gx = (sx * bw << 8) / screen_width;
              int gy = (sy * bh << 8) / screen_height;
              int bx = gx >> 8;
              int by = gy >> 8;
              if (bx >= bw - 1) bx = bw - 2;
              if (by >= bh - 1) by = bh - 2;
              if (bx < 0) bx = 0;
              if (by < 0) by = 0;
              int dx = gx & 0xFF;
              int dy = gy & 0xFF;
              const uint8_t *p00 = pixels + (by * bw + bx) * 4;
              const uint8_t *p10 = p00 + 4;
              const uint8_t *p01 = pixels + ((by + 1) * bw + bx) * 4;
              const uint8_t *p11 = p01 + 4;
              int w00 = (256 - dx) * (256 - dy);
              int w10 = dx * (256 - dy);
              int w01 = (256 - dx) * dy;
              int w11 = dx * dy;
              uint8_t r = (p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11) >> 16;
              uint8_t g = (p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11) >> 16;
              uint8_t b = (p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11) >> 16;
              buf[sy * screen_width + sx] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
          }
          stbi_image_free(pixels);
          kfree(raw);
          print_serial("LS: Wallpaper rendering complete.\n");
          return;
        } else {
          print_serial("LS: stbi_load_from_memory failed.\n");
        }
      } else {
        print_serial("LS: fs_read failed.\n");
      }
      kfree(raw);
    } else {
      print_serial("LS: kmalloc failed for raw data.\n");
    }
  } else {
    print_serial("LS: No custom wallpaper file found.\n");
  }

  if (global_config.wallpaper_type == 3) {
    print_serial("LS: PNG Wallpaper loading...\n");
    int bw, bh, channels;
    unsigned char *pixels = stbi_load_from_memory(
        wallpaper_png_data, wallpaper_png_size, &bw, &bh, &channels, 4);

    if (pixels) {
      print_serial("LS: PNG Decoded. Rendering to backbuffer...\n");
      for (int sy = 0; sy < screen_height; sy++) {
        for (int sx = 0; sx < screen_width; sx++) {
          int gx = (sx * bw << 8) / screen_width;
          int gy = (sy * bh << 8) / screen_height;

          int bx = gx >> 8;
          int by = gy >> 8;

          if (bx >= bw - 1)
            bx = bw - 2;
          if (by >= bh - 1)
            by = bh - 2;
          if (bx < 0)
            bx = 0;
          if (by < 0)
            by = 0;

          int dx = gx & 0xFF;
          int dy = gy & 0xFF;

          const uint8_t *p00 = pixels + (by * bw + bx) * 4;
          const uint8_t *p10 = p00 + 4;
          const uint8_t *p01 = pixels + ((by + 1) * bw + bx) * 4;
          const uint8_t *p11 = p01 + 4;

          int w00 = (256 - dx) * (256 - dy);
          int w10 = dx * (256 - dy);
          int w01 = (256 - dx) * dy;
          int w11 = dx * dy;

          uint8_t r =
              (p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11) >> 16;
          uint8_t g =
              (p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11) >> 16;
          uint8_t b =
              (p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11) >> 16;

          buf[sy * screen_width + sx] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
      stbi_image_free(pixels);
      return;
    }
  }

  const uint8_t *bmp = wallpaper_bmp_data;
  if (bmp && bmp[0] == 'B' && bmp[1] == 'M') {
    uint32_t poff =
        bmp[10] | (bmp[11] << 8) | (bmp[12] << 16) | (bmp[13] << 24);
    int bw = bmp[18] | (bmp[19] << 8) | (bmp[20] << 16) | (bmp[21] << 24);
    int bh = bmp[22] | (bmp[23] << 8) | (bmp[24] << 16) | (bmp[25] << 24);
    int bpp = bmp[28] | (bmp[29] << 8);

    if (bpp == 24 && bw > 0 && bh > 0) {
      int row_stride = ((bw * 3 + 3) & ~3);
      const uint8_t *pixels = bmp + poff;
      for (int sy = 0; sy < screen_height; sy++) {
        for (int sx = 0; sx < screen_width; sx++) {
          int gx = (sx * bw << 8) / screen_width;
          int gy = (sy * bh << 8) / screen_height;

          int bx = gx >> 8;
          int by = bh - 1 - (gy >> 8);

          if (bx >= bw - 1)
            bx = bw - 2;
          if (by <= 0)
            by = 1;

          int dx = gx & 0xFF;
          int dy = gy & 0xFF;

          const uint8_t *p00 = pixels + by * row_stride + bx * 3;
          const uint8_t *p10 = p00 + 3;
          const uint8_t *p01 = pixels + (by - 1) * row_stride + bx * 3;
          const uint8_t *p11 = p01 + 3;

          int w00 = (256 - dx) * (256 - dy);
          int w10 = dx * (256 - dy);
          int w01 = (256 - dx) * dy;
          int w11 = dx * dy;

          uint8_t b =
              (p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11) >> 16;
          uint8_t g =
              (p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11) >> 16;
          uint8_t r =
              (p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11) >> 16;

          buf[sy * screen_width + sx] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
      return;
    }
  }

  /* Fallback gradient */
  for (int y = 0; y < screen_height; y++) {
    uint32_t c = ls_argb(255, 0, (uint8_t)(120 - y * 80 / screen_height),
                         (uint8_t)(215 - y * 80 / screen_height));
    for (int x = 0; x < screen_width; x++)
      buf[y * screen_width + x] = c;
  }
}

/* ======================== MAIN LOCK SCREEN ======================== */

/* PS/2 keyboard port helpers */
static inline uint8_t ls_inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

/* Scancode tables (US QWERTY) */
static const char ls_scancode_lower[] = {
    0,   27,   '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0};

static const char ls_scancode_upper[] = {
    0,   27,   '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0};

void lockscreen_show(void) {
  lockscreen_active = 1;

  if (!backbuffer || !real_lfb) {
    return;
  }

  ls_render_wallpaper(backbuffer);
  print_serial("LS: Wallpaper prepared in backbuffer.\n");

  // Apply background blur (Windows-style)
  print_serial("LS: Applying background blur...\n");
  compositor_blur_rect(backbuffer, 0, 0, screen_width, screen_height, 20); // First pass
  compositor_blur_rect(backbuffer, 0, 0, screen_width, screen_height, 20); // Second pass for deeper blur

  // PRE-COPY background to all 3 BGA buffers (triple buffering) to avoid
  // flicker
  for (int i = 0; i < 3; i++) {
    uint32_t *vram = gfx_device_get_render_buffer();
    if (vram) {
      print_serial("LS: Pre-copying background to VRAM page...\n");
      memcpy(vram, backbuffer, screen_width * screen_height * 4);
      gfx_device_flip();
    }
  }

  char password[32] = {0};
  int pw_len = 0, wrong = 0, tick_count = 0;

  /* Local shift state for direct polling */
  int ls_shift = 0;

  /* Flush any pending scancodes from the PS/2 controller */
  while (ls_inb(0x64) & 0x01) {
    ls_inb(0x60);
  }

  keyboard_reset_buffer();
  print_serial("LS: Entering input loop...\n");

  /* Load logo with multiple path attempts */
  uint32_t *logo_pixels = NULL;
  int lw = 0, lh = 0;
  const char *lpaths[] = {"/ICONS/LOGO.PNG", "LOGO.PNG", "/LOGO.PNG", "/icons/logo.png", "logo.png"};
  file_entry_t *lfe = NULL;
  for (int i = 0; i < 5; i++) {
    lfe = fs_find(lpaths[i]);
    if (lfe) break;
  }
  
  if (lfe) {
    uint8_t *lraw = (uint8_t *)kmalloc(lfe->size);
    if (lraw) {
      if (fs_read(lfe->name, lraw) > 0) {
        print_serial("LS: Loading logo: ");
        print_serial(lfe->name);
        print_serial("\n");
        int lc;
        logo_pixels = (uint32_t *)stbi_load_from_memory(lraw, lfe->size, &lw, &lh, &lc, 4);
      }
      kfree(lraw);
    }
  }

  uint32_t *ls_frontbuffer = (uint32_t *)kmalloc(screen_width * screen_height * 4);
  if (ls_frontbuffer) {
      memcpy(ls_frontbuffer, backbuffer, screen_width * screen_height * 4);
  }

  while (1) {
    /* Use Hardware acceleration: Get the inactive VRAM page */
    uint32_t *ls_buffer = gfx_device_get_render_buffer();
    if (!ls_buffer)
      ls_buffer = real_lfb;
      
    uint32_t *draw_buf = ls_frontbuffer ? ls_frontbuffer : ls_buffer;

    int center_x = screen_width / 2;
    int center_y = screen_height / 2;
    
    /* Card Layout - Liquid Glass Card containing avatar, name, and password */
    int card_w = 360;
    int card_h = 300;
    int card_r = 24;
    int card_x = center_x - card_w / 2;
    int card_y = center_y - card_h / 2 - 20;

    int erase_x = card_x - 20;
    int erase_y = card_y - 20;
    int erase_w = card_w + 40;
    int erase_h = card_h + 40;
    for (int j = 0; j < erase_h; j++) {
      int py = erase_y + j;
      if (py >= 0 && py < screen_height) {
        memcpy(&draw_buf[py * screen_width + erase_x],
               &backbuffer[py * screen_width + erase_x], erase_w * 4);
      }
    }

    ls_draw_liquid_glass_card(draw_buf, card_x, card_y, card_w, card_h, card_r);

    int avatar_radius = 56;
    int avatar_center_y = card_y + 50 + avatar_radius;

    if (logo_pixels) {
      for (int dy = -avatar_radius; dy <= avatar_radius; dy++) {
        for (int dx = -avatar_radius; dx <= avatar_radius; dx++) {
          int d2 = dx * dx + dy * dy;
          if (d2 <= avatar_radius * avatar_radius) {
            int px = center_x + dx;
            int py = avatar_center_y + dy;
            if (px >= 0 && px < screen_width && py >= 0 && py < screen_height) {
              int sx = (dx + avatar_radius) * lw / (2 * avatar_radius);
              int sy = (dy + avatar_radius) * lh / (2 * avatar_radius);
              if (sx >= 0 && sx < lw && sy >= 0 && sy < lh) {
                uint32_t c = logo_pixels[sy * lw + sx];
                uint8_t lr = c & 0xFF;
                uint8_t lg = (c >> 8) & 0xFF;
                uint8_t lb = (c >> 16) & 0xFF;
                uint8_t la = (c >> 24) & 0xFF;

                if (d2 > (avatar_radius - 1) * (avatar_radius - 1)) {
                  la = (la * (avatar_radius * avatar_radius - d2)) / (2 * avatar_radius - 1);
                }

                if (la == 255) {
                  draw_buf[py * screen_width + px] = 0xFF000000 | (lr << 16) | (lg << 8) | lb;
                } else if (la > 0) {
                  uint32_t bg = draw_buf[py * screen_width + px];
                  uint8_t ba, br, bgg, bb;
                  ls_from_argb(bg, &ba, &br, &bgg, &bb);
                  uint8_t nr = (lr * la + br * (255 - la)) >> 8;
                  uint8_t ng = (lg * la + bgg * (255 - la)) >> 8;
                  uint8_t nb = (lb * la + bb * (255 - la)) >> 8;
                  draw_buf[py * screen_width + px] = 0xFF000000 | (nr << 16) | (ng << 8) | nb;
                }
              }
            }
          }
        }
      }
    } else {
      ls_fill_circle(draw_buf, center_x, avatar_center_y, avatar_radius, 0xFFE06C75);
      ls_draw_text(draw_buf, center_x - 12, avatar_center_y - 16, "R", 0xFF1E2228, 4);
    }

    int name_scale = 2;
    int name_y = avatar_center_y + avatar_radius + 18;
    int name_width = 5 * 8 * name_scale;
    ls_draw_text(draw_buf, center_x - (name_width / 2), name_y, "rudra", theme_get()->fg, name_scale);

    int input_w = 270;
    int input_h = 50;
    int input_r = 16;
    int input_x = center_x - input_w / 2;
    int input_y = name_y + 32;

    ls_draw_rounded_frosted_rect(draw_buf, input_x, input_y, input_w, input_h, input_r, 25, 28, 35, 200);
    ls_draw_rounded_rect_outline(draw_buf, input_x, input_y, input_w, input_h, input_r, wrong ? 0xFFFF6464 : 0xAAFFFFFF);

    for (int i = 0; i < pw_len; i++) {
      ls_fill_circle(draw_buf, input_x + 24 + i * 18, input_y + input_h / 2, 6, theme_get()->fg);
    }

    tick_count++;
    if ((tick_count / 20) % 2 == 0) {
      ls_fill_rect(draw_buf, input_x + 24 + pw_len * 18, input_y + input_h / 2 - 12, 2, 24, theme_get()->accent);
    }

    if (pw_len == 0)
      ls_draw_text(draw_buf, input_x + 20, input_y + input_h / 2 - 8, "Enter password...", 0xFF969696, 1);
    if (wrong)
      ls_draw_text(draw_buf, center_x - 80, input_y + input_h + 12, "Wrong password!", 0xFFFF6464, 1);

    ls_draw_text(draw_buf, center_x - 32, screen_height - 40, "PureOS", 0xFFB4B4B4, 1);

    if (ls_frontbuffer) {
      for (int j = 0; j < erase_h; j++) {
        int py = erase_y + j;
        if (py >= 0 && py < screen_height) {
          memcpy(&ls_buffer[py * screen_width + erase_x],
                 &ls_frontbuffer[py * screen_width + erase_x], erase_w * 4);
        }
      }
      for (int j = 0; j < 20; j++) {
        int py = (screen_height - 40) + j;
        if (py >= 0 && py < screen_height) {
          memcpy(&ls_buffer[py * screen_width + (center_x - 40)],
                 &ls_frontbuffer[py * screen_width + (center_x - 40)], 100 * 4);
        }
      }
    }

    /* Hardware Flip: Instant switch */
    gfx_device_flip();

    /* Ensure interrupts are enabled so keyboard IRQ fires */
    __asm__ volatile("sti");

    /* Read from keyboard buffer (filled by interrupt handler directly) */
    char c;
    while ((c = keyboard_getc()) != 0) {
      /* Debug */
      char debug_buf[2] = {c, 0};
      print_serial("LS KEY: ");
      print_serial(debug_buf);
      print_serial("\n");

      /* Process the character */
      if (c == '\n' || c == '\r') {
        print_serial("LS: Checking password...\n");
        if (strcmp(password, global_config.lock_password) == 0) {
          print_serial("LS: SUCCESS!\n");

          extern void desktop_draw(void);
          desktop_draw(); // Loads clean wallpaper into backbuffer

          extern void compositor_clear_vram();
          compositor_clear_vram(); // Sync clean desktop to ALL 3 pages

          extern void compositor_reset_history();
          compositor_reset_history(); // Clear dirty rects to prevent "ghosting"

          extern void compositor_invalidate_rect(int x, int y, int w, int h);
          compositor_invalidate_rect(0, 0, screen_width, screen_height);

          lockscreen_active = 0;
          if (ls_frontbuffer) kfree(ls_frontbuffer);
          return;
        } else {
          wrong = 1;
          pw_len = 0;
          password[0] = 0;
        }
      } else if (c == '\b') {
        if (pw_len > 0)
          password[--pw_len] = 0;
        wrong = 0;
      } else if (c >= ' ' && c <= '~' && pw_len < 31) {
        password[pw_len++] = c;
        password[pw_len] = 0;
        wrong = 0;
      }
    }

    // Yield briefly — hlt waits for next interrupt
    extern void desktop_process_messages(void);
    desktop_process_messages();
    __asm__ volatile("hlt");
  }
}
