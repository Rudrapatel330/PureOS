#include "syscall.h"
#include "../drivers/rtc.h"
#include "../drivers/vga.h"
#include "heap.h"
#include "isr.h"
#include "ipc.h"

#include "../drivers/keyboard.h"
#include "../drivers/mouse.h"
#include "../fs/vfs.h"
#include "task.h"

extern void print_serial(const char *);

int syscall_trace_enabled = 0;

// Validate that a range of memory is entirely within user-space pages
int is_user_range(const void *addr, size_t len) {
  if (!addr) return 0;
  uint64_t uaddr = (uint64_t)addr;
  // User space is below kernel higher half
  if (uaddr + len < uaddr) return 0; // overflow
  if (uaddr + len >= 0xC0000000ULL) return 0;
  if (uaddr < 0x1000) return 0; // NULL page trap
  return 1;
}

int is_user_string(const char *str) {
  if (!str) return 0;
  if ((uint64_t)str >= 0xC0000000ULL) return 0;
  if ((uint64_t)str < 0x1000) return 0;
  // Quick length check: ensure the string is within user space
  // (limit scan to prevent infinite loop on bad pointer)
  for (int i = 0; i < 4096; i++) {
    if ((uint64_t)&str[i] >= 0xC0000000ULL) return 0;
    if (str[i] == '\0') return 1;
  }
  return 0; // string too long - invalid
}

