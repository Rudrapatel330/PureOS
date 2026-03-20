#include "../../kernel/syscall.h"
#include <stdint.h>

// Minimal Dynamic Linker (ld-pureos.so)
// This is actually an ELF executable that acts as the interpreter.

void _start() {
  // 1. In a real dynamic linker, we would:
  //    - Get the main executable info from the stack (Auxiliary Vector)
  //    - Parse PT_DYNAMIC of the main executable
  //    - Load DT_NEEDED libraries
  //    - Perform relocations
  //    - Jump to the main entry point

  // For now, just a placeholder that syscalls to print
  const char *msg = "ld-pureos: Dynamic linker active (STUB)\n";

  // SYS_WRITE (rax=1, rbx=ptr, rcx=len)
  __asm__ volatile("mov $1, %%rax\n"
                   "mov %0, %%rbx\n"
                   "int $0x80\n"
                   :
                   : "r"(msg)
                   : "rax", "rbx");

  // EXIT
  __asm__ volatile("mov $0, %%rax\n"
                   "int $0x80\n"
                   :
                   :
                   : "rax");
}
