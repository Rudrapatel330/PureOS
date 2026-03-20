#ifndef ISR_H
#define ISR_H

#include <stdint.h>

// Registers struct matching isr.asm pusha/push logic in 64-bit
typedef struct registers {
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
  uint64_t int_no, err_code;         // Interrupt info
  uint64_t rip, cs, rflags, rsp, ss; // Pushed by CPU
} __attribute__((packed)) registers_t;

// Function pointer for ISR handlers
typedef uint64_t (*isr_t)(registers_t *);

// Function to install ISRs
void isr_install();

// Main ISR handler called from Assembly
uint64_t isr_handler(registers_t *regs);

// Function to register a custom handler (for drivers)
void register_interrupt_handler(uint8_t n, isr_t handler);

// Function to handle IRQs
uint64_t irq_handler(registers_t *regs);

#endif
