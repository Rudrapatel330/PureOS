; brute_search.asm
[BITS 16]
[ORG 0x7E00]

start:
    ; Clear screen
    mov ax, 0x0003
    int 0x10
    
    mov si, title
    call print
    
    ; Search from sector 1 to 50
    mov cx, 1          ; Start sector
    mov dh, 0          ; Head 0
    
.search_loop:
    ; Print which sector we're trying (every 5 sectors to avoid spam)
    test cl, 3
    jnz .msg_skip
    
    mov ax, cx
    call print_dec
    mov si, trying
    call print
.msg_skip:

    ; Try to read sector
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 1          ; Read 1 sector
    mov ch, 0          ; Cylinder 0 (we'll handle later)
    ; CL already holds sector number? No wait, CX holds loop counter.
    push cx
    mov cl, cl         ; Sector number (in CL) (Wait, CX is conflated loop/sector here. Let's fix loop logic)
    pop cx
    nop ; Logic fix below in proper loop structure
    
    ; RE-DESIGN LOOP:
    ; CX = Sector number to try (1-50)
    push cx ; Save loop counter
    
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, cl ; Sector matches loop counter
    mov dh, 0
    mov dl, 0x00 ; Floppy A:
    int 0x13
    jc .failed
    
    ; Check if sector has non-zero data
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]
    pop ds
    
    cmp al, 0
    je .empty
    
    ; Found non-zero data!
    pop cx ; Restore stack
    
    mov si, found_msg
    call print
    
    ; Print Sector Number
    mov ax, cx
    call print_dec
    
    mov si, hex_prefix
    call print
    
    ; Print first byte
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]
    call print_hex
    pop ds
    
    mov si, crlf
    call print
    
    jmp .next ; Contine searching to see what else matches

.failed:
    ; mov si, fail_msg
    ; call print
    jmp .next_pop

.empty:
    ; mov si, empty_msg
    ; call print
    jmp .next_pop

.next_pop:
    pop cx

.next:
    ; Next sector
    inc cx
    cmp cx, 50         ; Max sector to check
    jl .search_loop
    
    ; Searched all
    mov si, done_msg
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

print_dec:
    ; Print AL as decimal (0-99)
    push ax
    push bx
    mov ah, 0
    mov bl, 10
    div bl      ; AH = remainder, AL = quotient
    
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

nibble:
    cmp al, 10
    jl .digit
    add al, 'A' - 10 - '0'
.digit:
    add al, '0'
    ret

; ========== MESSAGES ==========
title: db 'Scanning sectors 1-50...', 13, 10, 0
trying: db '.', 0
found_msg: db 13, 10, 'Found Data @ Sec ', 0
hex_prefix: db ': 0x', 0
crlf: db 13, 10, 0
done_msg: db 13, 10, 'Scan Complete.', 13, 10, 0

times 4096-($-$$) db 0
