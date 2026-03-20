@echo off
echo Building Minimal Test...

REM Clean build directory
if not exist build mkdir build

REM Assemble boot sector
echo Building boot_sect.bin...
nasm -f bin src\boot\boot_sect_test.asm -o build\boot_sect.bin
if %errorlevel% neq 0 (
    echo Boot sector assembly failed!
    pause
    exit /b 1
)

REM Assemble stage2
echo Building stage2.bin...
nasm -f bin src\boot\stage2_test.asm -o build\stage2.bin
if %errorlevel% neq 0 (
    echo Stage2 assembly failed!
    pause
    exit /b 1
)

REM Check file sizes
echo.
echo File sizes:
dir build\boot_sect.bin
dir build\stage2.bin

REM Create disk image
echo.
echo Creating pureos.img...
python -c "with open('build/pureos.img', 'wb') as f: boot = open('build/boot_sect.bin', 'rb').read(); f.write(boot); s2 = open('build/stage2.bin', 'rb').read(); f.write(s2); f.seek(1474560-1); f.write(b'\x00')"

if %errorlevel% neq 0 (
    echo Image creation failed!
    pause
    exit /b 1
)

echo.
echo Checking boot signature...
python -c "with open('build/pureos.img', 'rb') as f: data = f.read(512); print('First 16 bytes:', ' '.join(f'{b:02X}' for b in data[:16])); print('Boot signature:', 'OK' if data[510] == 0x55 and data[511] == 0xAA else 'MISSING!')"

REM Copy to Bochs
echo.
echo Deploying to Bochs...
copy /Y build\pureos.img D:\bochs\Bochs-3.0\test.img

echo.
echo Done! Launching Bochs...
D:\bochs\Bochs-3.0\bochs.exe -f D:\bochs\Bochs-3.0\bochsrc.bxrc -q
