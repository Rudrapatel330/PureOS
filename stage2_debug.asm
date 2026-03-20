[BITS 16]
[ORG 0x7E00]

start:
    ; Clear screen
    mov ax, 0x0003
    int 0x10
    
    mov si, msg1
    call print
    
    ; ========== LOAD KERNEL ==========
    mov si, msg_load
    call print
    
    mov ax, 0x1000      ; Segment for 0x10000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, 16          ; Load 16 sectors (8KB)
    mov ch, 0
    mov cl, 13          ; SECTOR 13
    mov dh, 0
    mov dl, 0x00
    int 0x13
    jc disk_error
    
    ; DEBUG: Show we loaded something
    mov si, msg_loaded
    call print
    
    ; Show first 4 bytes
    push ds
    mov ax, 0x1000
    mov ds, ax
    
    mov al, [0x0000]
    call print_hex
    mov al, [0x0001]
    call print_hex  
    mov al, [0x0002]
    call print_hex
    mov al, [0x0003]
    call print_hex
    
    pop ds
    
    ; ========== ENABLE A20 ==========
    mov si, msg_a20
    call print
    
    ; Try multiple A20 methods
    call enable_a20
    jc a20_error
    
    mov si, msg_a20_ok
    call print
    
    ; ========== SWITCH TO PM ==========
    mov si, msg_pm
    call print
    
    cli
    lgdt [gdt_descriptor]
    
    ; DEBUG: Before switch
    mov si, msg_switch
    call print
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Jump to 32-bit
    jmp 0x08:protected_mode

[BITS 32]
protected_mode:
    ; Set up segments IMMEDIATELY before touching memory!
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set stack
    mov esp, 0x90000

    ; DEBUG: Show we're in PM (direct to video)
    mov eax, 0xB8000
    mov byte [eax], 'P'
    mov byte [eax+1], 0x0F
    mov byte [eax+2], 'M'
    mov byte [eax+3], 0x0F
    mov byte [eax+4], '3'
    mov byte [eax+5], 0x0F
    mov byte [eax+6], '2'
    mov byte [eax+7], 0x0F
    
    ; Clear rest of screen
    mov edi, 0xB8000 + 80*2
    mov ecx, 80*24
    mov ax, 0x0F20
    rep stosw
    
    ; Print "KERNEL CALL"
    mov byte [0xB8000 + 160], 'C'
    mov byte [0xB8000 + 161], 0x0A
    mov byte [0xB8000 + 162], 'A'
    mov byte [0xB8000 + 163], 0x0A
    mov byte [0xB8000 + 164], 'L'
    mov byte [0xB8000 + 165], 0x0A
    mov byte [0xB8000 + 166], 'L'
    mov byte [0xB8000 + 167], 0x0A
    mov byte [0xB8000 + 168], 'I'
    mov byte [0xB8000 + 169], 0x0A
    mov byte [0xB8000 + 170], 'N'
    mov byte [0xB8000 + 171], 0x0A
    mov byte [0xB8000 + 172], 'G'
    mov byte [0xB8000 + 173], 0x0A
    
    ; Call kernel
    call 0x10000
    
    ; If kernel returns
    mov byte [0xB8000 + 320], 'R'
    mov byte [0xB8000 + 321], 0x0C
    mov byte [0xB8000 + 322], 'E'
    mov byte [0xB8000 + 323], 0x0C
    mov byte [0xB8000 + 324], 'T'
    mov byte [0xB8000 + 325], 0x0C
    
    cli
.hang:
    hlt
    jmp .hang

[BITS 16]
enable_a20:
    ; Try BIOS method
    mov ax, 0x2401
    int 0x15
    ret

disk_error:
    mov si, msg_disk_err
    call print
    jmp hang

a20_error:
    mov si, msg_a20_err
    call print
    jmp hang

hang:
    jmp $

; ========== 16-bit functions ==========
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

nibble:
    cmp al, 10
    jl .digit
    add al, 'A' - 10 - '0'
.digit:
    add al, '0'
    ret

; ========== Messages ==========
msg1: db 'Stage2: Loading kernel...', 13, 10, 0
msg_load: db 'Reading sector 13...', 13, 10, 0
msg_loaded: db 'First 4 bytes: 0x', 0
msg_a20: db 'A20...', 13, 10, 0
msg_a20_ok: db 'A20 OK', 13, 10, 0
msg_pm: db 'PM...', 13, 10, 0
msg_switch: db 'Switching!', 13, 10, 0
msg_disk_err: db 'Disk error!', 13, 10, 0
msg_a20_err: db 'A20 error!', 13, 10, 0

; ========== GDT ==========
gdt_start:
    dq 0
gdt_code:
    dw 0xFFFF
    dw 0
    db 0
    db 0x9A
    db 0xCF
    db 0
gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 0x92
    db 0xCF
    db 0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 4096-($-$$) db 0
