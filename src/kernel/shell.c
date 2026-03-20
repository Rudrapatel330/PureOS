#include "../drivers/vga.h"
#include "../fs/fat12.h"
#include "../gui/window.h"
#include "string.h"
#include "task.h"
#include <stddef.h>
#include <stdint.h>

extern void terminal_clear();

void shell_execute(Window *win, char *cmd) {
  if (!win)
    return;

  // Trim
  int len = strlen(cmd);
  while (len > 0 && (cmd[len - 1] == '\n' || cmd[len - 1] == ' ')) {
    cmd[--len] = 0;
  }
  if (len == 0)
    return;

  if (strcmp(cmd, "help") == 0) {
    terminal_print(win, "Available commands:\n");
    terminal_print(win, "  ls       - List files\n");
    terminal_print(win, "  cat [f]  - View file\n");
    terminal_print(win, "  del [f]  - Delete file\n");
    terminal_print(win, "  ren [a] [b] - Rename a to b\n");
    terminal_print(win, "  copy [a] [b] - Copy a to b\n");
    terminal_print(win, "  write [f] [t] - Write text to file\n");
    terminal_print(win, "  exec [f] - Run binary\n");
    terminal_print(win, "  clear    - Clear screen\n");
    terminal_print(win, "  memtest  - Test Heap\n");
    terminal_print(win, "  rect     - Draw Demo\n");
    terminal_print(win, "  reboot   - Restart\n");
    terminal_print(win, "  dbg tasks  - List tasks\n");
    terminal_print(win, "  dbg heap   - Heap stats\n");
    terminal_print(win, "  dbg mem [addr] - Memory dump\n");
    terminal_print(win, "  dbg kill [pid] - Kill task\n");
  } else if (strcmp(cmd, "ls") == 0) {
    // We need a version of fs_list that outputs to string or prints to
    // terminal fs_list() currently calls print_str() which maps to
    // serial/debug or raw VGA. We should redirect print_str or rewrite
    // fs_list to use terminal_print. For now, let's just use the FileInfo
    // list.
    FileInfo files[16];
    int count = fs_list_files(files, 16);
    if (count == 0)
      terminal_print(win, "(No files)\n");
    for (int i = 0; i < count; i++) {
      terminal_print(win, files[i].name);
      if (files[i].is_dir)
        terminal_print(win, " <DIR>");
      else {
        terminal_print(win, " ");
        // char num[16]; // Unused
        // manual itoa for size
        int sz = files[i].size;
        if (sz == 0)
          terminal_print(win, "0");
        else {
          // Lazy way
          int d = 1;
          while (sz / d >= 10)
            d *= 10;
          while (d > 0) {
            char c[2] = {(sz / d) + '0', 0};
            terminal_print(win, c);
            sz %= d;
            d /= 10;
          }
        }
        terminal_print(win, "b");
      }
      terminal_print(win, "\n");
    }
  } else if (strcmp(cmd, "clear") == 0) {
    terminal_clear();
  } else if (strncmp(cmd, "cd ", 3) == 0) {
    char *path = cmd + 3;
    if (fs_cd(path)) {
      // Success
    } else {
      terminal_print(win, "Directory not found.\n");
    }
  } else if (strncmp(cmd, "mkdir ", 6) == 0) {
    char *name = cmd + 6;
    fs_mkdir(name); // Prints TODO
  } else if (strcmp(cmd, "pwd") == 0) {
    char buffer[256];
    fs_pwd(buffer);
    terminal_print(win, buffer);
    terminal_print(win, "\n");
  } else if (strncmp(cmd, "cat ", 4) == 0) {
    char *filename = cmd + 4;
    static uint8_t file_buffer[512];
    memset(file_buffer, 0, 512);
    if (fs_read(filename, file_buffer)) {
      terminal_print(win, (char *)file_buffer);
      terminal_print(win, "\n");
    } else {
      terminal_print(win, "File not found.\n");
    }
  }
  // DELETE
  else if (strncmp(cmd, "del ", 4) == 0) {
    char *filename = cmd + 4;
    if (fs_delete(filename))
      terminal_print(win, "Deleted.\n");
    else
      terminal_print(win, "File not found.\n");
  }
  // RENAME
  else if (strncmp(cmd, "ren ", 4) == 0) {
    char *args = cmd + 4;
    char *space = strchr(args, ' ');
    if (space) {
      *space = 0;
      char *oldn = args;
      char *newn = space + 1;
      if (fs_rename(oldn, newn))
        terminal_print(win, "Renamed.\n");
      else
        terminal_print(win, "Failed.\n");
    } else
      terminal_print(win, "Usage: ren old new\n");
  }
  // COPY
  else if (strncmp(cmd, "copy ", 5) == 0) {
    char *args = cmd + 5;
    char *space = strchr(args, ' ');
    if (space) {
      *space = 0;
      char *src = args;
      char *dst = space + 1;

      static uint8_t copy_buf[512];
      memset(copy_buf, 0, 512);

      // Need size... fs_read doesn't return size easily with current API.
      // But we can check file list? Or update fs_read to return size?
      // fs_read return 1/0.
      // For now, assume <512 bytes and just write what we read (string?
      // binary?). FAT12 blocks are 512.

      if (fs_read(src, copy_buf)) {
        // How much to write? strlen if text...
        // If binary, we don't know size without lookup.
        // Hack: Write 512? Or strlen.
        // Let's assume text for now.
        int sz = strlen((char *)copy_buf);
        if (fs_write(dst, copy_buf, sz))
          terminal_print(win, "Copied.\n");
        else
          terminal_print(win, "Write failed.\n");
      } else
        terminal_print(win, "Source not found.\n");
    } else
      terminal_print(win, "Usage: copy src dst\n");
  }
  // WRITE
  else if (strncmp(cmd, "write ", 6) == 0) {
    char *args = cmd + 6;
    char *space = strchr(args, ' ');
    if (space) {
      *space = 0;
      if (fs_write(args, (uint8_t *)(space + 1), strlen(space + 1)))
        terminal_print(win, "File written.\n");
      else
        terminal_print(win, "Write failed.\n");
    }
  }
  // ===================== KERNEL DEBUGGER =====================
  else if (strcmp(cmd, "dbg tasks") == 0) {
    extern task_t *get_current_task();
    task_t *cur = get_current_task();
    if (!cur) {
      terminal_print(win, "No tasks.\n");
    } else {
      terminal_print(win, "PID  PRI  STATE  NAME\n");
      terminal_print(win, "---- ---- ------ ----\n");
      task_t *t = cur;
      char buf[64];
      do {
        // Format: PID
        k_itoa(t->id, buf);
        terminal_print(win, buf);
        terminal_print(win, "    ");
        // Priority
        k_itoa(t->priority, buf);
        terminal_print(win, buf);
        terminal_print(win, "    ");
        // State
        const char *states[] = {"RUN", "RDY", "STP", "ZMB"};
        if (t->state >= 0 && t->state <= 3)
          terminal_print(win, states[t->state]);
        else
          terminal_print(win, "???");
        terminal_print(win, "    ");
        // Name
        terminal_print(win, t->name);
        terminal_print(win, "\n");
        t = t->next;
      } while (t && t != cur);
    }
  } else if (strcmp(cmd, "dbg heap") == 0) {
    extern uint32_t heap_get_used_bytes();
    extern uint32_t heap_get_total_bytes();
    extern void heap_stats();
    heap_stats(); // Prints to serial

    char buf[32];
    terminal_print(win, "Heap Total: ");
    k_itoa(heap_get_total_bytes() / 1024, buf);
    terminal_print(win, buf);
    terminal_print(win, " KB\n");

    terminal_print(win, "Heap Used:  ");
    k_itoa(heap_get_used_bytes() / 1024, buf);
    terminal_print(win, buf);
    terminal_print(win, " KB\n");

    terminal_print(win, "Heap Free:  ");
    k_itoa((heap_get_total_bytes() - heap_get_used_bytes()) / 1024, buf);
    terminal_print(win, buf);
    terminal_print(win, " KB\n");
  } else if (strncmp(cmd, "dbg mem ", 8) == 0) {
    // Parse hex address
    char *addr_str = cmd + 8;
    uint32_t addr = 0;
    for (int i = 0; addr_str[i]; i++) {
      char c = addr_str[i];
      addr <<= 4;
      if (c >= '0' && c <= '9')
        addr |= (c - '0');
      else if (c >= 'a' && c <= 'f')
        addr |= (c - 'a' + 10);
      else if (c >= 'A' && c <= 'F')
        addr |= (c - 'A' + 10);
    }

    // Dump 64 bytes at addr
    char buf[8];
    terminal_print(win, "Memory @ 0x");
    k_itoa_hex(addr, buf);
    terminal_print(win, buf);
    terminal_print(win, ":\n");

    uint8_t *ptr = (uint8_t *)(uintptr_t)addr;
    for (int row = 0; row < 4; row++) {
      // Address prefix
      k_itoa_hex(addr + row * 16, buf);
      terminal_print(win, buf);
      terminal_print(win, ": ");
      for (int col = 0; col < 16; col++) {
        uint8_t b = ptr[row * 16 + col];
        // Print hex byte
        char hex[4];
        hex[0] = "0123456789ABCDEF"[b >> 4];
        hex[1] = "0123456789ABCDEF"[b & 0xF];
        hex[2] = ' ';
        hex[3] = 0;
        terminal_print(win, hex);
      }
      terminal_print(win, "\n");
    }
  } else if (strcmp(cmd, "dbg trace") == 0) {
    extern int syscall_trace_enabled;
    syscall_trace_enabled = !syscall_trace_enabled;
    if (syscall_trace_enabled)
      terminal_print(win, "Syscall Tracing: ENABLED (logging to serial).\n");
    else
      terminal_print(win, "Syscall Tracing: DISABLED.\n");
  } else if (strcmp(cmd, "dbg tests") == 0) {
    terminal_print(win, "Running kernel test suite. Check serial log...\n");
    extern void run_kernel_tests();
    run_kernel_tests();
  } else if (strncmp(cmd, "dbg kill ", 9) == 0) {
    int pid = atoi(cmd + 9);
    if (pid <= 1) { // PID 1 is usually the idle task or kernel init
      terminal_print(win, "Cannot kill kernel or idle task.\n");
    } else {
      task_kill(pid);
      terminal_print(win, "Killed PID ");
      char buf[16];
      k_itoa(pid, buf);
      terminal_print(win, buf);
      terminal_print(win, "\n");
    }
  }
  // REBOOT
  else if (strcmp(cmd, "reboot") == 0) {
    struct {
      uint16_t limit;
      uint32_t base;
    } __attribute__((packed)) idtr = {0, 0};
    __asm__ volatile("lidt %0" : : "m"(idtr));
    __asm__ volatile("int3");
  } else if (strcmp(cmd, "calc") == 0) {
    launch_calculator();
  } else if (strcmp(cmd, "rect") == 0) {
    terminal_print(win, "Drawing Demo Rects...\n");
    vga_draw_rect(100, 50, 50, 50, 4);
    vga_draw_rect(160, 50, 50, 50, 2);
  } else {
    terminal_print(win, "Unknown command: ");
    terminal_print(win, cmd);
    terminal_print(win, "\n");
  }
}
