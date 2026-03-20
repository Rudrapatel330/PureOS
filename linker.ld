OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {
    . = 0xC0100000; /* Kernel Virtual Address (3GB + 1MB) */
    
    .text : AT(0x100000) {
        *(.multiboot)
        *(.text)
    }
    
    .rodata : AT(ADDR(.rodata) - 0xC0000000) {
        *(.rodata*)
    }
    
    .data : AT(ADDR(.data) - 0xC0000000) {
        *(.data)
    }
    
    .bss : AT(ADDR(.bss) - 0xC0000000) {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    }
    
    /DISCARD/ : {
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.note*)
        *(.comment)
    }
    
    _kernel_end = .;
}
