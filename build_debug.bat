@echo off
echo ===== MAXIMUM DEBUG BUILD =====

REM Clean
if exist build mkdir build 2>nul
del /Q build\*.* 2>nul

REM 1. Build with debug
echo 1. Building debug stage2...
nasm -f bin stage2_debug.asm -o build\stage2.bin

REM 2. Build simple kernel
echo 2. Building simple kernel...
i686-elf-gcc -m32 -ffreestanding -c test_simple.c -o build\kernel.o
nasm -f elf32 src\boot\kernel_entry.asm -o build\kernel_entry.o
i686-elf-ld -m elf_i386 -Ttext 0x10000 -o build\kernel.elf build\kernel_entry.o build\kernel.o
i686-elf-objcopy -O binary build\kernel.elf build\kernel.bin

REM 3. Create disk
echo 3. Creating disk...
python make_debug_disk.py
if %errorlevel% neq 0 ( exit /b 1 )

REM 4. Create bochsrc that WORKS
echo 4. Creating guaranteed bochsrc...
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
REM Combine CPU options into one line
echo cpu: reset_on_triple_fault=1
echo display_library: win32
) > build\bochsrc_works.bxrc

echo.
echo ===== WHAT TO EXPECT =====
echo If successful, you should see:
echo   1. "Stage2: Loading kernel..."
echo   2. "Reading sector 13..."
echo   3. "First 4 bytes: 0x55...."
echo   4. "A20... A20 OK"
echo   5. "PM... Switching!"
echo   6. Top-left shows "PM32" (white)
echo   7. Second line shows "CALLING" (green)
echo   8. Center shows "KERNEL OK!" (green)
echo.
pause
