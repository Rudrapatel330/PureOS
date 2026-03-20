#include "simd.h"
#include "string.h"

void simd_fill_32(uint32_t *dest, uint32_t value, size_t count) {
  if (count == 0)
    return;

  // Fill until 16-byte aligned
  while (((uintptr_t)dest & 15) && count > 0) {
    *dest++ = value;
    count--;
  }

  if (count >= 4) {
    size_t chunks = count / 4;
    __asm__ volatile("movd %[val], %%xmm0\n\t"
                     "pshufd $0, %%xmm0, %%xmm0\n\t" // Broadcast
                     "1:\n\t"
                     "movdqa %%xmm0, (%[dest])\n\t"
                     "add $16, %[dest]\n\t"
                     "dec %[chunks]\n\t"
                     "jnz 1b\n\t"
                     : [dest] "+r"(dest), [chunks] "+r"(chunks)
                     : [val] "r"(value)
                     : "xmm0", "memory");
    count %= 4;
    __asm__ volatile("sfence" ::: "memory");
  }

  while (count > 0) {
    *dest++ = value;
    count--;
  }
}

void simd_memcpy_32(uint32_t *dest, const uint32_t *src, size_t count) {
  if (count == 0)
    return;

  // If aligned and same offset, use fastest path
  if ((((uintptr_t)dest & 15) == ((uintptr_t)src & 15)) && count >= 4) {
    while (((uintptr_t)dest & 15) && count > 0) {
      *dest++ = *src++;
      count--;
    }

    size_t chunks = count / 4;
    if (chunks > 0) {
      __asm__ volatile(
          "1:\n\t"
          "movaps (%[src]), %%xmm0\n\t"
          "movdqa %%xmm0, (%[dest])\n\t"
          "add $16, %[src]\n\t"
          "add $16, %[dest]\n\t"
          "dec %[chunks]\n\t"
          "jnz 1b\n\t"
          : [dest] "+r"(dest), [src] "+r"(src), [chunks] "+r"(chunks)
          :
          : "xmm0", "memory");
      count %= 4;
      __asm__ volatile("sfence" ::: "memory");
    }
  } else if (count >= 4) {
    // Unaligned path using movups
    size_t chunks = count / 4;
    __asm__ volatile("1:\n\t"
                     "movups (%[src]), %%xmm0\n\t"
                     "movups %%xmm0, (%[dest])\n\t"
                     "add $16, %[src]\n\t"
                     "add $16, %[dest]\n\t"
                     "dec %[chunks]\n\t"
                     "jnz 1b\n\t"
                     : [dest] "+r"(dest), [src] "+r"(src), [chunks] "+r"(chunks)
                     :
                     : "xmm0", "memory");
    count %= 4;
    __asm__ volatile("sfence" ::: "memory");
  }

  while (count > 0) {
    *dest++ = *src++;
    count--;
  }
}

void simd_clear_buffer(uint32_t *dest, uint32_t *src, size_t count) {
  simd_memcpy_32(dest, src, count);
}

