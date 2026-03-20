// timer.h
#ifndef TIMER_H
#define TIMER_H

#include "../kernel/types.h"

void init_timer(uint32_t freq);
uint32_t get_timer_ticks();
void timer_wait(uint32_t ticks);
void calibrate_tsc();
uint64_t get_timer_ms_hires();
void sleep(uint32_t ticks);

#endif
