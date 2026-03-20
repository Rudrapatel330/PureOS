bits 32
global _start
extern _bss_start
extern _bss_end
extern kernel_main

section .text
_start:
    ; DEBUG: Write 'K' to top left (Red on White)
    mov byte [0xB8000], 'K'
    mov byte [0xB8001], 0x4F
    
    ; Set up stack
    mov esp, stack_top

    ; Zero out .bss
    mov edi, _bss_start
    mov ecx, _bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb
    
    ; Call kernel main
    call kernel_main
    
    ; Hang if kernel returns
    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 65536  ; 64KB stack
stack_top:
