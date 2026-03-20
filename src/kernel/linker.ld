OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {
    /* The kernel is relocated to 0x100000 by Stage2, and linked at 0xC0100000 virtually */
    . = 0xC0100000;
    _kernel_virtual_start = .;
    
    .text : AT(0x100000) {
        *(.text)
    }
    
    .rodata : AT(0x100000 + (ADDR(.rodata) - 0xC0100000)) {
        *(.rodata)
    }
    
    .data : AT(0x100000 + (ADDR(.data) - 0xC0100000)) {
        *(.data)
    }
    
    .bss : AT(0x100000 + (ADDR(.bss) - 0xC0100000)) {
        *(COMMON)
        *(.bss)
    }
    
    _kernel_end = .;
}
