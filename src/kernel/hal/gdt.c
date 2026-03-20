// gdt.c
#include "gdt.h"
#include "../string.h" // For memset
#include <stdint.h>

struct gdt_entry
    gdt[7]; // Space for Null, KCode, KData, UCode, UData, TSS (2 slots)
struct gdt_ptr gp;

// 64-bit TSS structure
typedef struct tss_entry {
  uint32_t reserved0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

static tss_entry_t tss;

// Forward declarations
void gdt_set_gate(int num, uint64_t base, uint64_t limit, uint8_t access,
                  uint8_t gran);
extern void tss_flush(); // In gdt_flush.asm

void init_tss() {
  extern uint64_t kernel_stack_top;
  memset(&tss, 0, sizeof(tss_entry_t));
  tss.rsp0 = (uint64_t)&kernel_stack_top;
  tss.iomap_base = sizeof(tss_entry_t);
}

void gdt_init() {
  gp.limit = (sizeof(struct gdt_entry) * 7) - 1;
  gp.base = (uint64_t)&gdt;

  gdt_set_gate(0, 0, 0, 0, 0);                // Null
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF); // KCode (64-bit, L=1)
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xAF); // KData
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xAF); // UCode (64-bit, L=1)
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xAF); // UData

  init_tss();
  // TSS takes two entries (16 bytes) in 64-bit GDT at index 5 and 6
  uint64_t tss_base = (uint64_t)&tss;
  uint32_t tss_limit = sizeof(tss_entry_t) - 1;

  // Entry 5: Lower 64 bits of the 128-bit descriptor
  gdt_set_gate(5, tss_base, tss_limit, 0x89, 0x00);

  // Entry 6: Upper 64 bits of the 128-bit descriptor
  // In 64-bit mode, the second part of the TSS descriptor contains the upper 32
  // bits of the base
  struct gdt_entry *upper = &gdt[6];
  uint32_t base_high = (uint32_t)(tss_base >> 32);
  upper->limit_low = (uint16_t)(base_high & 0xFFFF);
  upper->base_low = (uint16_t)(base_high >> 16);
  upper->base_middle = 0;
  upper->access = 0;
  upper->granularity = 0;
  upper->base_high = 0;

  gdt_flush((uint64_t)&gp);
  tss_flush(); // Load TR with 0x28
}

// Helper to set GDT gate
void gdt_set_gate(int num, uint64_t base, uint64_t limit, uint8_t access,
                  uint8_t gran) {
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;

  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = ((limit >> 16) & 0x0F);
  gdt[num].granularity |= (gran & 0xF0);
  gdt[num].access = access;
}

void tss_set_stack(uint64_t rsp0) { tss.rsp0 = rsp0; }
