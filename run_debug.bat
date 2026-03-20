@echo off
echo ===== PROPER DEBUG RUN =====

REM 1. Clean previous build
if exist build rmdir /s /q build
mkdir build

REM 2. Compile components
echo Compiling...
nasm -f bin src\boot\boot_sect.asm -o build\boot.bin
nasm -f bin stage2_debug.asm -o build\stage2.bin
i686-elf-gcc -m32 -ffreestanding -c test_simple.c -o build\kernel.o
nasm -f elf32 src\boot\kernel_entry.asm -o build\kernel_entry.o
i686-elf-ld -m elf_i386 -Ttext 0x10000 -o build\kernel.elf build\kernel_entry.o build\kernel.o
i686-elf-objcopy -O binary build\kernel.elf build\kernel.bin

REM 3. Create Disk Image
echo Creating Disk Image...
python make_debug_disk.py

REM 4. Create Configuration
echo Creating Config...
(
echo megs: 32
echo romimage: file=D:\bochs\Bochs-3.0\BIOS-bochs-latest
echo vgaromimage: file=D:\bochs\Bochs-3.0\VGABIOS-lgpl-latest
echo boot: floppy
echo floppya: 1_44=build/pureos_test.img, status=inserted
echo log: bochslog.txt
echo panic: action=report
echo error: action=report
echo info: action=report
echo cpu: reset_on_triple_fault=1
echo display_library: win32
) > build\bochsrc_debug.bxrc

REM 5. Launch
echo.
echo LAUNCHING BOCHS...
echo Look for "PM32" and "KERNEL OK!" on the screen.
"D:\bochs\Bochs-3.0\bochs.exe" -f build\bochsrc_debug.bxrc -q
pause
