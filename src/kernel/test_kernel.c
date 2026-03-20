void kernel_main() {
    // Simple VGA output
    volatile char* vga = (volatile char*)0xB8000;
    
    // Clear screen
    for(int i = 0; i < 80*25*2; i += 2) {
        vga[i] = ' ';
        vga[i+1] = 0x07;
    }
    
    // Write a message
    const char* msg = "KERNEL LOADED SUCCESSFULLY!";
    for(int i = 0; msg[i]; i++) {
        vga[i*2] = msg[i];
        vga[i*2 + 1] = 0x1F;  // White on blue
    }
    
    // Hang
    while(1);
}
