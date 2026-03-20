#include "blitter.h"

void blit_fill_32(uint32_t *dst, uint32_t color, int count) {
  if (!dst || count <= 0)
    return;
  // Use rep stosd for fast fill
  __asm__ volatile("rep stosl"
                   : "+D"(dst), "+c"(count)
                   : "a"(color)
                   : "memory");
}

void blit_copy_32(uint32_t *dst, const uint32_t *src, int count) {
  if (!dst || !src || count <= 0)
    return;
  // Use rep movsd for fast copy
  __asm__ volatile("rep movsl"
                   : "+D"(dst), "+S"(src), "+c"(count)
                   :
                   : "memory");
}

void blit_blend_row(uint32_t *dst, const uint32_t *src, int count,
                    uint8_t alpha) {
  if (!dst || !src || count <= 0 || alpha == 0)
    return;

  if (alpha == 255) {
    blit_copy_32(dst, src, count);
    return;
  }

  uint32_t inv_alpha = 255 - alpha;
  for (int i = 0; i < count; i++) {
    uint32_t s = src[i];
    uint32_t d = dst[i];

    uint32_t sr = (s >> 16) & 0xFF;
    uint32_t sg = (s >> 8) & 0xFF;
    uint32_t sb = s & 0xFF;

    uint32_t dr = (d >> 16) & 0xFF;
    uint32_t dg = (d >> 8) & 0xFF;
    uint32_t db = d & 0xFF;

    uint32_t r = (sr * alpha + dr * inv_alpha) >> 8;
    uint32_t g = (sg * alpha + dg * inv_alpha) >> 8;
    uint32_t b = (sb * alpha + db * inv_alpha) >> 8;

    dst[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
  }
}

void blit_fill_rect_32(uint32_t *dst, int dst_stride, int x, int y, int w,
                       int h, uint32_t color) {
  if (!dst || w <= 0 || h <= 0)
    return;
  for (int row = 0; row < h; row++) {
    blit_fill_32(&dst[(y + row) * dst_stride + x], color, w);
  }
}

void blit_copy_rect_32(uint32_t *dst, int dst_stride, const uint32_t *src,
                       int src_stride, int dx, int dy, int sx, int sy, int w,
                       int h) {
  if (!dst || !src || w <= 0 || h <= 0)
    return;
  for (int row = 0; row < h; row++) {
    blit_copy_32(&dst[(dy + row) * dst_stride + dx],
                 &src[(sy + row) * src_stride + sx], w);
  }
}

void blit_blend_rect_32(uint32_t *dst, int dst_stride, const uint32_t *src,
                        int src_stride, int dx, int dy, int sx, int sy, int w,
                        int h, uint8_t alpha) {
  if (!dst || !src || w <= 0 || h <= 0 || alpha == 0)
    return;
  if (alpha == 255) {
    blit_copy_rect_32(dst, dst_stride, src, src_stride, dx, dy, sx, sy, w, h);
    return;
  }
  for (int row = 0; row < h; row++) {
    blit_blend_row(&dst[(dy + row) * dst_stride + dx],
                   &src[(sy + row) * src_stride + sx], w, alpha);
  }
}
