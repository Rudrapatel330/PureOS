// idt.h
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT Entry Structure
struct idt_entry {
  uint16_t base_low;
  uint16_t selector;
  uint8_t zero1; // IST
  uint8_t flags;
  uint16_t base_mid;
  uint32_t base_high;
  uint32_t zero2;
} __attribute__((packed));

// IDT Pointer Structure (for LIDT)
struct idt_ptr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

// Initialization function
void idt_init();
void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);

// Assembly function to load IDT
// Defined in idt_load.asm
extern void idt_load(uint64_t idt_ptr_address);

#endif
