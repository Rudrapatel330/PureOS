#include "hal.h"
#include "../smp.h"
#include "acpi.h"
#include "apic.h"
#include "string.h"

extern void print_serial(const char *str);

void hal_init_early() {
  print_serial("HAL: Core arch initialization...\n");
  gdt_init();
  idt_init();
  isr_install();
}

void hal_init() {
  print_serial("HAL: Full hardware initialization...\n");

  paging_init(); // Paging must be before ACPI/APIC if they need mapping

  // Initialize standard PC hardware
  acpi_init();
  lapic_init();
  pic_init();

  // smp_init(); // DISABLED FOR DEBUGGING TASKING GPF/BLACK SCREEN

  print_serial("HAL: Layer Initialized.\n");
}

void hal_enable_interrupts() { __asm__ volatile("sti"); }

void hal_disable_interrupts() { __asm__ volatile("cli"); }

void hal_halt() { __asm__ volatile("hlt"); }
