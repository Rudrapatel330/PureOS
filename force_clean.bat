@echo off
echo ========================================
echo      NUCLEAR CLEANUP - KILLING BOCHS
echo ========================================

echo Closing all Bochs windows...
taskkill /F /FI "WINDOWTITLE eq Bochs*" 2>nul
taskkill /F /IM bochs.exe 2>nul
taskkill /F /IM bochsdbg.exe 2>nul

echo Killing at process level...
wmic process where "name like '%%bochs%%'" delete 2>nul

echo Waiting 3 seconds...
timeout /t 3 /nobreak >nul

echo Deleting all related files...
del /f /q *.img 2>nul
del /f /q *.lock 2>nul
del /f /q bochs*.txt 2>nul
del /f /q lock 2>nul

echo ========================================
echo      STARTING BUILD
echo ========================================
call build_clean.bat
