#include "../../kernel/elf.h"
#include "../../kernel/syscall.h"
#include <stdint.h>
#include <stdio.h>


// The dynamic linker itself is a static ELF or loaded by the kernel.
// It receives control at its entry point.

void _start_interp(void) {
  // 1. Relocate ourselves (if not static)
  // 2. Load the main executable
  // 3. Resolve symbols
  // 4. Jump to main executable entry

  // For now, just a stub to prove the kernel loaded us
  // We'll print to serial via syscall 1 (SYS_PRINT/WRITE)
  const char *msg = "LD_SO: Dynamic Linker Started\n";
  __asm__ volatile("mov $1, %%rax\n"
                   "mov %0, %%rbx\n"
                   "int $0x80" ::"r"(msg)
                   : "rax", "rbx");

  // Exit for now
  __asm__ volatile("mov $0, %%rax\n"
                   "int $0x80" ::
                       : "rax");
}
