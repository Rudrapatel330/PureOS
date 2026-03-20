; progressive_fixed.asm
[BITS 16]
[ORG 0x7E00]

start:
    ; Clear screen
    mov ax, 0x0003
    int 0x10
    
    mov si, step1_msg
    call print_string
    
    ; ================================
    ; TRY MULTIPLE SECTOR LOCATIONS
    ; ================================
    
    ; Try sector 100 first
    mov si, trying_sector100
    call print_string
    
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 100     ; SECTOR 100
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc .try_sector50
    
    ; Check if we got non-zero data
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]
    pop ds
    
    or al, al
    jz .try_sector50 ; If zero, try next
    
    mov si, found_msg
    call print_string
    
    call print_hex_byte
    jmp .done
    
.try_sector50:
    ; Try sector 50
    mov si, trying_sector50
    call print_string
    
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 50      ; SECTOR 50
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc .try_sector20
    
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]
    pop ds
    
    or al, al
    jz .try_sector20
    
    mov si, found_msg
    call print_string
    
    call print_hex_byte
    jmp .done
    
.try_sector20:
    ; Try sector 20
    mov si, trying_sector20
    call print_string
    
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 20      ; SECTOR 20
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc .disk_error
    
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]
    pop ds
    
    mov si, found_msg
    call print_string
    
    call print_hex_byte
    jmp .done

.disk_error:
    mov si, error_msg
    call print_string
    jmp $

.done:
    ; Hang
    jmp $

; ================================
; UTILITY FUNCTIONS
; ================================
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

print_hex_byte:
    ; Print AL as hex
    push ax
    mov bl, al
    
    shr al, 4
    call nibble_to_hex
    mov ah, 0x0E
    int 0x10
    
    mov al, bl
    and al, 0x0F
    call nibble_to_hex
    mov ah, 0x0E
    int 0x10
    
    pop ax
    ret

nibble_to_hex:
    cmp al, 10
    jl .digit
    add al, 'A' - 10 - '0'
.digit:
    add al, '0'
    ret

; ================================
; MESSAGES
; ================================
step1_msg: db 'Step 1: Stage2 loaded', 13, 10, 0
trying_sector100: db 'Trying sector 100... ', 0
trying_sector50: db 13, 10, 'Trying sector 50... ', 0
trying_sector20: db 13, 10, 'Trying sector 20... ', 0
found_msg: db 'Found: 0x', 0
error_msg: db 13, 10, 'All disk reads failed!', 13, 10, 0
boot_drive: db 0

; Pad
times 2048-($-$$) db 0
