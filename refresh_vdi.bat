@echo off
set "VBOX_MANAGE=C:\Program Files\Oracle\VirtualBox\VBoxManage.exe"
set "VM_NAME=PureOS"
set "DISK_PATH=%~dp0pureos_v37.vdi"

echo [1/4] Closing VirtualBox interfaces...
taskkill /IM VirtualBox.exe /F >nul 2>&1
taskkill /IM VBoxSVC.exe /F >nul 2>&1

echo [2/4] Removing old disk reference from VirtualBox...
"%VBOX_MANAGE%" closemedium disk "%DISK_PATH%" --delete >nul 2>&1

echo [3/4] Re-creating VDI from image...
if exist "%DISK_PATH%" del "%DISK_PATH%"
"%VBOX_MANAGE%" convertfromraw "%~dp0pureos.img" "%DISK_PATH%" --format VDI

echo [4/4] Attaching new disk to VM...
"%VBOX_MANAGE%" storageattach "%VM_NAME%" --storagectl "AHCI" --port 0 --device 0 --type hdd --medium "%DISK_PATH%"

echo.
echo Refresh Complete! You can now start the VM: "%VM_NAME%"
pause
