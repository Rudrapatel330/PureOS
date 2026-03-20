#ifndef DEBUG_H
#define DEBUG_H

#include "isr.h"
#include <stdint.h>


// Panic screen: renders directly to LFB, bypassing compositor
void debug_panic_screen(const char *message, registers_t *regs);

// Debug shell: serial/keyboard backend
void debug_shell();

// Helper to dump heap status
void debug_dump_heap();

// Helper to dump task list
void debug_dump_tasks();

#endif
