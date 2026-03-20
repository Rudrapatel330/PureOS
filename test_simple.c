// test_simple.c - User Mode Test App
#include <stdint.h>

void user_test_entry() {
  const char *msg = "User Mode Active! Testing Syscalls...\n";

  __asm__ volatile("mov $1, %%rax\n"
                   "mov %0, %%rbx\n"
                   "int $0x80\n"
                   :
                   : "r"(msg)
                   : "rax", "rbx");

  // Call SYS_EXIT (0) to return to kernel
  __asm__ volatile("mov $0, %%rax\n"
                   "int $0x80\n"
                   :
                   :
                   : "rax");

  while (1)
    ;
}
