// mouse.c - PS/2 Mouse Driver
#include "mouse.h"
#include "ports.h"

extern int mouse_x;
extern int mouse_y;
extern int ui_dirty;
extern int screen_width;
extern int screen_height;

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);
void mouse_handler(int dx, int dy, int buttons, int scroll);
void receive_mouse_byte(uint8_t byte);

static void ps2_wait_input(void) {
  int timeout = 1000000;
  while (timeout-- > 0) {
    if (!(inb(0x64) & 0x02))
      return;
  }
}

static void ps2_wait_output(void) {
  int timeout = 1000000;
  while (timeout-- > 0) {
    if (inb(0x64) & 0x01)
      return;
  }
}

static void ps2_write_ctrl(uint8_t cmd) {
  ps2_wait_input();
  outb(0x64, cmd);
}

static void ps2_mouse_write(uint8_t data) {
  ps2_wait_input();
  outb(0x64, 0xD4);
  ps2_wait_input();
  outb(0x60, data);
}

// Low-level helper to wait for and consume an ACK (0xFA)
static int ps2_mouse_expect_ack(const char *cmd_name) {
  ps2_wait_output();
  uint8_t ack = inb(0x60);
  if (ack == 0xFA) {
    return 1;
  }
  print_serial("MOUSE: ACK FAIL [");
  print_serial(cmd_name);
  print_serial("] got ");
  char buf[16];
  k_itoa(ack, buf);
  print_serial(buf);
  print_serial("\n");
  return 0;
}

static int ps2_mouse_send_cmd(uint8_t data, const char *name) {
  for (int retry = 0; retry < 3; retry++) {
    ps2_mouse_write(data);
    if (ps2_mouse_expect_ack(name)) {
      return 1;
    }
  }
  return 0;
}

// Global flag
static int has_scroll_wheel = 0;

void init_ps2_mouse(void) {
  print_serial("MOUSE: Starting Robust Init...\n");

  // 1. Enable Aux Device
  ps2_write_ctrl(0xA8);
  // Some controllers might not ACK 0xA8, but we'll try to sync anyway

  // 2. Device Reset
  ps2_mouse_write(0xFF);
  if (ps2_mouse_expect_ack("Reset")) {
    ps2_wait_output();
    uint8_t bat = inb(0x60); // BAT (0xAA)
    ps2_wait_output();
    inb(0x60); // Consume ID after reset
    if (bat != 0xAA)
      print_serial("MOUSE: Reset BAT failed\n");
  }

  // 3. Set Defaults
  ps2_mouse_send_cmd(0xF6, "Set Defaults");

  // 4. --- IntelliMouse "Knock" Sequence ---
  print_serial("MOUSE: Sending IntelliMouse Knock...\n");
  ps2_mouse_send_cmd(0xF3, "Set Rate Cmd 1");
  ps2_mouse_send_cmd(200, "Rate 200");
  ps2_mouse_send_cmd(0xF3, "Set Rate Cmd 2");
  ps2_mouse_send_cmd(100, "Rate 100");
  ps2_mouse_send_cmd(0xF3, "Set Rate Cmd 3");
  ps2_mouse_send_cmd(80, "Rate 80");

  // 5. Verify ID
  ps2_mouse_write(0xF2); // Get ID
  if (ps2_mouse_expect_ack("Get ID")) {
    ps2_wait_output();
    uint8_t mouse_id = inb(0x60);
    if (mouse_id == 0x03) {
      print_serial("MOUSE: IntelliMouse DETECTED (ID 3)\n");
      has_scroll_wheel = 1;
    } else {
      print_serial("MOUSE: Standard mouse (ID ");
      char idbuf[16];
      k_itoa(mouse_id, idbuf);
      print_serial(idbuf);
      print_serial(")\n");
      has_scroll_wheel = 0;
    }
  }

  // 6. Enable Interrupts in CCB
  ps2_write_ctrl(0x20); // Read
  ps2_wait_output();
  uint8_t ccb = inb(0x60);
  ccb |= 0x02;          // Enable IRQ12
  ccb &= ~0x20;         // Enable Clock
  ps2_write_ctrl(0x60); // Write
  ps2_wait_input();
  outb(0x60, ccb);

  // 7. Enable Data Reporting
  ps2_mouse_send_cmd(0xF4, "Enable Reporting");

  register_interrupt_handler(44, &mouse_callback);
  print_serial("MOUSE: Init Complete.\n");
}

uint64_t mouse_callback(registers_t *regs) {
  (void)regs;
  uint8_t status = inb(0x64);
  if (status & 0x01) {
    uint8_t byte = inb(0x60);
    if (status & 0x20) {
      receive_mouse_byte(byte);
    }
  }
  return (uint64_t)regs;
}

#include "../kernel/task.h"

void receive_mouse_byte(uint8_t byte) {
  static uint8_t cycle = 0;
  static uint8_t data[4];

  // Synchronization check
  if (cycle == 0 && !(byte & 0x08)) {
    return;
  }

  data[cycle++] = byte;

  int packet_size = has_scroll_wheel ? 4 : 3;

  if (cycle == packet_size) {
    cycle = 0; // Reset for the next packet

    // Check for overflow bits. If set, the packet is invalid.
    if (data[0] & 0xC0) {
      return;
    }

    // Decode movement
    int dx = data[1];
    int dy = data[2];
    if (data[0] & 0x10)
      dx -= 256;
    if (data[0] & 0x20)
      dy -= 256;

    int buttons = data[0] & 0x07;
    int scroll = 0;

    if (has_scroll_wheel) {
      // The 4th byte standard IntelliMouse:
      // Bit 0-3: Z movement (2's complement, -8 to +7)
      // Bits 4-5: 4th and 5th buttons
      int8_t z = (int8_t)(data[3] & 0x0F);
      // Correct sign extension for 4-bit
      if (z & 0x08)
        z |= 0xF0;

      scroll = (int)z;
    }

    if (dx != 0 || dy != 0 || buttons != 0 || scroll != 0) {
      msg_t m;
      m.type = MSG_MOUSE_MOVE;
      m.d1 = dx;
      m.d2 = dy;
      m.d3 = buttons;
      m.d4 = scroll;
      msg_send_to_name("desktop", &m);
    }
  }
}

extern void mouse_handler(int dx, int dy, int buttons, int scroll);

void init_mouse(void) {
  // Nothing
}
