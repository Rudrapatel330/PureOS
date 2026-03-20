#include "../kernel/hal/isr.h"
#include "../kernel/task.h"
#include "../kernel/window.h"
#include <stdint.h>

// Port I/O
static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

// State
int shift_pressed = 0;
int alt_pressed = 0;
int ctrl_pressed = 0;
int caps_lock = 0;

// Keyboard Buffer
#define KB_BUFFER_SIZE 256
static char kb_buffer[KB_BUFFER_SIZE];
static int kb_head = 0;
static int kb_tail = 0;

void kb_buffer_push(char c) {
  int next = (kb_tail + 1) % KB_BUFFER_SIZE;
  if (next != kb_head) {
    kb_buffer[kb_tail] = c;
    kb_tail = next;
  }
}

char keyboard_getc() {
  if (kb_head == kb_tail)
    return 0;
  char c = kb_buffer[kb_head];
  kb_head = (kb_head + 1) % KB_BUFFER_SIZE;
  return c;
}

char *keyboard_get_buffer() {
  // Legacy support: returns pointer to the start of the ring buffer (not ideal
  // but avoids breaks)
  return &kb_buffer[kb_head];
}

void keyboard_reset_buffer() { kb_head = kb_tail = 0; }

// Scancodes (US QWERTY)
const char scancode_ascii_lower[] = {
    0,   27,   '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0};

const char scancode_ascii_upper[] = {
    0,   27,   '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0};

// Keyboard Handler
// Polling Handler
void receive_keyboard_byte(uint8_t scancode) {
  // Check for Shift Key
  if (scancode == 0x2A || scancode == 0x36) {
    shift_pressed = 1;
  } else if (scancode == 0xAA || scancode == 0xB6) {
    shift_pressed = 0;
  }
  // Alt Key
  else if (scancode == 0x38) {
    alt_pressed = 1;
  } else if (scancode == 0xB8) {
    alt_pressed = 0;
    extern int alt_tab_active;
    if (alt_tab_active) {
      alt_tab_active = 0;
      extern int ui_dirty;
      ui_dirty = 1;
    }
  }
  // Ctrl Key
  else if (scancode == 0x1D) {
    ctrl_pressed = 1;
  } else if (scancode == 0x9D) {
    ctrl_pressed = 0;
  }

  // Ignore Break Codes (Key Release) for other keys
  else if (!(scancode & 0x80)) {
    // Global shortcuts (intercept before window forwarding)
    if (scancode == 0x32) { // M
      if (alt_pressed && shift_pressed) {
        extern int magnifier_enabled;
        magnifier_enabled = !magnifier_enabled;
        extern void compositor_invalidate_rect(int x, int y, int w, int h);
        extern int screen_width, screen_height;
        compositor_invalidate_rect(0, 0, screen_width, screen_height);
        extern int ui_dirty;
        ui_dirty = 1;
        return;
      }
    }
    if (alt_pressed) {
      if (scancode == 0x3E) { // F4
        extern void winmgr_close_active(void);
        winmgr_close_active();
        extern int ui_dirty;
        ui_dirty = 1;
        return;
      }
      if (scancode == 0x0F) { // Tab (Alt+Tab)
        extern void winmgr_cycle_window(void);
        extern int alt_tab_active;
        alt_tab_active = 1;
        winmgr_cycle_window();
        extern int ui_dirty;
        ui_dirty = 1;
        return;
      }
      if (scancode == 0x39) { // Space
        msg_t m;
        m.type = MSG_SHOW_SEARCH;
        msg_send_to_name("desktop", &m);
        return;
      }
    }
    // F3 = Mission Control toggle (no modifier needed)
    if (scancode == 0x3D) {
      extern int mission_control_active;
      mission_control_active = !mission_control_active;
      extern int ui_dirty;
      ui_dirty = 1;
      extern void compositor_invalidate_rect(int x, int y, int w, int h);
      extern int screen_width, screen_height;
      compositor_invalidate_rect(0, 0, screen_width, screen_height);
      return;
    }
    if (ctrl_pressed) {
      if (scancode == 0x0F) { // Tab (Ctrl+Tab)
        extern void winmgr_cycle_window(void);
        winmgr_cycle_window();
        extern int ui_dirty;
        ui_dirty = 1;
        return;
      }
      // Clipboard Shortcuts
      if (scancode == 0x2E) { // C
        extern void print_serial(const char *);
        print_serial("KEYBOARD.C: Ctrl+C detected.\n");
        extern void winmgr_handle_copy(void);
        winmgr_handle_copy();
        return;
      }
      if (scancode == 0x2F) { // V
        extern void print_serial(const char *);
        print_serial("KEYBOARD.C: Ctrl+V detected.\n");
        extern void winmgr_handle_paste(void);
        winmgr_handle_paste();
        return;
      }
      if (scancode == 0x2D) { // X
        extern void print_serial(const char *);
        print_serial("KEYBOARD.C: Ctrl+X detected.\n");
        extern void winmgr_handle_cut(void);
        winmgr_handle_cut();
        return;
      }
    }

    // Lookup ASCII
    char ascii = 0;

    // Special Key Handling
    if (scancode == 0x1C)
      ascii = '\n'; // Enter
    else if (scancode == 0x0E)
      ascii = '\b'; // Backspace
    else if (scancode < sizeof(scancode_ascii_lower)) {
      if (shift_pressed) {
        ascii = scancode_ascii_upper[scancode];
      } else {
        ascii = scancode_ascii_lower[scancode];
      }
    }

    if (ascii != 0 || scancode == 0x1C || scancode == 0x0E) {
      // Push to keyboard buffer
      kb_buffer_push(ascii);

      // Send to window system
      extern int search_active;
      if (search_active) {
        extern void desktop_handle_search_key(int, char);
        desktop_handle_search_key(scancode, ascii);
      } else {
        window_handle_key(scancode, ascii);
      }
    }
    // Special keys (no ASCII) - still forward scancode
    // Arrows: 0x48, 0x4B (Left), 0x4D (Right), 0x50
    // F-Keys: F1-F10 (0x3B-0x44), F11(0x57), F12(0x58)
    else if ((scancode >= 0x3B && scancode <= 0x44) || scancode == 0x57 ||
             scancode == 0x58 || scancode == 0x48 || scancode == 0x4B ||
             scancode == 0x4D || scancode == 0x50 || scancode == 0x49 ||
             scancode == 0x51) {
      if (scancode == 0x58) { // F12
        extern void debug_shell();
        debug_shell();
        return;
      }
      if (scancode == 0x44) { // F10
        static int p_enabled = 0;
        p_enabled = !p_enabled;
        extern void profiler_enable(int);
        extern void profiler_dump();
        profiler_enable(p_enabled);
        if (!p_enabled)
          profiler_dump();
        return;
      }
      extern int search_active;
      if (search_active) {
        extern void desktop_handle_search_key(int, char);
        desktop_handle_search_key(scancode, 0);
      } else {
        window_handle_key(scancode, 0);
      }
    }
    // Print Screen (0x37)
    if (scancode == 0x37) {
      msg_t m;
      m.type = MSG_SCREENSHOT;
      msg_send_to_name("desktop", &m);
      return;
    }
  }
}

// raw_kb_buffer removed in favor of IPC msg_queue
void keyboard_process_queue(void) {
  // Legacy support for manual processing if needed,
  // but now handled by Desktop task message loop
}

uint64_t keyboard_handler(registers_t *regs) {
  uint8_t status = inb(0x64);

  if ((status & 0x01) && !(status & 0x20)) {
    uint8_t scancode = inb(0x60);

    // During lockscreen, process directly (bypass IPC for reliability)
    extern int lockscreen_active;
    if (lockscreen_active) {
      receive_keyboard_byte(scancode);
    } else {
      // Normal mode: Send to Desktop thread via IPC
      msg_t m;
      m.type = MSG_KEYBOARD;
      m.d1 = scancode;
      m.d2 = 0;
      msg_send_to_name("desktop", &m);
    }
  }

  return (uint64_t)regs;
}

void init_keyboard() {
  // Register keyboard interrupt
  register_interrupt_handler(33, keyboard_handler);

  // Flush buffer
  while (inb(0x64) & 0x01) {
    inb(0x60);
  }
}
