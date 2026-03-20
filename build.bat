@echo off
if not exist build mkdir build
del /Q build\*.o
del /Q build\*.bin
del /Q os-image.bin pureos.img serial_log.txt

echo Processing Wallpaper...
copy /Y "Gemini_Generated_Image_bjqk4objqk4objqk (1) (2) - Copy.png" src\fs\wallpaper.png >NUL
python optimize_ultra.py >NUL
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Wallpaper Blobs...
.\tools\bin\x86_64-elf-gcc.exe -c src/fs/wallpaper_blobs.S -o build/wallpaper_blobs.o
if %errorlevel% neq 0 exit /b %errorlevel%

echo Assembling Bootloader Stage 1...
nasm -f bin src/boot/boot_sect.asm -o build/boot_sect.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo Assembling Bootloader Stage 2...
nasm -f bin src/boot/stage2.asm -o build/stage2.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo Assembling Interrupts...
nasm -f elf64 src/kernel/hal/interrupt.asm -o build/interrupt.o
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Kernel Entry...
nasm -f elf64 src/kernel/kernel_entry.asm -o build/kernel_entry.o
if %errorlevel% neq 0 exit /b %errorlevel%

:: TEMPORARY: Build C Kernel with ASM Entry
echo Assembling ASM Entry...
nasm src/kernel/pure_kernel.asm -f elf64 -o build/pure_kernel.o
if %errorlevel% neq 0 exit /b %errorlevel%

echo Assembling ISR Stubs...
nasm -f elf64 src/kernel/hal/isr.asm -o build/isr_stubs.o
if %errorlevel% neq 0 exit /b %errorlevel%

echo Assembling GDT and IDT helpers...
nasm -f elf64 src/kernel/hal/idt_load.asm -o build/idt_load.o
if %errorlevel% neq 0 exit /b %errorlevel%

nasm -f elf64 src/kernel/hal/gdt_flush.asm -o build/gdt_flush.o
if %errorlevel% neq 0 exit /b %errorlevel%

nasm -f elf64 src/kernel/hal/trampoline.asm -o build/trampoline.o
if %errorlevel% neq 0 exit /b %errorlevel%

