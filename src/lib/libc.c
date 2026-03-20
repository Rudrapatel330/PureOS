#include "../kernel/string.h"
#include "../kernel/syscall.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

// User-space strlen implementation since we can't use kernel's directly without
// more headers
size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

static inline uint64_t _syscall(uint64_t num, uint64_t a1, uint64_t a2,
                                uint64_t a3, uint64_t a4, uint64_t a5) {
  uint64_t ret;
  __asm__ volatile("mov %1, %%rax\n"
                   "mov %2, %%rbx\n"
                   "mov %3, %%rcx\n"
                   "mov %4, %%rdx\n"
                   "mov %5, %%rsi\n"
                   "mov %6, %%rdi\n"
                   "int $0x80\n"
                   "mov %%rax, %0"
                   : "=r"(ret)
                   : "r"(num), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5)
                   : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory");
  return ret;
}

int open(const char *pathname, int flags) {
  return (int)_syscall(SYS_OPEN, (uint64_t)pathname, (uint64_t)flags, 0, 0, 0);
}

int close(int fd) { return (int)_syscall(SYS_CLOSE, (uint64_t)fd, 0, 0, 0, 0); }

size_t read(int fd, void *buf, size_t count) {
  return (size_t)_syscall(SYS_READ, (uint64_t)fd, (uint64_t)buf,
                          (uint64_t)count, 0, 0);
}

size_t write(int fd, const void *buf, size_t count) {
  return (size_t)_syscall(SYS_WRITE, (uint64_t)fd, (uint64_t)buf,
                          (uint64_t)count, 0, 0);
}

void *malloc(size_t size) {
  // Simple: map a new region for every malloc. Very wasteful but works for
  // early userland.
  return (void *)_syscall(SYS_MMAP, 0, (uint64_t)size, 0, 0, 0);
}

void free(void *ptr) {
  // TODO: Implement MUNMAP
  (void)ptr;
}

int printf(const char *format, ...) {
  char buf[1024];
  va_list args;
  va_start(args, format);
  // Note: Simple vsnprintf would go here. For now we use kernel string helpers
  // if available. However, this is userland, so we should have our own string
  // formatting. For now, let's just use write(1, ...) for raw strings.
  write(1, format, strlen(format));
  va_end(args);
  return 0;
}
dirent_t *readdir(int fd, int index) {
  // We use malloc to allocate a dirent. The user should free it.
  // This matches the kernel's vfs_readdir behavior for now.
  dirent_t *d = (dirent_t *)malloc(sizeof(dirent_t));
  if (_syscall(SYS_LISTDIR, (uint64_t)fd, (uint64_t)index, (uint64_t)d, 0, 0) ==
      0) {
    return d;
  }
  free(d);
  return NULL;
}

void exit(int status) {
  _syscall(SYS_EXIT, (uint64_t)status, 0, 0, 0, 0);
  while (1)
    ;
}
