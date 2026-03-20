@echo off
cd /d "d:\1os"
echo ===== LAUNCHING BOCHS =====
echo.
echo Looking for:
echo   B L 2 K = Boot successful chain
echo   B E = Disk read failure
echo   B only = Boot sector ran, disk read failed
echo.
echo If Bochs exits immediately:
echo   - Check bochsrc.bxrc for errors
echo   - Verify build\pureos.img exists
echo.

REM Make sure image exists
if not exist build\pureos.img (
    echo ERROR: build\pureos.img not found!
    echo Run verify_boot.bat first to build the image.
    pause
    exit /b 1
)

REM Copy to Bochs directory
copy /Y build\pureos.img D:\bochs\Bochs-3.0\test.img

echo Starting Bochs...
echo.
"D:\bochs\Bochs-3.0\bochs.exe" -f "D:\bochs\Bochs-3.0\bochsrc.bxrc" -q

echo.
echo Bochs exited. Checking logs...
if exist D:\bochs\Bochs-3.0\serial.txt (
    echo.
    echo ===== SERIAL OUTPUT =====
    type D:\bochs\Bochs-3.0\serial.txt
)

echo.
pause
