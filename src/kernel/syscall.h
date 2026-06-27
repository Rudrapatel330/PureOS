#ifndef SYSCALL_H
#define SYSCALL_H

#include "isr.h"
#include <stddef.h>
#include <stdint.h>

// Syscall convention:
// RAX = syscall number
// RBX = arg1, RCX = arg2, RDX = arg3
// RSI = arg4, RDI = arg5
// Return in RAX

#define SYS_EXIT 0
#define SYS_PRINT 1
#define SYS_READKEY 2
#define SYS_GETMOUSE 3
#define SYS_SLEEP 4
#define SYS_MALLOC 5
#define SYS_FREE 6
#define SYS_GETTIME 7

#define SYS_OPEN 10
#define SYS_CLOSE 11
#define SYS_READ 12
#define SYS_WRITE 13
#define SYS_LSEEK 14
#define SYS_STAT 15
#define SYS_MKDIR 16
#define SYS_UNLINK 17
#define SYS_LISTDIR 18

#define SYS_FORK 20
#define SYS_EXECVE 21
#define SYS_WAITPID 22
#define SYS_GETPID 23
#define SYS_MMAP 24
#define SYS_MUNMAP 25
#define SYS_PIPE 26
#define SYS_DUP2 27
#define SYS_CHMOD 28
#define SYS_CHOWN 29
#define SYS_RENAME 30
#define SYS_READDIR 31
#define SYS_FSTAT 32
#define SYS_CREAT 33
#define SYS_SYMLINK 34
#define SYS_READLINK 35

// === Phase 1: New Production Syscalls ===
#define SYS_GETUID 40
#define SYS_GETGID 41
#define SYS_SETUID 42
#define SYS_SETGID 43
#define SYS_UMASK 44

// Shared Memory IPC
#define SYS_SHM_CREATE 50
#define SYS_SHM_ATTACH 51
#define SYS_SHM_DETACH 52
#define SYS_SHM_STAT 53

// Unix Domain Sockets
#define SYS_SOCKETPAIR 60
#define SYS_SENDMSG 61
#define SYS_RECVMSG 62

// Semaphore / Synchronization
#define SYS_SEM_CREATE 70
#define SYS_SEM_WAIT 71
#define SYS_SEM_POST 72

// Window Server IPC
#define SYS_WM_CONNECT 80
#define SYS_WM_CREATE_WINDOW 81
#define SYS_WM_SUBMIT_FRAME 82
#define SYS_WM_GET_EVENT 83
#define SYS_WM_GET_MOUSE 84

void syscall_init();
uint64_t syscall_handler(registers_t *regs);
void syscall_fast_entry(void);

// Syscall fast path (syscall/sysret) support
void arch_syscall_init(void);

// User pointer validation
int is_user_range(const void *addr, size_t len);
int is_user_string(const char *str);

// User mode entry/exit
void enter_user_mode(void *entry_point);
void return_to_kernel(void);

#endif
