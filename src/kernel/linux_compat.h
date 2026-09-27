#ifndef LINUX_COMPAT_H
#define LINUX_COMPAT_H

#include <stdint.h>

// Rust-side Linux syscall dispatcher
// Called from syscall_handler when current task is a Linux process.
// Returns the (possibly updated) registers pointer.
uint64_t linux_syscall_handler(uint64_t *regs);

// C helper functions called by the Rust compat module via FFI
uint64_t linux_helper_get_pid(void);
uint64_t linux_helper_get_uid(void);
uint64_t linux_helper_get_gid(void);
uint64_t linux_helper_get_brk(void);
void     linux_helper_set_brk(uint64_t brk);
void     linux_helper_exit(void);
uint64_t linux_helper_get_mmap(void);
void     linux_helper_set_mmap(uint64_t mmap_addr);
int      linux_helper_mprotect(uint64_t addr, uint64_t len, int prot);
int      linux_helper_munmap(uint64_t addr, uint64_t len);
uint32_t linux_helper_frame_alloc(void);
void     linux_helper_frame_free(uint32_t phys);
int      linux_helper_eventfd2(uint32_t initval, int flags);
int      linux_helper_is_eventfd(int fd);
int      linux_helper_eventfd_poll(int fd, int events);
int      linux_helper_pipe2(int fds[2], int flags);
int64_t  linux_helper_clone_thread(void *regs, uint64_t flags, uint64_t child_stack, uint64_t parent_tid, uint64_t child_tid, uint64_t child_tls);
uint64_t linux_helper_get_tid(void);
uint64_t linux_helper_get_fs_base(void);
void     linux_helper_set_clear_tid(uint64_t addr);
int      linux_helper_get_errno(void);
void     linux_helper_yield(void);

#endif
