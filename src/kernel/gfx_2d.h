#ifndef GFX_2D_H
#define GFX_2D_H

#include <stddef.h>
#include <stdint.h>


// 2D Hardware Abstraction Layer (HAL)
// This API provides a central point for accelerated 2D graphics.
// Currently backed by SIMD CPU operations, ready for future hardware GPU
// offloading.

// Pitch values are in pixels (uint32_t counts).

void gfx_fill_rect(uint32_t *dest, uint32_t color, int width, int height,
                   int dest_pitch);

void gfx_blit_rect(uint32_t *dest, const uint32_t *src, int width, int height,
                   int dest_pitch, int src_pitch);

void gfx_blend_rect(uint32_t *dest, const uint32_t *src, uint8_t global_alpha,
                    int width, int height, int dest_pitch, int src_pitch);

#endif
