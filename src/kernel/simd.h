#ifndef SIMD_H
#define SIMD_H

#include "window.h"
#include <stddef.h>
#include <stdint.h>

// SIMD-accelerated 32-bit memory fill (count is number of uint32_t)
void simd_fill_32(uint32_t *dest, uint32_t value, size_t count);

// SIMD-accelerated memory copy (count is number of uint32_t)
void simd_memcpy_32(uint32_t *dest, const uint32_t *src, size_t count);

// SIMD-accelerated 32-bit ARGB alpha blending
// Blends count pixels from src to dest with a global alpha.
void simd_blend_32(uint32_t *dest, const uint32_t *src, uint8_t global_alpha,
                   size_t count);

// SIMD-accelerated Desktop/Buffer clear
void simd_clear_buffer(uint32_t *dest, uint32_t *src, size_t count);

#endif
