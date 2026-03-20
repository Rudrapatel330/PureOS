[BITS 16]
[ORG 0x7C00]

_start:
    ; Disable interrupts
    cli
    
    ; Set up segments and stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Enable interrupts
    sti
    
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Enable A20 line (Fast A20)
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Serial trace 'S' (Start)
    mov al, 'S'
    mov dx, 0x3F8
    out dx, al
    
    ; DEBUG: Show boot sector
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], 'B'
    mov byte [es:1], 0x0A
    
    ; Print message
    mov si, loading_msg
    call print_string
    mov si, loading_msg
    call ser_print_string
    
    ; RESET DISK DRIVE FIRST
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    
    ; Serial trace 'R' (Reset)
    mov al, 'R'
    mov dx, 0x3F8
    out dx, al
    
    ; === TRY LOADING STAGE 2 ===
    
    ; Method 1: Try LBA (Logical Block Addressing) - Most robust for SATA
    push es
    mov ax, 0x07E0
    mov es, ax
    xor bx, bx
    
    mov ah, 0x42      ; Extended Read
    mov dl, [boot_drive]
    mov si, dap       ; Disk Address Packet
    int 0x13
    pop es
    jnc .load_ok
    
    ; Serial trace 'W' (Warn/FAIL LBA)
    mov al, 'W'
    mov dx, 0x3F8
    out dx, al

    ; Method 2: Fallback to CHS (Sector 2)
    mov ax, 0x07E0
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 4
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jnc .load_ok
    
    ; If all fail
    jmp disk_error

.load_ok:
    ; Serial trace 'J' (Jump)
    mov al, 'J'
    mov dx, 0x3F8
    out dx, al

    ; Final check - pass boot drive in DL
    mov dl, [boot_drive]

    ; Jump to Stage 2
    jmp 0x07E0:0x0000

disk_error:
    ; Show error
    mov ax, 0xB800
    mov es, ax
    mov byte [es:4], 'E'
    mov byte [es:5], 0x0C
    
    mov si, error_msg
    call print_string
    
    ; Hang
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

ser_print_string:
    push dx
    push ax
    mov dx, 0x3F8
.ser_loop:
    lodsb
    or al, al
    jz .ser_done
    out dx, al
    jmp .ser_loop
.ser_done:
    pop ax
    pop dx
    ret

; Disk Address Packet for LBA
dap:
    db 0x10           ; Size of packet (16 bytes)
    db 0              ; Always 0
    dw 4              ; Number of sectors to read
    dw 0x0000         ; Offset
    dw 0x07E0         ; Segment
    dd 1              ; Starting LBA (sector 1)
    dd 0              ; Upper 32 bits of LBA

loading_msg: db 'Loading PureOS...', 0
error_msg: db ' Disk Error!', 0
boot_drive: db 0

; Fill boot sector
times 510-($-$$) db 0
dw 0xAA55
