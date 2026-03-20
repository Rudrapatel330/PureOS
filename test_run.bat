@echo off
echo Assembling Test Bootloader...
nasm -f bin test_boot.asm -o boot.bin
if %errorlevel% neq 0 (
    echo Error assembling bootloader
    pause
    exit /b %errorlevel%
)

echo Assembling Test Kernel...
nasm -f bin test_kernel.asm -o kernel.bin
if %errorlevel% neq 0 (
    echo Error assembling kernel
    pause
    exit /b %errorlevel%
)

echo Creating Test Image...
copy /b boot.bin+kernel.bin test.img
if %errorlevel% neq 0 (
    echo Error creating disk image
    pause
    exit /b %errorlevel%
)

echo Launching QEMU...
qemu-system-i386 -fda test.img -monitor stdio
pause
