void kernel_main() {
    // Get video memory
    volatile char* video = (volatile char*)0xB8000;
    
    // Clear screen
    for(int i = 0; i < 80*25*2; i += 2) {
        video[i] = ' ';
        video[i+1] = 0x0F;
    }
    
    // Print "KERNEL RUNNING!"
    char* msg = "KERNEL RUNNING!";
    for(int i = 0; i < 15; i++) {
        video[i*2] = msg[i];
        video[i*2+1] = 0x0A;  // Green
    }
    
    // Hang
    while(1);
}
