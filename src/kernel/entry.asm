[BITS 32]

global _start
extern kernel_main

section .text
_start:
    ; Set up stack pointer (16KB stack)
    mov esp, stack_top
    
    ; Clear direction flag
    cld
    
    ; Call the C kernel
    call kernel_main
    
    ; If kernel_main returns, hang
    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 65536
stack_top:
