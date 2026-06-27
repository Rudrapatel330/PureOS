#include "smp.h"
#include "../drivers/timer.h"
#include "acpi.h"
#include "apic.h"
#include "gdt.h"
#include "heap.h"
#include "isr.h"
#include "paging.h"
#include "string.h"
#include "task.h"


extern uint8_t trampoline_start[];
extern uint8_t trampoline_end[];

#define TRAMPOLINE_ADDR 0x8000
#define OFFSET_PML4 8
#define OFFSET_STACK 16
#define OFFSET_ENTRY 24
#define OFFSET_CPUID 32

static volatile int booted_cpus = 1;
static volatile int scheduler_ready = 0;

extern void print_serial(const char *);
extern void timer_wait(uint32_t ticks);

cpu_core_t cpu_cores[MAX_CPUS];
volatile int active_cpus = 1;

static int apic_id_to_index[256];
static int apic_id_map_initialized = 0;

static void build_apic_id_map(void) {
  for (int i = 0; i < 256; i++)
    apic_id_to_index[i] = -1;
  for (int i = 0; i < num_cpus && i < MAX_CPUS; i++) {
    apic_id_to_index[cpus[i].apic_id] = i;
  }
  apic_id_map_initialized = 1;
}

int get_core_id(void) {
  if (!apic_id_map_initialized) return 0;
  volatile uint32_t *lapic = (volatile uint32_t *)(uintptr_t)local_apic_phys_addr;
  uint8_t apic_id = (lapic[0x20 / 4] >> 24) & 0xFF;
  int idx = apic_id_to_index[apic_id];
  return (idx >= 0) ? idx : 0;
}

extern void irq16(void);

static uint8_t ap_tick_throttle[MAX_CPUS];

static uint64_t lapic_timer_scheduling_handler(registers_t *regs) {
  int core = get_core_id();
  task_t *current = cpu_cores[core].current_task;
  if (current) {
    current->cpu_ticks++;
  }

  uint64_t new_rsp;
  uint8_t t = ++ap_tick_throttle[core];
  if ((t & 3) == 0) {
    new_rsp = task_switch((uint64_t)regs);
  } else {
    new_rsp = (uint64_t)regs;
  }

  lapic_eoi();
  return new_rsp;
}

// For AP LAPIC timer: calibrate PIT-based to match ~250Hz (4ms period)
// Divided initcnt yields ~4ms fire rate; throttle 4 → ~16ms per task switch
static uint32_t lapic_timer_initcnt_val = 0;

static void ap_init_percpu(int cpu_num) {
  cpu_cores[cpu_num].apic_id = cpus[cpu_num].apic_id;
  cpu_cores[cpu_num].cpu_number = cpu_num;
  cpu_cores[cpu_num].current_task = 0;
  cpu_cores[cpu_num].tss = 0;
  cpu_cores[cpu_num].lapic_timer_freq = 0;

  gdt_init_cpu(cpu_num);

  lapic_write(LAPIC_TPR, 0);
  lapic_write(LAPIC_SIVR, 0x100 | 0xFF);
}

static void ap_scheduler_idle(void) {
  while (1) {
    __asm__ volatile("sti; hlt");
  }
}

void ap_kernel_entry_common(int cpu_num) {
  char buf[2] = {'0' + (cpu_num % 10), 0};
  print_serial("AP: CPU ");
  print_serial(buf);
  print_serial(" is online in Long Mode.\n");

  __asm__ volatile("lock incl %0" : "+m"(booted_cpus));

  ap_init_percpu(cpu_num);

  print_serial("AP: CPU ");
  print_serial(buf);
  print_serial(" waiting for scheduler...\n");

  while (!scheduler_ready) {
    __asm__ volatile("pause; pause; pause");
  }

  print_serial("AP: CPU ");
  print_serial(buf);
  print_serial(" starting LAPIC timer...\n");

  uint32_t lapic_flags = 48 | 0x20000;
  lapic_write(LAPIC_TIMER_DIV, 0x03);
  lapic_write(LAPIC_TIMER_INITCNT, lapic_timer_initcnt_val);
  lapic_write(LAPIC_LVT_TIMER, lapic_flags);

  __asm__ volatile("sti");
  print_serial("AP: CPU ");
  print_serial(buf);
  print_serial(" scheduler active.\n");

  ap_scheduler_idle();
}

