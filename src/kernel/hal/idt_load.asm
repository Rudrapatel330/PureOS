; idt_load.asm
bits 64
global idt_load

; void idt_load(uint64_t idt_ptr_address);
idt_load:
    lidt [rdi]          ; Load the IDT
    ret
