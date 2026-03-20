<div align="center">
  <h1>💿 PureOS</h1>
  <p><b>A custom-built, lightweight x86 operating system featuring a windowed GUI, networking stack, and built-in desktop apps.</b></p>
</div>

---

PureOS is an actively developed standalone operating system engineered almost entirely from scratch in C and Assembly. It aims to provide a rich graphical desktop experience combined with robust modern networking functionality, all while bypassing traditional heavy Linux or Windows kernels.

## ✨ Features

* **Custom Bootloader & Kernel:** Custom assembly boot sectors mapped directly to a 32-bit/64-bit capable C kernel.
* **Advanced GUI Desktop:** A fully integrated windowing system complete with a Compositor, dynamic Start Menu, Taskbar, System Monitor, and customizable wallpapers.
* **Modern Networking Stack:** Built-in hardware support for drivers like Realtek RTL8139 / PCnet. Features an impressive from-scratch stack for `IPv4` / `TCP` / `UDP` / `DNS` / `DHCP` and even secure **HTTPS/TLS** connections powered by a ported BearSSL library.
* **Robust Filesystem (VFS):** Seamless support for multiple filesystems including `FAT`, `RAMFS`, and `Ext2`.
* **Application Suite:**
  * **System Utilities:** Terminal Simulator, Advanced File Manager (Explorer), Task Manager, and Calculator.
  * **Creative Tools:** Integrated Paint application and Text Editor.
  * **Media Viewers:** Photos app, Video Player, and a powerful **PDF Reader** utilizing a lightweight port of MuPDF.
  * **Web Browser:** A native desktop web browser with custom DOM parsing, layout rendering, and CSS support over raw sockets!

## 🛠️ Build Requirements

*(Currently designed and evaluated primarily within Windows environments).*
* The associated `x86_64-elf-gcc` hardware cross-compiler.
* Python 3 (For optimizing resource files).
* NASM 
* Bochs / QEMU / VirtualBox for local virtualization.

## 🚀 How to Build

To dynamically compile and assemble the entire system into a bootable drive image, execute the integrated build system from the root directory:

```bat
.\build.bat
```

Executing this process automatically compiles the C kernel, low-level HAL interrupts, interface windows, embedded fonts, and dynamic applications, outputting a finished `pureos.img` raw image file (as well as an `os-image.bin`) ready to be plugged into standard virtual-machine emulators!
