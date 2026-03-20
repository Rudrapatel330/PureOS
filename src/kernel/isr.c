// isr.c - Full Implementation with Debug
#include "isr.h"
#include "../drivers/ports.h"
#include "../drivers/vga.h"
#include "debug.h"
#include "hal/hal.h"
#include "idt.h"
#include "string.h"
#include "task.h"

isr_t interrupt_handlers[256];

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void isr_install() {
  for (int i = 0; i < 256; i++) {
    interrupt_handlers[i] = 0;
  }

  // Exceptions
  idt_set_gate(0, (uint64_t)isr0, 0x08, 0x8E);
  idt_set_gate(1, (uint64_t)isr1, 0x08, 0x8E);
  idt_set_gate(2, (uint64_t)isr2, 0x08, 0x8E);
  idt_set_gate(3, (uint64_t)isr3, 0x08, 0x8E);
  idt_set_gate(4, (uint64_t)isr4, 0x08, 0x8E);
  idt_set_gate(5, (uint64_t)isr5, 0x08, 0x8E);
  idt_set_gate(6, (uint64_t)isr6, 0x08, 0x8E);
  idt_set_gate(7, (uint64_t)isr7, 0x08, 0x8E);
  idt_set_gate(8, (uint64_t)isr8, 0x08, 0x8E);
  idt_set_gate(9, (uint64_t)isr9, 0x08, 0x8E);
  idt_set_gate(10, (uint64_t)isr10, 0x08, 0x8E);
  idt_set_gate(11, (uint64_t)isr11, 0x08, 0x8E);
  idt_set_gate(12, (uint64_t)isr12, 0x08, 0x8E);
  idt_set_gate(13, (uint64_t)isr13, 0x08, 0x8E);
  idt_set_gate(14, (uint64_t)isr14, 0x08, 0x8E);
  idt_set_gate(15, (uint64_t)isr15, 0x08, 0x8E);

  // IRQs
  idt_set_gate(32, (uint64_t)irq0, 0x08, 0x8E);
  idt_set_gate(33, (uint64_t)irq1, 0x08, 0x8E);
  idt_set_gate(34, (uint64_t)irq2, 0x08, 0x8E);
  idt_set_gate(35, (uint64_t)irq3, 0x08, 0x8E);
  idt_set_gate(36, (uint64_t)irq4, 0x08, 0x8E);
  idt_set_gate(37, (uint64_t)irq5, 0x08, 0x8E);
  idt_set_gate(38, (uint64_t)irq6, 0x08, 0x8E);
  idt_set_gate(39, (uint64_t)irq7, 0x08, 0x8E);
  idt_set_gate(40, (uint64_t)irq8, 0x08, 0x8E);
  idt_set_gate(41, (uint64_t)irq9, 0x08, 0x8E);
  idt_set_gate(42, (uint64_t)irq10, 0x08, 0x8E);
  idt_set_gate(43, (uint64_t)irq11, 0x08, 0x8E);
  idt_set_gate(44, (uint64_t)irq12, 0x08, 0x8E);
  idt_set_gate(45, (uint64_t)irq13, 0x08, 0x8E);
  idt_set_gate(46, (uint64_t)irq14, 0x08, 0x8E);
  idt_set_gate(47, (uint64_t)irq15, 0x08, 0x8E);

  // Syscall (int 0x80, DPL 3)
  extern void isr128();
  idt_set_gate(128, (uint64_t)isr128, 0x08, 0xEE);
}

#include "paging.h"

static const char *exception_names[] = {
    "Divide by Zero",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD FP Exception",
    "Virtualization",
    "Control Protection",
};

uint64_t isr_handler(registers_t *regs) {
  if (regs->int_no < 32) {
    // --- Page Fault has its own handler ---
    if (regs->int_no == 14) {
      page_fault_handler(regs);
      return (uint64_t)regs;
    }

    // --- Determine exception name ---
    const char *exc_name = (regs->int_no < 22) ? exception_names[regs->int_no]
                                               : "Unknown Exception";

    // --- Dump registers to serial ---
    char hex_buf[20];
    print_serial("\n--- CPU EXCEPTION ---\n");
    print_serial("Exception: ");
    print_serial(exc_name);
    print_serial(" (");
    char n_str[4];
    k_itoa(regs->int_no, n_str);
    print_serial(n_str);
    print_serial(")\n");
    print_serial("RIP: 0x");
    k_itoa_hex(regs->rip, hex_buf);
    print_serial(hex_buf);
    print_serial("  RSP: 0x");
    k_itoa_hex(regs->rsp, hex_buf);
    print_serial(hex_buf);
    print_serial("\n");
    print_serial("ERR: 0x");
    k_itoa_hex(regs->err_code, hex_buf);
    print_serial(hex_buf);
    print_serial("  CS: 0x");
    k_itoa_hex(regs->cs, hex_buf);
    print_serial(hex_buf);
    print_serial("\n");

    // --- User-mode fault: kill the process, don't crash the kernel ---
    if ((regs->cs & 0x3) == 3) {
      task_t *cur = get_current_task();
      print_serial("USER FAULT in task '");
      if (cur)
        print_serial(cur->name);
      print_serial("' — killing process\n");
      if (cur)
        task_kill(cur->id);
      return (uint64_t)regs;
    }

    // --- Kernel-mode fault: unrecoverable, panic ---
    char exc_msg[64];
    strcpy(exc_msg, "Kernel EXCEPTION: ");
    strcat(exc_msg, exc_name);

    // Disable interrupts and show visual panic screen with EXACT registers
    hal_disable_interrupts();
    debug_panic_screen(exc_msg, regs);

    while (1) {
      hal_halt();
    }
  }

  if (interrupt_handlers[regs->int_no] != 0) {
    isr_t handler = interrupt_handlers[regs->int_no];
    return handler(regs);
  }

  return (uint64_t)regs;
}

uint64_t irq_handler(registers_t *regs) {
  uint64_t ret_esp = (uint64_t)regs;
  if (interrupt_handlers[regs->int_no] != 0) {
    isr_t handler = interrupt_handlers[regs->int_no];
    ret_esp = handler(regs);
  }

  // Send EOI to PICs
  if (regs->int_no >= 40) {
    outb(0xA0, 0x20); // Slave
  }
  outb(0x20, 0x20); // Master

  return ret_esp;
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
  interrupt_handlers[n] = handler;
}
