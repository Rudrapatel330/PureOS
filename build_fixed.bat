@echo off
echo ===== FIXED BUILD =====

REM 1. Build stage2
echo Building progressive stage2...
nasm -f bin src\boot\progressive_fixed.asm -o build\stage2.bin
if %errorlevel% neq 0 ( exit /b 1 )

REM 2. Build kernel
echo Building kernel...
i686-elf-gcc -m32 -ffreestanding -c test_kernel.c -o build\kernel.o
i686-elf-ld -m elf_i386 -Ttext 0x10000 -o build\kernel.elf build\kernel.o
i686-elf-objcopy -O binary build\kernel.elf build\kernel.bin
if %errorlevel% neq 0 ( exit /b 1 )

REM 3. Create disk
echo Creating disk image...
python make_disk.py
if %errorlevel% neq 0 ( exit /b 1 )

REM 4. Show what we built
echo.
echo ===== VERIFICATION =====
python -c "try: f=open('build/kernel.bin','rb'); first=f.read(8); print('Kernel starts:', first.hex()); f.close(); f=open('build/pureos.img','rb'); f.seek(100*512); sec100=f.read(8); print('Sector 100:', sec100.hex()); print('MATCH!' if first==sec100 else 'NO MATCH'); f.close(); except Exception as e: print(e)"

echo.
echo ===== RUNNING =====
"D:\bochs\Bochs-3.0\bochs.exe" -f D:\bochs\Bochs-3.0\bochsrc.bxrc -q
pause
