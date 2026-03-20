[BITS 64]
[GLOBAL _start]
[EXTERN main]
[EXTERN exit]

section .text
_start:
    ; Standard userland entry
    ; Setup stack frames if needed
    xor rbp, rbp
    
    ; Call main
    call main
    
    ; If main returns, call exit(rax)
    mov rdi, rax
    call exit
    
    ; Should never reach here
    hlt
