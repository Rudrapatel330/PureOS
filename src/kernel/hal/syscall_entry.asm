[BITS 64]
[GLOBAL syscall_entry_asm]
[EXTERN syscall_handler]
[EXTERN kernel_stack_top]

default rel

; 
; Fast syscall entry point (via MSR LSTAR)
;

syscall_entry_asm:
    ; Swap to kernel GS base (for per-CPU data)
    swapgs
    
    ; Save user RSP to r8 (temp), then load per-CPU kernel stack
    mov r8, rsp
    mov rsp, [gs:8]
    sub rsp, 512  ; Make room
    
    ; Save user registers (syscall clobbers RCX and R11)
    push r15
    push r14
    push r13
    push r12
    push r11      ; r11 = saved RFLAGS
    push r10
    push r9
    push r8       ; r8 = user RSP
    push rdi
    push rsi
    push rbp
    push rbx
    push rdx
    push rcx      ; rcx = user RIP
    push rax
    
    ; Enable SMAP access to user pages (stac = Set AC Flag)
    stac
    
    ; Set up arguments for C handler
    mov rdi, rsp
    
    ; Call the C syscall handler
    call syscall_handler
    
    ; Disable SMAP access to user pages (clac = Clear AC Flag)
    clac
    
    ; Restore regs from returned regs_t pointer (rax)
    mov rsp, rax
    
    ; Pop registers in reverse order
    pop rax
    pop rcx       ; rcx = user RIP (for sysret)
    pop rdx
    pop rbx
    pop rbp
    pop rsi
    pop rdi
    pop r8        ; r8 = user RSP (for sysret)
    pop r9
    pop r10
    pop r11       ; r11 = saved RFLAGS (for sysret)
    pop r12
    pop r13
    pop r14
    pop r15
    
    ; Restore user RSP
    mov rsp, r8
    
    ; Swap GS back
    swapgs
    
    ; Return to user mode via sysret
    ; RCX = RIP, R11 = RFLAGS
    sysretq
