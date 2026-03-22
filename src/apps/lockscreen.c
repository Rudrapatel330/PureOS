/*
 * lockscreen.c - PureOS Lock Screen (32-bit ARGB optimized)
 * Password: 123456
 */
#include "../drivers/bga.h"
#include "../drivers/keyboard.h"
#include "../kernel/config.h"
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
      if (dx * dx + dy * dy <= r * r) {
        int px = cx + dx;
        int py = cy + dy;
        if (px >= 0 && px < screen_width && py >= 0 && py < screen_height)
          buf[py * screen_width + px] = color;
      }
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

  // PRE-COPY background to all 3 BGA buffers (triple buffering) to avoid
  // flicker
  for (int i = 0; i < 3; i++) {
    uint32_t *vram = bga_get_render_buffer();
    if (vram) {
      print_serial("LS: Pre-copying background to VRAM page...\n");
      memcpy(vram, backbuffer, screen_width * screen_height * 4);
      bga_flip();
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

  while (1) {
    /* Use BGA hardware acceleration: Get the inactive VRAM page */
    uint32_t *ls_buffer = bga_get_render_buffer();
    if (!ls_buffer)
      ls_buffer = real_lfb;

    int center_x = screen_width / 2;
    int card_y = screen_height / 2 - 50;

    // Erase UI area (Frosted rect and Avatar)
    int erase_y = card_y - 100;
    int erase_h = 250;
    for (int j = 0; j < erase_h; j++) {
      int py = erase_y + j;
      if (py >= 0 && py < screen_height) {
        memcpy(&ls_buffer[py * screen_width + (center_x - 150)],
               &backbuffer[py * screen_width + (center_x - 150)], 300 * 4);
      }
    }

    /* Avatar */
    ls_fill_circle(ls_buffer, center_x, card_y - 40, 40, 0xFFE06C75);
    ls_draw_text(ls_buffer, center_x - 12, card_y - 56, "R", 0xFF1E2228, 4);

    /* User name */
    ls_draw_text(ls_buffer, center_x - 30, card_y + 16, "rudra", theme_get()->fg, 2);

    /* Password Card */
    int cw = 280, ch = 44;
    int cx = center_x - cw / 2, cy = card_y + 48;
    ls_frosted_rect(ls_buffer, cx, cy, cw, ch, 30, 34, 40, 160);
    ls_draw_rect(ls_buffer, cx, cy, cw, ch, wrong ? 0xFFFF6464 : theme_get()->accent);

    /* Dots */
    for (int i = 0; i < pw_len; i++) {
      ls_fill_circle(ls_buffer, cx + 24 + i * 16, cy + 22, 5, theme_get()->fg);
    }

    /* Cursor */
    tick_count++;
    if ((tick_count / 20) % 2 == 0) {
      ls_fill_rect(ls_buffer, cx + 24 + pw_len * 16, cy + 12, 2, 20,
                   theme_get()->accent);
    }

    if (pw_len == 0)
      ls_draw_text(ls_buffer, cx + 20, cy + 16, "Enter password...", 0xFF969696,
                   1);
    if (wrong)
      ls_draw_text(ls_buffer, center_x - 80, cy + 56, "Wrong password!",
                   0xFFFF6464, 1);

    /* Branding */
    ls_draw_text(ls_buffer, center_x - 32, screen_height - 40, "PureOS",
                 0xFFB4B4B4, 1);

    /* Hardware Flip: Instant switch */
    bga_flip();

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
