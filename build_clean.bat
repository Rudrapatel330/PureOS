@echo off
echo ========================================
echo      PUREOS BUILD - DYNAMIC IMAGE
echo ========================================

REM === 1. KILL & CLEANUP (Best Effort) ===
echo Killing Bochs...
taskkill /F /IM bochs.exe >nul 2>&1
taskkill /F /IM bochsdbg.exe >nul 2>&1
timeout /t 1 /nobreak >nul

echo Cleaning old images...
REM Try to delete old images, but continue if locked
echo Y | del /f /q pureos_*.img >nul 2>&1
echo Y | del /f /q bochsout.txt 2>nul
echo Y | del /f /q bochslog.txt 2>nul
echo Y | del /f /q lock 2>nul
echo Y | del /f /q *.lock 2>nul

REM === 2. STATIC IMAGE NAME ===
set IMG_NAME=pureos.img
echo Using disk image: %IMG_NAME%

echo.
echo === Building PureOS ===

REM Clean build dir
echo Y | del /q build\*.* 2>nul
if not exist build mkdir build

REM Assemble
nasm -f bin src\boot\boot.asm -o build\boot.bin
if errorlevel 1 goto error

nasm -f bin src\boot\stage2.asm -o build\stage2.bin
if errorlevel 1 goto error

REM echo Assembling Kernel (ASM Mode)...
REM nasm -f bin src\kernel\kernel.asm -o build\kernel.bin
REM if errorlevel 1 goto error

echo Assembling Entry Point...
nasm -f elf32 src\kernel\entry.asm -o build\entry.o
if errorlevel 1 goto error

echo Assembling User Programs...
nasm -f bin src\programs\hello.asm -o build\hello.bin
if errorlevel 1 goto error

echo Assembling Interrupts...
nasm -f elf32 src\kernel\interrupt.asm -o build\interrupt.o
if errorlevel 1 goto error

echo Compiling Kernel (Minimal 32-bit C)...
i686-elf-gcc -c src\kernel\kernel.c -o build\kernel.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers

echo Compiling Keyboard Driver...
i686-elf-gcc -c src\drivers\keyboard.c -o build\keyboard.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers

echo Compiling String Lib...
i686-elf-gcc -c src\kernel\string.c -o build\string.o -ffreestanding -O2 -Wall -Wextra -I src\kernel

echo Compiling Shell...
i686-elf-gcc -c src\kernel\shell.c -o build\shell.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers -I src\fs

echo Compiling Memory Manager...
i686-elf-gcc -c src\kernel\mem.c -o build\memory.o -ffreestanding -O2 -Wall -Wextra -I src\kernel

echo Compiling Editor...
i686-elf-gcc -c src\kernel\editor.c -o build\editor.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers -I src\fs

echo Compiling VGA Driver...
i686-elf-gcc -c src\drivers\vga.c -o build\vga.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers

echo Compiling Mouse Driver...
i686-elf-gcc -c src\drivers\mouse.c -o build\mouse.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers

echo Compiling Ports...
i686-elf-gcc -c src\drivers\ports.c -o build\ports.o -ffreestanding -O2 -Wall -Wextra -I src\drivers

echo Compiling RTC...
i686-elf-gcc -c src\drivers\rtc.c -o build\rtc.o -ffreestanding -O2 -Wall -Wextra -I src\drivers

echo Compiling Font...
i686-elf-gcc -c src\drivers\font.c -o build\font.o -ffreestanding -O2 -Wall -Wextra -I src\drivers

echo Compiling ATA Driver...
i686-elf-gcc -c src\drivers\ata.c -o build\ata.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers

echo Compiling FAT12 Driver...
i686-elf-gcc -c src\fs\fat12.c -o build\fat12.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers

echo Compiling GUI...
i686-elf-gcc -c src\gui\window.c -o build\window.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers

echo Linking...
i686-elf-gcc -T linker.ld -o build\kernel.elf -ffreestanding -O2 -nostdlib build\entry.o build\kernel.o build\interrupt.o build\keyboard.o build\string.o build\shell.o build\memory.o build\editor.o build\vga.o build\mouse.o build\ports.o build\font.o build\ata.o build\fat12.o build\window.o build\rtc.o -lgcc
if errorlevel 1 goto error

