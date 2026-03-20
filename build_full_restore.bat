@echo off
echo === Building PureOS (Full Restore) ===

REM 1. Clean
del /Q build\*.bin build\*.img build\*.o build\*.elf 2>nul
mkdir build 2>nul

REM 2. Bootloader
echo Assembling Boot...
nasm -f bin src/boot/boot_sect.asm -o build/boot.bin
nasm -f bin src/boot/stage2.asm -o build/stage2.bin

REM 3. Kernel Entry
echo Assembling Kernel Entry...
nasm -f elf32 src/boot/kernel_entry.asm -o build/kernel_entry.o

REM 4. Kernel Stubs (ASM)
echo Assembling Stubs...
nasm -f elf32 src/kernel/interrupt.asm -o build/interrupt.o
nasm -f elf32 src/kernel/idt_load.asm -o build/idt_load.o
nasm -f elf32 src/kernel/gdt_flush.asm -o build/gdt_flush.o

REM 5. Compile C Kernel Core & Drivers
echo Compiling C Code...
REM Added -I src/kernel to find types.h
set FLAGS=-I src/include -I src/kernel -ffreestanding -O2 -Wall -Wextra -m32 -c

i686-elf-gcc %FLAGS% src/kernel/kernel.c -o build/kernel.o
i686-elf-gcc %FLAGS% src/kernel/isr.c -o build/isr.o
i686-elf-gcc %FLAGS% src/kernel/idt.c -o build/idt.o
i686-elf-gcc %FLAGS% src/kernel/gdt.c -o build/gdt.o
i686-elf-gcc %FLAGS% src/kernel/heap.c -o build/heap.o
REM i686-elf-gcc %FLAGS% src/kernel/mem.c -o build/mem.o  <-- Removed Duplicate
i686-elf-gcc %FLAGS% src/kernel/string.c -o build/string.o

echo Compiling Drivers...
i686-elf-gcc %FLAGS% src/drivers/vga.c -o build/vga.o
i686-elf-gcc %FLAGS% src/drivers/keyboard.c -o build/keyboard.o
i686-elf-gcc %FLAGS% src/drivers/timer.c -o build/timer.o
i686-elf-gcc %FLAGS% src/drivers/ports.c -o build/ports.o
i686-elf-gcc %FLAGS% src/drivers/ata.c -o build/ata.o

echo Compiling Shell & FS...
i686-elf-gcc %FLAGS% src/shell/shell.c -o build/shell.o
i686-elf-gcc %FLAGS% src/fs/fat12.c -o build/fat12.o

REM 6. Link
echo Linking...
i686-elf-gcc -T linker.ld -o build/kernel.elf -ffreestanding -O2 -nostdlib ^
    build/kernel_entry.o ^
    build/interrupt.o build/idt_load.o build/gdt_flush.o ^
    build/kernel.o build/isr.o build/idt.o build/gdt.o ^
    build/heap.o build/string.o ^
    build/vga.o build/keyboard.o build/timer.o build/ports.o build/ata.o ^
    build/shell.o build/fat12.o ^
    -lgcc -m32

if errorlevel 1 goto error

REM 7. Binary Extraction
echo Extracting Binary...
i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin

REM 8. Create Disk
echo Creating Disk...
python tools/make_image_simple.py

REM 9. Launch
echo.
echo === LAUNCHING BOCHS ===
pause
D:\bochs\Bochs-3.0\bochs.exe -q -f bochsrc.txt

pause
goto end

:error
echo Build Failed!
pause
exit /b 1

:end
