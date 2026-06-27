// timer.c
#include "timer.h"
#include "../kernel/hal/isr.h"
#include "../kernel/profiler.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "ports.h"
#include "../kernel/hal/atomic.h"
#include "../kernel/hal/cpu.h"
volatile uint32_t tick = 0;
uint32_t sys_ticks = 0; // Aliased for legacy animation support

static uint64_t tsc_freq_ms = 0;

uint32_t get_timer_ticks() { return tick; }

static inline uint64_t rdtsc(void) {
  uint32_t lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

void calibrate_tsc() {
  // Configure PIT for rate generator (Mode 2) to poll
  outb(0x43, 0x34); 
  outb(0x40, 0xFF);
  outb(0x40, 0xFF);

  uint64_t tsc_start = rdtsc();

  // Wait ~40ms using PIT polling
  uint32_t target_ticks = 40 * 1193;
  uint32_t elapsed_pit = 0;
  
  outb(0x43, 0x00);
  uint8_t lo = inb(0x40);
  uint8_t hi = inb(0x40);
  uint16_t last_count = (hi << 8) | lo;
  
  while (elapsed_pit < target_ticks) {
      outb(0x43, 0x00);
      lo = inb(0x40);
      hi = inb(0x40);
      uint16_t count = (hi << 8) | lo;
      
      if (count > last_count) {
          elapsed_pit += last_count + (0xFFFF - count);
      } else {
          elapsed_pit += (last_count - count);
      }
      last_count = count;
  }

  // Restore PIT to 250Hz Square Wave Mode (Mode 3)
  outb(0x43, 0x36);
  uint32_t divisor = 1193180 / 250;
  outb(0x40, divisor & 0xFF);
  outb(0x40, (divisor >> 8) & 0xFF);

  uint64_t tsc_end = rdtsc();
  uint32_t ms_elapsed = 40;

  tsc_freq_ms = (tsc_end - tsc_start) / ms_elapsed;
  if (tsc_freq_ms == 0)
    tsc_freq_ms = 1; // Prevent div-by-zero

  extern void print_serial(const char *);
  print_serial("TSC: Calibrated: ");
  // Simple print for debug
  char buf[32];
  k_itoa((uint32_t)tsc_freq_ms, buf);
  print_serial(buf);
  print_serial(" cycles/ms\n");
}

uint64_t get_timer_ms_hires() {
  return (uint64_t)tick * 4; // Use stable PIT timer (250Hz = 4ms per tick) to avoid cross-core TSC desync on SMP
}

static uint64_t timer_callback(registers_t *regs) {
  if (get_core_id() != 0) {
    extern void lapic_eoi(void);
    lapic_eoi();
    return (uint64_t)regs;
  }
  
  uint64_t current_tsc = rdtsc();
  static uint64_t last_tick_tsc = 0;
  
  // Use a spinlock or just rely on the 2ms threshold to debounce broadcast IRQ0
  // tsc_freq_ms is cycles per millisecond. Timer is 250Hz (4ms).
  // We only increment if at least 2ms have passed since the last increment.
  int did_tick = 0;
  if (current_tsc - last_tick_tsc >= tsc_freq_ms * 2) {
    last_tick_tsc = current_tsc;
    atomic_inc(&tick);
    atomic_inc(&sys_ticks);
    did_tick = 1;
  }

  // Sample the instruction pointer for the profiler
  profiler_sample(regs);

  task_t *current = get_current_task();
  if (current) {
    current->cpu_ticks++;
  }

  // Only the core that processed the tick should handle the global stats/switch
  if (did_tick && tick % 250 == 0) {
    extern void update_cpu_stats(void);
    update_cpu_stats();
  }

  // Trigger task switch every 2 ticks (8ms at 250Hz)
  if (did_tick && tick % 2 == 0) {
    return task_switch((uint64_t)regs);
  }

  return (uint64_t)regs;
}

void init_timer(uint32_t freq) {
  register_interrupt_handler(32, &timer_callback);

  // Hardened: Ensure frequency is within reasonable bounds (18Hz to 1000Hz)
  if (freq < 18)
    freq = 18;
  if (freq > 1000)
    freq = 1000;

  uint32_t divisor = 1193180 / freq;

  outb(0x43, 0x36);
  uint8_t l = (uint8_t)(divisor & 0xFF);
  uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

  outb(0x40, l);
  outb(0x40, h);
}

void timer_wait(uint32_t ticks) {
  uint32_t start_tick = tick;
  while (tick < start_tick + ticks) {
    __asm__ volatile("pause");
  }
}

// Wrapper to support legacy/external drivers
void sleep(uint32_t ticks) { timer_wait(ticks); }
