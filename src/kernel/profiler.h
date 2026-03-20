#ifndef PROFILER_H
#define PROFILER_H

#include "isr.h"
#include <stdint.h>


void profiler_init(void);
void profiler_sample(registers_t *regs);
void profiler_dump(void);
void profiler_enable(int enable);

#endif
