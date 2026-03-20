[BITS 16]
[ORG 0x7E00]

start:
    ; Clear screen
    mov ax, 0x0003
    int 0x10
    
    mov si, loading_msg
    call print
    
    ; ========== LOAD KERNEL FROM SECTOR 13 ==========
    mov si, load_msg
    call print
    
    mov ax, 0x1000      ; Segment for 0x10000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02        ; Read sectors
    mov al, 8           ; Read 8 sectors (4KB - enough for kernel)
    mov ch, 0           ; Cylinder 0
    mov cl, 13          ; SECTOR 13 (where kernel actually is!)
    mov dh, 0           ; Head 0
    mov dl, 0x00        ; Floppy A:
    int 0x13
    jc disk_error
    
    ; Verify we got kernel (first byte should be 0x55)
    push ds
    mov ax, 0x1000
    mov ds, ax
    mov al, [0x0000]    ; First byte at 0x10000
    pop ds
    
    mov si, verify_msg
    call print
    call print_hex
    
    cmp al, 0x55
    je kernel_ok
    
    ; Wrong byte
    mov si, wrong_byte
    call print
    jmp hang

kernel_ok:
    mov si, success_msg
    call print
    
    ; ========== ENABLE A20 ==========
    mov si, a20_msg
    call print
    
    mov ax, 0x2401
    int 0x15
    jc a20_error
    
    ; ========== SWITCH TO PROTECTED MODE ==========
    mov si, pm_msg
    call print
    
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Jump to 32-bit code
    jmp 0x08:protected_mode

[BITS 32]
protected_mode:
    ; Set up segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set stack
    mov esp, 0x90000
    
    ; Clear screen and show PM message
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ax, 0x0F20      ; White space on black
    rep stosw
    
    ; Print "PM OK"
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F
    mov byte [0xB8002], 'M'
    mov byte [0xB8003], 0x0F
    mov byte [0xB8004], ' '
    mov byte [0xB8005], 0x0F
    mov byte [0xB8006], 'O'
    mov byte [0xB8007], 0x0F
    mov byte [0xB8008], 'K'
    mov byte [0xB8009], 0x0F
    
    ; Jump to kernel at 0x10000
    jmp 0x10000

[BITS 16]
disk_error:
    mov si, disk_err_msg
    call print
    jmp hang

a20_error:
    mov si, a20_err_msg
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

nibble:
    cmp al, 10
    jl .digit
    add al, 'A' - 10 - '0'
.digit:
    add al, '0'
    ret

; ========== MESSAGES ==========
loading_msg: db 'Stage2 running...', 13, 10, 0
load_msg: db 'Loading kernel from sector 13...', 13, 10, 0
verify_msg: db 'First byte: 0x', 0
success_msg: db ' (Kernel OK!)', 13, 10, 0
wrong_byte: db ' (Wrong byte!)', 13, 10, 0
a20_msg: db 'Enabling A20...', 13, 10, 0
pm_msg: db 'Switching to PM...', 13, 10, 0
disk_err_msg: db 'Disk error!', 13, 10, 0
a20_err_msg: db 'A20 error!', 13, 10, 0

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
