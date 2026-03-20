@echo off
echo Building test kernel...

REM Assemble bootloader
nasm -f bin src\boot\boot_sect.asm -o build\boot_sect.bin
nasm -f bin src\boot\stage2.asm -o build\stage2.bin

REM Compile kernel
gcc -m32 -ffreestanding -c src\kernel\kernel.c -o build\kernel.o -I src\kernel -I src\drivers
nasm -f elf32 src\boot\kernel_entry.asm -o build\kernel_entry.o

REM Link
ld -m elf_i386 -T linker.ld -o build\kernel.bin -nostdlib build\kernel_entry.o build\kernel.o

REM Create disk image
python tools\make_image.py

echo Done! Run bochs now.
.\build_full.bat run
