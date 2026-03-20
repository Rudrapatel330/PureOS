@echo off
echo Building for Bochs...

echo Assembling Bootloader...
nasm -f bin src/boot/boot_sect.asm -o build/boot_sect.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo Assembling Stage 2...
nasm -f bin src/boot/stage2.asm -o build/stage2.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Test Kernel...
i686-elf-gcc -m32 -ffreestanding -nostdlib -c kernel_test.c -o build/kernel_test.o
if %errorlevel% neq 0 exit /b %errorlevel%

echo Assembling Kernel Entry...
nasm -f elf32 src/kernel/pure_kernel.asm -o build/pure_kernel.o
if %errorlevel% neq 0 exit /b %errorlevel%

echo Linking...
i686-elf-ld -m elf_i386 -T linker.ld build/pure_kernel.o build/kernel_test.o -o build/kernel.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo Creating OS Image...
copy /b build\boot_sect.bin+build\stage2.bin+build\kernel.bin os-image.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo Build Complete! os-image.bin created.
