// gdt.h
#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// GDT Entry Structure
struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

// GDT Pointer Structure (for LGDT)
struct gdt_ptr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

// Initialization function
void gdt_init();
void tss_set_stack(uint64_t rsp0);

// Assembly function to reload segments
// Defined in gdt_flush.asm
extern void gdt_flush(uint64_t gdt_ptr_address);

#endif
