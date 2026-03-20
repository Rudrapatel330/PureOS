[ORG 0x7E00]
[BITS 16]

start:
    ; DEBUG: S2
    mov al, 'S'
    out 0xe9, al
    mov al, '2'
    out 0xe9, al
    ; Show "S2" (Stage 2)
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], 'S'
    mov byte [es:1], 0x07
    mov byte [es:2], '2'
    mov byte [es:3], 0x07
    
    ; Load kernel to 0x1000
    ; We load from Sector 5 (following Boot + 3 sectors of Stage2 reservation)
    xor ax, ax    ; ES = 0
    mov es, ax
    mov bx, 0x1000 ; Address = 0x1000
    
    mov ah, 0x02  ; Read sectors
    mov al, 4     ; Number of sectors (read enough for small kernel)
    mov ch, 0     ; Cylinder
    mov cl, 5     ; Sector 5
    mov dh, 0     ; Head
    mov dl, [boot_drive]  ; Drive (passed from boot sector)
    int 0x13
    
    jc disk_error
    
    ; Show "LD" (Loaded)
    mov ax, 0xB800
    mov es, ax
    mov byte [es:4], 'L'
    mov byte [es:5], 0x07
    mov byte [es:6], 'D'
    mov byte [es:7], 0x07

    ; Enter protected mode
    cli
    lgdt [gdt_descriptor]
    
    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp 0x08:protected_mode

disk_error:
    mov ax, 0xB800
    mov es, ax
    mov byte [es:10], 'E'
    mov byte [es:11], 0x0C
    jmp $

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; DEBUG: PM
    mov al, 'P'
    out 0xe9, al
    mov al, 'M'
    out 0xe9, al
    
    ; Show "PM" (Protected Mode)
    mov edi, 0xB8000 + 160
    mov word [edi], 0x0750  ; P
    mov word [edi+2], 0x074D ; M
    
    ; Call kernel at 0x1000
    call 0x1000
    
    ; Hang if returns
    cli
    hlt

boot_drive: db 0

gdt_start:
    dq 0
    dq 0x00CF9A000000FFFF ; Code
    dq 0x00CF92000000FFFF ; Data
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 512-($-$$) db 0
