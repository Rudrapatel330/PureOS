@echo off
cd /d d:\1os
echo ===== FRESH BUILD (FIXED) =====

REM Clean
if exist build mkdir build 2>nul
del /Q build\*.* 2>nul

REM 1. Build boot sector
echo 1. Building boot sector...
nasm -f bin boot_sect.asm -o build\boot.bin

REM 2. Build stage2
echo 2. Building stage2...
nasm -f bin stage2_final.asm -o build\stage2.bin

REM 3. Build kernel
echo 3. Building kernel...
i686-elf-gcc -m32 -ffreestanding -c kernel_simple.c -o build\kernel.o
i686-elf-ld -m elf_i386 -Ttext 0x10000 -o build\kernel.elf build\kernel.o
i686-elf-objcopy -O binary build\kernel.elf build\kernel.bin

REM 4. Create disk image MANUALLY
echo 4. Creating disk image...

REM Calculation:
REM Boot sector = 512 bytes (offset 0-511)
REM Stage2      = 4096 bytes (offset 512-4607)
REM Kernel needs to be at sector 100 = offset 51200
REM Padding needed = 51200 - 4608 = 46592 bytes

python -c "import struct; f=open('build/pureos.img','wb'); f.write(open('build/boot.bin','rb').read()); f.write(open('build/stage2.bin','rb').read()); f.write(b'\x00'*(51200-512-4096)); f.write(open('build/kernel.bin','rb').read()); f.seek(1474560-1); f.write(b'\x00'); f.close()"

echo.
echo ===== VERIFICATION =====
echo Checking kernel location...
python -c "import os; f=open('build/kernel.bin', 'rb'); first=f.read(8); print(f'Kernel starts: {first.hex()}'); f.close(); f=open('build/pureos.img', 'rb'); f.seek(100*512); at_kernel=f.read(8); print(f'At sector 100: {at_kernel.hex()}'); print('MATCH!' if at_kernel==first else 'NO MATCH'); f.close()"

echo.
echo ===== READY =====
echo Run Bochs and look for:
echo   "Reading sector 100... Got: 0xB8"
echo.
pause
