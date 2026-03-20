#include "syscall.h"
#include "../drivers/rtc.h"
#include "../drivers/vga.h"
#include "heap.h"
#include "isr.h"

#include "../drivers/keyboard.h"
#include "../drivers/mouse.h"
#include "../fs/vfs.h"
#include "task.h"

extern void print_serial(const char *);

int syscall_trace_enabled = 0;

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
  case 0: // SYS_EXIT
    print_serial("SYSCALL: User process exiting.\n");
    // Get current task PID and kill it
    task_kill(get_current_task()->id);
    break;

  case 1: // SYS_WRITE (Simplified)
    vga_puts(0, -1, (const char *)regs->rbx, 0x07);
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
    rtc_read((rtc_time_t *)regs->rbx);
    break;

  case SYS_OPEN:
    ret = vfs_open((const char *)regs->rbx, (int)regs->rcx);
    break;

  case SYS_READ:
    ret = vfs_read((int)regs->rbx, (uint8_t *)regs->rcx, (uint32_t)regs->rdx);
    break;

  case SYS_WRITE:
    ret = vfs_write((int)regs->rbx, (const uint8_t *)regs->rcx,
                    (uint32_t)regs->rdx);
    break;

  case SYS_CLOSE:
    vfs_close((int)regs->rbx);
    break;

  case SYS_GETPID:
    ret = get_current_task()->id;
    break;

  case SYS_FORK:
    ret = task_fork(regs);
    break;

  case SYS_EXECVE:
    ret = task_execve((const char *)regs->rbx, regs);
    break;

  case SYS_MMAP: {
    uint64_t addr = regs->rbx;
    uint32_t len = (uint32_t)regs->rcx;
    // Map 'len' bytes at 'addr' (if addr is 0, we should pick one, but for now
    // just use it)
    if (addr == 0)
      addr = 0x40000000; // Basic heap start for now

    uint32_t num_pages = (len + 4095) / 4096;
    for (uint32_t i = 0; i < num_pages; i++) {
      uint32_t phys;
      kmalloc_ap(4096, &phys);
      paging_map_user_page(get_current_task()->pagedir, addr + i * 4096, phys,
                           0x7); // User, RW, Present
    }
    ret = addr;
    break;
  }

  case SYS_PIPE:
    // TODO: Implement actual pipe in VFS
    print_serial("SYSCALL: pipe() not fully implemented\n");
    ret = 0;
    break;

  case SYS_DUP2:
    // TODO: Implement dup2 in VFS
    print_serial("SYSCALL: dup2() not fully implemented\n");
    ret = regs->rcx;
    break;

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

void syscall_init() { register_interrupt_handler(0x80, syscall_handler); }
