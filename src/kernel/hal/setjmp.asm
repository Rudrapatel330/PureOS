global setjmp
global longjmp

section .text

; int setjmp(jmp_buf env)
; rcx/rdi = env
setjmp:
    ; System V AMD64 ABI: rdi is the first argument
    ; However, PureOS might be using Microsoft x64 ABI where rcx is the first argument
    ; Let's save for both just in case, or adapt. Wait, x86_64-elf-gcc uses System V ABI
    ; where rdi = 1st arg, rsi = 2nd arg.
    
    ; rdi contains the pointer to jmp_buf
    mov [rdi + 0x00], rbx
    mov [rdi + 0x08], rbp
    mov [rdi + 0x10], r12
    mov [rdi + 0x18], r13
    mov [rdi + 0x20], r14
    mov [rdi + 0x28], r15

    ; Save stack pointer (as it was before the call, so add 8 to rsp to skip return address)
    lea rax, [rsp + 8]
    mov [rdi + 0x30], rax

    ; Save return address (currently on top of stack)
    mov rax, [rsp]
    mov [rdi + 0x38], rax

    ; Return 0
    xor rax, rax
    ret

; void longjmp(jmp_buf env, int val)
longjmp:
    ; System V ABI: rdi = env, rsi = val
    mov rbx, [rdi + 0x00]
    mov rbp, [rdi + 0x08]
    mov r12, [rdi + 0x10]
    mov r13, [rdi + 0x18]
    mov r14, [rdi + 0x20]
    mov r15, [rdi + 0x28]

    ; Restore stack pointer
    mov rsp, [rdi + 0x30]

    ; Push return address onto the stack so we can jump to it
    mov rdx, [rdi + 0x38]

    ; Set return value (val). If val == 0, return 1 instead
    mov rax, rsi
    test rax, rax
    jnz .done
    mov rax, 1
.done:
    ; Jump to the saved return address
    jmp rdx
