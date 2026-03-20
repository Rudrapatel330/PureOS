@echo off
cd /d d:\1os
echo ===== SCAN BUILD =====

REM Clean
if exist build mkdir build 2>nul
del /Q build\*.* 2>nul

REM 1. Build files
echo 1. Building...
nasm -f bin boot_sect.asm -o build\boot.bin
nasm -f bin stage2_final.asm -o build\stage2.bin
i686-elf-gcc -m32 -ffreestanding -c kernel_simple.c -o build\kernel.o
i686-elf-ld -m elf_i386 -Ttext 0x10000 -o build\kernel.elf build\kernel.o
i686-elf-objcopy -O binary build\kernel.elf build\kernel.bin

REM 2. Create disk with kernel at sector 12
echo 2. Creating disk...
python -c "import struct; f=open('build/pureos.img','wb'); f.write(open('build/boot.bin','rb').read()); f.write(open('build/stage2.bin','rb').read()); f.write(b'\x00'*(12*512-512-4096)); f.write(open('build/kernel.bin','rb').read()); f.write(b'\x00'*(512)); f.write(open('build/kernel.bin','rb').read()); f.seek(1474560-1); f.write(b'\x00'); f.close()"

REM 3. Verify
echo.
echo 3. Verifying...
python find_kernel.py

echo.
echo ===== READY =====
echo Run Bochs and look for:
echo   "Sec 12: 5589E583... <- FOUND KERNEL (0x55)!"
echo.
pause
