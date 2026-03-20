@echo off
echo ===== BOOT VERIFICATION BUILD =====

REM Clean
if not exist build mkdir build

echo 1. Building boot sector...
nasm -f bin src\boot\boot_sect.asm -o build\boot_sect.bin
if %errorlevel% neq 0 (
    echo ERROR: Boot sector failed!
    pause
    exit /b 1
)

echo 2. Building simplified stage2...
nasm -f bin src\boot\stage2_simple.asm -o build\stage2.bin
if %errorlevel% neq 0 (
    echo ERROR: Stage2 failed!
    pause
    exit /b 1
)

echo.
echo 3. Disk layout verification...
python -c "import struct; f = open('build/boot_sect.bin', 'rb'); boot = f.read(); print('Boot sector size:', len(boot), 'bytes'); print('Boot signature:', 'OK' if boot[510] == 0x55 and boot[511] == 0xAA else 'BAD'); f.close(); f2 = open('build/stage2.bin', 'rb'); s2 = f2.read(); print('Stage2 size:', len(s2), 'bytes'); print('Stage2 first 16 bytes:', s2[:16].hex()); f2.close()"

echo.
echo 4. Creating disk image...
python -c "with open('build/pureos.img', 'wb') as f: boot = open('build/boot_sect.bin', 'rb').read(); f.write(boot); s2 = open('build/stage2.bin', 'rb').read(); f.write(s2); f.seek(1474560-1); f.write(b'\x00')"

echo.
echo 5. Verifying image structure...
python -c "f = open('build/pureos.img', 'rb'); f.seek(510); sig = f.read(2); print('Image boot sig:', 'OK' if sig[0] == 0x55 and sig[1] == 0xAA else 'BAD'); f.seek(512); s2_start = f.read(16); print('Stage2 @ sector 1:', s2_start.hex()); f.close()"

echo.
echo 6. File sizes:
dir build\boot_sect.bin | find "boot_sect.bin"
dir build\stage2.bin | find "stage2.bin"

echo.
echo ===== DEPLOYING =====
copy /Y build\pureos.img D:\bochs\Bochs-3.0\test.img

echo.
echo ===== EXPECTED OUTPUT =====
echo Look for at top-left of Bochs screen:
echo   B = Boot sector executed (green)
echo   L = Stage2 loaded successfully (green)
echo   2 = Stage2 is running (cyan)
echo   K = Kernel load attempted (yellow)
echo   E = Disk error (red)
echo.
echo Second line should say "STAGE2"
echo.
pause
D:\bochs\Bochs-3.0\bochs.exe -f D:\bochs\Bochs-3.0\bochsrc.bxrc -q
