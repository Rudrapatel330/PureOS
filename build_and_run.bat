@echo off
echo ===== Building and Launching PureOS in Bochs =====
echo.

REM Kill any existing Bochs to release file locks
taskkill /F /IM bochs.exe /T >nul 2>&1
timeout /t 1 /nobreak >nul

REM Build the OS first
call build.bat
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo ===== Copying image to Bochs directory =====
REM Create floppy image with proper structure
python make_debug_disk.py
python make_data_disk.py
copy /Y pureos.img D:\bochs\Bochs-3.0\test.img
if %errorlevel% neq 0 (
    echo Failed to copy image!
    exit /b 1
)

echo.
echo ===== Launching Bochs =====
echo.
echo Looking for successful boot indicators:
echo   - Graphics mode initialization
echo   - Mouse cursor appearing
echo   - Desktop rendering
echo.

"D:\bochs\Bochs-3.0\bochs.exe" -f "bochsrc.txt" -q

echo.
echo Bochs exited.
echo.
if exist D:\bochs\Bochs-3.0\serial.txt (
    echo ===== SERIAL OUTPUT =====
    type D:\bochs\Bochs-3.0\serial.txt
)
