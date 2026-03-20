#include "smp.h"
#include "../drivers/timer.h"
#include "acpi.h"
#include "apic.h"
#include "heap.h"
#include "paging.h"
#include "string.h"


extern uint8_t trampoline_start[];
extern uint8_t trampoline_end[];

// Physical location where trampoline is copied
#define TRAMPOLINE_ADDR 0x8000

// Offsets in trampoline (must match trampoline.asm)
#define OFFSET_PML4 8
#define OFFSET_STACK 16
#define OFFSET_ENTRY 24
#define OFFSET_CPUID 32

static volatile int booted_cpus = 1; // BSP is always booted

extern void print_serial(const char *);
extern void timer_wait(uint32_t ticks);

void ap_kernel_entry_common(int cpu_num) {
  // This is the 64-bit C entry point for APs
  print_serial("AP: CPU ");
  // Convert cpu_num to char (simple)
  char buf[2] = {'0' + (cpu_num % 10), 0};
  print_serial(buf);
  print_serial(" is online in Long Mode.\n");

  // Atomic increment of booted_cpus count
  __asm__ volatile("lock incl %0" : "+m"(booted_cpus));

  // Busy wait forever for now (BSP will later initialize a scheduler)
  while (1) {
    __asm__ volatile("hlt");
  }
}

void smp_init(void) {
  if (num_cpus <= 1) {
    print_serial("SMP: Only 1 CPU detected. Skipping AP initialization.\n");
    return;
  }

  print_serial("SMP: Initializing Multi-Processor support...\n");

  // 1. Copy trampoline to real-mode accessible low memory (0x8000)
  size_t trampoline_size =
      (uintptr_t)trampoline_end - (uintptr_t)trampoline_start;
  memcpy((void *)TRAMPOLINE_ADDR, trampoline_start, trampoline_size);

  uint64_t current_pml4_val;
  __asm__ volatile("mov %%cr3, %0" : "=r"(current_pml4_val));

  for (int i = 0; i < num_cpus; i++) {
    // Skip BSP (assuming BSP has a specific identifier or we just skip index 0
    // if it's BSP) In acpi.c we set is_bsp. Let's find it.
    if (cpus[i].is_bsp)
      continue;

    print_serial("SMP: Waking up AP ");
    char buf[2] = {'0' + (i % 10), 0};
    print_serial(buf);
    print_serial(" (APIC ID: ");
    char buf2[4] = {'0' + (cpus[i].apic_id % 10), 0}; // simplified
    print_serial(buf2);
    print_serial(")\n");

    // 2. Prepare AP data in trampoline
    // Allocate a 4KB stack for this AP
    void *ap_stack = kmalloc(4096);
    uint64_t stack_top = (uintptr_t)ap_stack + 4096;

    *(uint32_t *)(uintptr_t)(TRAMPOLINE_ADDR + OFFSET_PML4) =
        (uint32_t)current_pml4_val;
    *(uint64_t *)(uintptr_t)(TRAMPOLINE_ADDR + OFFSET_STACK) = stack_top;
    *(uint64_t *)(uintptr_t)(TRAMPOLINE_ADDR + OFFSET_ENTRY) =
        (uintptr_t)ap_kernel_entry_common;
    *(uint32_t *)(uintptr_t)(TRAMPOLINE_ADDR + OFFSET_CPUID) = i;

    // 3. Send INIT IPI
    lapic_send_ipi(cpus[i].apic_id, ICR_INIT | ICR_ASSERT | ICR_LEVEL);
    timer_wait(2); // ~10-20ms

    // 4. Send Startup IPI (SIPI) - Vector 0x08 (starts at 0x8000)
    lapic_send_ipi(cpus[i].apic_id, ICR_STARTUP | 0x08);
    timer_wait(1);

    // Optional: Second SIPI for compatibility
    lapic_send_ipi(cpus[i].apic_id, ICR_STARTUP | 0x08);
    timer_wait(1);
  }

  print_serial("SMP: AP initialization sequence complete.\n");
}

int smp_get_cpu_count(void) { return booted_cpus; }
