#include "shell.h"
#include "../apps/mail/mail.h"
#include "../apps/mail/mail_app.h"
#include "../apps/terminal.h"
#include "../drivers/ata.h"
#include "../drivers/keyboard.h"
#include "../drivers/ports.h"
#include "../drivers/vga.h"
#include "../fs/fs.h"
#include "../fs/vfs.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "../net/net.h"

#include "../kernel/mail_core.h"
#include "../kernel/string.h"
#include "../kernel/syscall.h"
#include "../kernel/window.h"

#define PROMPT "PureOS> "

// Global Shell Context
static void *current_shell_window = 0;
static int text_color = 0x00FF00;
static char shell_cwd[128] = "/";

void shell_print_wrapper(int x, int y, const char *str, uint32_t color) {
  if (current_shell_window) {
    terminal_print((window_t *)current_shell_window, str);
  } else {
    vga_puts(x, y, str, color);
  }
}

// Redirect vga_puts to wrapper
#undef vga_puts
#define vga_puts(x, y, str, color) shell_print_wrapper(x, y, str, color)

void cmd_info() {
  vga_puts(0, -1, "==========================\n", text_color);
  vga_puts(0, -1, "       PureOS Info        \n", text_color);
  vga_puts(0, -1, "==========================\n", text_color);
  vga_puts(0, -1, "Kernel: v0.5 (32-bit)\n", text_color);
  vga_puts(0, -1, "Shell:  v0.2 Enhanced\n", text_color);
  vga_puts(0, -1, "Video:  VBE Graphics (16bpp)\n", text_color);
  vga_puts(0, -1, "Syscalls: Hardware Recovery Fix\n", text_color);
}

void cmd_help() {
  vga_puts(0, -1, "Available Commands:\n", text_color);
  vga_puts(0, -1, "  help        - Show this list\n", text_color);
  vga_puts(0, -1, "  info        - System information\n", text_color);
  vga_puts(0, -1, "  clear       - Clear terminal\n", text_color);
  vga_puts(0, -1, "  pwd         - Print working dir\n", text_color);
  vga_puts(0, -1, "  ls [path]   - List files\n", text_color);
  vga_puts(0, -1, "  cd [path]   - Change directory\n", text_color);
  vga_puts(0, -1, "  mkdir [dir] - Create directory\n", text_color);
  vga_puts(0, -1, "  touch [f]   - Create empty file\n", text_color);
  vga_puts(0, -1, "  cat [file]  - Read file content\n", text_color);
  vga_puts(0, -1, "  rm [file]   - Delete file/dir\n", text_color);
  vga_puts(0, -1, "  echo [text] - Print text\n", text_color);
  vga_puts(0, -1, "  echo t > f  - Write text to file\n", text_color);
  vga_puts(0, -1, "  beep        - Play Sound\n", text_color);
  vga_puts(0, -1, "  reboot      - Restart PC\n", text_color);
  vga_puts(0, -1, "  taskmgr     - Open Task Manager\n", text_color);
  vga_puts(0, -1, "  mail        - Open PureOS Mail\n", text_color);
  vga_puts(0, -1, "  exec [file] - Run ELF program\n", text_color);
  vga_puts(0, -1, "  nice [pid][p]- Set priority\n", text_color);
  vga_puts(0, -1, "  writetest   - Test disk writing\n", text_color);
  vga_puts(0, -1, "  mailtest [h][p][u][pass]- Test POP3\n", text_color);
  vga_puts(0, -1, "  maillist [user] - List stored emails\n", text_color);
  vga_puts(0, -1, "Keys: Up/Down=History Tab=Complete\n", text_color);
}

// Native User Program (Assembler written in C)
void __attribute__((naked)) user_program_test(void) {
  __asm__ volatile("mov $1, %%eax\n"   // SYS_WRITE
                   "mov $msg, %%ebx\n" // String pointer
                   "int $0x80\n"

                   "mov $0, %%eax\n" // SYS_EXIT
                   "int $0x80\n"

                   "msg: .asciz \"[USER] Hello from Native Ring 3!\\n\"\n"
                   :
                   :
                   : "eax", "ebx");
}

void cmd_writetest(void) {
  const char *data = "Hello, disk!";
  vga_puts(0, -1, "Testing write to /test.txt...\n", 0x00FFFF);
  if (fs_write("/test.txt", (uint8_t *)data, strlen(data))) {
    vga_puts(0, -1, "test.txt written successfully\n", 0x00FF00);
  } else {
    vga_puts(0, -1, "test.txt write FAILED\n", 0xFF0000);
  }
}

// Helper: build full path from CWD + relative/absolute input
static void resolve_path(const char *input, char *out) {
  if (input[0] == '/') {
    strcpy(out, input);
  } else {
    strcpy(out, shell_cwd);
    if (out[strlen(out) - 1] != '/')
      strcat(out, "/");
    strcat(out, input);
  }
}

