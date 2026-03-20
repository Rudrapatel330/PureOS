[BITS 16]
[ORG 0x7C00]

_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    mov [boot_drive], dl
    
    ; Print message
    mov si, msg
    call print
    
    ; Load stage2 (sectors 2-9)
    mov ax, 0x07E0
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 8          ; 8 sectors = 4KB
    mov ch, 0
    mov cl, 2          ; Sector 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc error
    
    ; Jump to stage2
    jmp 0x07E0:0x0000

error:
    mov si, err_msg
    call print
    jmp $

print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print
.done:
    ret

msg: db 'Booting...', 0
err_msg: db ' Error!', 0
boot_drive: db 0

times 510-($-$$) db 0
dw 0xAA55
