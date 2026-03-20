[bits 32]
section .text
global _start
extern kernel_main
global kernel_stack_top

_start:
    cli

    ; Setup 64-bit Paging (PML4 -> PDPT -> PD)
    ; 1. ZERO BSS FIRST (before we fill paging tables in BSS)
    extern _bss_start
    extern _bss_end
    mov edi, _bss_start
    and edi, 0x3FFFFFFF ; Physical address
    mov ecx, _bss_end
    and ecx, 0x3FFFFFFF
    sub ecx, edi
    shr ecx, 2          ; 4 bytes at a time (dwords)
    xor eax, eax
    rep stosd

    ; 2. Link tables
    mov edi, pml4_table
    and edi, 0x3FFFFFFF 
    ; ecx is 0 now from rep stosd

    ; Link PML4 -> PDPT
    mov eax, pdpt_table
    and eax, 0x3FFFFFFF
    or eax, 0x03 ; Present, R/W
    mov edi, pml4_table
    and edi, 0x3FFFFFFF
    mov [edi], eax

    ; Link PDPT -> PD_low (for identity map)
    mov eax, pd_table_low
    and eax, 0x3FFFFFFF
    or eax, 0x03
    mov edi, pdpt_table
    and edi, 0x3FFFFFFF
    mov [edi], eax

    ; Link PDPT -> PD_high (for 0xC0000000)
    mov eax, pd_table_high
    and eax, 0x3FFFFFFF
    or eax, 0x03
    mov [edi + 3 * 8], eax  ; edi is still pdpt_table physical

    ; Map first 32MB in PD_low and PD_high using 16x 2MB pages
    mov ebx, pd_table_low
    and ebx, 0x3FFFFFFF
    mov ecx, pd_table_high
    and ecx, 0x3FFFFFFF
    
    mov eax, 0x00000083 ; Start at physical 0, Present, RW, 2MB size
    mov edx, 0 ; Loop counter (0 to 511 for 1GB)
.map_1gb:
    mov [ebx + edx * 8], eax
    mov [ecx + edx * 8], eax
    add eax, 0x200000 ; Next 2MB physical address
    inc edx
    cmp edx, 512
    jl .map_1gb

    ; Map LFB (0xE0000000) - Expanded to 16MB (8 entries)
    mov eax, 0xE0000083
    mov [ecx + 256 * 8], eax
    mov eax, 0xE0200083
    mov [ecx + 257 * 8], eax
    mov eax, 0xE0400083
    mov [ecx + 258 * 8], eax
    mov eax, 0xE0600083
    mov [ecx + 259 * 8], eax
    mov eax, 0xE0800083
    mov [ecx + 260 * 8], eax
    mov eax, 0xE0A00083
    mov [ecx + 261 * 8], eax
    mov eax, 0xE0C00083
    mov [ecx + 262 * 8], eax
    mov eax, 0xE0E00083
    mov [ecx + 263 * 8], eax

    ; Enable PAE, PSE, and SSE
    mov eax, cr4
    or eax, 0x00000630 ; PAE = bit 5, PSE = bit 4, OSFXSR = bit 9, OSXMMEXCPT = bit 10
    mov cr4, eax

    ; Set CR3 to PML4
    mov eax, pml4_table
    and eax, 0x3FFFFFFF
    mov cr3, eax

    ; Enable Long Mode (EFER.LME = 1)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x00000100 ; LME = bit 8
    wrmsr

    ; Enable Paging
    mov eax, cr0
    or eax, 0x80010001 ; PG + PE + WP
    mov cr0, eax

    ; Load 64-bit GDT (physical pointer access since EIP implies physical)
    mov eax, gdt64.pointer
    and eax, 0x3FFFFFFF
    lgdt [eax]

    ; Far jump to 64-bit code
    jmp 0x08:long_mode_entry

[bits 64]
extern _bss_start
extern _bss_end

long_mode_entry:
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up higher-half stack
    mov rsp, kernel_stack_top

    ; BSS already zeroed in 32-bit mode

    ; Call C Kernel
    mov rax, kernel_main
    call rax

    cli
.hang:
    hlt
    jmp .hang

section .bss
align 4096
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table_low:
    resb 4096
pd_table_high:
    resb 4096

section .data

align 8
gdt64:
    dq 0 ; Null Descriptor
.code: equ $ - gdt64
    ; 64-bit code descriptor: L=1, P=1, DPL=0, Type=RX
    dw 0
    dw 0
    db 0
    db 0x9A
    db 0x20 ; L=1
    db 0
.data: equ $ - gdt64
    ; 64-bit data descriptor: P=1, DPL=0, Type=RW
    dw 0
    dw 0
    db 0
    db 0x92
    db 0
    db 0
.pointer:
    dw $ - gdt64 - 1
    dd gdt64

section .bss
align 16
kernel_stack_bottom:
    resb 65536 ; 64KB Stack
kernel_stack_top:
