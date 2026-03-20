// timer.c
#include "timer.h"
#include "../kernel/hal/isr.h"
#include "../kernel/profiler.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "ports.h"

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
  // Wait for a fresh tick
  uint32_t start_tick = tick;
  while (tick == start_tick)
    __asm__ volatile("pause");

  uint64_t tsc_start = rdtsc();
  uint32_t target_tick = tick + 10; // Wait 10 ticks (40ms @ 250Hz)
  while (tick < target_tick)
    __asm__ volatile("pause");

  uint64_t tsc_end = rdtsc();
  uint32_t ticks_elapsed = target_tick - (start_tick + 1);
  if (ticks_elapsed == 0)
    ticks_elapsed = 1;
  uint32_t ms_elapsed = (ticks_elapsed * 1000) / 250;
  if (ms_elapsed == 0)
    ms_elapsed = 1;

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
  if (tsc_freq_ms == 0)
    return tick * 4; // Fallback to 250Hz PIT
  return rdtsc() / tsc_freq_ms;
}

static uint64_t timer_callback(registers_t *regs) {
  tick++;
  sys_ticks++;

  // Sample the instruction pointer for the profiler
  profiler_sample(regs);

  task_t *current = get_current_task();
  if (current) {
    current->cpu_ticks++;
  }

  if (tick % 250 == 0) {
    print_cpu_stats();
  }

  // Trigger task switch every 2 ticks (8ms at 250Hz)
  if (tick % 2 == 0) {
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
