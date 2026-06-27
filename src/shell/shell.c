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
#include "../drivers/speaker.h"

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
  vga_puts(0, -1, "=== File Operations ===\n", text_color);
  vga_puts(0, -1, "  ls [path]     - List directory\n", text_color);
  vga_puts(0, -1, "  cd [path]     - Change directory\n", text_color);
  vga_puts(0, -1, "  pwd           - Print working dir\n", text_color);
  vga_puts(0, -1, "  cat [file]    - Read file content\n", text_color);
  vga_puts(0, -1, "  head [file]   - Show first 10 lines\n", text_color);
  vga_puts(0, -1, "  tail [file]   - Show last 10 lines\n", text_color);
  vga_puts(0, -1, "  touch [file]  - Create empty file\n", text_color);
  vga_puts(0, -1, "  mkdir [dir]   - Create directory\n", text_color);
  vga_puts(0, -1, "  rm [file]     - Delete file/dir\n", text_color);
  vga_puts(0, -1, "  cp [s] [d]    - Copy file\n", text_color);
  vga_puts(0, -1, "  mv [s] [d]    - Move/rename file\n", text_color);
  vga_puts(0, -1, "  ln -s [t] [l] - Create symlink\n", text_color);
  vga_puts(0, -1, "  stat [file]   - File metadata\n", text_color);
  vga_puts(0, -1, "  wc [file]     - Word/line/byte count\n", text_color);
  vga_puts(0, -1, "=== Permissions ===\n", text_color);
  vga_puts(0, -1, "  chmod [mode] [file]\n", text_color);
  vga_puts(0, -1, "  chown [uid:gid] [file]\n", text_color);
  vga_puts(0, -1, "=== System ===\n", text_color);
  vga_puts(0, -1, "  mount         - Show mount points\n", text_color);
  vga_puts(0, -1, "  df            - Show disk free space\n", text_color);
  vga_puts(0, -1, "  echo [text]   - Print text\n", text_color);
  vga_puts(0, -1, "  echo t > file - Write to file\n", text_color);
  vga_puts(0, -1, "  clear         - Clear terminal\n", text_color);
  vga_puts(0, -1, "  info          - System information\n", text_color);
  vga_puts(0, -1, "  reboot        - Restart PC\n", text_color);
  vga_puts(0, -1, "  exec [file]   - Run ELF program\n", text_color);
  vga_puts(0, -1, "  taskmgr       - Open Task Manager\n", text_color);
  vga_puts(0, -1, "  mail          - Open PureOS Mail\n", text_color);
  vga_puts(0, -1, "  nice [pid][p] - Set priority\n", text_color);
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
      memset(fbuf, 0, 4096);
      int bytes = fs_read(full, (uint8_t *)fbuf);
      if (bytes > 0) {
        fbuf[bytes < 4095 ? bytes : 4095] = 0;
        vga_puts(0, -1, fbuf, 0x00FF00);
        vga_puts(0, -1, "\n", 0x00FF00);
      } else {
        vga_puts(0, -1, "File not found.\n", 0xFF0000);
      }
      kfree(fbuf);
    }
  } else if (strcmp(cmd, "echo") == 0) {
    // Get full remaining text from original input (strtok is destructive)
    char *text = strtok(NULL, "");
    if (!text) {
      vga_puts(0, -1, "\n", text_color);
      return;
    }
    // Check for > redirection
    char *redir = 0;
    int append = 0;
    for (int i = 0; text[i]; i++) {
      if (text[i] == '>' && text[i+1] == '>') {
        text[i] = 0;
        redir = text + i + 2;
        append = 1;
        break;
      } else if (text[i] == '>') {
        text[i] = 0;
        redir = text + i + 1;
        break;
      }
    }
    if (redir) {
      // Strip leading spaces from filename
      while (*redir == ' ') redir++;
      char full[128];
      resolve_path(redir, full);
      // Trim trailing spaces from text
      int tl = strlen(text);
      while (tl > 0 && text[tl-1] == ' ') { text[--tl] = 0; }
      
      if (append) {
        // Append: read existing + append new
        char *existing = (char *)kmalloc(4096);
        int elen = 0;
        if (existing) {
          elen = fs_read(full, (uint8_t *)existing);
          if (elen < 0) elen = 0;
        }
        char *combined = (char *)kmalloc(elen + tl + 2);
        if (combined) {
          if (elen > 0) memcpy(combined, existing, elen);
          memcpy(combined + elen, text, tl);
          combined[elen + tl] = '\n';
          fs_write(full, (uint8_t *)combined, elen + tl + 1);
          kfree(combined);
        }
        if (existing) kfree(existing);
      } else {
        fs_write(full, (uint8_t *)text, tl);
      }
    } else {
      vga_puts(0, -1, text, text_color);
      vga_puts(0, -1, "\n", text_color);
    }
  } else if (strcmp(cmd, "cp") == 0) {
    char *src_arg = strtok(NULL, " ");
    char *dst_arg = strtok(NULL, " ");
    if (!src_arg || !dst_arg) {
      vga_puts(0, -1, "Usage: cp [source] [dest]\n", 0xFF0000);
      return;
    }
    char src[128], dst[128];
    resolve_path(src_arg, src);
    resolve_path(dst_arg, dst);
    if (vfs_copy_file(src, dst) == 0) {
      vga_puts(0, -1, "Copied.\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Error: Copy failed.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "mv") == 0) {
    char *src_arg = strtok(NULL, " ");
    char *dst_arg = strtok(NULL, " ");
    if (!src_arg || !dst_arg) {
      vga_puts(0, -1, "Usage: mv [source] [dest]\n", 0xFF0000);
      return;
    }
    char src[128], dst[128];
    resolve_path(src_arg, src);
    resolve_path(dst_arg, dst);
    if (vfs_rename(src, dst) == 0) {
      vga_puts(0, -1, "Moved.\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Error: Move failed.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "stat") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: stat [file]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    vfs_stat_t st;
    if (vfs_stat(full, &st) == 0) {
      char buf[16];
      vga_puts(0, -1, "  File: ", 0x00FFFF);
      vga_puts(0, -1, full, 0xFFFFFF);
      vga_puts(0, -1, "\n", 0xFFFFFF);
      
      vga_puts(0, -1, "  Size: ", 0x00FFFF);
      k_itoa(st.st_size, buf);
      vga_puts(0, -1, buf, 0xFFFFFF);
      vga_puts(0, -1, " bytes\n", 0xFFFFFF);
      
      vga_puts(0, -1, " Inode: ", 0x00FFFF);
      k_itoa(st.st_ino, buf);
      vga_puts(0, -1, buf, 0xFFFFFF);
      vga_puts(0, -1, "\n", 0xFFFFFF);
      
      vga_puts(0, -1, " Links: ", 0x00FFFF);
      k_itoa(st.st_nlink, buf);
      vga_puts(0, -1, buf, 0xFFFFFF);
      vga_puts(0, -1, "\n", 0xFFFFFF);
      
      // Mode as rwx string
      vga_puts(0, -1, "  Mode: ", 0x00FFFF);
      char mode_str[11] = "----------";
      uint32_t m = st.st_mode;
      if (m & VFS_DIRECTORY) mode_str[0] = 'd';
      else if (m & VFS_SYMLINK) mode_str[0] = 'l';
      if (m & 0400) mode_str[1] = 'r';
      if (m & 0200) mode_str[2] = 'w';
      if (m & 0100) mode_str[3] = 'x';
      if (m & 0040) mode_str[4] = 'r';
      if (m & 0020) mode_str[5] = 'w';
      if (m & 0010) mode_str[6] = 'x';
      if (m & 0004) mode_str[7] = 'r';
      if (m & 0002) mode_str[8] = 'w';
      if (m & 0001) mode_str[9] = 'x';
      vga_puts(0, -1, mode_str, 0xFFFFFF);
      vga_puts(0, -1, "\n", 0xFFFFFF);
      
      vga_puts(0, -1, "   Uid: ", 0x00FFFF);
      k_itoa(st.st_uid, buf);
      vga_puts(0, -1, buf, 0xFFFFFF);
      vga_puts(0, -1, "  Gid: ", 0x00FFFF);
      k_itoa(st.st_gid, buf);
      vga_puts(0, -1, buf, 0xFFFFFF);
      vga_puts(0, -1, "\n", 0xFFFFFF);
      
      vga_puts(0, -1, " Mtime: ", 0x00FFFF);
      k_itoa(st.st_mtime, buf);
      vga_puts(0, -1, buf, 0xFFFFFF);
      vga_puts(0, -1, "s\n", 0xFFFFFF);
    } else {
      vga_puts(0, -1, "Error: File not found.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "chmod") == 0) {
    char *mode_str = strtok(NULL, " ");
    char *path_arg = strtok(NULL, " ");
    if (!mode_str || !path_arg) {
      vga_puts(0, -1, "Usage: chmod [mode] [file]\n", 0xFF0000);
      return;
    }
    // Parse octal mode (e.g., 755)
    uint32_t mode = 0;
    for (int i = 0; mode_str[i]; i++) {
      mode = mode * 8 + (mode_str[i] - '0');
    }
    char full[128];
    resolve_path(path_arg, full);
    if (vfs_chmod(full, mode) == 0) {
      vga_puts(0, -1, "Permissions changed.\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Error: chmod failed.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "chown") == 0) {
    char *ids = strtok(NULL, " ");
    char *path_arg = strtok(NULL, " ");
    if (!ids || !path_arg) {
      vga_puts(0, -1, "Usage: chown [uid:gid] [file]\n", 0xFF0000);
      return;
    }
    // Parse uid:gid
    uint32_t uid = 0, gid = 0;
    int i = 0;
    while (ids[i] && ids[i] != ':') { uid = uid * 10 + (ids[i] - '0'); i++; }
    if (ids[i] == ':') {
      i++;
      while (ids[i]) { gid = gid * 10 + (ids[i] - '0'); i++; }
    }
    char full[128];
    resolve_path(path_arg, full);
    if (vfs_chown(full, uid, gid) == 0) {
      vga_puts(0, -1, "Ownership changed.\n", 0x00FF00);
    } else {
      vga_puts(0, -1, "Error: chown failed.\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "ln") == 0) {
    char *flag = strtok(NULL, " ");
    if (!flag) {
      vga_puts(0, -1, "Usage: ln -s [target] [link]\n", 0xFF0000);
      return;
    }
    if (strcmp(flag, "-s") == 0) {
      char *target = strtok(NULL, " ");
      char *linkpath = strtok(NULL, " ");
      if (!target || !linkpath) {
        vga_puts(0, -1, "Usage: ln -s [target] [link]\n", 0xFF0000);
        return;
      }
      char full_target[128], full_link[128];
      resolve_path(target, full_target);
      resolve_path(linkpath, full_link);
      if (vfs_symlink(full_target, full_link) == 0) {
        vga_puts(0, -1, "Symlink created.\n", 0x00FF00);
      } else {
        vga_puts(0, -1, "Error: symlink failed.\n", 0xFF0000);
      }
    } else {
      vga_puts(0, -1, "Hard links not supported yet. Use: ln -s\n", 0xFF0000);
    }
  } else if (strcmp(cmd, "head") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: head [file]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    char *fbuf = (char *)kmalloc(4096);
    if (fbuf) {
      memset(fbuf, 0, 4096);
      int bytes = fs_read(full, (uint8_t *)fbuf);
      if (bytes > 0) {
        fbuf[bytes < 4095 ? bytes : 4095] = 0;
        int lines = 0;
        for (int i = 0; fbuf[i] && lines < 10; i++) {
          char ch[2] = {fbuf[i], 0};
          vga_puts(0, -1, ch, 0x00FF00);
          if (fbuf[i] == '\n') lines++;
        }
        if (lines == 0) vga_puts(0, -1, "\n", 0x00FF00);
      } else {
        vga_puts(0, -1, "File not found.\n", 0xFF0000);
      }
      kfree(fbuf);
    }
  } else if (strcmp(cmd, "tail") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: tail [file]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    char *fbuf = (char *)kmalloc(4096);
    if (fbuf) {
      memset(fbuf, 0, 4096);
      int bytes = fs_read(full, (uint8_t *)fbuf);
      if (bytes > 0) {
        fbuf[bytes < 4095 ? bytes : 4095] = 0;
        // Count total lines
        int total_lines = 0;
        int flen = strlen(fbuf);
        for (int i = 0; i < flen; i++) {
          if (fbuf[i] == '\n') total_lines++;
        }
        // Print last 10 lines
        int skip = total_lines > 10 ? total_lines - 10 : 0;
        int lines = 0;
        for (int i = 0; fbuf[i]; i++) {
          if (lines >= skip) {
            char ch[2] = {fbuf[i], 0};
            vga_puts(0, -1, ch, 0x00FF00);
          }
          if (fbuf[i] == '\n') lines++;
        }
      } else {
        vga_puts(0, -1, "File not found.\n", 0xFF0000);
      }
      kfree(fbuf);
    }
  } else if (strcmp(cmd, "wc") == 0) {
    char *path_arg = strtok(NULL, " ");
    if (!path_arg) {
      vga_puts(0, -1, "Usage: wc [file]\n", 0xFF0000);
      return;
    }
    char full[128];
    resolve_path(path_arg, full);
    char *fbuf = (char *)kmalloc(4096);
    if (fbuf) {
      memset(fbuf, 0, 4096);
      int bytes = fs_read(full, (uint8_t *)fbuf);
      if (bytes > 0) {
        fbuf[bytes < 4095 ? bytes : 4095] = 0;
        int lines = 0, words = 0, b_count = strlen(fbuf);
        int in_word = 0;
        for (int i = 0; fbuf[i]; i++) {
          if (fbuf[i] == '\n') lines++;
          if (fbuf[i] == ' ' || fbuf[i] == '\n' || fbuf[i] == '\t') {
            in_word = 0;
          } else if (!in_word) {
            in_word = 1;
            words++;
          }
        }
        char buf[16];
        vga_puts(0, -1, " ", 0x00FF00);
        k_itoa(lines, buf); vga_puts(0, -1, buf, 0x00FF00);
        vga_puts(0, -1, " ", 0x00FF00);
        k_itoa(words, buf); vga_puts(0, -1, buf, 0x00FF00);
        vga_puts(0, -1, " ", 0x00FF00);
        k_itoa(b_count, buf); vga_puts(0, -1, buf, 0x00FF00);
        vga_puts(0, -1, " ", 0x00FF00);
        vga_puts(0, -1, path_arg, 0x00FF00);
        vga_puts(0, -1, "\n", 0x00FF00);
      } else {
        vga_puts(0, -1, "File not found.\n", 0xFF0000);
      }
      kfree(fbuf);
    }
  } else if (strcmp(cmd, "df") == 0) {
    char buf[16];
    extern uint32_t fs_get_total_size(void);
    extern uint32_t fs_get_used_size(void);
    uint32_t total = fs_get_total_size();
    uint32_t used = fs_get_used_size();
    uint32_t avail = total > used ? total - used : 0;
    
    vga_puts(0, -1, "Filesystem  Size    Used    Avail   Mount\n", 0x00FFFF);
    vga_puts(0, -1, "/dev/hda    ", 0xFFFFFF);
    k_itoa(total / 1024, buf); vga_puts(0, -1, buf, 0xFFFFFF);
    vga_puts(0, -1, "KB   ", 0xFFFFFF);
    k_itoa(used / 1024, buf); vga_puts(0, -1, buf, 0xFFFFFF);
    vga_puts(0, -1, "KB   ", 0xFFFFFF);
    k_itoa(avail / 1024, buf); vga_puts(0, -1, buf, 0xFFFFFF);
    vga_puts(0, -1, "KB   /\n", 0xFFFFFF);
    vga_puts(0, -1, "ramfs       -       -       -       /ram\n", 0xFFFFFF);
    vga_puts(0, -1, "devfs       -       -       -       /dev\n", 0xFFFFFF);
    vga_puts(0, -1, "procfs      -       -       -       /proc\n", 0xFFFFFF);
  } else if (strcmp(cmd, "mount") == 0) {
    vga_puts(0, -1, "Active mount points:\n", 0x00FFFF);
    extern vfs_mount_t *mount_list;
    vfs_mount_t *m = mount_list;
    while (m) {
      vga_puts(0, -1, "  ", 0xFFFFFF);
      vga_puts(0, -1, m->path, 0x00FF00);
      if (m->fstype[0]) {
        vga_puts(0, -1, " type ", 0xFFFFFF);
        vga_puts(0, -1, m->fstype, 0x00FF00);
      }
      vga_puts(0, -1, "\n", 0xFFFFFF);
      m = m->next;
    }
  } else if (strcmp(cmd, "writetest") == 0) {
    cmd_writetest();
  } else if (strcmp(cmd, "beep") == 0) {
    beep();
    vga_puts(0, -1, "Beep!\n", text_color);
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
