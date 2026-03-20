[BITS 16]
[ORG 0x7E00]

stage2_start:
    ; Print Stage 2 message
    mov si, msg
    call print_string
    
    ; Hang here for testing
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

msg: db 'Stage 2 Loaded!', 13, 10, 0

; Fill to 3 sectors (1536 bytes)
times 1536-($-$$) db 0
