[BITS 16]
[ORG 0x7C00]

start:
    ; Clear screen
    mov ax, 0x0003
    int 0x10
    
    ; Print message
    mov ah, 0x0E
    mov si, msg
print_loop:
    lodsb
    or al, al
    jz hang
    int 0x10
    jmp print_loop

hang:
    cli
    hlt
    jmp hang

msg db 'TEST BOOTLOADER WORKS!', 0

times 510-($-$$) db 0
dw 0xAA55
