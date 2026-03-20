#include "debug.h"
#include "../drivers/bga.h"
#include "../kernel/string.h"
#include "../kernel/task.h"

extern void print_serial(const char *);
extern const uint8_t font8x8_basic[256][8];

static void debug_draw_char(int x, int y, char c, uint32_t color) {
  if (!bga_lfb)
    return;
  uint8_t *glyph = (uint8_t *)font8x8_basic[(uint8_t)c];
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      if (glyph[j] & (1 << (7 - i))) {
        int px = x + i;
        int py = y + j;
        if (px >= 0 && px < bga_width && py >= 0 && py < bga_height) {
          bga_lfb[py * bga_width + px] = color;
        }
      }
    }
  }
}

static void debug_print_text(int x, int y, const char *text, uint32_t color) {
  int cur_x = x;
  while (*text) {
    if (*text == '\n') {
      y += 10;
      cur_x = x;
    } else {
      debug_draw_char(cur_x, y, *text, color);
      cur_x += 8;
    }
    text++;
  }
}

void debug_panic_screen(const char *message, registers_t *regs) {
  if (!bga_lfb) {
    print_serial("PANIC: ");
    print_serial(message);
    print_serial(" (No LFB available for visual panic)\n");
    return;
  }

  // Force mode to page 0
  bga_write_register(VBE_DISPI_INDEX_Y_OFFSET, 0);

  // Blue screen of death (PureOS style)
  for (int i = 0; i < bga_width * bga_height; i++) {
    bga_lfb[i] = 0xFF003366;
  }

  int y = 50;
  debug_print_text(50, y, "!!! PUREOS KERNEL PANIC !!!", 0xFFFFFFFF);
  y += 30;
  debug_print_text(50, y, "Message: ", 0xFFCCCCCC);
  debug_print_text(150, y, message, 0xFFFF6464);

  y += 40;
  debug_print_text(50, y, "CPU STATE:", 0xFFFFFFFF);
  y += 20;

  char buf[32];
  debug_print_text(50, y, "RAX: 0x", 0xFFBBBBBB);
  k_itoa_hex(regs->rax, buf);
  debug_print_text(120, y, buf, 0xFFFFFF);
  debug_print_text(300, y, "RBX: 0x", 0xFFBBBBBB);
  k_itoa_hex(regs->rbx, buf);
  debug_print_text(370, y, buf, 0xFFFFFF);
  y += 12;
  debug_print_text(50, y, "RCX: 0x", 0xFFBBBBBB);
  k_itoa_hex(regs->rcx, buf);
  debug_print_text(120, y, buf, 0xFFFFFF);
  debug_print_text(300, y, "RDX: 0x", 0xFFBBBBBB);
  k_itoa_hex(regs->rdx, buf);
  debug_print_text(370, y, buf, 0xFFFFFF);
  y += 12;
  debug_print_text(50, y, "RIP: 0x", 0xFFBBBBBB);
  k_itoa_hex(regs->rip, buf);
  debug_print_text(120, y, buf, 0xFFFF6464);
  debug_print_text(300, y, "RSP: 0x", 0xFFBBBBBB);
  k_itoa_hex(regs->rsp, buf);
  debug_print_text(370, y, buf, 0xFFFFFF);

  y += 40;
  debug_print_text(50, y, "SYSTEM STATUS:", 0xFFFFFFFF);
  y += 20;
  task_t *curr = get_current_task();
  if (curr) {
    debug_print_text(50, y, "Active Task: ", 0xFFBBBBBB);
    debug_print_text(150, y, curr->name, 0xFFFFFF);
    debug_print_text(300, y, " (PID: ", 0xFFBBBBBB);
    k_itoa(curr->id, buf);
    debug_print_text(360, y, buf, 0xFFFFFF);
    debug_print_text(380, y, ")", 0xFFBBBBBB);
  }

  y += 40;
  debug_print_text(50, y, "The system has been halted to prevent damage.",
                   0xFFEEEEEE);
  debug_print_text(50, y + 15, "Please restart your computer.", 0xFFEEEEEE);
}

void debug_dump_heap() {
  extern void heap_stats();
  print_serial("\n--- HEAP STATUS ---\n");
  heap_stats();
}

void debug_dump_tasks() {
  extern task_t *get_current_task();
  print_serial("\n--- TASK LIST ---\n");
  task_t *curr = get_current_task();
  if (curr) {
    print_serial("Current PID: ");
    char buf[16];
    k_itoa(curr->id, buf);
    print_serial(buf);
    print_serial(" (");
    print_serial(curr->name);
    print_serial(")\n");
  }
}

void debug_shell() {
  print_serial("\n=== PureOS Debug Shell (v1) ===\n");
  print_serial("F12 Pressed. System state saved to serial.\n");

  debug_dump_heap();
  debug_dump_tasks();

  print_serial("Continuing execution...\n");
}
