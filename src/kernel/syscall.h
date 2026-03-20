#ifndef SYSCALL_H
#define SYSCALL_H

#include "isr.h"
#include <stdint.h>

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

void syscall_init();
uint64_t syscall_handler(registers_t *regs);

// User mode entry/exit
void enter_user_mode(void *entry_point);
void return_to_kernel(void);

#endif
