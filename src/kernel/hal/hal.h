#ifndef HAL_H
#define HAL_H
#include "../types.h"
#include "apic.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "paging.h"
#include "pic.h"


// Unified Hardware Abstraction Layer Interface
// This header provides a single entry point for architecture-specific functions

typedef struct {
  void (*init)();
  void (*enable_interrupts)();
  void (*disable_interrupts)();
  void (*halt)();
  // Add more abstraction points as needed
} hal_interface_t;

// Global HAL functions
void hal_init_early();
void hal_init();
void hal_enable_interrupts();
void hal_disable_interrupts();
void hal_halt();

#endif // HAL_H
