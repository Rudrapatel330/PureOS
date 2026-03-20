#include "random.h"
#include "io.h"

int errno = 0; // Provide errno for compiler intrinsics

extern uint32_t get_timer_ticks(void);

// Check if RDRAND is supported via CPUID
static int has_rdrand(void) {
  uint32_t ecx;
  __asm__ volatile("cpuid" : "=c"(ecx) : "a"(1) : "ebx", "edx");
  return (ecx & (1 << 30)) != 0;
}

// Low-level RDRAND call
static int read_rdrand(uint32_t *val) {
  uint8_t ok;
  __asm__ volatile("rdrand %0; setc %1" : "=r"(*val), "=qm"(ok));
  return ok;
}

// TSC readout
static uint64_t rdtsc(void) {
  uint32_t lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

uint32_t k_rand(void) {
  uint32_t val;
  if (has_rdrand()) {
    if (read_rdrand(&val))
      return val;
  }

  // Fallback: Use TSC and timer ticks
  // This isn't cryptographically secure on its own, but it's okay for a seed
  // combined with other jitter in a hobby OS context.
  static uint32_t state = 0x12345678;
  uint64_t t = rdtsc();
  state ^= (uint32_t)t;
  state ^= (uint32_t)(t >> 32);
  state ^= get_timer_ticks();

  // Simple Xorshift
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;

  return state;
}

void get_entropy(void *buf, int len) {
  uint8_t *p = (uint8_t *)buf;
  while (len > 0) {
    uint32_t r = k_rand();
    int chunk = (len > 4) ? 4 : len;
    for (int i = 0; i < chunk; i++) {
      *p++ = (r >> (i * 8)) & 0xFF;
    }
    len -= chunk;
  }
}
