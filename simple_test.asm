; simple_test.asm
org 0x7C00
bits 16

start:
    ; Set text mode
    mov ax, 0x0003
    int 0x10
    
    ; Print message
    mov si, msg
print_loop:
    lodsb
    test al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_loop

done:
    ; Wait for key
    mov ah, 0x00
    int 0x16
    
    ; Display the key
    mov ah, 0x0E
    int 0x10
    
    ; Print confirmation
    mov si, confirm
print_loop2:
    lodsb
    test al, al
    jz hang
    mov ah, 0x0E
    int 0x10
    jmp print_loop2

hang:
    jmp $

msg db 'Bochs Test: Press any key', 13, 10, 0
confirm db ' key pressed!', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55