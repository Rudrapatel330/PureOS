# PureOS 64-bit Makefile
CC = .\tools\bin\x86_64-elf-gcc.exe
LD = .\tools\bin\x86_64-elf-ld.exe
NASM = D:\nasm\nasm.exe

CFLAGS = -ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -mcmodel=large \
         -Iinclude -Isrc/kernel -Isrc/kernel/hal -Isrc/gui -Isrc/drivers -Isrc/fs -Isrc/apps -Isrc/net

LDFLAGS = -T linker.ld

KERNEL_ASM_SOURCES = src/kernel/kernel_entry.asm \
                     src/kernel/pure_kernel.asm \
                     src/kernel/hal/interrupt.asm \
                     src/kernel/hal/isr.asm \
                     src/kernel/hal/idt_load.asm \
                     src/kernel/hal/gdt_flush.asm \
                     src/kernel/hal/trampoline.asm

KERNEL_C_SOURCES = src/kernel/kernel.c \
                   src/kernel/hal/idt.c \
                   src/kernel/hal/isr.c \
                   src/kernel/hal/gdt.c \
                   src/kernel/hal/hal.c \
                   src/kernel/hal/pic.c \
                   src/kernel/hal/paging.c \
                   src/kernel/mem.c \
                   src/kernel/heap.c \
                   src/kernel/string.c \
                   src/kernel/profiler.c \
                   src/kernel/simd.c \
                   src/kernel/gfx_2d.c \
                   src/kernel/gfx_2d_test.c \
                   src/kernel/task.c \
                   src/kernel/syscall.c \
                   src/kernel/panic.c \
                   src/kernel/debug.c \
                   src/kernel/screenshot_core.c \
                   src/drivers/bga.c \
                   src/drivers/keyboard.c \
                   src/drivers/mouse.c \
                   src/drivers/timer.c \
                   src/drivers/pci.c \
                   src/fs/vfs.c \
                   src/fs/fat.c \
                   src/fs/ext2.c \
                   src/kernel/desktop.c \
                   src/kernel/apps.c \
                   src/kernel/window.c \
                   src/kernel/compositor.c \
                   src/kernel/config.c \
                   src/kernel/anim.c \
                   src/gui/taskbar.c \
                   src/gui/startmenu.c \
                   src/gui/sysmenu.c \
                   src/drivers/camera.c \
                   src/apps/camera.c \
                   src/apps/chat.c \
                   src/drivers/usb/uvc.c \
                   src/drivers/usb/ohci.c

KERNEL_ASM_OBJS = $(patsubst %.asm, build/%_asm.o, $(notdir $(KERNEL_ASM_SOURCES)))
KERNEL_C_OBJS = $(patsubst %.c, build/%.o, $(notdir $(KERNEL_C_SOURCES)))

all: pureos.img

pureos.img: build/boot_sect.bin build/stage2.bin build/kernel.bin
	python make_debug_disk.py

build/kernel.bin: $(KERNEL_ASM_OBJS) $(KERNEL_C_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ --oformat binary

build/boot_sect.bin: src/boot/boot_sect.asm
	$(NASM) -f bin $< -o $@

build/stage2.bin: src/boot/stage2.asm
	$(NASM) -f bin $< -o $@

# Pattern rules for kernel objects
build/%_asm.o: src/kernel/%.asm
	$(NASM) -f elf64 $< -o $@
build/%_asm.o: src/kernel/hal/%.asm
	$(NASM) -f elf64 $< -o $@

build/%.o: src/kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@
build/%.o: src/kernel/hal/%.c
	$(CC) $(CFLAGS) -c $< -o $@
build/%.o: src/drivers/%.c
	$(CC) $(CFLAGS) -c $< -o $@
build/%.o: src/fs/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/gui/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/apps/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Userland Rules
USER_CFLAGS = -ffreestanding -mno-red-zone -O2 -Wall -Iinclude
USER_LDFLAGS = -T src/lib/user.ld

USER_OBJS = build/crt0.o build/libc.o

userland: build/ld-pureos.so build/purebox.exe build/pkg.exe

build/crt0.o: src/lib/crt0.asm
	$(NASM) -f elf64 $< -o $@

build/libc.o: src/lib/libc.c
	$(CC) $(USER_CFLAGS) -c $< -o $@

build/ld-pureos.so: src/lib/ld_so.c $(USER_OBJS)
	$(CC) $(USER_CFLAGS) -fPIC -shared -Wl,-T,src/lib/user.ld $< $(USER_OBJS) -o $@

build/purebox.exe: src/apps/purebox.c $(USER_OBJS)
	$(CC) $(USER_CFLAGS) $< $(USER_OBJS) $(USER_LDFLAGS) -o $@

build/pkg.exe: src/apps/pkg.c $(USER_OBJS)
	$(CC) $(USER_CFLAGS) $< $(USER_OBJS) $(USER_LDFLAGS) -o $@

clean:
	del /Q build\*.o build\*.bin build\*.exe build\*.so os-image.bin pureos.img
