#include "gfx_2d.h"
#include "simd.h"

void gfx_fill_rect(uint32_t *dest, uint32_t color, int width, int height,
                   int dest_pitch) {
  if (width <= 0 || height <= 0 || !dest)
    return;
  for (int row = 0; row < height; row++) {
    simd_fill_32(dest + row * dest_pitch, color, width);
  }
}

void gfx_blit_rect(uint32_t *dest, const uint32_t *src, int width, int height,
                   int dest_pitch, int src_pitch) {
  if (width <= 0 || height <= 0 || !dest || !src)
    return;
  // Safety: clamp width to source pitch
  if (width > src_pitch) width = src_pitch;
  
  for (int row = 0; row < height; row++) {
    simd_memcpy_32(dest + row * dest_pitch, src + row * src_pitch, width);
  }
}

void gfx_blend_rect(uint32_t *dest, const uint32_t *src, uint8_t global_alpha,
                    int width, int height, int dest_pitch, int src_pitch) {
  if (width <= 0 || height <= 0 || !dest || !src)
    return;
  // Safety: clamp width to source pitch to prevent OOB reads if metadata is stale
  if (width > src_pitch) width = src_pitch;
  
  for (int row = 0; row < height; row++) {
    simd_blend_32(dest + row * dest_pitch, src + row * src_pitch, global_alpha,
                  width);
  }
}
