@echo off
echo Assembling Simple OS...
nasm -f bin simple_os.asm -o os_test.img
if %errorlevel% neq 0 (
    echo Assembly Failed!
    pause
    exit /b %errorlevel%
)

echo Copying to Bochs directory...
copy /Y os_test.img D:\bochs\Bochs-3.0\test.img
if %errorlevel% neq 0 (
    echo Copy Failed!
    pause
    exit /b %errorlevel%
)

echo Launching Bochs...
D:\bochs\Bochs-3.0\bochs.exe -f D:\bochs\Bochs-3.0\bochsrc.bxrc -q
