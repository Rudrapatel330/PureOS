[BITS 16]
[ORG 0x7C00]

start_jump:
    jmp short start
    nop

    ; === BIOS PARAMETER BLOCK (BPB) placeholder ===
    ; The create_fat12_disk.py script will overwrite these values.
    b_oem           db 'PUREOS  '   ; 8 bytes
    b_bytes_per_sec dw 512
    b_sec_per_clus  db 1
    b_reserved_sec  dw 1
    b_fat_count     db 2
    b_root_entries  dw 224
    b_total_sec     dw 2880
    b_media         db 0xF0
    b_sec_per_fat   dw 9
    b_sec_per_trk   dw 18
    b_num_heads     dw 2
    b_hidden_sec    dd 0
    b_total_sec_big dd 0
    b_drive_num     db 0
    b_reserved      db 0
    b_boot_sig      db 0x29
    b_vol_id        dd 0x12345678
    b_vol_label     db 'PUREOS BOOT' ; 11 bytes
    b_fs_type       db 'FAT12   '    ; 8 bytes

start:
    ; Disable interrupts
    cli
    
    ; Set up stack
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFF
    
    ; Enable interrupts
    sti

    ; Save boot drive
    mov [boot_drive], dl
    
    ; Print loading message
    mov si, loading_msg
    call print_string
    
    ; Load stage2 (sectors 1-3)
    ; FROM: Sector 2 (1-based)
    ; SIZE: 3 Sectors
    mov ah, 0x02        ; BIOS read sector function
    mov al, 100         ; Number of sectors to read (100 = 50KB)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Sector 2 (sector 1 is bootloader)
    mov dh, 0           ; Head 0
    mov dl, [boot_drive] ; Drive
    mov bx, 0x7E00      ; Load at 0x7E00 (right after bootloader)
    int 0x13
    
    jc disk_error       ; Jump if error (carry flag set)
    
    ; Success - jump to stage2
    mov si, success_msg
    call print_string
    jmp 0x7E00

disk_error:
    mov si, error_msg
    call print_string
    
    ; Print error code
    mov al, ah          ; Error code is in AH
    call print_hex
    jmp $               ; Hang

print_string:
    mov ah, 0x0E        ; BIOS teletype function
.print_char:
    lodsb               ; Load next character
    or al, al           ; Check for null terminator
    jz .done
    int 0x10            ; Print character
    jmp .print_char
.done:
    ret

print_hex:
    ; Print AL as hex
    pusha
    mov cx, 2           ; Print 2 hex digits
.hex_loop:
    rol al, 4           ; Rotate to get next digit
    mov bl, al
    and bl, 0x0F        ; Get low nibble
    cmp bl, 10
    jl .digit
    add bl, 7           ; Convert to A-F
.digit:
    add bl, '0'         ; Convert to ASCII
    mov ah, 0x0E
    mov al, bl
    int 0x10
    loop .hex_loop
    popa
    ret

; Data
loading_msg:   db 'Loading stage2...', 13, 10, 0
success_msg:   db 'Stage2 loaded!', 13, 10, 0
error_msg:     db 'Disk error: 0x', 0
boot_drive:    db 0

; Fill with zeros
times 510-($-$$) db 0

; Boot signature
dw 0xAA55
