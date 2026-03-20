// idt.c
#include "idt.h"
#include "../drivers/vga.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
  idt[num].base_low = (base & 0xFFFF);
  idt[num].base_mid = (base >> 16) & 0xFFFF;
  idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
  idt[num].selector = sel;
  idt[num].zero1 = 0;
  idt[num].zero2 = 0;
  idt[num].flags = flags;
}

void idt_init() {
  idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
  idtp.base = (uint64_t)&idt;

  // Clear out the IDT
  for (int i = 0; i < 256; i++) {
    idt_set_gate(i, 0, 0, 0);
  }

  // Load the IDT
  idt_load((uint64_t)&idtp);
}
