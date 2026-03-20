@echo off
echo ===== DISK IMAGE DIAGNOSIS =====

echo 1. Checking file sizes...
dir build\*.bin

echo.
echo 2. Checking disk image structure...
python -c "import struct; f=open('build/pureos.img', 'rb'); f.seek(510); sig=f.read(2); print('Boot sig:', 'OK' if sig==b'\x55\xAA' else sig.hex()); f.seek(512); print('Stage2 start:', f.read(16).hex()); f.seek(6656); print('Kernel start:', f.read(16).hex()); f.close()"

echo.
pause
