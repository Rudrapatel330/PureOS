@echo off
echo ===== BUILD TEST KERNEL =====

REM 1. Build stage2
echo Building stage2...
nasm -f bin src\boot\stage2_simple.asm -o build\stage2.bin
if %errorlevel% neq 0 (
    echo ERROR: Stage2 assembly failed
    pause
    exit /b 1
)

REM 2. Build test kernel
echo Building test kernel...
i686-elf-gcc -m32 -ffreestanding -c test_kernel.c -o build\kernel.o -I src\kernel
if %errorlevel% neq 0 (
    echo ERROR: Kernel compilation failed
    pause
    exit /b 1
)

nasm -f elf32 src\boot\kernel_entry.asm -o build\kernel_entry.o
if %errorlevel% neq 0 (
    echo ERROR: Kernel entry assembly failed
    pause
    exit /b 1
)

REM 3. Link kernel at 0x10000
echo Linking...
i686-elf-ld -m elf_i386 -Ttext 0x10000 -o build\kernel.elf build\kernel_entry.o build\kernel.o
if %errorlevel% neq 0 (
    echo ERROR: Linking failed
    pause
    exit /b 1
)

i686-elf-objcopy -O binary build\kernel.elf build\kernel.bin
if %errorlevel% neq 0 (
    echo ERROR: Objcopy failed
    pause
    exit /b 1
)

REM 4. Create disk image
echo Creating disk image...
python make_test_image.py
if %errorlevel% neq 0 (
    echo ERROR: Image creation failed
    pause
    exit /b 1
)

echo.
echo ===== READY =====
echo Run: D:\bochs\Bochs-3.0\bochs.exe -f D:\bochs\Bochs-3.0\bochsrc.bxrc
pause