// SIMD Alpha Blending for 32-bit ARGB (Ported Over operator)
// Formula: dst = (src * (src_a * global_a) + dst * (255 - (src_a * global_a)))
// / 255
void simd_blend_32(uint32_t *dest, const uint32_t *src, uint8_t global_alpha,
                   size_t count) {
  if (count == 0 || global_alpha == 0)
    return;

  // Fast path for opaque global alpha is NOT simply memcpy if src has per-pixel
  // alpha! But if global_alpha is 255, we just do ARGB blending.

  uint32_t g_alpha = global_alpha;
  uint32_t val_255 = 255;

  size_t chunks = count / 2; // Process 2 pixels at a time
  if (chunks > 0) {
    __asm__ volatile(
        "pxor %%xmm7, %%xmm7\n\t"        // xmm7 = 0 (for unpacking)
        "movd %[ga], %%xmm6\n\t"         // xmm6 = [0,0,0,0,0,0,0,global_alpha]
        "pshuflw $0, %%xmm6, %%xmm6\n\t" // xmm6 =
                                         // [0,global_alpha,0,global_alpha,0,global_alpha,0,global_alpha]
        "punpcklqdq %%xmm6, %%xmm6\n\t" // xmm6 =
                                        // [global_alpha,global_alpha,global_alpha,global_alpha,global_alpha,global_alpha,global_alpha,global_alpha]
                                        // (16-bit words)

        "movd %[ff], %%xmm5\n\t"         // xmm5 = [0,0,0,0,0,0,0,255]
        "pshuflw $0, %%xmm5, %%xmm5\n\t" // xmm5 = [255,255,255,255,...]
        "punpcklqdq %%xmm5, %%xmm5\n\t"  // Broadcast 255 to upper 64 bits

        "1:\n\t"
        "movq (%[s]), %%xmm0\n\t" // Load 2 pixels (8 bytes) from src
        "movq (%[d]), %%xmm1\n\t" // Load 2 pixels (8 bytes) from dest

        "movaps %%xmm0, %%xmm2\n\t"    // Copy src to xmm2
        "punpcklbw %%xmm7, %%xmm0\n\t" // Unpack src: 8-bit to 16-bit
        "movaps %%xmm1, %%xmm3\n\t"    // Copy dest to xmm3
        "punpcklbw %%xmm7, %%xmm1\n\t" // Unpack dest: 8-bit to 16-bit

        // Calculate effective_alpha = (src_alpha * global_alpha) / 256
        "movaps %%xmm0, %%xmm2\n\t" // xmm2 gets 16-bit unpacked src pixels
        "pshuflw $0xFF, %%xmm2, %%xmm2\n\t" // Broadcast A0 to lower 4 words
        "pshufhw $0xFF, %%xmm2, %%xmm2\n\t" // Broadcast A1 to upper 4 words

        "pmullw %%xmm6, %%xmm2\n\t" // xmm2 = src_alpha * global_alpha

        // Accurate eff_a: (x + (x >> 8) + 1) >> 8
        "movaps %%xmm2, %%xmm4\n\t"
        "psrlw $8, %%xmm4\n\t"
        "paddw %%xmm4, %%xmm2\n\t"
        "pcmpeqw %%xmm4, %%xmm4\n\t"
        "psrlw $15, %%xmm4\n\t" // xmm4 = 1
        "paddw %%xmm4, %%xmm2\n\t"
        "psrlw $8, %%xmm2\n\t" // xmm2 = eff_a

        "movaps %%xmm5, %%xmm4\n\t" // Copy 255 to xmm4
        "psubw %%xmm2, %%xmm4\n\t"  // xmm4 = inv_effective_alpha = 255 - eff_a

        // Blend: (src * effective_alpha + dest * inv_effective_alpha)
        "pmullw %%xmm2, %%xmm0\n\t" // xmm0 = src * effective_alpha
        "pmullw %%xmm4, %%xmm1\n\t" // xmm1 = dest * inv_effective_alpha

        "paddw %%xmm1, %%xmm0\n\t" // xmm0 = Sum = (src * eff_a) + (dest *
                                   // inv_eff_a)

        // Accurate / 255: (x + (x >> 8) + 1) >> 8
        "movaps %%xmm0, %%xmm2\n\t"
        "psrlw $8, %%xmm2\n\t"
        "paddw %%xmm2, %%xmm0\n\t"

        "pcmpeqw %%xmm2, %%xmm2\n\t"  // All FFFF
        "psrlw $15, %%xmm2\n\t"       // xmm2 = 1
        "paddw %%xmm2, %%xmm0\n\t"    // Sum + (Sum>>8) + 1
        "psrlw $8, %%xmm0\n\t"        // >> 8
        "packuswb %%xmm7, %%xmm0\n\t" // Pack back to 8-bit
        "movq %%xmm0, (%[d])\n\t"     // Store 2 pixels

        "add $8, %[s]\n\t"
        "add $8, %[d]\n\t"
        "dec %[chunks]\n\t"
        "jnz 1b\n\t"
        : [d] "+r"(dest), [s] "+r"(src), [chunks] "+r"(chunks)
        : [ga] "r"(g_alpha), [ff] "r"(val_255)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory");
    count %= 2;
  }

  if (count > 0) {
    // Final pixel
    uint8_t a = global_alpha;
    uint32_t s = *src;
    uint32_t d = *dest;

    // effective alpha = (src_a * global_a) / 255
    uint32_t src_a = (s >> 24) & 0xFF;
    uint32_t sum_a = src_a * a;
    uint32_t eff_a = (sum_a + (sum_a >> 8) + 1) >> 8;

    if (eff_a == 0)
      return;
    if (eff_a >= 255) { // Use 255 here
      *dest = s;
      return;
    }

    uint32_t inv_a = 255 - eff_a;
    uint32_t sum_r = ((s >> 16) & 0xFF) * eff_a + ((d >> 16) & 0xFF) * inv_a;
    uint32_t sum_g = ((s >> 8) & 0xFF) * eff_a + ((d >> 8) & 0xFF) * inv_a;
    uint32_t sum_b = (s & 0xFF) * eff_a + (d & 0xFF) * inv_a;

    uint32_t r = (sum_r + (sum_r >> 8) + 1) >> 8;
    uint32_t g = (sum_g + (sum_g >> 8) + 1) >> 8;
    uint32_t b = (sum_b + (sum_b >> 8) + 1) >> 8;
    *dest = (0xFF << 24) | (r << 16) | (g << 8) | b;
  }
}
