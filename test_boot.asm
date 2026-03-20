; test_boot.asm - Boots directly to test kernel
bits 16
org 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Load test kernel to 0x1000 linear address
    ; Segment 0x100 * 16 = 0x1000
    mov ax, 0x100
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02  ; Read sectors
    mov al, 4     ; 4 sectors (2KB kernel just to be safe)
    mov ch, 0     ; Cylinder 0
    mov cl, 2     ; Sector 2
    mov dh, 0     ; Head 0
    int 0x13
    
    ; Set VGA mode 13h
    mov ax, 0x13
    int 0x10

    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp 0x08:pm_start

bits 32
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000
    
    ; Jump to test kernel
    jmp 0x1000

; GDT
gdt_start:
    dq 0x0000000000000000  ; Null
    dq 0x00CF9A000000FFFF  ; Code (0x08)
    dq 0x00CF92000000FFFF  ; Data (0x10)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55
