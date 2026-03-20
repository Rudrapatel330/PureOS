; gdt_flush.asm
bits 64
global gdt_flush

; void gdt_flush(uint64_t gdt_ptr_address);
gdt_flush:
    lgdt [rdi]          ; Load the GDT
    
    ; Reload segment registers
    mov ax, 0x10        ; Data segment selector (Offset 0x10 = 16 bytes = 2nd entry)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to reload CS in 64-bit mode
    push 0x08
    lea rax, [rel .flush]
    push rax
    retfq
    
.flush:
    ret

global tss_flush
tss_flush:
    mov ax, 0x28      ; TSS selector is at index 5 (5 * 8 = 0x28)
    ltr ax
    ret
