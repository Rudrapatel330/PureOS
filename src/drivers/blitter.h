#ifndef BLITTER_H
#define BLITTER_H

#include <stdint.h>

// Fast 32-bit memory fill (uses rep stosd)
void blit_fill_32(uint32_t *dst, uint32_t color, int count);

// Fast 32-bit memory copy (uses rep movsd)
void blit_copy_32(uint32_t *dst, const uint32_t *src, int count);

// Alpha blend a row of pixels (no SIMD, but loop-optimized)
void blit_blend_row(uint32_t *dst, const uint32_t *src, int count,
                    uint8_t alpha);

// 2D Rect Operations
void blit_fill_rect_32(uint32_t *dst, int dst_stride, int x, int y, int w,
                       int h, uint32_t color);
void blit_copy_rect_32(uint32_t *dst, int dst_stride, const uint32_t *src,
                       int src_stride, int dx, int dy, int sx, int sy, int w,
                       int h);
void blit_blend_rect_32(uint32_t *dst, int dst_stride, const uint32_t *src,
                        int src_stride, int dx, int dy, int sx, int sy, int w,
                        int h, uint8_t alpha);

#endif
