[BITS 32]
[ORG 0x20000] ; We will load programs at 128KB mark

start:
    ; Print "HELLO FROM PROGRAM!" to VGA (Line 10)
    mov edi, 0xB8000 + (10 * 160) ; Row 10
    mov esi, msg
    mov ah, 0x0E ; Yellow on Black

.loop:
    lodsb
    or al, al
    jz .done
    mov [edi], ax
    add edi, 2
    jmp .loop

.done:
    ; simple infinite loop for now so we can see it
    ; later we can try 'ret' but need to preserve stack
    jmp $

msg: db 'HELLO FROM EXTERNAL PROGRAM!', 0
