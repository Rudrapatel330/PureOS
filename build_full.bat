@echo off
echo Building PureOS (Fixing Interrupts)...

set GCC_FLAGS=-m32 -ffreestanding -nostdlib -Isrc/kernel -Isrc/drivers

echo Assembling Bootloaders...
nasm -f bin src/boot/boot_sect.asm -o build/boot_sect.bin
if %errorlevel% neq 0 exit /b 1
nasm -f bin src/boot/stage2.asm -o build/stage2.bin
nasm -f elf32 src/boot/kernel_entry.asm -o build/kernel_entry.o

echo Assembling Stubs...
nasm -f elf32 src/kernel/gdt_flush.asm -o build/gdt_flush.o
if %errorlevel% neq 0 exit /b 1
nasm -f elf32 src/kernel/isr.asm -o build/isr_stubs.o
if %errorlevel% neq 0 exit /b 1
nasm -f elf32 src/kernel/idt_load.asm -o build/idt_load.o
if %errorlevel% neq 0 exit /b 1

echo Compiling Kernel Core...
i686-elf-gcc -I src/include -ffreestanding -m32 -g -c src/kernel/kernel.c -o build/kernel.o -Wall -Wextra
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/kernel/gdt.c -o build/gdt.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/kernel/idt.c -o build/idt.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/kernel/isr.c -o build/isr.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/kernel/string.c -o build/string.o
if %errorlevel% neq 0 (
    echo Warning: string.c failed or missing, skipping...
)

i686-elf-gcc %GCC_FLAGS% -c src/kernel/heap.c -o build/heap.o
if %errorlevel% neq 0 exit /b 1

i686-elf-gcc %GCC_FLAGS% -c src/fs/fat12.c -o build/fat12.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/apps/paint.c -o build/paint.o
if %errorlevel% neq 0 exit /b 1

echo Compiling Drivers...
i686-elf-gcc %GCC_FLAGS% -c src/drivers/vga.c -o build/vga.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/drivers/ports.c -o build/ports.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/drivers/timer.c -o build/timer.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/drivers/keyboard.c -o build/keyboard.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/shell/shell.c -o build/shell.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/drivers/ata.c -o build/ata.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/drivers/vga_gfx.c -o build/vga_gfx.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/drivers/mouse.c -o build/mouse.o
if %errorlevel% neq 0 exit /b 1

i686-elf-gcc %GCC_FLAGS% -c src/kernel/mem.c -o build/mem.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/drivers/rtc.c -o build/rtc.o
if %errorlevel% neq 0 exit /b 1
i686-elf-gcc %GCC_FLAGS% -c src/gui/window.c -o build/window.o
if %errorlevel% neq 0 exit /b 1

echo Linking...
i686-elf-ld -m elf_i386 -T linker.ld --oformat binary -o build/kernel.bin build/kernel_entry.o build/isr_stubs.o build/gdt_flush.o build/idt_load.o build/kernel.o build/gdt.o build/idt.o build/isr.o build/heap.o build/fat12.o build/paint.o build/vga.o build/ports.o build/timer.o build/keyboard.o build/shell.o build/string.o build/ata.o build/vga_gfx.o build/mouse.o build/mem.o build/rtc.o build/window.o
if %errorlevel% neq 0 (
    echo Link Failed!
    pause
    exit /b 1
)

echo Creating OS Image...
python tools/make_image.py
if %errorlevel% neq 0 (
    echo Image Creation Failed!
    exit /b 1
)

echo Deploying to Bochs...
copy /Y test.img D:\bochs\Bochs-3.0\test.img

echo Build Complete! Launching...
D:\bochs\Bochs-3.0\bochs.exe -f D:\bochs\Bochs-3.0\bochsrc.bxrc -q
if %errorlevel% neq 0 (
    echo Bochs Crashed or Failed to Launch!
    pause
    exit /b 1
)
