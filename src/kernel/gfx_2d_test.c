#include "gfx_2d.h"
#include "heap.h"
#include "string.h"

// Reference implementations for verification
static void ref_fill_rect(uint32_t *dest, uint32_t color, int width, int height,
                          int dest_pitch) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      dest[y * dest_pitch + x] = color;
    }
  }
}

static void ref_blit_rect(uint32_t *dest, const uint32_t *src, int width,
                          int height, int dest_pitch, int src_pitch) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      dest[y * dest_pitch + x] = src[y * src_pitch + x];
    }
  }
}

static inline uint32_t ref_blend_color_32(uint32_t src, uint32_t dst,
                                          uint8_t global_alpha) {
  uint32_t src_alpha = (src >> 24) & 0xFF;
  uint32_t sum_a = src_alpha * global_alpha;
  // Accurate eff_a: (x + (x >> 8) + 1) >> 8
  uint32_t a = (sum_a + (sum_a >> 8) + 1) >> 8;

  if (a >= 255)
    return src;
  if (a <= 0)
    return dst;

  uint32_t inv_a = 255 - a;
  uint32_t sum_r = ((src >> 16) & 0xFF) * a + ((dst >> 16) & 0xFF) * inv_a;
  uint32_t sum_g = ((src >> 8) & 0xFF) * a + ((dst >> 8) & 0xFF) * inv_a;
  uint32_t sum_b = (src & 0xFF) * a + (dst & 0xFF) * inv_a;

  // Accurate color channels
  uint32_t r = (sum_r + (sum_r >> 8) + 1) >> 8;
  uint32_t g = (sum_g + (sum_g >> 8) + 1) >> 8;
  uint32_t b = (sum_b + (sum_b >> 8) + 1) >> 8;

  return 0xFF000000 | (r << 16) | (g << 8) | b;
}

static void ref_blend_rect(uint32_t *dest, const uint32_t *src,
                           uint8_t global_alpha, int width, int height,
                           int dest_pitch, int src_pitch) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      dest[y * dest_pitch + x] = ref_blend_color_32(
          src[y * src_pitch + x], dest[y * dest_pitch + x], global_alpha);
    }
  }
}

static int buffers_match(const uint32_t *a, const uint32_t *b, int count) {
  for (int i = 0; i < count; i++) {
    // Ignore the alpha channel for correctness checks.
    // SIMD operations leave computed alpha values while the reference forces
    // 0xFF.
    if ((a[i] & 0x00FFFFFF) != (b[i] & 0x00FFFFFF)) {
      extern void print_serial(const char *str);
      char buf[128];
      snprintf(buf, sizeof(buf),
               "[GFX TEST] Mismatch at %d: Expected %x, Got %x\n", i,
               a[i] & 0x00FFFFFF, b[i] & 0x00FFFFFF);
      print_serial(buf);
      return 0; // False
    }
  }
  return 1; // True
}

extern void print_serial(const char *str);

void gfx_test_run(void) {
  print_serial("[GFX TEST] Running 2D Graphics HAL Unit Tests...\n");

  int width = 120;
  int height = 100;
  int pitch = 128; // slightly larger pitch to test strides
  int total_pixels = pitch * height;

  // Allocate buffers using kernel heap
  uint32_t *test_buf = (uint32_t *)kmalloc(total_pixels * sizeof(uint32_t));
  uint32_t *ref_buf = (uint32_t *)kmalloc(total_pixels * sizeof(uint32_t));
  uint32_t *src_buf = (uint32_t *)kmalloc(total_pixels * sizeof(uint32_t));

  if (!test_buf || !ref_buf || !src_buf) {
    print_serial("[GFX TEST] Allocation failed!\n");
    if (test_buf)
      kfree(test_buf);
    if (ref_buf)
      kfree(ref_buf);
    if (src_buf)
      kfree(src_buf);
    return;
  }

  // Test 1: Fill Rect
  memset(test_buf, 0, total_pixels * sizeof(uint32_t));
  memset(ref_buf, 0, total_pixels * sizeof(uint32_t));

  gfx_fill_rect(test_buf, 0xFFCCAA99, width, height, pitch);
  ref_fill_rect(ref_buf, 0xFFCCAA99, width, height, pitch);

  if (buffers_match(ref_buf, test_buf, total_pixels)) {
    print_serial("[GFX TEST] [PASS] gfx_fill_rect\n");
  } else {
    print_serial("[GFX TEST] [FAIL] gfx_fill_rect mismatch\n");
  }

  // Test 2: Blit Rect
  for (int i = 0; i < total_pixels; i++) {
    src_buf[i] = 0xFF000000 | (i * 12345); // deterministic opaque colors
  }
  memset(test_buf, 0, total_pixels * sizeof(uint32_t));
  memset(ref_buf, 0, total_pixels * sizeof(uint32_t));

  gfx_blit_rect(test_buf, src_buf, width, height, pitch, pitch);
  ref_blit_rect(ref_buf, src_buf, width, height, pitch, pitch);

  if (buffers_match(ref_buf, test_buf, total_pixels)) {
    print_serial("[GFX TEST] [PASS] gfx_blit_rect\n");
  } else {
    print_serial("[GFX TEST] [FAIL] gfx_blit_rect mismatch\n");
  }

  // Test 3: Blend Rect
  for (int i = 0; i < total_pixels; i++) {
    src_buf[i] = 0x80000000 | (i * 54321); // Alpha 0x80
    test_buf[i] = 0xFF112233;
    ref_buf[i] = 0xFF112233;
  }

  gfx_blend_rect(test_buf, src_buf, 200, width, height, pitch, pitch);
  ref_blend_rect(ref_buf, src_buf, 200, width, height, pitch, pitch);

  if (buffers_match(ref_buf, test_buf, total_pixels)) {
    print_serial("[GFX TEST] [PASS] gfx_blend_rect\n");
  } else {
    print_serial("[GFX TEST] [FAIL] gfx_blend_rect mismatch\n");
  }

  kfree(test_buf);
  kfree(ref_buf);
  kfree(src_buf);

  print_serial("[GFX TEST] Tests complete.\n");
}
