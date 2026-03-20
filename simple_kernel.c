void kernel_main() {
    // Video memory
    char* video_memory = (char*)0xB8000;
    
    // Clear screen with Green background to distinguish from ASM test
    for (int i = 0; i < 80*25*2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i+1] = 0x2F;  // Green background, white text
    }
    
    // Print "C KERNEL OK"
    char* msg = "C KERNEL OK";
    for (int i = 0; msg[i] != 0; i++) {
        video_memory[i*2] = msg[i];
        video_memory[i*2+1] = 0x2F;
    }
    
    // Hang
    while(1) {}
}
