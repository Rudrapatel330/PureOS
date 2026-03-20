<div align="center">
  <h1>💿 PureOS</h1>
  <p><b>A custom-built, lightweight x86 operating system featuring a windowed GUI, networking stack, and built-in desktop apps.</b></p>
</div>

---

PureOS is an actively developed standalone operating system engineered almost entirely from scratch in C and Assembly. It aims to provide a rich graphical desktop experience combined with robust modern networking functionality, all while bypassing traditional heavy Linux or Windows kernels.

## ✨ Core OS Features

* **Custom Bootloader & Kernel:** Custom assembly boot sectors mapped directly to a 32-bit/64-bit capable C kernel.
* **Advanced GUI Desktop:** A fully integrated windowing system complete with a Window Compositor, dynamic Start Menu, Taskbar, System Monitor, and customizable wallpapers.
* **Modern Networking Stack:** Built-in hardware support for network drivers like Realtek RTL8139 / PCnet. It features an impressive from-scratch stack for `IPv4` / `TCP` / `UDP` / `DNS` / `DHCP` and secure **HTTPS/TLS** connections powered by a ported BearSSL library.
* **Robust Filesystem (VFS):** Seamless support for multiple filesystems including `FAT12/FAT16/FAT32`, `RAMFS`, and `Ext2`.

## 📦 Application Suite

PureOS comes bundled with an impressive suite of native desktop applications:
* **System Utilities:** Terminal Simulator, Advanced File Manager (Explorer), Task Manager, and Calculator.
* **Creative Tools:** Integrated Paint application and Text Editor.
* **Media Viewers:** Photos viewer, embedded Video Player, and a powerful **PDF Reader** (utilizing a native lightweight port of MuPDF).
* **Web Browser:** A standalone desktop web browser with custom DOM parsing, layout rendering, and CSS support over raw TCP sockets!
* **Email Client:** A robust Mail app capable of fetching and sending live emails via SMTP.

---

## 📧 How to Setup the Email App

PureOS features a fully standalone SMTP Mail Client allowing you to send live emails from inside the OS! To securely protect your real password, it requires a secure **App Password** from your email provider.

**To configure your Google Account:**
1. Go to your Google Account -> **Security** -> **2-Step Verification** -> **App Passwords**.
2. Generate a new App Password (it will look like `abcd efgh ijkl mnop`).
3. Open `src/apps/mail/mail_app.c` in the PureOS source tree.
4. Scroll down to the SMTP configuration (around line 376) and enter your credentials:
   ```c
   const char *user = "your_email@gmail.com";
   const char *pass = "abcd efgh ijkl mnop";
   ```
5. Re-compile the OS using `build.bat` and run it! You can now send emails natively.

---

## 🛠️ Build Requirements

*(Currently designed and evaluated primarily within Windows environments).*
* The associated `x86_64-elf-gcc` hardware cross-compiler.
* Python 3 (For optimizing resource files).
* NASM 
* Bochs / QEMU / VirtualBox for local virtualization.

## 🚀 How to Build

To dynamically compile and assemble the entire system into a bootable drive image, execute the integrated build system:

1. **First, compile the PDF library:** Open PowerShell and run `.\build_mupdf.ps1` (You only need to do this once).
2. **Build the OS:** Run the main build script:
   ```bat
   .\build.bat
   ```

Executing this process automatically compiles the C kernel, interface windows, embedded fonts, and dynamic applications, outputting a finished `pureos.img` raw image file ready to be plugged into standard virtual-machine emulators!
