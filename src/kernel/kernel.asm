[BITS 16]
[ORG 0x0000]  ; Loaded at offset 0 in segment 0x1000

kernel_start:
    ; We're in REAL MODE, segment 0x1000
    ; Set up our own segment
    mov ax, cs
    mov ds, ax
    mov es, ax
    
    ; Print directly using BIOS (still in real mode)
    mov ah, 0x0E
    mov al, 'K'
    int 0x10
    mov al, 'E'
    int 0x10
    mov al, 'R'
    int 0x10
    mov al, 'N'
    int 0x10
    mov al, 'E'
    int 0x10
    mov al, 'L'
    int 0x10
    mov al, ' '
    int 0x10
    mov al, 'O'
    int 0x10
    mov al, 'K'
    int 0x10
    
    ; Also write to video memory directly (Green on Black)
    mov ax, 0xB800      ; Video segment
    mov es, ax
    mov di, 0           ; Row 0, Col 0
    
    mov byte [es:di], 'A'
    mov byte [es:di+1], 0x0A
    mov byte [es:di+2], 'S'
    mov byte [es:di+3], 0x0A
    mov byte [es:di+4], 'M'
    mov byte [es:di+5], 0x0A
    
    ; Hang
    cli
    hlt
    
    ; Pad kernel to at least 512 bytes
    times 512-($-kernel_start) db 0
