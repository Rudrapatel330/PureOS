#ifndef _SETJMP_H
#define _SETJMP_H

#include <stdint.h>

/*
 * x86_64 jmp_buf requires space for:
 * rbx, rbp, r12, r13, r14, r15, rsp,rip
 * 8 registers * 8 bytes = 64 bytes
 */
typedef struct {
  uint64_t regs[8];
} jmp_buf[1];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#endif
