; simple_os.asm - Complete minimal OS
org 0x7C00
bits 16

start:
    ; Switch to VGA mode 13h (320x200, 256 colors)
    mov ax, 0x13
    int 0x10
    
    ; Set up segments for video memory
    mov ax, 0xA000
    mov es, ax
    
    ; Clear screen to blue
    xor di, di
    mov al, 0x01    ; Blue color
    mov cx, 32000   ; 320*200 pixels (Wait, 320*200=64000 bytes. user said cx, 320*200. CX is 16-bit, max 65535. 64000 fits.
                    ; But rep stosb uses CX. 64000 is fine.)
    mov cx, 64000
    rep stosb
    
    ; Draw "OS TEST" in white - simple diagonal pattern
    mov di, 20*320 + 20
    mov cx, 50
.draw_text:
    mov byte [es:di], 0x0F  ; White
    add di, 321             ; Move diagonally
    loop .draw_text
    
    ; Draw input prompt ">"
    mov di, 100*320 + 20
    mov cx, 8
.draw_prompt:
    mov byte [es:di], 0x0F  ; White
    inc di
    loop .draw_prompt
    
    ; Enable A20 line (Fast A20)
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    jmp CODE_SEG:init_pm

bits 32
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000
    
    ; Now in protected mode - draw something
    mov edi, 0xA0000 + 50*320 + 100
    mov ecx, 20
.draw_pm:
    mov byte [edi], 0x0E    ; Yellow
    add edi, 322
    loop .draw_pm
    
    ; Poll keyboard continuously
keyboard_loop:
    ; Check if keyboard has data
    in al, 0x64
    test al, 0x01
    jz keyboard_loop
    
    ; Read scancode
    in al, 0x60
    
    ; Draw pixel based on scancode (Visual feedback)
    movzx ebx, al
    and ebx, 0x7F           ; Clear release bit
    imul ebx, ebx, 7        ; Spread pixels
    
    mov edi, 0xA0000 + 150*320
    add edi, ebx
    
    ; Check bounds to avoid crash
    cmp edi, 0xA0000 + 64000
    jge .skip_pixel
    
    mov byte [edi], 0x0F    ; White pixel
.skip_pixel:

    ; If 'A' key (scancode 0x1E), draw special Red Line
    cmp al, 0x1E
    jne .not_a
    mov edi, 0xA0000 + 180*320 + 100
    mov ecx, 10
.draw_a:
    mov byte [edi], 0x0C    ; Red
    inc edi
    loop .draw_a
    
.not_a:
    jmp keyboard_loop

; GDT
gdt_start:
    dq 0x0                 ; Null descriptor
gdt_code:
    dw 0xFFFF              ; Limit
    dw 0x0                 ; Base
    db 0x0                 ; Base
    db 10011010b           ; Flags (Code, Readable)
    db 11001111b           ; Granularity (4KB, 32-bit)
    db 0x0                 ; Base
gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0                 ; Base
    db 10010010b           ; Flags (Data, Writable)
    db 11001111b           ; Granularity
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

times 510-($-$$) db 0
dw 0xAA55
