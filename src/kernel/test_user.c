#include <stdint.h>

void syscall_write(const char *str) {
  __asm__ volatile("mov $1, %%rax; mov %0, %%rbx; int $0x80" ::"r"(str)
                   : "rax", "rbx");
}

void syscall_exit() { __asm__ volatile("mov $0, %%rax; int $0x80" ::: "rax"); }

void main() {
  syscall_write("Hello from isolated user mode!\n");
  syscall_exit();
}
