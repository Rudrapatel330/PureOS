[bits 16]

; VBE Info Block (512 bytes)
VBE_INFO_BLOCK equ 0x8000
; Mode Info Block (256 bytes)
MODE_INFO_BLOCK equ 0x9000

switch_to_vbe_graphics:
    mov si, msg_attempt_gfx
    call print_string

    ; 1. Check VBE Availability
    mov ax, 0x4F00
    mov di, VBE_INFO_BLOCK
    int 0x10
    cmp ax, 0x004F
    jne .vbe_not_available

    mov si, msg_vbe_ok
    call print_string

    ; 2. Try Modes
    ; FOR DEBUGGING: SKIP VBE MODES AND FORCE VGA 13h
    jmp .vbe_not_available
    
    ; Try 1024x768x32 (Mode 0x4118)
    ; mov cx, 0x4118
    ; call try_set_mode
    ; jc .success
    
    ; mov cx, 0x4115 ; 800x600x32
    ; call try_set_mode
    ; jc .success
    
    ; mov cx, 0x4112 ; 640x480x32
    ; call try_set_mode
    ; jc .success

    ; If we get here, VBE failed. Fallthrough to VGA.

.vbe_not_available:
    mov si, msg_vbe_fail
    call print_string
    ; Fallback to VGA 0x13
    jmp setup_vga_mode_13h

.success:
    mov si, msg_mode_success
    call print_string
    
    ; Store Info for Kernel at 0x7E04
    ; Framebuffer
    mov eax, [MODE_INFO_BLOCK + 40]
    mov [0x7E04], eax
    
    ; Width
    mov ax, [MODE_INFO_BLOCK + 18]
    mov [0x7E08], ax
    
    ; Height
    mov ax, [MODE_INFO_BLOCK + 20]
    mov [0x7E0A], ax
    
    ; BPP
    mov al, [MODE_INFO_BLOCK + 25]
    mov [0x7E0C], al
    
    ; Pitch
    mov ax, [MODE_INFO_BLOCK + 16]
    mov [0x7E0E], ax

    ret

; Input: CX = Mode Number (including LFB bit if needed, but we OR it here)
; Output: Carry Set on Success, Clear on Failure
try_set_mode:
    pusha
    
    ; Get Mode Info
    mov ax, 0x4F01
    mov di, MODE_INFO_BLOCK
    int 0x10
    cmp ax, 0x004F
    jne .failed

    ; Set Mode
    mov ax, 0x4F02
    mov bx, cx
    or bx, 0x4000 ; LFB
    int 0x10
    cmp ax, 0x004F
    jne .failed

    popa
    stc ; Success
    ret

.failed:
    popa
    clc ; Fail
    ret

setup_vga_mode_13h:
    mov si, msg_vga_fallback
    call print_string
    
    mov ax, 0x0013
    int 0x10
    
    ; DEBUG: FILL SCREEN BLUE/GREEN IN REAL MODE
    ; This verifies if Mode 13h works at all
    mov ax, 0xA000
    mov es, ax
    xor di, di
    
    ; First half Blue (32000 pixels)
    mov al, 1     ; Blue
    mov cx, 32000
    rep stosb
    
    ; Second half Green (32000 pixels)
    mov al, 2     ; Green
    mov cx, 32000
    rep stosb
    
    ; HANG REMOVED - Return to caller to proceed to PM & Kernel
    ; jmp $
    
    ret
    
    ; Store Info for Kernel (Unreachable now but keeps offsets)
    mov dword [0x7E04], 0xA0000 ; VGA Memory
    mov word [0x7E08], 320
    mov word [0x7E0A], 200
    mov byte [0x7E0C], 8       ; 8 BPP
    mov word [0x7E0E], 320     ; Pitch
    
    ret

msg_attempt_gfx db "Attempting graphics switch...", 13, 10, 0
msg_vbe_ok db "VBE Available.", 13, 10, 0
msg_vbe_fail db "VBE Failed.", 13, 10, 0
msg_mode_success db "Graphics Mode Set!", 13, 10, 0
msg_vga_fallback db "Falling back to VGA 13h...", 13, 10, 0
