; kernel_entry.asm - Flat binary kernel
bits 32

global _start
_start:
    ; Set up stack
    mov esp, stack_top
    
    ; DEBUG: Write 'E' to Port 0xE9 (Bochs Debug) to confirm entry
    mov al, 'E'
    out 0xE9, al
    
    ; Call main kernel  function
    extern kernel_main
    call kernel_main
    
    ; DEBUG: Write 'D' (Done) to Port 0xE9
    mov al, 'D'
    out 0xE9, al

    ; Hang
    cli
.hang:
    hlt
    jmp .hang

section .bss
stack_bottom:
    resb 16384  ; 16KB stack
stack_top:
global kernel_stack_top
kernel_stack_top:
