@echo off
echo ===== BRUTE FORCE SEARCH =====

REM Build brute search stage2
nasm -f bin brute_search.asm -o build\stage2_brute.bin

REM Create simple disk with it
python -c "
# Create disk with boot + brute search
try:
    with open('build/pureos_brute.img', 'wb') as f:
        # Boot sector
        with open('build/boot.bin', 'rb') as b:
            f.write(b.read())
        
        # Brute search stage2
        with open('build/stage2_brute.bin', 'rb') as s:
            f.write(s.read())
        
        # Pad to 1.44MB
        f.seek(1474560-1)
        f.write(b'\x00')

    print('Disk with brute search created')
except Exception as e:
    print(e)
"

echo.
echo ===== LAUNCHING =====
echo This will search sectors 1-50 for any non-zero data.
echo It should find the kernel SOMEWHERE...
echo.
copy /y build\pureos_brute.img build\pureos.img
"D:\bochs\Bochs-3.0\bochs.exe" -f bochsrc_new.bxrc
pause
