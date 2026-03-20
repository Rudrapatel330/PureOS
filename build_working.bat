@echo off
echo ===== RESTORING WORKING OS (16bpp) =====

REM Clean
if exist build mkdir build 2>nul
del /Q build\*.* 2>nul

echo 1. Assembling Bootloader...
nasm -f bin src/boot/boot_sect.asm -o build/boot.bin
nasm -f bin src/boot/stage2.asm -o build/stage2.bin

echo 2. Compiling Kernel Components...
i686-elf-gcc -m32 -ffreestanding -c src/kernel/kernel.c -o build/kernel.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/string.c -o build/string.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/heap.c -o build/heap.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/paging.c -o build/paging.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/syscall.c -o build/syscall.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/task.c -o build/task.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/clipboard.c -o build/clipboard.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/window.c -o build/window.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/anim.c -o build/anim.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/isr.c -o build/isr.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/idt.c -o build/idt.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/gdt.c -o build/gdt.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/pic.c -o build/pic.o
i686-elf-gcc -m32 -ffreestanding -c src/drivers/vga.c -o build/vga.o
i686-elf-gcc -m32 -ffreestanding -c src/drivers/ports.c -o build/ports.o
i686-elf-gcc -m32 -ffreestanding -c src/drivers/mouse.c -o build/mouse.o
i686-elf-gcc -m32 -ffreestanding -c src/drivers/keyboard.c -o build/keyboard.o
i686-elf-gcc -m32 -ffreestanding -c src/drivers/timer.c -o build/timer.o
i686-elf-gcc -m32 -ffreestanding -c src/drivers/font.c -o build/font.o
i686-elf-gcc -m32 -ffreestanding -c src/drivers/ata.c -o build/ata.o
i686-elf-gcc -m32 -ffreestanding -c src/drivers/rtc.c -o build/rtc.o
i686-elf-gcc -m32 -ffreestanding -c src/fs/fat12.c -o build/fat12.o
i686-elf-gcc -m32 -ffreestanding -c src/apps/terminal.c -o build/terminal.o
i686-elf-gcc -m32 -ffreestanding -c src/apps/calculator.c -o build/calculator.o
i686-elf-gcc -m32 -ffreestanding -c src/apps/editor.c -o build/editor.o
i686-elf-gcc -m32 -ffreestanding -c src/apps/paint.c -o build/paint.o
i686-elf-gcc -m32 -ffreestanding -c src/apps/explorer.c -o build/explorer.o
i686-elf-gcc -m32 -ffreestanding -c src/kernel/desktop.c -o build/desktop.o
i686-elf-gcc -m32 -ffreestanding -c src/shell/shell.c -o build/shell.o
i686-elf-gcc -m32 -ffreestanding -c src/ui/ctxmenu.c -o build/ctxmenu.o
i686-elf-gcc -m32 -ffreestanding -c src/apps/autocomplete.c -o build/autocomplete.o

nasm -f elf32 src/boot/kernel_entry.asm -o build/kernel_entry.o
nasm -f elf32 src/kernel/interrupt.asm -o build/interrupt.o
nasm -f elf32 src/kernel/isr.asm -o build/isr_stub.o
nasm -f elf32 src/kernel/idt_load.asm -o build/idt_load.o
nasm -f elf32 src/kernel/gdt_flush.asm -o build/gdt_flush.o

echo 3. Linking Kernel...
i686-elf-ld -m elf_i386 -Ttext 0x20000 -o build/kernel.elf build/kernel_entry.o build/interrupt.o build/isr_stub.o build/idt_load.o build/gdt_flush.o build/kernel.o build/heap.o build/paging.o build/syscall.o build/task.o build/clipboard.o build/window.o build/anim.o build/pic.o build/isr.o build/idt.o build/gdt.o build/vga.o build/ports.o build/mouse.o build/keyboard.o build/timer.o build/font.o build/ata.o build/rtc.o build/fat12.o build/terminal.o build/calculator.o build/editor.o build/paint.o build/explorer.o build/desktop.o build/shell.o build/string.o build/ctxmenu.o build/autocomplete.o

echo 4. Extracting Binary...
i686-elf-objcopy -O binary build/kernel.elf build/kernel.bin

echo 5. Creating Floppy Image...
python make_debug_disk.py

echo.
echo BUILD SUCCESSFUL!
echo Launching Bochs...
"D:\bochs\Bochs-3.0\bochs.exe" -f bochsrc.txt -q
pause