void shell_execute(void *win, char *input) {
  current_shell_window = win;
  if (strlen(input) == 0)
    return;

  // Make a copy of input because strtok is destructive
  char cmd_line[256];
  strncpy(cmd_line, input, 255);
  cmd_line[255] = 0;

  char *cmd = strtok(cmd_line, " ");
  if (!cmd)
    return;

  if (strcmp(cmd, "help") == 0) {
    cmd_help();
  } else if (strcmp(cmd, "info") == 0) {
    cmd_info();
  } else if (strcmp(cmd, "clear") == 0) {
    if (win) {
      extern void terminal_clear();
      terminal_clear();
    } else {
      vga_clear_screen(text_color >> 4);
    }
  } else if (strcmp(cmd, "pwd") == 0) {
    vga_puts(0, -1, shell_cwd, text_color);
    vga_puts(0, -1, "\n", text_color);
  } else if (strcmp(cmd, "reboot") == 0) {
    uint8_t good = 0x02;
    while (good & 0x02)
      good = inb(0x64);
    outb(0x64, 0xFE);
  } else if (strcmp(cmd, "beep") == 0) {
#include "../drivers/speaker.h"
    beep();
    vga_puts(0, -1, "Beep!\n", text_color);
  } else if (strcmp(cmd, "rm") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: rm [file/dir]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    if (fs_delete(full)) {
      vga_puts(0, -1, "Deleted: ", 0x00FF00);
      vga_puts(0, -1, full, 0x00FF00);
      vga_puts(0, -1, "\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Error: Could not delete.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "touch") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: touch [file]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    uint8_t empty = 0;
    if (fs_write(full, &empty, 0)) {
      vga_puts(0, -1, "Created: ", 0x00FF00);
      vga_puts(0, -1, full, 0x00FF00);
      vga_puts(0, -1, "\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Error: Could not create file.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "user") == 0) {
    vga_puts(0, -1, "Entering Native User Mode...\n", 0x0E);
    void *code_dest = (void *)0x500000;
    memcpy(code_dest, user_program_test, 1024);
    enter_user_mode(code_dest);
    vga_puts(0, -1, "Returned to Shell.\n", 0x00FF00);
  } else if (strcmp(cmd, "pagefault") == 0) {
    vga_puts(0, -1, "Triggering Page Fault (accessing 0xFFFFFFFF)...\n",
             text_color);
    uint32_t *ptr = (uint32_t *)0xFFFFFFFF;
    volatile uint32_t val = *ptr;
    (void)val;
  } else if (strcmp(cmd, "taskmgr") == 0) {
    extern void taskmgr_init();
    taskmgr_init();
  } else if (strcmp(cmd, "mail") == 0) {
    mail_app_init();
  } else if (strcmp(cmd, "mkdir") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: mkdir [dir]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    if (fs_mkdir(full)) {
      vga_puts(0, -1, "Directory created.\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Error: Could not create directory.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "exec") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: exec [file]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    extern int elf_load_file(const char *path);
    if (elf_load_file(full)) {
      vga_puts(0, -1, "Process started.\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Error: Failed to load ELF.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "nice") == 0) {
    char *pid_str = strtok(NULL, " ");
    char *pri_str = strtok(NULL, " ");
    if (pid_str && pri_str) {
      int pid = atoi(pid_str);
      int pri = atoi(pri_str);
      task_set_priority(pid, pri);
      vga_puts(0, -1, "Priority set.\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Usage: nice [pid] [priority]\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "cd") == 0) {
    char *path = strtok(NULL, " ");
    if (!path) {
      vga_puts(0, -1, "Usage: cd [path]\n", 0xFF0000);
      return;
    }
    char new_path[128];
    if (path[0] == '/') {
      strcpy(new_path, path);
    } else {
      strcpy(new_path, shell_cwd);
      if (new_path[strlen(new_path) - 1] != '/')
        strcat(new_path, "/");
      strcat(new_path, path);
    }
    // Canonicalize ... (simplified)
    if (strcmp(new_path, "/") == 0 || fs_list_files(new_path, 0, 0) >= 0) {
      strcpy(shell_cwd, new_path);
      vga_puts(0, -1, "CWD: ", text_color);
      vga_puts(0, -1, shell_cwd, text_color);
      vga_puts(0, -1, "\n", text_color);
    } else {
      vga_puts(0, -1, "Error: Directory not found.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "ls") == 0) {
    char *path = shell_cwd;
    char req_path[128];
    char *path_arg = strtok(NULL, " ");
    if (path_arg) {
      resolve_path(path_arg, req_path);
      path = req_path;
    }
    FileInfo files[16];
    int count = fs_list_files(path, files, 16);
    if (count < 0) {
      vga_puts(0, -1, "Error: Path not found.\n", 0xFF0000);
    } else {
      for (int i = 0; i < count; i++) {
        char line[64];
        strcpy(line, files[i].name);
        if (files[i].is_dir)
          strcat(line, "/");
        strcat(line, "\n");
        vga_puts(0, -1, line, 0x00FF00);
      }
    }
  } else if (strcmp(cmd, "cat") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: cat [file]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    char *fbuf = (char *)kmalloc(4096);
    if (fbuf) {
      if (fs_read(full, (uint8_t *)fbuf)) {
        vga_puts(0, -1, fbuf, 0x00FF00);
        vga_puts(0, -1, "\n", 0x00FF00);
      } else {
        vga_puts(0, -1, "File not found.\n", 0xFF0000);
      }
      kfree(fbuf);
    }
  } else if (strcmp(cmd, "echo") == 0) {
    char *text = strtok(NULL, ""); // Get rest of string
    if (!text) {
      vga_puts(0, -1, "\n", text_color);
      return;
    }
    vga_puts(0, -1, text, text_color);
    vga_puts(0, -1, "\n", text_color);
  } else if (strcmp(cmd, "mailtest") == 0) {
    char *host = strtok(NULL, " ");
    char *p_port = strtok(NULL, " ");
    char *user = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");

    if (host && p_port && user && pass) {
      int port = atoi(p_port);
      mail_account_t acc;
      strcpy(acc.hostname, host);
      acc.port = port;
      strcpy(acc.username, user);
      strcpy(acc.password, pass);
      acc.use_tls = (port == 995 || port == 465);
      acc.type = MAIL_TYPE_POP3;

      vga_puts(0, -1, "Connecting to ", text_color);
      vga_puts(0, -1, host, text_color);
      vga_puts(0, -1, "...\n", text_color);

      mail_session_t *s = (mail_session_t *)kmalloc(sizeof(mail_session_t));
      if (!s) {
        vga_puts(0, -1, "Error: OOM\n", 0xFF0000);
        return;
      }

      if (mail_open_session(s, &acc) == 0) {
        vga_puts(0, -1, "Connected. Logging in...\n", text_color);
        if (pop3_login(s) == 0) {
          int count, size;
          if (pop3_stat(s, &count, &size) == 0) {
            vga_puts(0, -1, "Logged in! Messages: ", 0x00FF00);
            char b[16];
            k_itoa(count, b);
            vga_puts(0, -1, b, 0x00FF00);
            vga_puts(0, -1, ", Size: ", 0x00FF00);
            k_itoa(size, b);
            vga_puts(0, -1, b, 0x00FF00);
            vga_puts(0, -1, " bytes\n", 0x00FF00);

            if (count > 0) {
              vga_puts(0, -1, "Fetching first message...\n", text_color);
              mail_message_t *msg =
                  (mail_message_t *)kmalloc(sizeof(mail_message_t));
              if (msg) {
                if (pop3_retrieve_message(s, 1, msg) == 0) {
                  vga_puts(0, -1, "Saved to disk.\n", 0x00FF00);
                  mail_storage_init(user);
                  mail_save_message(user, "inbox", msg);
                }
                mail_free_message(msg);
                kfree(msg);
              }
            }
          }
          pop3_quit(s);
        } else {
          vga_puts(0, -1, "Login failed.\n", 0xFF0000);
          mail_close_session(s);
        }
      } else {
        vga_puts(0, -1, "Error: Could not open session.\n", 0xFF0000);
      }
      kfree(s);
    } else {
      vga_puts(0, -1, "Usage: mailtest [host] [port] [user] [pass]\n",
               0xFF0000);
    }
  } else if (strcmp(cmd, "maillist") == 0) {
    char *user = strtok(NULL, " ");
    if (!user) {
      vga_puts(0, -1, "Usage: maillist <user>\n", 0xFFFFFF);
    } else {
      mail_header_t headers[10];
      int count = mail_list_messages(user, headers, 10);
      char b[16];
      k_itoa(count, b);
      vga_puts(0, -1, "Messages: ", 0x00FF00);
      vga_puts(0, -1, b, 0x00FF00);
      vga_puts(0, -1, "\n", 0x00FF00);
      for (int i = 0; i < count; i++) {
        vga_puts(0, -1, " - UID: ", 0xFFFFFF);
        vga_puts(0, -1, headers[i].uid, 0xFFFFFF);
        vga_puts(0, -1, "\n", 0xFFFFFF);
      }
    }
  } else {
    vga_puts(0, -1, "Unknown command: ", 0xFF0000);
    vga_puts(0, -1, cmd, 0xFF0000);
    vga_puts(0, -1, "\n", 0xFF0000);
  }
}

void shell_run() {
  vga_clear_screen(0x01); // Blue background
  vga_puts(0, 0, "PureOS Shell v0.2. Type 'help'.\n", 0x1F);
  vga_puts(0, -1, PROMPT, 0x1F);

  char input[100];
  int len = 0;

  while (1) {
    char c = keyboard_getc();
    if (c == 0)
      continue;

    if (c == '\n') {
      vga_puts(0, -1, "\n", text_color);
      input[len] = 0;
      shell_execute(0, input);
      len = 0;
      vga_puts(0, -1, PROMPT, text_color);
    } else if (c == '\b') {
      if (len > 0) {
        len--;
        input[len] = 0;
        vga_puts(0, -1, "\b \b", text_color);
      }
    } else if (c >= ' ' && c <= '~') {
      if (len < 99) {
        input[len++] = c;
        char temp[2] = {c, 0};
        vga_puts(0, -1, temp, text_color);
      }
    }
  }
}
