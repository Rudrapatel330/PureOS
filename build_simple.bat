@echo off
cd /d d:\1os
echo ===== SIMPLE BUILD =====

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

REM 2. Create disk with kernel at sector 13 (and 12 for safety)
echo 2. Creating disk...
REM Boot = 512
REM Stage2 = 4096 (8 sectors). Ends at 512+4096 = 4608 (Sector 9 start)
REM Padding to Sector 13 (6656 bytes). Need 6656 - 4608 = 2048 bytes padding.
python -c "import struct; f=open('build/pureos.img','wb'); f.write(open('build/boot.bin','rb').read()); f.write(open('build/stage2.bin','rb').read()); f.write(b'\x00'*(12*512-512-4096)); f.write(open('build/kernel.bin','rb').read()); f.write(b'\x00'*(512)); f.write(open('build/kernel.bin','rb').read()); f.seek(1474560-1); f.write(b'\x00'); f.close()"

REM 3. Verify
echo.
echo 3. Verifying...
python -c "import os; size = os.path.getsize('build/kernel.bin'); print(f'Kernel size: {size} bytes'); f=open('build/kernel.bin', 'rb'); first=f.read(8); f.close(); f=open('build/pureos.img', 'rb'); f.seek(12*512); sec12=f.read(8); f.seek(13*512); sec13=f.read(8); print(f'Sector 12: {sec12.hex()}'); print(f'Sector 13: {sec13.hex()}'); print(f'Kernel   : {first.hex()}'); print('MATCH 12!' if sec12==first else 'NO MATCH 12'); print('MATCH 13!' if sec13==first else 'NO MATCH 13'); f.close()"

echo.
echo ===== READY =====
echo Run Bochs and look for:
echo   "Trying sector 12/13... OK!"
echo   "First byte: 0xB8" (or similar)
echo.
pause
