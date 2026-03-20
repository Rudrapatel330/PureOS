@echo off
echo ===== BUILD PROGRESSIVE TEST =====

REM 1. Build progressive stage2
echo Building progressive stage2...
nasm -f bin src\boot\progressive.asm -o build\stage2.bin
if %errorlevel% neq 0 (
    echo ERROR: Stage2 assembly failed
    pause
    exit /b 1
)

REM 2. Reuse kernel build (make sure kernel.bin exists)
if not exist build\kernel.bin (
    echo ERROR: Kernel binary missing! Run build_test.bat first.
    pause
    exit /b 1
)

REM 3. Create disk image
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
