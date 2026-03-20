[BITS 16]
[ORG 0x7C00]

_start:
    ; Disable interrupts
    cli
    
    ; Set up stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Enable interrupts
    sti
    
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Print loading message
    mov si, loading_msg
    call print_string
    
    ; Load Stage 2 (sectors 2-4)
    mov ax, 0x7E00    ; Load to 0x7E00 (right after boot sector)
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02      ; Read sectors
    mov al, 3         ; Read 3 sectors (Stage 2)
    mov ch, 0         ; Cylinder 0
    mov cl, 2         ; Sector 2 (boot sector is sector 1)
    mov dh, 0         ; Head 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Jump to Stage 2
    jmp 0x7E00:0x0000

disk_error:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

loading_msg: db 'Loading PureOS...', 13, 10, 0
error_msg: db 'Disk Error!', 13, 10, 0
boot_drive: db 0

; Fill boot sector
times 510-($-$$) db 0
dw 0xAA55
