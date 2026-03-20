@echo off
cd /d D:\86box

REM Create a simple configuration inline
echo Creating 86Box configuration...

REM Run 86Box with settings for PureOS
start "" "D:\86box\86Box.exe" ^
    -M 586 ^
    -cpu 586 ^
    -memory 1024 ^
    -floppya "D:\1os\pureos.img" ^
    -boot a ^
    -log "D:\1os\86box.log" ^
    -ne2000 "mac=b0:c4:20:00:00:01,eth=slirp" ^
    -com1 "dev:null" ^
    -L .