// Calibrate the LAPIC timer on BSP using PIT ticks.
// Sets lapic_timer_initcnt_val so APs fire at ~250Hz (~4ms period).
static void calibrate_lapic_timer(void) {
  extern volatile uint32_t tick;
  extern void print_serial(const char *);
  extern void k_itoa(int, char *);

  // One-shot, masked so no stray interrupt fires during calibration
  lapic_write(LAPIC_LVT_TIMER, 48 | 0x10000);
  lapic_write(LAPIC_TIMER_DIV, 0x03);

  // Wait for a fresh PIT tick
  uint32_t st = tick;
  while (tick == st) { __asm__ volatile("pause"); }

  // Start LAPIC timer with a large INITCNT
  uint32_t test_initcnt = 0x800000;
  lapic_write(LAPIC_TIMER_INITCNT, test_initcnt);

  // Wait 10 PIT ticks = 40ms at 250Hz
  uint32_t target = tick + 10;
  while (tick < target) { __asm__ volatile("pause"); }

  uint32_t remaining = lapic_read(LAPIC_TIMER_CURCNT);
  uint32_t elapsed = test_initcnt - remaining;

  // elapsed counts = 40ms of LAPIC timer ticking at bus_freq/16
  // We want a 4ms target period (250Hz rate before throttle)
  // initcnt_4ms = elapsed * 4 / 40 = elapsed / 10
  lapic_timer_initcnt_val = elapsed / 10;

  // Safety floor: must be > 0
  if (lapic_timer_initcnt_val == 0) lapic_timer_initcnt_val = 1;

  // Debug: print bus frequency
  // bus_freq = elapsed * 16 / 0.040 = elapsed * 400
  uint32_t bus_freq_khz = (elapsed * 400) / 1000;
  char buf[32];
  k_itoa((int)bus_freq_khz, buf);
  print_serial("LAPIC: Calibrated bus ~");
  print_serial(buf);
  print_serial(" kHz, INITCNT=");
  k_itoa((int)lapic_timer_initcnt_val, buf);
  print_serial(buf);
  print_serial("\n");
}

void smp_init(void) {
  if (num_cpus <= 1) {
    print_serial("SMP: Only 1 CPU detected. Skipping AP initialization.\n");
    return;
  }

  print_serial("SMP: Initializing Multi-Processor support...\n");

  build_apic_id_map();

  register_interrupt_handler(48, lapic_timer_scheduling_handler);

  for (int i = 0; i < num_cpus; i++) {
    cpu_cores[i].apic_id = cpus[i].apic_id;
    cpu_cores[i].cpu_number = i;
    cpu_cores[i].current_task = 0;
    cpu_cores[i].tss = 0;
    cpu_cores[i].lapic_timer_freq = 0;
  }

  cpu_cores[0].apic_id = cpus[0].apic_id;
  cpu_cores[0].cpu_number = 0;

  size_t trampoline_size =
      (uintptr_t)trampoline_end - (uintptr_t)trampoline_start;
  memcpy((void *)TRAMPOLINE_ADDR, trampoline_start, trampoline_size);

  uint64_t current_pml4_val;
  __asm__ volatile("mov %%cr3, %0" : "=r"(current_pml4_val));

  for (int i = 0; i < num_cpus; i++) {
    if (cpus[i].is_bsp) continue;

    print_serial("SMP: Waking up AP ");
    char buf[2] = {'0' + (i % 10), 0};
    print_serial(buf);
    print_serial(" (APIC ID: ");
    char buf2[4] = {'0' + (cpus[i].apic_id % 10), 0};
    print_serial(buf2);
    print_serial(")\n");

    void *ap_stack = kmalloc(16384);
    uint64_t stack_top = (uintptr_t)ap_stack + 16384;

    *(uint32_t *)(uintptr_t)(TRAMPOLINE_ADDR + OFFSET_PML4) =
        (uint32_t)current_pml4_val;
    *(uint64_t *)(uintptr_t)(TRAMPOLINE_ADDR + OFFSET_STACK) = stack_top;
    *(uint64_t *)(uintptr_t)(TRAMPOLINE_ADDR + OFFSET_ENTRY) =
        (uintptr_t)ap_kernel_entry_common;
    *(uint32_t *)(uintptr_t)(TRAMPOLINE_ADDR + OFFSET_CPUID) = i;

    lapic_send_ipi(cpus[i].apic_id, ICR_INIT | ICR_ASSERT | ICR_LEVEL);
    for (volatile int w = 0; w < 500000; w++) __asm__ volatile("pause");

    lapic_send_ipi(cpus[i].apic_id, ICR_STARTUP | 0x08);
    for (volatile int w = 0; w < 100000; w++) __asm__ volatile("pause");

    lapic_send_ipi(cpus[i].apic_id, ICR_STARTUP | 0x08);
    for (volatile int w = 0; w < 100000; w++) __asm__ volatile("pause");
  }

  print_serial("SMP: AP initialization sequence complete.\n");
}

void smp_start_aps(void) {
  print_serial("SMP: Calibrating LAPIC timer...\n");
  calibrate_lapic_timer();

  print_serial("SMP: Releasing APs into scheduler...\n");
  __asm__ volatile("mfence");
  scheduler_ready = 1;
}

int smp_get_cpu_count(void) { return booted_cpus; }
