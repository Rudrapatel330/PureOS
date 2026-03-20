#include "../drivers/ports.h"
#include "assert.h"
#include "hal/hal.h"
#include "string.h"

extern void print_serial(const char *str);
extern void draw_text_32bpp(int x, int y, const char *text, uint32_t color);

static void unwind_stack(uint64_t rbp) {
  print_serial("Stack Trace:\n");
  uint64_t *frame = (uint64_t *)rbp;
  int depth = 0;
  char hex_buf[20];

  while (frame && depth < 20) {
    uint64_t rip = frame[1];
    if (rip == 0)
      break;

    print_serial("  [");
    k_itoa(depth, hex_buf);
    print_serial(hex_buf);
    print_serial("] 0x");
    k_itoa_hex(rip, hex_buf);
    print_serial(hex_buf);
    print_serial("\n");

    uint64_t next_frame = frame[0];
    if (next_frame <= (uint64_t)frame)
      break; // Avoid loops or downward growth
    if (next_frame & 7)
      break; // Alignment check

    frame = (uint64_t *)next_frame;
    depth++;
  }
}

#include "debug.h"

void kpanic(const char *message, const char *file, int line) {
  hal_disable_interrupts();

  char full_msg[256];
  strcpy(full_msg, message);
  strcat(full_msg, " at ");
  strcat(full_msg, file);
  strcat(full_msg, ":");
  char lstr[16];
  k_itoa(line, lstr);
  strcat(full_msg, lstr);

  print_serial("\n!!! KERNEL PANIC !!!\n");
  print_serial(full_msg);
  print_serial("\n");

  unwind_stack((uint64_t)__builtin_frame_address(0));

  // Try to show visual panic screen
  // Since we don't have full regs here, we pass dummy/captured ones
  registers_t dummy_regs = {0};
  __asm__ volatile("mov %%rax, %0\n"
                   "mov %%rbx, %1\n"
                   "mov %%rcx, %2\n"
                   "mov %%rdx, %3\n"
                   "mov %%rsi, %4\n"
                   "mov %%rdi, %5\n"
                   "mov %%rbp, %6\n"
                   "mov %%rsp, %7\n"
                   : "=m"(dummy_regs.rax), "=m"(dummy_regs.rbx),
                     "=m"(dummy_regs.rcx), "=m"(dummy_regs.rdx),
                     "=m"(dummy_regs.rsi), "=m"(dummy_regs.rdi),
                     "=m"(dummy_regs.rbp), "=m"(dummy_regs.rsp));
  // Capture RIP if possible (next instruction)
  // dummy_regs.rip = ...;

  debug_panic_screen(full_msg, &dummy_regs);

  while (1) {
    hal_halt();
  }
}
