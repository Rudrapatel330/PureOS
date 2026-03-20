; isr.asm - 64-bit Version
bits 64
global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

extern isr_handler
extern irq_handler

section .text

%macro PUSHALL 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro POPALL 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

; Common ISR stub
isr_common_stub:
    PUSHALL
    
    mov rbx, rsp   ; Save original rsp (rbx is callee-saved in SysV)
    sub rsp, 512
    and rsp, ~15   ; Align to 16 bytes for fxsave
    fxsave [rsp]
    
    mov rdi, rbx   ; Pass registers_t pointer as first argument
    call isr_handler
    
    mov rsp, rax   ; Switch stack (or keep same if rax == rbx)
    
    ; Unconditionally restore FPU state for the incoming task
    ; The state was saved exactly 512 bytes below its registers_t, 16-byte aligned
    mov rbx, rsp
    sub rsp, 512
    and rsp, ~15
    fxrstor [rsp]
    mov rsp, rbx   ; Restore rsp to point back to registers_t

    
    POPALL
    add rsp, 16    ; Clean up error code and ISR number (8 bytes each)
    iretq

; Common IRQ stub
irq_common_stub:
    PUSHALL
    
    mov rbx, rsp   ; Save original rsp
    sub rsp, 512
    and rsp, ~15
    fxsave [rsp]
    
    mov rdi, rbx   ; Pass registers_t pointer as first argument
    call irq_handler
    
    mov rsp, rax   ; Switch stack (or keep same if rax == rbx)
    
    ; Unconditionally restore FPU state for the incoming task
    mov rbx, rsp
    sub rsp, 512
    and rsp, ~15
    fxrstor [rsp]
    mov rsp, rbx

    
    POPALL
    add rsp, 16    ; Clean up error code and ISR number
    iretq

; Macro for ISRs without error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push qword 0
    push qword %1
    jmp isr_common_stub
%endmacro

; Macro for ISRs with error code
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push qword %1
    jmp isr_common_stub
%endmacro

; Macro for IRQs
%macro IRQ 2
global irq%1
irq%1:
    push qword 0
    push qword %2
    jmp irq_common_stub
%endmacro

; Exception handlers
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15

; IRQ handlers
IRQ  0, 32
IRQ  1, 33
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; Syscall handler
ISR_NOERRCODE 128