nasm -f elf64 src/kernel/hal/setjmp.asm -o build/setjmp.o
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling C Kernel...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/kernel.c -o build/kernel.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/screenshot_core.c -o build/screenshot_core.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui -Isrc/drivers -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Font...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/gui/font.c -o build/font.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling IDT/ISR...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/idt.c -o build/idt.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/isr.c -o build/isr.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Drivers/Utils...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/vga.c -o build/vga.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/bga.c -o build/bga.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/ports.c -o build/ports.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/keyboard.c -o build/keyboard.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/mouse.c -o build/mouse.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/timer.c -o build/timer.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/rtc.c -o build/rtc.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/ata.c -o build/ata.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/speaker.c -o build/speaker.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/pcnet.c -o build/pcnet.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/pci.c -o build/pci.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/ahci.c -o build/ahci.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/es1370.c -o build/es1370.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/usb/uhci.c -o build/uhci.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers/usb
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/usb/usb.c -o build/usb.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers/usb
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/wav.c -o build/wav.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/blitter.c -o build/blitter.o -Iinclude -Isrc/drivers -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Kernel Components...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/acpi.c -o build/acpi.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/kernel -Isrc/kernel/hal/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/apic.c -o build/apic.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/kernel -Isrc/kernel/hal/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/smp.c -o build/smp.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/kernel -Isrc/kernel/hal/hal -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/gdt.c -o build/gdt.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/kernel -Isrc/kernel/hal/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/hal.c -o build/hal.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/heap.c -o build/heap.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/mem.c -o build/mem.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/pic.c -o build/pic.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/kernel -Isrc/kernel/hal/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/string.c -o build/string.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Profiler...
if exist build\profiler.o del /Q build\profiler.o
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/profiler.c -o build/profiler.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling SIMD (SSE2)...
if exist build\simd.o del /Q build\simd.o
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/simd.c -o build/simd.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling 2D Graphics API...
if exist build\gfx_2d.o del /Q build\gfx_2d.o
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/gfx_2d.c -o build/gfx_2d.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling 2D Graphics Unit Tests...
if exist build\gfx_2d_test.o del /Q build\gfx_2d_test.o
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/gfx_2d_test.c -o build/gfx_2d_test.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Debugger...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/debug.c -o build/debug.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling WiFi...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/wifi.c -o build/wifi.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/window.c -o build/window.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/anim.c -o build/anim.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/desktop.c -o build/desktop.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/apps.c -o build/apps.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui -Isrc/drivers -Isrc/apps
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/compositor.c -o build/compositor.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/ui/ctxmenu.c -o build/ctxmenu.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui -Isrc/drivers -Isrc/ui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/task.c -o build/task.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/config.c -o build/config.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/theme.c -o build/theme.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/workspace.c -o build/workspace.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/clipboard.c -o build/clipboard.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/random.c -o build/random.o
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/base64.c -o build/base64.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/syscall.c -o build/syscall.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/mail_core.c -o build/mail_core.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Image Support...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/image.c -o build/image.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/lib
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/hal/paging.c -o build/paging.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/kernel -Isrc/kernel/hal/hal
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/kernel/elf.c -o build/elf.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs -Isrc/apps
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling POSIX Compatibility...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/lib/math.c -o build/math.o -Iinclude
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -Wno-int-conversion -c src/lib/stdio_wrapper.c -o build/stdio_wrapper.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Shell and Filesystem...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/shell/shell.c -o build/shell.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/fs -Isrc/apps/mail
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/fs/ramfs.c -o build/ramfs.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/fs/fat.c -o build/fat.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/fs/fs.c -o build/fs.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/fs/vfs.c -o build/vfs.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/fs/devfs.c -o build/devfs.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/fs/pipe.c -o build/pipe.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/fs/ext2.c -o build/ext2.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Applications...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/terminal.c -o build/terminal.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/apps
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/autocomplete.c -o build/autocomplete.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/apps -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/calculator.c -o build/calculator.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/paint.c -o build/paint.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/filemgr.c -o build/filemgr.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/explorer.c -o build/explorer.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/editor.c -o build/editor.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -Wno-unused-parameter -mcmodel=large -c src/lib/mupdf_stubs.c -o build/mupdf_stubs.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Imupdf/include
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -mcmodel=large -c src/apps/pdfreader.c -o build/pdfreader.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Imupdf/include -DFZ_ENABLE_ICC=0 -DFZ_ENABLE_JS=0 -DFZ_ENABLE_SVG=0 -DFZ_ENABLE_EPUB=0 -DFZ_ENABLE_HTML=0 -DFZ_ENABLE_CBZ=0 -DFZ_ENABLE_XPS=0 -DNOCJK
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/settings.c -o build/settings.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/lockscreen.c -o build/lockscreen.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/taskmgr.c -o build/taskmgr.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/photos.c -o build/photos.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/fs
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/mail/mail_storage.c -o build/mail_storage.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/fs -Isrc/apps/mail
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/mail/mail_app.c -o build/mail_app.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/fs -Isrc/apps/mail
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/gui/taskbar.c -o build/taskbar.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/gui/startmenu.c -o build/startmenu.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/gui/sysmenu.c -o build/sysmenu.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/gui/sysmon.c -o build/sysmon.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Video Support...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/drivers/video_dev.c -o build/video_dev.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/fs -Isrc/lib
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/videoplayer.c -o build/videoplayer.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/browser/dom.c -o build/dom.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/apps
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/browser/css.c -o build/css.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/apps
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/browser/layout.c -o build/layout.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/apps
if %errorlevel% neq 0 exit /b %errorlevel%

if exist build\js.o del /Q build\js.o
if exist build\browser.o del /Q build\browser.o

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/browser/js.c -o build/js.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/apps
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/apps/browser.c -o build/browser.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui -Isrc/net -Isrc/apps/browser
if %errorlevel% neq 0 exit /b %errorlevel%

echo Compiling Network Stack...
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/net_core.c -o build/net_core.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/arp.c -o build/arp.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/ipv4.c -o build/ipv4.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/udp.c -o build/udp.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/tcp.c -o build/tcp.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/dns.c -o build/dns.o
.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/smtp.c -o build/smtp.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/http.c -o build/http.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/tls.c -o build/tls.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net -Isrc/net/lib/bearssl/inc
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/dhcp.c -o build/dhcp.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net
if %errorlevel% neq 0 exit /b %errorlevel%

REM tls12.c disabled for now - causes page faults
REM .\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c src/net/tls12.c -o build/tls12.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/net -Isrc/net/lib/bearssl/inc

echo Compiling BearSSL (This may take a moment)...
powershell -ExecutionPolicy Bypass -File build_bearssl.ps1
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -fno-omit-frame-pointer -Wall -Wextra -mcmodel=large -c src/kernel/panic.c -o build/panic.o -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/drivers -Isrc/gui
if %errorlevel% neq 0 exit /b %errorlevel%

.\tools\bin\x86_64-elf-gcc.exe -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large -c test_simple.c -o build/test_simple.o -Iinclude -Isrc/kernel -Isrc/kernel/hal
if %errorlevel% neq 0 exit /b %errorlevel%

echo Assembling Fonts...
nasm -f elf64 src/lib/fonts.asm -o build/fonts.o

