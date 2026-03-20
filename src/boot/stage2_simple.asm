[BITS 16]
[ORG 0x7E00]

stage2_start:
    ; Clear screen first
    mov ax, 0x0003
    int 0x10
    
    ; Print stage2 message
    mov si, stage2_msg
    call print_string
    
    ; DEBUG: Show we're alive
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], '2'   ; Cyan '2'
    mov byte [es:1], 0x0B
    
    ; ================================
    ; STEP 1: LOAD TEST KERNEL
    ; ================================
    mov si, load_kernel_msg
    call print_string
    
    ; Load simple test kernel to 0x10000
    mov ax, 0x1000      ; Segment for 0x10000
    mov es, ax
    xor bx, bx
    
    ; Load from sector 13 (after boot + stage2 (4KB/8 sectors))
    mov ah, 0x02
    mov al, 4           ; 4 sectors = 2KB
    mov ch, 0           ; Cylinder 0
    mov cl, 13          ; Sector 13
    mov dh, 0           ; Head 0
    mov dl, [boot_drive]        ; Use saved boot drive
    int 0x13
    jc disk_error
    
    ; DEBUG: Show kernel loaded
    mov ax, 0xB800
    mov es, ax
    mov byte [es:2], 'K'
    mov byte [es:3], 0x0E  ; Yellow
    
    mov si, kernel_loaded_msg
    call print_string
    
    ; ================================
    ; STEP 2: ENABLE A20 LINE
    ; ================================
    mov si, a20_msg
    call print_string
    
    mov ax, 0x2401
    int 0x15
    jc a20_error
    
    ; ================================
    ; STEP 3: SWITCH TO PROTECTED MODE
    ; ================================
    mov si, pm_msg
    call print_string
    
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Far jump to 32-bit code
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
    
    ; Clear screen and show we're in PM
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ax, 0x0F20
    rep stosw
    
    ; Print "PM32 OK"
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F
    mov byte [0xB8002], 'M'
    mov byte [0xB8003], 0x0F
    mov byte [0xB8004], '3'
    mov byte [0xB8005], 0x0F
    mov byte [0xB8006], '2'
    mov byte [0xB8007], 0x0F
    mov byte [0xB8008], ' '
    mov byte [0xB8009], 0x0F
    mov byte [0xB80010], 'O'
    mov byte [0xB80011], 0x0F
    mov byte [0xB80012], 'K'
    mov byte [0xB80013], 0x0F
    
    ; Jump to kernel at 0x10000
    jmp 0x10000

[BITS 16]
disk_error:
    mov si, disk_error_msg
    call print_string
    mov ax, 0xB800
    mov es, ax
    mov byte [es:4], 'E'
    mov byte [es:5], 0x0C
    jmp $

a20_error:
    mov si, a20_error_msg
    call print_string
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

; Messages
stage2_msg: db 'Stage 2 Loaded!', 13, 10, 0
load_kernel_msg: db 'Loading kernel...', 13, 10, 0
kernel_loaded_msg: db 'Kernel loaded!', 13, 10, 0
a20_msg: db 'Enabling A20...', 13, 10, 0
pm_msg: db 'Switching to PM...', 13, 10, 0
disk_error_msg: db 'Disk error!', 13, 10, 0
a20_error_msg: db 'A20 error!', 13, 10, 0
boot_drive: db 0

; GDT
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

; Fill to 4KB (8 sectors)
times 4096-($-$$) db 0
