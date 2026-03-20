#include "apic.h"
#include "../drivers/ports.h"
#include "acpi.h"

extern void print_serial(const char *);

static void disable_pic(void) {
  // Disable legacy PIC by masking all interrupts
  outb(0x21, 0xFF);
  outb(0xA1, 0xFF);
}

void lapic_write(uint32_t reg, uint32_t data) {
  // Memory mapped write
  volatile uint32_t *apic =
      (volatile uint32_t *)(uintptr_t)(local_apic_phys_addr + reg);
  *apic = data;
}

uint32_t lapic_read(uint32_t reg) {
  // Memory mapped read
  volatile uint32_t *apic =
      (volatile uint32_t *)(uintptr_t)(local_apic_phys_addr + reg);
  return *apic;
}

void lapic_eoi(void) { lapic_write(LAPIC_EOI, 0); }

void lapic_init(void) {
  if (local_apic_phys_addr == 0) {
    print_serial("APIC: Error: Local APIC address not discovered by ACPI\n");
    return;
  }

  print_serial("APIC: Initializing Local APIC...\n");
  // disable_pic(); // Keep PIC enabled for keyboard/timer until IOAPIC is ready

  // Enable APIC via Spurious Interrupt Vector Register
  // Set spurious interrupt vector to 0xFF, and set the APIC Enable bit (bit 8)
  lapic_write(LAPIC_SIVR, 0x100 | 0xFF);

  // Configure timer to mask (we'll keep using PIT or APIC timer later)
  lapic_write(LAPIC_LVT_TIMER, 0x10000); // masked

  // Mask LINT0 and LINT1
  lapic_write(LAPIC_LVT_LINT0, 0x10000); // masked
  lapic_write(LAPIC_LVT_LINT1, 0x10000); // masked

  // Mask Error Register
  lapic_write(LAPIC_LVT_ERROR, 0x10000); // masked

  // Clear Error Status Register
  lapic_write(LAPIC_ESR, 0);
  lapic_write(LAPIC_ESR, 0);

  // Ack any pending interrupts
  lapic_write(LAPIC_EOI, 0);

  // Set Task Priority to 0 to accept all interrupts
  lapic_write(LAPIC_TPR, 0);

  print_serial("APIC: Local APIC Initialized & Enabled\n");
}

void lapic_send_ipi(uint8_t target_apic_id, uint32_t flags) {
  // Send IPI
  lapic_write(LAPIC_ICR_HIGH, ((uint32_t)target_apic_id) << 24);
  lapic_write(LAPIC_ICR_LOW, flags);

  // Wait for delivery status bit to clear
  while (lapic_read(LAPIC_ICR_LOW) & (1 << 12)) {
    __asm__ volatile("pause");
  }
}
