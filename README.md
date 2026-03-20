<div align="center">
  <h1>💿 PureOS</h1>
  <p><b>A custom-built x86-64 operating system with a full windowed GUI, TCP/IP networking stack, TLS encryption, and 15+ built-in desktop applications — all written from scratch in C and Assembly.</b></p>
  <br>
  <img src="https://img.shields.io/badge/Language-C%20%2F%20x86--64%20Assembly-blue" alt="Language">
  <img src="https://img.shields.io/badge/License-MIT-green" alt="License">
  <img src="https://img.shields.io/badge/Platform-x86--64-orange" alt="Platform">
</div>

---

## 📖 Table of Contents
- [Overview](#-overview)
- [System Architecture](#-system-architecture)
- [Desktop Environment & UI](#-desktop-environment--ui)
- [Built-in Applications](#-built-in-applications)
- [Networking Stack](#-networking-stack)
- [Email Client (SMTP)](#-email-client-smtp)
- [Web Browser](#-web-browser)
- [Filesystem Support](#-filesystem-support)
- [Hardware & Drivers](#-hardware--drivers)
- [Build Instructions](#-build-instructions)
- [Email Setup Guide](#-email-setup-guide)

---

## 🌟 Overview

PureOS is a fully functional desktop operating system engineered entirely from scratch — no Linux kernel, no POSIX libraries, no borrowed OS code. Every layer is hand-written: from the bootloader and kernel, through memory management and interrupt handling, all the way up to a composited windowed desktop with animations, a TCP/IP network stack with TLS encryption, and a suite of native GUI applications.

---

## 🏗️ System Architecture

The OS is structured in clean layers, each built on top of the previous one:

```mermaid
graph TB
    subgraph Hardware Layer
        CPU[x86-64 CPU]
        PCI[PCI Bus]
        DISK[ATA / AHCI Disk]
        NIC[PCnet / NE2000 NIC]
        GPU[VGA / BGA Display]
        KBD[PS/2 Keyboard]
        MOUSE[PS/2 Mouse]
        AUDIO[ES1370 Audio]
        USB_HW[USB UHCI]
    end

    subgraph Kernel Layer
        BOOT[Custom 2-Stage Bootloader] --> KERNEL[64-bit C Kernel]
        KERNEL --> GDT[GDT / IDT / ISR]
        KERNEL --> PAGING[Virtual Memory & Paging]
        KERNEL --> HEAP[Dynamic Heap Allocator]
        KERNEL --> SCHED[Task Scheduler & SMP]
        KERNEL --> SYSCALL[System Calls]
        KERNEL --> ACPI_K[ACPI Power Management]
    end

    subgraph Driver Layer
        DRIVERS[Device Drivers]
        DRIVERS --> VGA_D[VGA / BGA Graphics]
        DRIVERS --> KBD_D[Keyboard Driver]
        DRIVERS --> MOUSE_D[Mouse Driver]
        DRIVERS --> ATA_D[ATA / AHCI Storage]
        DRIVERS --> NET_D[PCnet / NE2000 Network]
        DRIVERS --> AUDIO_D[ES1370 + WAV Audio]
        DRIVERS --> USB_D[USB UHCI Driver]
        DRIVERS --> TIMER_D[PIT Timer + RTC Clock]
        DRIVERS --> PCI_D[PCI Bus Enumeration]
    end

    subgraph Filesystem Layer
        VFS[Virtual Filesystem Switch]
        VFS --> FAT[FAT12 / FAT16 / FAT32]
        VFS --> EXT2[Ext2]
        VFS --> RAMFS[RAM Filesystem]
        VFS --> DEVFS[Device FS]
        VFS --> PIPES[Unix-style Pipes]
    end

    subgraph Network Layer
        NET_STACK[Network Stack]
        NET_STACK --> ETH[Ethernet Frames]
        NET_STACK --> ARP[ARP Resolution]
        NET_STACK --> IPV4[IPv4 Routing]
        NET_STACK --> ICMP_N[UDP / TCP]
        NET_STACK --> DNS_N[DNS Resolver]
        NET_STACK --> DHCP_N[DHCP Client]
        NET_STACK --> HTTP_N[HTTP 1.1 Client]
        NET_STACK --> TLS_N[TLS 1.2 via BearSSL]
        NET_STACK --> SMTP_N[SMTP Email Client]
    end

    subgraph Desktop Environment
        COMPOSITOR[Window Compositor]
        COMPOSITOR --> TASKBAR_D[Taskbar + System Tray]
        COMPOSITOR --> STARTMENU_D[Start Menu]
        COMPOSITOR --> SYSMENU_D[System Quick-Settings Menu]
        COMPOSITOR --> ANIMATIONS[Spring & Ease Animations]
        COMPOSITOR --> THEMES[Theme Engine]
        COMPOSITOR --> WORKSPACES[Virtual Workspaces]
        COMPOSITOR --> CLIPBOARD_D[Clipboard Manager]
    end

    subgraph Applications
        APPS[15+ Native GUI Apps]
    end

    Hardware Layer --> Kernel Layer
    Kernel Layer --> Driver Layer
    Driver Layer --> Filesystem Layer
    Driver Layer --> Network Layer
    Kernel Layer --> Desktop Environment
    Desktop Environment --> Applications
```

---

## 🖥️ Desktop Environment & UI

PureOS features a modern, composited desktop environment with rich visual effects:

| Feature | Description |
|---|---|
| **Window Compositor** | Real-time composited rendering with proper Z-ordering, transparency, and overlapping window support |
| **Taskbar** | Windows-style taskbar showing running applications with click-to-focus switching |
| **Start Menu** | App launcher with categorized application list and quick-access shortcuts |
| **System Tray Menu** | Quick-settings panel for WiFi, Bluetooth, Volume, Brightness, and Power options |
| **System Monitor** | Live CPU and memory usage displayed with animated liquid-fill gauges |
| **Window Animations** | Smooth open/close/minimize animations powered by spring physics and easing curves |
| **Theme Engine** | Customizable color themes with dark mode support |
| **Virtual Workspaces** | Multiple desktop workspaces for organizing windows |
| **Clipboard** | System-wide copy/paste clipboard |
| **Lock Screen** | Secure password-protected lock screen with blurred background |
| **Context Menus** | Right-click context menus throughout the desktop |
| **Custom Wallpaper** | High-resolution PNG wallpaper with optimized rendering |
| **Keyboard Shortcuts** | `Alt+F4` close, window dragging, resize, and focus management |

---

## 📦 Built-in Applications

PureOS ships with **15+ native desktop applications**, all built directly into the kernel:

### System Utilities
| App | Description |
|---|---|
| 🖥️ **Terminal** | Full terminal emulator with command history, tab auto-completion, and a built-in shell supporting `ls`, `cd`, `cat`, `mkdir`, `rm`, `cp`, `mv`, `ping`, `ifconfig`, `wget`, and more |
| 📁 **File Manager** | Dual-pane graphical file manager with icon/list views, file operations (copy, move, delete, rename), and directory navigation |
| 🗂️ **Explorer** | Advanced file explorer with breadcrumb navigation, search, and detailed file information |
| 📊 **Task Manager** | Shows running windows/processes with the ability to kill unresponsive applications |
| ⚙️ **Settings** | System configuration panel with pages for Home, Personalization, Accounts, System Info, and About |
| 🔒 **Lock Screen** | Secure login screen with password input and blurred desktop background |
| 📸 **Screenshot** | Capture the current screen and save to disk |

### Creative & Productivity
| App | Description |
|---|---|
| 🎨 **Paint** | Pixel-level drawing application with freehand brush, color palette, and canvas |
| 📝 **Text Editor** | Multi-line text editor with keyboard input, scrolling, cursor navigation, and file save/load |
| 🧮 **Calculator** | Graphical calculator with button grid supporting basic arithmetic operations |

### Media & Documents
| App | Description |
|---|---|
| 🖼️ **Photos** | Image viewer supporting BMP and PNG formats with zoom and navigation |
| 🎬 **Video Player** | Embedded MPEG video playback with frame decoding and audio sync |
| 📄 **PDF Reader** | Full-featured PDF viewer powered by a native port of the **MuPDF** library — renders real PDF documents with fonts, images, and vector graphics |

### Internet & Communication
| App | Description |
|---|---|
| 🌐 **Web Browser** | Built-in web browser with custom HTML/DOM parser, CSS engine, JavaScript interpreter, and layout renderer — connects over raw TCP sockets |
| 📧 **Mail Client** | Full SMTP email client that can send real emails through Gmail — see [Email Setup Guide](#-email-setup-guide) below |

---

## 🌐 Networking Stack

PureOS implements a **complete TCP/IP networking stack from scratch** — no external libraries for the core protocols:

```mermaid
graph TB
    subgraph Physical
        NIC[Network Card<br>PCnet / NE2000]
    end

    subgraph Data Link
        ETH[Ethernet Frame<br>Parser & Builder]
    end

    subgraph Network
        ARP[ARP<br>Address Resolution]
        IPV4[IPv4<br>Packet Routing]
        DHCP[DHCP<br>Auto IP Config]
    end

    subgraph Transport
        TCP[TCP<br>Reliable Streams<br>3-Way Handshake<br>Retransmission]
        UDP_P[UDP<br>Datagram Protocol]
    end

    subgraph Application
        DNS[DNS<br>Domain Resolution]
        HTTP[HTTP 1.1<br>GET / POST Requests]
        SMTP[SMTP<br>Email Sending]
        TLS[TLS 1.2<br>Encrypted Connections<br>via BearSSL]
    end

    NIC --> ETH
    ETH --> ARP
    ETH --> IPV4
    IPV4 --> TCP
    IPV4 --> UDP_P
    UDP_P --> DNS
    UDP_P --> DHCP
    TCP --> HTTP
    TCP --> SMTP
    TCP --> TLS
    TLS --> HTTP
    TLS --> SMTP

    style TLS fill:#2d6a4f,color:#fff
    style SMTP fill:#e63946,color:#fff
    style HTTP fill:#457b9d,color:#fff
    style DNS fill:#f4a261,color:#000
```

### Protocol Capabilities

| Protocol | Implementation Details |
|---|---|
| **Ethernet** | Raw frame construction and parsing, MAC address handling |
| **ARP** | Address Resolution Protocol with ARP cache and request/reply |
| **IPv4** | Full IP packet routing, header checksum, fragmentation support |
| **DHCP** | Automatic IP address, subnet, gateway and DNS configuration |
| **UDP** | Connectionless datagram transport for DNS and DHCP |
| **TCP** | Full implementation: 3-way handshake, sequence tracking, ACK management, retransmission, connection teardown |
| **DNS** | Domain name resolution with query building and response parsing |
| **HTTP 1.1** | GET/POST requests, header parsing, chunked transfer decoding |
| **TLS 1.2** | Secure encrypted connections via an integrated **BearSSL** library port (RSA, AES, SHA-256, X.509 certificates) |
| **SMTP** | Authenticated email sending with STARTTLS / direct TLS (port 465) |

---

## 📧 Email Client (SMTP)

The built-in Mail app supports **composing and sending real emails** through Gmail's SMTP servers:

```mermaid
sequenceDiagram
    participant User as 👤 User
    participant MailApp as 📧 Mail App
    participant TCP as 🔌 TCP Stack
    participant TLS as 🔒 TLS/BearSSL
    participant Gmail as 📬 smtp.gmail.com

    User->>MailApp: Click "Compose"
    User->>MailApp: Fill To, Subject, Body
    User->>MailApp: Click "SEND"

    MailApp->>TCP: Connect to smtp.gmail.com:465
    TCP->>TLS: Establish TLS 1.2 Handshake
    TLS->>Gmail: Encrypted Connection Ready

    Gmail-->>MailApp: 220 Service Ready
    MailApp->>Gmail: EHLO pureos
    Gmail-->>MailApp: 250 OK

    MailApp->>Gmail: AUTH LOGIN (Base64 credentials)
    Gmail-->>MailApp: 235 Authentication Successful

    MailApp->>Gmail: MAIL FROM:<sender>
    Gmail-->>MailApp: 250 OK
    MailApp->>Gmail: RCPT TO:<recipient>
    Gmail-->>MailApp: 250 OK

    MailApp->>Gmail: DATA
    MailApp->>Gmail: Subject + Body + CRLF.CRLF
    Gmail-->>MailApp: 250 Message Accepted

    MailApp->>Gmail: QUIT
    MailApp-->>User: ✅ Email Sent Successfully!
```

### Mail App Features
- **Compose Mode** — To, Subject, and Body fields with Tab key navigation
- **Account Sidebar** — Multiple account support stored in `/mail/` directory
- **Message List** — View received emails with sender and subject preview
- **Message Reader** — Full email body display panel
- **POP3 Sync** — Fetch emails from POP3 servers (with demo/mock mode fallback)
- **Live SMTP Send** — Real email delivery through Gmail's secure SMTP servers

---

## 🌐 Web Browser

The PureOS browser is a fully native web renderer — no WebKit, no Chromium, no external engine:

```mermaid
graph LR
    URL[URL Input] --> HTTP_R[HTTP/HTTPS<br>Request]
    HTTP_R --> HTML[Raw HTML<br>Response]
    HTML --> DOM[DOM Parser<br>Build Element Tree]
    DOM --> CSS_P[CSS Parser<br>Style Resolution]
    CSS_P --> LAYOUT[Layout Engine<br>Block/Inline/Float]
    LAYOUT --> RENDER[Pixel Renderer<br>Draw to Window]
    RENDER --> SCREEN[Screen Output]

    DOM --> JS[JavaScript<br>Interpreter]
    JS --> DOM

    style JS fill:#f0ad4e,color:#000
    style CSS_P fill:#5bc0de,color:#000
    style DOM fill:#5cb85c,color:#fff
```

**Components:**
| Module | File | Capability |
|---|---|---|
| **DOM Parser** | `dom.c` | Parses raw HTML into a DOM element tree with tag attributes |
| **CSS Engine** | `css.c` | Parses inline and `<style>` CSS, resolves properties per element |
| **Layout Engine** | `layout.c` | Computes block/inline positioning, width/height, margins, padding |
| **JS Interpreter** | `js.c` | Basic JavaScript execution: variables, functions, DOM manipulation |
| **Browser Shell** | `browser.c` | URL bar, navigation, page fetch over HTTP/HTTPS, rendering orchestration |

---

## 💾 Filesystem Support

```mermaid
graph TB
    VFS[Virtual Filesystem Switch<br>Unified API]
    VFS --> FAT12[FAT12<br>Floppy Disks]
    VFS --> FAT16[FAT16<br>Small Partitions]
    VFS --> FAT32[FAT32<br>Modern Drives]
    VFS --> EXT2[Ext2<br>Linux Compatible]
    VFS --> RAMFS[RAMFS<br>In-Memory FS]
    VFS --> DEVFS[DevFS<br>Device Nodes]
    VFS --> PIPE[Pipes<br>IPC Channels]

    style VFS fill:#264653,color:#fff
    style FAT32 fill:#2a9d8f,color:#fff
    style EXT2 fill:#e9c46a,color:#000
    style RAMFS fill:#f4a261,color:#000
```

| Filesystem | Capabilities |
|---|---|
| **FAT12/16/32** | Full read/write, directory listing, file creation/deletion, long filename support |
| **Ext2** | Read support for Linux-compatible ext2 partitions |
| **RAMFS** | Fast in-memory filesystem for temporary data and mail storage |
| **DevFS** | Device file nodes (similar to Linux `/dev/`) |
| **Pipes** | Unix-style inter-process communication pipes |

---

## 🔧 Hardware & Drivers

| Category | Drivers |
|---|---|
| **Display** | VGA text mode, VGA graphics mode, Bochs BGA (high-res framebuffer) |
| **Input** | PS/2 Keyboard (scancode translation, shift/caps), PS/2 Mouse (movement + buttons) |
| **Storage** | ATA PIO, AHCI (SATA) |
| **Network** | AMD PCnet-PCI II, NE2000 (Realtek 8029) |
| **Audio** | Ensoniq ES1370 (AudioPCI), WAV file decoder and playback |
| **USB** | UHCI host controller, USB device enumeration |
| **System** | PCI bus enumeration, PIT timer, RTC real-time clock, PC speaker, ACPI, APIC, SMP multi-core |

---

## 🛠️ Build Instructions

### Requirements
- **OS:** Windows 10/11
- **Compiler:** `x86_64-elf-gcc` cross-compiler (included in `tools/` directory)
- **Assembler:** NASM
- **Scripting:** Python 3
- **Emulator:** Bochs, QEMU, or VirtualBox

### Steps

**1. First-time setup — build the PDF library (only needed once):**
```powershell
powershell -ExecutionPolicy Bypass -File build_mupdf.ps1
```

**2. Build the entire OS:**
```bat
.\build.bat
```

This compiles the bootloader, kernel, all drivers, GUI, applications, and networking stack, then produces:
- `os-image.bin` — Raw bootable binary
- `pureos.img` — Ready-to-run disk image for emulators

**3. Run in an emulator (Bochs example):**
```bat
.\run_bochs.bat
```

---

## 📧 Email Setup Guide

PureOS can send **real emails** through Gmail. To configure:

1. **Enable 2-Step Verification** on your Google Account
2. Go to **Google Account → Security → App Passwords**
3. Generate a new App Password (format: `abcd efgh ijkl mnop`)
4. Open `src/apps/mail/mail_app.c` and update line ~376:
   ```c
   const char *user = "your_email@gmail.com";
   const char *pass = "abcd efgh ijkl mnop";  // Your 16-char App Password
   ```
5. Rebuild with `.\build.bat` and launch the OS
6. Open **Mail** from the Start Menu → Click **Compose** → Fill in recipient, subject, body → Click **SEND**

> ⚠️ **Security Warning:** Never commit your real App Password to a public Git repository. Always replace it with a placeholder before pushing to GitHub.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

<div align="center">
  <b>Built with ❤️ from scratch — no Linux, no POSIX, no borrowed OS code.</b>
</div>
