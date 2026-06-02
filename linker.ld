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
    
    .init_array : AT(ADDR(.init_array) - 0xC0000000) {
        __init_array_start = .;
        KEEP(*(.init_array))
        KEEP(*(SORT(.init_array.*)))
        __init_array_end = .;
    }
    
    .ctors : AT(ADDR(.ctors) - 0xC0000000) {
        __ctors_start = .;
        KEEP(*(.ctors))
        KEEP(*(SORT(.ctors.*)))
        __ctors_end = .;
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
