// isr.c - Full Implementation with Debug
#include "isr.h"
#include "../../drivers/ports.h"
#include "../../drivers/vga.h"
#include "../string.h"
#include "idt.h"

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

uint64_t isr_handler(registers_t *regs) {
  if (regs->int_no < 32) {
    if (regs->int_no == 14) {
      print_serial("PAGE FAULT DETECTED!\n");
    }

    print_serial("EXCEPTION: ");
    char n_str[32];
    print_serial("Int:");
    k_itoa(regs->int_no, n_str);
    print_serial(n_str);
    print_serial(" RIP: 0x");
    k_itoa_hex(regs->rip, n_str);
    print_serial(n_str);
    print_serial(" ERR: 0x");
    k_itoa_hex(regs->err_code, n_str);
    print_serial(n_str);
    print_serial("\n");

    print_serial(" RSP: 0x");
    k_itoa_hex(regs->rsp, n_str);
    print_serial(n_str);
    print_serial(" CS: 0x");
    k_itoa_hex(regs->cs, n_str);
    print_serial(n_str);
    print_serial(" SS: 0x");
    k_itoa_hex(regs->ss, n_str);
    print_serial(n_str);
    print_serial("\n");

    // Hex dump of instruction at RIP
    print_serial(" Code: ");
    uint8_t *code = (uint8_t *)regs->rip;
    for (int i = 0; i < 8; i++) {
      k_itoa_hex(code[i], n_str);
      print_serial(n_str + 14); // Just the byte
      print_serial(" ");
    }
    print_serial("\n");

    if (regs->int_no == 14) {
      page_fault_handler(regs);
      return (uint64_t)regs;
    }

    print_serial("Halted.\n");
    while (1) {
      __asm__ volatile("hlt");
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