REM Binary extraction
i686-elf-objcopy -O binary build\kernel.elf build\kernel.bin

REM Create Disk
echo Creating disk image...
python tools/create_fat12_disk.py %IMG_NAME%
if errorlevel 1 goto error

echo.
echo === LAUNCHING BOCHS ===
"D:\bochs\Bochs-3.0\bochs.exe" -f bochsrc.txt -q
exit /b 0

REM --- DISABLED SECTIONS ---

echo Compiling Kernel and Drivers...
i686-elf-gcc -c src\kernel\kernel.c -o build\kernel.o -ffreestanding -O2 -Wall -Wextra -I src\kernel -I src\drivers
i686-elf-gcc -c src\kernel\string.c -o build\string.o -ffreestanding -O2 -I src\kernel
i686-elf-gcc -c src\kernel\heap.c -o build\heap.o -ffreestanding -O2 -I src\kernel
i686-elf-gcc -c src\kernel\gdt.c -o build\gdt.o -ffreestanding -O2 -I src\kernel
i686-elf-gcc -c src\kernel\idt.c -o build\idt.o -ffreestanding -O2 -I src\kernel
i686-elf-gcc -c src\kernel\isr.c -o build\isr.o -ffreestanding -O2 -I src\kernel
i686-elf-gcc -c src\drivers\ports.c -o build\ports.o -ffreestanding -O2 -I src\kernel -I src\drivers
i686-elf-gcc -c src\drivers\vga.c -o build\vga.o -ffreestanding -O2 -I src\kernel -I src\drivers
i686-elf-gcc -c src\drivers\timer.c -o build\timer.o -ffreestanding -O2 -I src\kernel -I src\drivers
i686-elf-gcc -c src\drivers\keyboard.c -o build\keyboard.o -ffreestanding -O2 -I src\kernel -I src\drivers
i686-elf-gcc -c src\drivers\mouse.c -o build\mouse.o -ffreestanding -O2 -I src\kernel -I src\drivers
i686-elf-gcc -c src\drivers\ata.c -o build\ata.o -ffreestanding -O2 -I src\kernel -I src\drivers
i686-elf-gcc -c src\fs\fat12.c -o build\fat12.o -ffreestanding -O2 -I src\kernel -I src\drivers

echo Linking...
i686-elf-gcc -T src\kernel\linker.ld -o build\kernel.elf -ffreestanding -O2 -nostdlib ^
    build\kernel_entry.o build\kernel.o ^
    build\interrupt.o build\gdt.o build\idt.o build\isr.o build\ports.o ^
    build\gdt_flush.o build\idt_load.o ^
    build\string.o build\heap.o build\timer.o ^
    build\vga.o build\keyboard.o build\mouse.o build\ata.o ^
    build\fat12.o ^
    -lgcc
if errorlevel 1 goto error

REM Binary
REM Binary
i686-elf-objcopy -O binary build\kernel.elf build\kernel.bin

REM Create Disk
echo Creating disk image...
python tools/create_fat12_disk.py %IMG_NAME%
if errorlevel 1 goto error

REM Bochs Config
echo megs: 32 > bochsrc.txt
echo romimage: file="D:/bochs/Bochs-3.0/BIOS-bochs-latest" >> bochsrc.txt
echo vgaromimage: file="D:/bochs/Bochs-3.0/VGABIOS-lgpl-latest.bin" >> bochsrc.txt
echo floppy_bootsig_check: disabled=0 >> bochsrc.txt
echo floppya: type=1_44, 1_44="%IMG_NAME%", status=inserted, write_protected=0 >> bochsrc.txt
echo boot: floppy >> bochsrc.txt
echo cpu: count=1, ips=10000000 >> bochsrc.txt
echo display_library: win32 >> bochsrc.txt
echo # No panic on triple fault (important!)
panic: action=ignore
error: action=report
debug: action=ignore
info: action=report >> bochsrc.txt

echo.
echo === LAUNCHING BOCHS ===
"D:\bochs\Bochs-3.0\bochs.exe" -f bochsrc.txt
if errorlevel 1 (
    echo Bochs exited with error!
    pause
)
exit /b 0

:error
echo Build failed!
pause
exit /b 1
