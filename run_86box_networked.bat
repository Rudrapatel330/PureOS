@echo off
cd /d D:\86box

echo Starting 86Box with networking...
echo.
echo Configuration:
echo - Machine: 586 (Pentium)
echo - Memory: 32MB
echo - Floppy: D:\1os\pureos.img
echo - Network: NE2000 with SLiRP
echo.
echo Check 86box.log for details after running.

start "86Box" "D:\86box\86Box.exe" "D:\86box\pureos.cfg"

echo 86Box should be starting...
pause
