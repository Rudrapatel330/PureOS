[BITS 32]

section .text
extern keyboard_handler
extern mouse_handler

global keyboard_handler_asm
global timer_handler_asm
global mouse_handler_asm

keyboard_handler_asm:
    cli
    pusha
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10   ; Kernel Data Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call keyboard_handler
    
    mov al, 0x20
    out 0x20, al
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    sti
    iret

timer_handler_asm:
    pusha
    ; Timer handler code here (Optional)
    mov al, 0x20
    out 0x20, al
    popa
    iret

mouse_handler_asm:
    pusha
    call mouse_handler
    mov al, 0x20
    out 0xA0, al ; EOI to Slave
    out 0x20, al ; EOI to Master
    popa
    iret