echo Linking Kernel...
.\tools\bin\x86_64-elf-ld.exe -T linker.ld -o build/kernel.bin build/pure_kernel.o build/isr_stubs.o build/idt_load.o build/gdt_flush.o build/trampoline.o build/setjmp.o build/kernel.o build/screenshot_core.o build/gdt.o build/smp.o build/heap.o build/mem.o build/pic.o build/idt.o build/isr.o build/vga.o build/bga.o build/font.o build/ports.o build/keyboard.o build/mouse.o build/timer.o build/rtc.o build/ata.o build/speaker.o build/pcnet.o build/pci.o build/ahci.o build/es1370.o build/uhci.o build/usb.o build/wav.o build/wifi.o build/blitter.o build/hal.o build/string.o build/simd.o build/gfx_2d.o build/gfx_2d_test.o build/profiler.o build/random.o build/base64.o build/math.o build/stdio_wrapper.o build/acpi.o build/apic.o build/window.o build/anim.o build/compositor.o build/desktop.o build/apps.o build/ctxmenu.o build/task.o build/clipboard.o build/syscall.o build/mail_core.o build/mail_storage.o build/mail_app.o build/image.o build/paging.o build/elf.o build/shell.o build/ramfs.o build/fat.o build/fs.o build/vfs.o build/devfs.o build/pipe.o build/ext2.o build/terminal.o build/autocomplete.o build/calculator.o build/paint.o build/filemgr.o build/explorer.o build/editor.o build/lockscreen.o build/taskmgr.o build/photos.o build/taskbar.o build/startmenu.o build/sysmenu.o build/sysmon.o build/settings.o build/config.o build/theme.o build/workspace.o build/video_dev.o build/videoplayer.o build/dom.o build/css.o build/layout.o build/js.o build/browser.o build/net_core.o build/arp.o build/ipv4.o build/udp.o build/tcp.o build/dns.o build/smtp.o build/http.o build/tls.o build/dhcp.o build/test_simple.o build/panic.o build/debug.o build/wallpaper_blobs.o build/mupdf_stubs.o build/pdfreader.o build/fonts.o build/mupdf.a build/bearssl.a --oformat binary
if %errorlevel% neq 0 exit /b %errorlevel%

echo Linking Kernel (ELF)...
.\tools\bin\x86_64-elf-ld.exe -T linker.ld -o build/kernel.elf build/pure_kernel.o build/isr_stubs.o build/idt_load.o build/gdt_flush.o build/trampoline.o build/setjmp.o build/kernel.o build/screenshot_core.o build/gdt.o build/smp.o build/heap.o build/mem.o build/pic.o build/idt.o build/isr.o build/vga.o build/bga.o build/font.o build/ports.o build/keyboard.o build/mouse.o build/timer.o build/rtc.o build/ata.o build/speaker.o build/pcnet.o build/pci.o build/ahci.o build/es1370.o build/uhci.o build/usb.o build/wav.o build/wifi.o build/blitter.o build/hal.o build/string.o build/simd.o build/gfx_2d.o build/gfx_2d_test.o build/profiler.o build/random.o build/base64.o build/math.o build/stdio_wrapper.o build/acpi.o build/apic.o build/window.o build/anim.o build/compositor.o build/desktop.o build/apps.o build/ctxmenu.o build/task.o build/clipboard.o build/syscall.o build/mail_core.o build/mail_storage.o build/mail_app.o build/image.o build/paging.o build/elf.o build/shell.o build/ramfs.o build/fat.o build/fs.o build/vfs.o build/devfs.o build/pipe.o build/ext2.o build/terminal.o build/autocomplete.o build/calculator.o build/paint.o build/filemgr.o build/explorer.o build/editor.o build/lockscreen.o build/taskmgr.o build/photos.o build/taskbar.o build/startmenu.o build/sysmenu.o build/sysmon.o build/settings.o build/config.o build/theme.o build/workspace.o build/video_dev.o build/videoplayer.o build/dom.o build/css.o build/layout.o build/js.o build/browser.o build/net_core.o build/arp.o build/ipv4.o build/udp.o build/tcp.o build/dns.o build/smtp.o build/http.o build/tls.o build/dhcp.o build/test_simple.o build/panic.o build/debug.o build/wallpaper_blobs.o build/mupdf_stubs.o build/pdfreader.o build/fonts.o build/mupdf.a build/bearssl.a
if %errorlevel% neq 0 exit /b %errorlevel%

echo Creating OS Image...
copy /b build\boot_sect.bin + build\stage2.bin + build\kernel.bin os-image.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo Creating Debug Disk (pureos.img)...
python make_debug_disk.py
if %errorlevel% neq 0 exit /b %errorlevel%

REM Upload PDFs to data disk
if exist pdftest.pdf (
    python upload_pdf.py pdftest.pdf
)
if exist "Building OS Networking and Search.pdf" (
    python upload_pdf.py "Building OS Networking and Search.pdf"
)

echo Done.
