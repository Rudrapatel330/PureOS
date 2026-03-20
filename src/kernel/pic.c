#include "../drivers/ports.h"
#include "isr.h"

// Initialize the PIC (Programmable Interrupt Controller)
void pic_init() {
    // ICW1: Init command
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    // ICW2: Vector Offsets
    // Master -> 32 (0x20)
    // Slave -> 40 (0x28)
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    
    // ICW3: Cascade setup
    outb(0x21, 0x04); // Master has slave on IRQ2
    outb(0xA1, 0x02); // Slave identity
    
    // ICW4: 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // OCW1: Unmask Interrupts
    // Unmask Timer (0), Keyboard (1), Cascade (2) and Mouse (12)
    outb(0x21, 0xF0); // Enable IRQ0,1,2,3,4,5,6,7
    
    // Slave: Unmask IRQ12 (mouse) - enable mouse interrupts, and IRQ10 (NE2000)
    outb(0xA1, 0xEB); // Enable IRQ12 (bit 4) and IRQ10 (bit 2)
}