uint64_t syscall_handler(registers_t *regs) {
  uint64_t syscall_num = regs->rax;
  uint64_t ret = 0;

  if (syscall_trace_enabled) {
    char buf[16];
    extern void k_itoa(int, char *);
    print_serial("SYSCALL [");
    k_itoa(syscall_num, buf);
    print_serial(buf);
    print_serial("] args: ");
    k_itoa(regs->rbx, buf);
    print_serial(buf);
    print_serial("\n");
  }

  switch (syscall_num) {
  case SYS_EXIT:
    print_serial("SYSCALL: User process exiting.\n");
    task_kill(get_current_task()->id);
    break;

  case SYS_PRINT:
    if (is_user_string((const char *)regs->rbx))
      vga_puts(0, -1, (const char *)regs->rbx, 0x07);
    else
      ret = -1;
    break;

  case SYS_READKEY:
    ret = keyboard_getc();
    break;

  case SYS_MALLOC:
    ret = (uint64_t)kmalloc(regs->rbx);
    break;

  case SYS_FREE:
    kfree((void *)regs->rbx);
    break;

  case SYS_GETTIME:
    if (is_user_range((void *)regs->rbx, sizeof(rtc_time_t)))
      rtc_read((rtc_time_t *)regs->rbx);
    else
      ret = -1;
    break;

  case SYS_OPEN:
    if (is_user_string((const char *)regs->rbx))
      ret = vfs_open((const char *)regs->rbx, (int)regs->rcx);
    else
      ret = -1;
    break;

  case SYS_READ:
    if (is_user_range((void *)regs->rcx, (uint32_t)regs->rdx))
      ret = vfs_read((int)regs->rbx, (uint8_t *)regs->rcx, (uint32_t)regs->rdx);
    else
      ret = -1;
    break;

  case SYS_WRITE:
    ret = vfs_write((int)regs->rbx, (const uint8_t *)regs->rcx,
                    (uint32_t)regs->rdx);
    break;

  case SYS_CLOSE:
    vfs_close((int)regs->rbx);
    break;

  case SYS_LSEEK:
    ret = vfs_lseek((int)regs->rbx, (uint32_t)regs->rcx, (int)regs->rdx);
    break;

  case SYS_GETPID:
    ret = get_current_task()->id;
    break;

  case SYS_FORK:
    ret = task_fork(regs);
    break;

  case SYS_EXECVE:
    if (is_user_string((const char *)regs->rbx))
      ret = task_execve((const char *)regs->rbx, regs);
    else
      ret = -1;
    break;

  case SYS_MMAP: {
    uint64_t addr = regs->rbx;
    uint32_t len = (uint32_t)regs->rcx;
    if (addr == 0)
      addr = 0x40000000;

    uint32_t num_pages = (len + 4095) / 4096;
    for (uint32_t i = 0; i < num_pages; i++) {
      uint32_t phys;
      kmalloc_ap(4096, &phys);
      paging_map_user_page(get_current_task()->pagedir, addr + i * 4096, phys,
                           0x7);
    }
    ret = addr;
    break;
  }

  case SYS_PIPE: {
    extern int pipe(int fds[2]);
    if (is_user_range((void *)regs->rbx, 2 * sizeof(int)))
      ret = pipe((int *)regs->rbx);
    else
      ret = -1;
    break;
  }

  case SYS_DUP2:
    ret = vfs_dup2((int)regs->rbx, (int)regs->rcx);
    break;

  case SYS_STAT:
    if (is_user_string((const char *)regs->rbx) &&
        is_user_range((void *)regs->rcx, sizeof(vfs_stat_t)))
      ret = vfs_stat((const char *)regs->rbx, (vfs_stat_t *)regs->rcx);
    else
      ret = -1;
    break;

  case SYS_MKDIR:
    if (is_user_string((const char *)regs->rbx))
      ret = vfs_mkdir((const char *)regs->rbx);
    else
      ret = -1;
    break;

  case SYS_UNLINK:
    if (is_user_string((const char *)regs->rbx))
      ret = vfs_unlink((const char *)regs->rbx);
    else
      ret = -1;
    break;

  case SYS_CHMOD:
    if (is_user_string((const char *)regs->rbx))
      ret = vfs_chmod((const char *)regs->rbx, (uint32_t)regs->rcx);
    else
      ret = -1;
    break;

  case SYS_CHOWN:
    if (is_user_string((const char *)regs->rbx))
      ret = vfs_chown((const char *)regs->rbx, (uint32_t)regs->rcx,
                      (uint32_t)regs->rdx);
    else
      ret = -1;
    break;

  case SYS_RENAME:
    if (is_user_string((const char *)regs->rbx) &&
        is_user_string((const char *)regs->rcx))
      ret = vfs_rename((const char *)regs->rbx, (const char *)regs->rcx);
    else
      ret = -1;
    break;

  case SYS_FSTAT:
    ret = vfs_fstat((int)regs->rbx, (vfs_stat_t *)regs->rcx);
    break;

  case SYS_CREAT:
    if (is_user_string((const char *)regs->rbx))
      ret = vfs_create_file((const char *)regs->rbx, (uint32_t)regs->rcx);
    else
      ret = -1;
    break;

  case SYS_SYMLINK:
    if (is_user_string((const char *)regs->rbx) &&
        is_user_string((const char *)regs->rcx))
      ret = vfs_symlink((const char *)regs->rbx, (const char *)regs->rcx);
    else
      ret = -1;
    break;

  case SYS_READLINK:
    if (is_user_string((const char *)regs->rbx) &&
        is_user_range((void *)regs->rcx, (uint32_t)regs->rdx))
      ret = vfs_readlink((const char *)regs->rbx, (char *)regs->rcx,
                         (uint32_t)regs->rdx);
    else
      ret = -1;
    break;

  // === Phase 1: Identity & Security ===
  case SYS_GETUID:
    ret = task_get_uid();
    break;

  case SYS_GETGID:
    ret = task_get_gid();
    break;

  case SYS_SETUID:
    task_set_uid((uint32_t)regs->rbx);
    ret = 0;
    break;

  case SYS_SETGID:
    task_set_gid((uint32_t)regs->rbx);
    ret = 0;
    break;

  // === Shared Memory IPC ===
  case SYS_SHM_CREATE: {
    size_t size = (size_t)regs->rbx;
    ret = shm_create(size);
    break;
  }

  case SYS_SHM_ATTACH: {
    int shmid = (int)regs->rbx;
    ret = shm_attach(get_current_task(), shmid);
    break;
  }

  case SYS_SHM_DETACH: {
    uint64_t vaddr = regs->rbx;
    ret = shm_detach(get_current_task(), vaddr);
    break;
  }

  case SYS_SHM_STAT: {
    int shmid = (int)regs->rbx;
    if (is_user_range((void *)regs->rcx, sizeof(shm_info_t)))
      ret = shm_stat(shmid, (shm_info_t *)regs->rcx);
    else
      ret = -1;
    break;
  }

  // === Unix Domain Sockets ===
  case SYS_SOCKETPAIR: {
    if (is_user_range((void *)regs->rbx, 2 * sizeof(int)))
      ret = ipc_socketpair((int *)regs->rbx);
    else
      ret = -1;
    break;
  }

  case SYS_SENDMSG: {
    int fd = (int)regs->rbx;
    if (is_user_range((void *)regs->rcx, (size_t)regs->rdx))
      ret = ipc_send(fd, (const void *)regs->rcx, (size_t)regs->rdx);
    else
      ret = -1;
    break;
  }

  case SYS_RECVMSG: {
    int fd = (int)regs->rbx;
    if (is_user_range((void *)regs->rcx, (size_t)regs->rdx))
      ret = ipc_recv(fd, (void *)regs->rcx, (size_t)regs->rdx);
    else
      ret = -1;
    break;
  }

  // === Window Server IPC ===
  case SYS_WM_CONNECT: {
    task_t *t = get_current_task();
    ret = wm_connect(t);
    break;
  }

  case SYS_WM_CREATE_WINDOW: {
    task_t *t = get_current_task();
    int w = (int)regs->rbx;
    int h = (int)regs->rcx;
    ret = wm_create_window(t, w, h);
    break;
  }

  case SYS_WM_SUBMIT_FRAME: {
    task_t *t = get_current_task();
    int win_id = (int)regs->rbx;
    uint64_t fb_addr = regs->rcx;
    ret = wm_submit_frame(t, win_id, (void *)fb_addr);
    break;
  }

  case SYS_WM_GET_EVENT: {
    task_t *t = get_current_task();
    if (is_user_range((void *)regs->rbx, sizeof(wm_event_t)))
      ret = wm_get_event(t, (wm_event_t *)regs->rbx);
    else
      ret = -1;
    break;
  }

  case SYS_WM_GET_MOUSE: {
    task_t *t = get_current_task();
    if (is_user_range((void *)regs->rbx, 3 * sizeof(int)))
      ret = wm_get_mouse(t, (int *)regs->rbx);
    else
      ret = -1;
    break;
  }

  default:
    print_serial("SYSCALL: Unknown call.\n");
    ret = -1;
  }

  if (syscall_trace_enabled) {
    char buf[16];
    extern void k_itoa(int, char *);
    print_serial("SYSCALL RET: ");
    k_itoa(ret, buf);
    print_serial(buf);
    print_serial("\n");
  }

  regs->rax = ret;
  return (uint64_t)regs;
}

