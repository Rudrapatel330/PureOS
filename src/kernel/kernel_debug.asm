[BITS 32]
[ORG 0x1000]

start:
    ; DEBUG: Kernel Start
    mov al, 'K'
    out 0xe9, al
    mov al, 'O'
    out 0xe9, al
    mov al, 'K'
    out 0xe9, al
    ; Set video memory address
    mov edi, 0xB8000
    
    ; Write "KERNEL OK!" in green on black
    ; K E R N E L   O K !
    mov dword [edi], 0x024B0245      ; E K (Little Endian: K then E) -> 'K' 'E'
    mov dword [edi+4], 0x024E0252    ; N R -> 'R' 'N'
    mov dword [edi+8], 0x024C0245    ; L E -> 'E' 'L'
    mov dword [edi+12], 0x02200220   ; Space Space
    mov dword [edi+16], 0x024B024F   ; K O -> 'O' 'K'
    mov dword [edi+20], 0x02210221   ; ! !
    
    ; HANG forever
    cli
    hlt
