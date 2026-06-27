#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "acpi.h"

// Force each struct onto its own 64-byte cache line
typedef struct {
    uint64_t current_pml4;   // offset 0
    uint64_t kernel_stack;   // offset 8
    uint32_t in_user_mode;   // offset 16
    uint32_t core_id;        // offset 20
    uint8_t  __pad[40];      // pad to 64 bytes
} __attribute__((aligned(64))) cpu_state_t;

extern cpu_state_t cpu_state[MAX_CPUS];

static inline uint64_t get_current_pml4(void) {
    uint64_t val;
    __asm__ volatile ("mov %%gs:0, %0" : "=r"(val));
    return val;
}

static inline void set_current_pml4(uint64_t cr3) {
    __asm__ volatile ("mov %0, %%gs:0" :: "r"(cr3) : "memory");
}

static inline uint32_t get_core_id(void) {
    uint32_t val;
    __asm__ volatile ("mov %%gs:20, %0" : "=r"(val));
    return val;
}

static inline void set_in_user_mode(uint32_t mode) {
    __asm__ volatile ("mov %0, %%gs:16" :: "r"(mode) : "memory");
}

static inline uint32_t get_in_user_mode(void) {
    uint32_t val;
    __asm__ volatile ("mov %%gs:16, %0" : "=r"(val));
    return val;
}

// MSR access wrappers
static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

#endif