void syscall_init() {
  register_interrupt_handler(0x80, syscall_handler);
  arch_syscall_init();
}

// MSR-based syscall/sysret setup
#define IA32_STAR 0xC0000081
#define IA32_LSTAR 0xC0000082
#define IA32_CSTAR 0xC0000083
#define IA32_SF_MASK 0xC0000084

extern void syscall_entry_asm(void);

void arch_syscall_init(void) {
  uint64_t star = 0;
  // STAR[47:32] = sysret CS (must be kernel CS + 0, user CS = kernel CS + 16)
  // STAR[31:0]  = syscall CS (kernel CS)
  // Kernel CS=0x08, User CS=0x1B
  // STAR[47:32] = 0x001B0008 (sysret: kernel=CS+0=0x08, user=CS+16=0x1B)
  // STAR[31:0]  = 0x00000008 (syscall CS)
  star = (0x001B0008ULL << 32) | 0x00000008ULL;

  __asm__ volatile("wrmsr" : : "c"(IA32_STAR), "a"((uint32_t)star),
                   "d"((uint32_t)(star >> 32)));

  uint64_t lstar = (uint64_t)syscall_entry_asm;
  __asm__ volatile("wrmsr" : : "c"(IA32_LSTAR), "a"((uint32_t)lstar),
                   "d"((uint32_t)(lstar >> 32)));

  // Mask IF (interrupts) on syscall entry
  uint64_t sf_mask = 0x200; // Clear IF flag
  __asm__ volatile("wrmsr" : : "c"(IA32_SF_MASK), "a"((uint32_t)sf_mask),
                   "d"((uint32_t)(sf_mask >> 32)));

  print_serial("SYSCALL: Fast syscall/sysret enabled (MSR).\n");
}
