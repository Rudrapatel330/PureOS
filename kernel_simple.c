// First byte will be 0xB8 (MOV instruction)
void kernel_main() {
    // This will compile to MOV instructions starting with 0xB8
    volatile char* video = (volatile char*)0xB8000;
    video[0] = 'K';
    video[1] = 0x0A;
    while(1);
}
