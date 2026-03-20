; stage2.asm - ROBUST CHS LOAD + VESA 16bpp
ORG 0x7E00
BITS 16

start:
    ; Save boot drive passed from stage1 (DL)
    mov [boot_drive], dl

    ; Serial trace '2' (Stage 2)
    mov al, '2'
    mov dx, 0x3F8
    out dx, al

    ; Setup segments (Standardize)
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Print "Stage2"
    mov si, msg_start
    call print_string
    mov si, msg_start
    call ser_print_string

    ; ============= VESA INIT =============
    ; Set video mode 0x117 (1024x768x16bpp)
    mov ax, 0x4F02
    mov bx, 0x4117
    or bx, 0x4000
    int 0x10
    cmp ax, 0x004F
    jne vesa_failed

    ; Serial trace 'V' (VESA OK)
    mov al, 'V'
    mov dx, 0x3F8
    out dx, al

    ; Get/Store FB Info
    mov ax, 0x4F01
    mov cx, 0x4117
    mov di, mode_info
    int 0x10
    
    mov eax, [mode_info + 40]
    mov [0x6000], eax

    ; ============= UNREAL MODE SETUP =============
    mov al, 'U'
    mov dx, 0x3F8
    out dx, al

    cli
    lgdt [gdt_desc]     ; Load Flat GDT
    mov eax, cr0
    or al, 1
    mov cr0, eax        ; Enter P-Mode
    jmp $+2             ; Flush pipeline

    mov bx, 0x10        ; Data Segment (4GB Flat)
    mov ds, bx          ; Load DS with 4GB limit
    mov es, bx          ; Load ES with 4GB limit
    mov fs, bx          ; Load FS with 4GB limit
    mov gs, bx          ; Load GS with 4GB limit
    
    and al, 0xFE
    mov cr0, eax        ; Return to Real Mode
    jmp $+2             ; Flush pipeline
    
    ; Setup segment registers for BIOS/Stage2 logic (Base 0, 64KB default)
    ; But FS and GS will retain their 4GB limits from P-Mode cache!
    xor ax, ax
    mov ds, ax
    mov es, ax
    sti

    mov al, 'T'
    mov dx, 0x3F8
    out dx, al

    ; ============= KERNEL LOAD (LBA) =============
    mov dword [kernel_target_ptr], 0x100000 ; Start at 1MB
    mov dword [lba_sector], 4               ; Kernel starts at Sector 4
    
    ; The sector count is injected into Sector 3 (0x83FC)
    mov eax, [0x83FC]
    mov [sectors_to_load], eax

    mov al, 'C'
    mov dx, 0x3F8
    out dx, al

load_loop:
    ; 1. Calculate how many sectors to read (max 64 sectors = 32KB)
    mov eax, [sectors_to_load]
    test eax, eax
    jz load_finished
    
    cmp eax, 64
    jbe .last_chunk
    mov word [dap_count], 64
    jmp .read
.last_chunk:
    mov [dap_count], ax 
    
.read:
    ; 2. Set DAP (Disk Address Packet)
    mov word [dap_segment], 0x1000 ; Into 0x1000:0000 = 0x10000
    mov word [dap_offset], 0x0000
    
    mov eax, [lba_sector]
    mov [dap_lba_low], eax
    ; dap_lba_high is 0
    
    ; 3. BIOS Extended Read
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap_size
    int 0x13
    jc load_err
    
    ; 4. Unreal Mode Burst Copy (from ES:0000 to GS:EDI)
    movzx ecx, word [dap_count]
    imul ecx, 128 ; (sectors * 512) / 4 = sectors * 128 dwords
    
    mov edi, [kernel_target_ptr]
    mov esi, 0x10000 ; Source physical address: ES:0000 = 0x1000:0x0000 = 0x10000
    
.copy_loop:
    mov eax, [esi]
    mov [gs:edi], eax
    add esi, 4
    add edi, 4
    loop .copy_loop

    ; 5. Update iteration state
    movzx eax, word [dap_count]
    add [lba_sector], eax
    sub [sectors_to_load], eax
    
    shl eax, 9 ; Offset = sectors * 512
    add [kernel_target_ptr], eax
    
    ; 6. Heartbeat '+'
    mov al, '+'
    mov dx, 0x3F8
    out dx, al
    
    jmp load_loop

load_finished:
    ; Total loaded: 15000 sectors = 7.5MB
    
    ; Serial trace 'L' (Loaded)
    mov al, 'L'
    mov dx, 0x3F8
    out dx, al

    mov si, msg_load_ok
    call print_string

    ; ============= PMODE SWITCH =============
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp dword 0x08:pmode

vesa_failed:
    ; Serial trace 'F' (VESA FAIL)
    mov al, 'F'
    mov dx, 0x3F8
    out dx, al
    jmp $

load_err:
    ; VESA is active, so we just write red pixels to the framebuffer to indicate an error
    mov edi, [0x6000]       ; Framebuffer address saved earlier
    mov ecx, 1024 * 768 / 2
    mov eax, 0xF800F800     ; Red pixels
    rep stosd
    jmp $

; UTILS
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

ser_print_string:
    push dx
    push ax
    mov dx, 0x3F8
.ser_loop:
    lodsb
    or al, al
    jz .ser_done
    out dx, al
    jmp .ser_loop
.ser_done:
    pop ax
    pop dx
    ret

boot_drive db 0
sectors_to_load dd 0
kernel_target_ptr dd 0

; Disk Address Packet
align 4
dap_size db 0x10
dap_zero db 0
dap_count dw 0
dap_offset dw 0
dap_segment dw 0
lba_sector:
dap_lba_low dd 0
dap_lba_high dd 0
msg_start db 'S2', 0
msg_load_ok db 'K', 0

align 4
gdt:
    dw 0, 0, 0, 0
    dw 0xFFFF, 0, 0x9A00, 0x00CF
    dw 0xFFFF, 0, 0x9200, 0x00CF
gdt_desc:
    dw $ - gdt - 1
    dd gdt

mode_info: times 256 db 0

BITS 32
pmode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Kernel is ALREADY loaded at 0x100000 due to Unreal Mode copy!
    ; Serial trace 'K' (Kernel Jump)
    mov al, 'K'
    mov dx, 0x3F8
    out dx, al

    jmp dword 0x08:0x100000

times 1536-($-$$) db 0
