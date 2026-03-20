[bits 16]

section .text
global trampoline_start
global trampoline_end

trampoline_start:
    jmp entry16

align 8
    ap_pml4_ptr:         dd 0
    ap_stack_ptr:        dq 0
    ap_kernel_ptr:       dq 0
    ap_num:              dd 0

entry16:
    cli
    cld
    
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Load 32-bit GDT
    lgdt [0x8000 + (gdt32_ptr - trampoline_start)]

    ; Enable Protected Mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp 0x08:(0x8000 + (ap_pm_entry - trampoline_start))

[bits 32]
ap_pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Enable PAE
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    ; Load CR3 (PML4 address)
    mov eax, [0x8000 + (ap_pml4_ptr - trampoline_start)]
    mov cr3, eax

    ; Enable Long Mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    ; Enable Paging
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [0x8000 + (gdt64_ptr - trampoline_start)]

    ; Jump to 64-bit code
    jmp 0x18:(0x8000 + (ap_long_mode_entry - trampoline_start))

[bits 64]
ap_long_mode_entry:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Load stack pointer
    mov rsp, [0x8000 + (ap_stack_ptr - trampoline_start)]
    mov rbp, rsp

    ; Jump to C entry point (64-bit)
    mov rax, [0x8000 + (ap_kernel_ptr - trampoline_start)]
    jmp rax

; Data structures
align 8
gdt32:
    dq 0x0000000000000000 ; Null
    dq 0x00cf9a000000ffff ; Code 32
    dq 0x00cf92000000ffff ; Data 32
gdt32_ptr:
    dw 23
    dd 0x8000 + (gdt32 - trampoline_start)

align 8
gdt64:
    dq 0x0000000000000000 ; Null
    dq 0x00cf9a000000ffff ; Code 32 (unused)
    dq 0x00cf92000000ffff ; Data 32 (unused)
    dq 0x00af9a000000ffff ; Code 64
    dq 0x00af92000000ffff ; Data 64
gdt64_ptr:
    dw 39
    dd 0x8000 + (gdt64 - trampoline_start)

trampoline_end:
