// kernel_test.c - MINIMAL TEST FOR BOCHS

// Inline definition of inb/outb to avoid dependency issues during minimal build
static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__("inb %1, %0" : "=a" (result) : "d" (port));
    return result;
}

void kernel_main(void) {
    unsigned char *vram = (unsigned char*)0xA0000;
    
    // Clear to blue (Mode 13h: 320x200, 256 colors)
    for (int i = 0; i < 320*200; i++) {
        vram[i] = 0x01;  // Blue
    }
    
    // Draw "BOCHS OK" pattern (simple diagonal lines)
    for (int i = 0; i < 8; i++) {
        for (int x = 0; x < 8; x++) {
            if (x == i) {
                vram[(20+i)*320 + (20+x)] = 0x0F;  // White
                vram[(20+i)*320 + (30+x)] = 0x0F;
            }
        }
    }
    
    // Draw input area (Black box with White border)
    // White border
    for (int x = 19; x < 301; x++) {
        vram[99*320 + x] = 0x0F;
        vram[120*320 + x] = 0x0F;
    }
    for (int y = 100; y < 120; y++) {
        vram[y*320 + 19] = 0x0F;
        vram[y*320 + 300] = 0x0F;
    }

    // Black box
    for (int y = 100; y < 120; y++) {
        for (int x = 20; x < 300; x++) {
            vram[y*320 + x] = 0x00;
        }
    }
    
    // Keyboard polling test
    int cursor_x = 25;
    int cursor_y = 105;
    
    while(1) {
        // Check keyboard status
        if ((inb(0x64) & 0x01)) {
            unsigned char scancode = inb(0x60);
            
            // Ignore key releases
            if (!(scancode & 0x80)) {
                // Simple 'A' key test
                if (scancode == 0x1E) {  // 'A'
                    vram[cursor_y*320 + cursor_x] = 0x0E;  // Yellow pixel
                    cursor_x += 2;
                    if (cursor_x > 290) {
                        cursor_x = 25;
                        cursor_y += 2;
                    }
                }
                // Enter key
                else if (scancode == 0x1C) {
                    cursor_y += 5;
                    cursor_x = 25;
                }
                // Backspace
                else if (scancode == 0x0E) {
                    if (cursor_x > 25) {
                        cursor_x -= 2;
                        vram[cursor_y*320 + cursor_x] = 0x00;  // Black (erase)
                    }
                }
                // Any other key - Green pixel
                else {
                    vram[cursor_y*320 + cursor_x] = 0x02; // Green
                    cursor_x += 2;
                }
            }
        }
        
        // Small delay
        for (volatile int i = 0; i < 10000; i++);
    }
}
