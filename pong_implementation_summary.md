# 🏓 Pong Game — PureOS Integration Summary

> **Commit**: `9805b0b` → pushed to [github.com/Rudrapatel330/PureOS](https://github.com/Rudrapatel330/PureOS)
> **Files Changed**: 7 files, +436 lines, -12 lines

![Pong Icon](C:\Users\Lenovo\.gemini\antigravity\brain\826d2d18-528d-4c44-9efd-8c1ae6f8dd89\pong_icon.png)

---

## 🎮 Game Modes

| Mode | Description | Controls |
|------|-------------|----------|
| **1 Player** | You vs AI Bot | ↑↓ Arrow Keys |
| **2 Players** | Local multiplayer | P1: `W`/`S` — P2: ↑↓ Arrow Keys |
| **Bot vs Bot** | Spectator mode — two AIs battle it out | None (just watch!) |

---

## 🏗️ System Architecture

The Pong game is deeply integrated into PureOS's kernel event loop, not running as an isolated process. Here's how every component connects:

```mermaid
graph TD
    A["🖥️ Kernel Main Loop<br/>(kernel.c → tick_elapsed)"] --> B["🏓 pong_update()<br/>Called every frame"]
    B --> C["⌨️ key_state[] Polling<br/>(keyboard.c)"]
    B --> D["⚽ Ball Physics<br/>Position + Velocity"]
    B --> E["🤖 Bot AI<br/>Trajectory Prediction"]
    B --> F["🔊 AC97 Sound<br/>PCM Square Waves"]
    
    C --> G["Paddle Movement<br/>8px/frame, 60fps"]
    D --> H["Collision Detection<br/>Walls + Paddles"]
    H --> F
    E --> I["Bounce Simulation<br/>5-bounce lookahead"]
    
    J["🖱️ Mouse Click"] --> K["Mode Selection<br/>Menu Screen"]
    K --> B
    
    F --> L["ac97_play_pcm()<br/>DMA to Sound Card"]
    L --> M["🔈 VirtualBox ICH AC97<br/>Host Audio Output"]
```

---

## 🔊 Audio Pipeline — From Math to Speaker

Instead of using the primitive PC Speaker (which VirtualBox ignores with ICH AC97), we synthesize digital audio waveforms in memory and send them directly to the sound card via DMA.

```mermaid
sequenceDiagram
    participant P as Pong Game
    participant G as generate_beep()
    participant K as kmalloc_ap()
    participant A as ac97_play_pcm()
    participant H as ICH AC97 Hardware

    Note over P: Game starts
    P->>G: Generate 3 beeps (226Hz, 459Hz, 490Hz)
    G->>K: Allocate physically contiguous memory
    K-->>G: Returns virtual ptr + physical address
    G->>G: Fill buffer with 48kHz 16-bit stereo square wave
    G-->>P: Store physical addresses

    Note over P: Ball hits paddle
    P->>A: ac97_play_pcm(paddle_phys, size, 48000, 16, 2)
    A->>H: Program BDL with physical address
    A->>H: Set DAC2 frame count + start DMA
    H-->>H: DMA reads PCM → analog output
    Note over H: 🔈 "Boop!" (459Hz for 96ms)
```

### Historical Arcade Frequencies

| Game Event | Frequency | Duration | Sound Character |
|-----------|-----------|----------|-----------------|
| Ball → Wall | 226 Hz | 16 ms | Quick, low "tick" |
| Ball → Paddle | 459 Hz | 96 ms | Satisfying mid "boop" |
| Player Scores | 490 Hz | 257 ms | Long, harsh buzz |

> **Fun Fact**: 459 Hz is exactly 2× 226 Hz — a perfect octave. Allan Alcorn designed the original 1972 Pong audio using the machine's own video sync frequencies!

---

## ⌨️ Input System — Zero-Delay Keyboard Polling

### The Problem with Standard Keyboard Input

```mermaid
graph LR
    A["Key Pressed"] -->|"Instant"| B["1st Event"]
    B -->|"~500ms Typematic Delay"| C["Nothing happens!"]
    C -->|"Then rapid repeats"| D["Events flood in"]
    
    style C fill:#ff4444,color:#fff
```

### Our Solution: Direct `key_state[]` Polling

```mermaid
graph LR
    A["Keyboard IRQ"] -->|"Scancode"| B["keyboard.c handler"]
    B -->|"Press: key_state[sc] = 1"| C["key_state[256] array"]
    B -->|"Release: key_state[sc] = 0"| C
    
    D["pong_update()<br/>Every frame @ 60fps"] -->|"Reads"| C
    D -->|"key_state[0x48]?"| E["Move paddle UP 8px"]
    D -->|"key_state[0x50]?"| F["Move paddle DOWN 8px"]
    
    style C fill:#44aa44,color:#fff
    style D fill:#4488ff,color:#fff
```

> The paddle moves at a constant 8 pixels per frame with **zero delay** — it starts moving the instant you press and stops the instant you release. No typematic stutter.

---

## 🤖 Bot AI — Trajectory Prediction

The AI doesn't just "follow the ball." It simulates the ball's future path including wall bounces:

```mermaid
graph TD
    A["Ball moving toward bot's paddle?"] -->|Yes| B["Calculate time to reach paddle X"]
    A -->|No| C["Drift toward screen center"]
    B --> D["Predict Y = ball_y + ball_vy × time"]
    D --> E{"Predicted Y out of bounds?"}
    E -->|"Y < 32 (top wall)"| F["Reflect: Y = 64 - Y"]
    E -->|"Y > height (bottom)"| G["Reflect: Y = 2×height - Y"]
    E -->|"In bounds"| H["Move toward predicted Y"]
    F --> I{"Bounces < 5?"}
    G --> I
    I -->|Yes| E
    I -->|No| H
    H --> J["Paddle slides at 5px/frame<br/>(intentionally slower than player's 8px)"]
    
    style A fill:#9C27B0,color:#fff
    style H fill:#4CAF50,color:#fff
```

### Bot vs Bot Asymmetry

| Property | Bot 1 (Blue/Left) | Bot 2 (Red/Right) |
|----------|-------------------|-------------------|
| Tracking Speed | 5.5 px/frame | 5.0 px/frame |
| Dead Zone | ±8 px | ±10 px |
| Return Speed | 2.5 px/frame | 2.0 px/frame |

> This slight asymmetry ensures Bot vs Bot matches aren't perfectly mirrored — one bot occasionally outplays the other!

---

## 📁 Files Changed

```mermaid
graph TB
    subgraph "New Files"
        A["src/apps/pong.c<br/>📄 ~330 lines"]
        B["icons/pong.png<br/>🖼️ Game icon"]
    end
    
    subgraph "Modified Files"
        C["src/drivers/keyboard.c<br/>+ key_state[256] array"]
        D["src/gui/startmenu.c<br/>+ Pong menu entry with icon"]
        E["src/kernel/apps.c<br/>+ pong_init registration"]
        F["src/kernel/kernel.c<br/>+ pong_update in tick loop"]
        G["build.bat<br/>+ pong.c compilation & linking"]
    end
    
    A --> F
    A --> C
    A --> E
    B --> D
    
    style A fill:#4CAF50,color:#fff
    style B fill:#4CAF50,color:#fff
```

| File | Change | Purpose |
|------|--------|---------|
| [pong.c](file:///d:/1os-copy/imp2%20current%20one/1os/src/apps/pong.c) | **NEW** | Complete game: physics, rendering, AI, audio |
| [keyboard.c](file:///d:/1os-copy/imp2%20current%20one/1os/src/drivers/keyboard.c) | **Modified** | Added `key_state[256]` for real-time key tracking |
| [startmenu.c](file:///d:/1os-copy/imp2%20current%20one/1os/src/gui/startmenu.c) | **Modified** | Added Pong to pinned apps with icon |
| [apps.c](file:///d:/1os-copy/imp2%20current%20one/1os/src/kernel/apps.c) | **Modified** | Registered `pong_init` in app list |
| [kernel.c](file:///d:/1os-copy/imp2%20current%20one/1os/src/kernel/kernel.c) | **Modified** | Hooked `pong_update` into main tick loop |
| [build.bat](file:///d:/1os-copy/imp2%20current%20one/1os/build.bat) | **Modified** | Added pong.c to compilation and linking pipeline |

---

## 🚀 How to Build & Run

```bash
# Build the OS, create disk image, convert to VDI
.\build.bat
python make_debug_disk.py
& "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" convertfromraw pureos.img pureos.vdi --format VDI

# Attach to VirtualBox VM and boot!
```

> [!IMPORTANT]
> VirtualBox Audio must be set to **ICH AC97** with **Audio Output enabled** for game sounds to work. Do NOT change to SoundBlaster 16 — it will break the Recorder and Phone apps.
