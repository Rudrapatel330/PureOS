#include "apic.h"
#include "../../drivers/ports.h"
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
  // PIC will be disabled after IOAPIC is ready

  // Enable APIC via Spurious Interrupt Vector Register
  // Set spurious interrupt vector to 0xFF, and set the APIC Enable bit (bit 8)
  lapic_write(LAPIC_SIVR, 0x100 | 0xFF);

  // Configure timer to mask (we'll keep using PIT or APIC timer later)
  lapic_write(LAPIC_LVT_TIMER, 0x10000); // masked

  // Mask LINT0 since we are using IOAPIC for legacy interrupts
  lapic_write(LAPIC_LVT_LINT0, 0x10000); // Masked
  lapic_write(LAPIC_LVT_LINT1, 0x400); // NMI

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

  extern uint32_t io_apic_phys_addr;
  if (io_apic_phys_addr) {
    print_serial("APIC: Initializing IOAPIC...\n");
    volatile uint32_t *ioapic = (volatile uint32_t *)(uintptr_t)io_apic_phys_addr;
    
    // Helper to write to IOAPIC
    void ioapic_write(uint8_t reg, uint32_t data) {
      ioapic[0] = (reg & 0xFF);
      ioapic[4] = data;
    }
    
    // Route GSI 2 (PIT) to vector 32 on BSP
    ioapic_write(0x10 + 2 * 2 + 1, ((uint32_t)cpus[0].apic_id) << 24);
    ioapic_write(0x10 + 2 * 2, 32); 
    
    // Route GSI 1 (Keyboard) to vector 33 on BSP
    ioapic_write(0x10 + 1 * 2 + 1, ((uint32_t)cpus[0].apic_id) << 24);
    ioapic_write(0x10 + 1 * 2, 33);

    // Route GSI 12 (Mouse) to vector 44 on BSP
    ioapic_write(0x10 + 12 * 2 + 1, ((uint32_t)cpus[0].apic_id) << 24);
    ioapic_write(0x10 + 12 * 2, 44);

    print_serial("APIC: IOAPIC configured, disabling legacy PIC.\n");
    disable_pic();
  }
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
