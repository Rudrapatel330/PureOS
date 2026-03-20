[BITS 16]
[ORG 0x7E00]

start:
    ; Clear screen
    mov ax, 0x0003
    int 0x10
    
    mov si, msg1
    call print
    
    ; ========== SCAN SECTORS 10 - 15 ==========
    ; We know kernel is at sector 12 in the file.
    ; This *should* be sector 12 in CHS (Cyl 0, Head 0, Sector 12).
    
    mov cx, 10          ; Start scanning at sector 10
scan_loop:
    push cx             ; Save sector number
    
    ; Print "Sec XX: "
    mov si, sec_msg
    call print
    mov ax, cx
    call print_dec
    mov si, colon_msg
    call print
    
    ; Read 1 sector
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, cl          ; Sector number from CX
    mov dh, 0
    mov dl, 0x00        ; Floppy A:
    int 0x13
    jc .scan_failed
    
    ; Print first 4 bytes
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]
    call print_hex
    mov al, [0x0001]
    call print_hex
    mov al, [0x0002]
    call print_hex
    mov al, [0x0003]
    call print_hex
    pop ds
    
    ; Check if match (0x55)
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]
    pop ds
    cmp al, 0x55
    je .found_kernel
    
    mov si, crlf
    call print
    jmp .scan_next

.scan_failed:
    mov si, fail_msg
    call print
    jmp .scan_next

.found_kernel:
    mov si, found_msg
    call print
    jmp hang

.scan_next:
    pop cx
    inc cx
    cmp cx, 16          ; Stop at sector 15
    jl scan_loop
    
    mov si, not_found_msg
    call print
    jmp hang

hang:
    jmp $

; ========== FUNCTIONS ==========
print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print
.done:
    ret

print_hex:
    push ax
    mov bl, al
    shr al, 4
    call nibble
    mov ah, 0x0E
    int 0x10
    mov al, bl
    and al, 0x0F
    call nibble
    mov ah, 0x0E
    int 0x10
    pop ax
    ret

nibble:
    cmp al, 10
    jl .digit
    add al, 'A' - 10 - '0'
.digit:
    add al, '0'
    ret

print_dec:
    push ax
    push bx
    mov ah, 0
    mov bl, 10
    div bl
    add al, '0'
    push ax
    mov ah, 0x0E
    int 0x10
    pop ax
    mov al, ah
    add al, '0'
    mov ah, 0x0E
    int 0x10
    pop bx
    pop ax
    ret

; ========== MESSAGES ==========
msg1: db 'Stage2 Scan Start!', 13, 10, 0
sec_msg: db 'Sec ', 0
colon_msg: db ': ', 0
crlf: db 13, 10, 0
fail_msg: db 'Read Error!', 13, 10, 0
found_msg: db ' <- FOUND KERNEL (0x55)!', 13, 10, 0
not_found_msg: db 'Scan done. Kernel not found.', 13, 10, 0

times 4096-($-$$) db 0
