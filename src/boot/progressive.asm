; progressive.asm
[BITS 16]
[ORG 0x7E00]

start:
    ; Step 1: Clear screen and show we're alive
    mov ax, 0x0003
    int 0x10
    
    mov si, step1_msg
    call print_string
    
    ; Step 2: Try to load ONE sector
    mov ax, 0x1000      ; Load to 0x10000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 1           ; JUST 1 sector
    mov ch, 0
    mov cl, 13          ; Sector 13
    mov dh, 0
    mov dl, [boot_drive] ; Use saved drive
    int 0x13
    jc .disk_error
    
    mov si, step2_msg
    call print_string
    
    ; Step 3: Verify we loaded something
    ; Check first byte at 0x10000
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]
    pop ds
    
    ; Print the byte as hex
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
    
    ; Hang
    jmp $

.disk_error:
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

nibble_to_hex:
    cmp al, 10
    jl .digit
    add al, 'A' - 10 - '0'
.digit:
    add al, '0'
    ret

step1_msg: db 'Step 1: Stage2 loaded', 13, 10, 0
step2_msg: db 13, 10, 'Step 2: Loaded sector, first byte: 0x', 0
error_msg: db 13, 10, 'Disk error!', 13, 10, 0
boot_drive: db 0

times 2048-($-$$) db 0
