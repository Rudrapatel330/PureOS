#include "task.h"
#include "heap.h"
#include "isr.h"
#include "smp.h"
#include "hal/cpu.h"
#include "hal/atomic.h"
#include "spinlock.h"
#include "string.h"

extern void print_serial(const char *str);

// Idle task — runs when no other task is READY.
// Its cpu_ticks represent "idle" time; sysmon uses this to compute usage.
static void idle_task_entry(void) {
  while (1) {
    __asm__ volatile("sti; hlt");
  }
}
extern uint32_t tick;

static task_t *task_queue = 0;
static int next_pid = 1;
spinlock_t task_queue_lock;

// Stack guard canary pattern
#define STACK_GUARD_MAGIC 0xDEADBEEFCAFEBABEULL
#define STACK_GUARD_SIZE 64 // 64 bytes of guard pattern at stack base



static inline task_t *get_current(void) {
  return cpu_cores[get_core_id()].current_task;
}

void init_multitasking() {
  task_t *init_task = (task_t *)kmalloc(sizeof(task_t));
  memset(init_task, 0, sizeof(task_t));
  init_task->id = atomic_fetch_inc(&next_pid);
  cpu_cores[0].current_task = init_task;
  task_t *current_task = init_task;
  strcpy(current_task->name, "kernel");
  current_task->state = TASK_RUNNING;
  current_task->priority = 1;
  current_task->time_slice = 2; // priority * 2
  current_task->slice_remaining = 2;
  current_task->cpu_ticks = 0;
  current_task->cpu_usage_percent = 0;

  extern uint64_t kernel_stack_top;
  current_task->stack_base = (uint64_t)&kernel_stack_top - 64 * 1024;

  uint64_t rsp_val;
  __asm__ volatile("mov %%rsp, %0" : "=r"(rsp_val));
  current_task->rsp = rsp_val;

  extern pml4_table_t *kernel_pml4;
  current_task->pagedir = kernel_pml4;

  current_task->msg_queue.head = 0;
  current_task->msg_queue.tail = 0;
  spinlock_irq_init(&current_task->msg_queue.lock);

  current_task->next = current_task;

  task_queue = current_task;

  // Create 16 idle tasks so SMP cores always have one available
  for (int i = 0; i < 16; i++) {
    task_t *idle = create_task(idle_task_entry, "idle");
    if (idle) {
      idle->priority = 0;       // Lowest possible priority
      idle->time_slice = 1;
      idle->slice_remaining = 1;
    }
  }
}

task_t *get_current_task() { return get_current(); }

task_t *create_task(void (*entry)(), char *name) {
  task_t *t = (task_t *)kmalloc(sizeof(task_t));
  if (!t) {
    print_serial("TASK: Failed to allocate task structure for '");
    print_serial(name);
    print_serial("'\n");
    return 0;
  }

  t->id = atomic_fetch_inc(&next_pid);
  strcpy(t->name, name);
  t->state = TASK_READY;
  memset(t->files, 0, sizeof(t->files));

  // Allocate 2MB stack for tasks (MuPDF is very stack heavy)
  uint32_t stack_size = 2 * 1024 * 1024;
  uint64_t stack_phys = (uint64_t)kmalloc(stack_size + 16);
  if (!stack_phys) {
    print_serial("TASK: Failed to allocate stack for '");
    print_serial(name);
    print_serial("'\n");
    kfree(t);
    return 0;
  }

  // Ensure 16-byte alignment for SSE/Floating point
  uint64_t rsp = (stack_phys + stack_size + 15) & ~0xF;

  // Write guard canary at stack base (bottom of stack)
  for (int i = 0; i < STACK_GUARD_SIZE / 8; i++) {
    ((uint64_t *)stack_phys)[i] = STACK_GUARD_MAGIC;
  }

  // x86_64 ABI requires (RSP + 8) % 16 == 0 at function entry
  // (or RSP % 16 == 8). Our stack top is 16-byte aligned, so subtract 8.
  rsp -= 8;

  // Set up initial stack state for the ASM stub to restore
  rsp -= sizeof(registers_t);
  registers_t *regs = (registers_t *)rsp;

  memset(regs, 0, sizeof(registers_t));
  // gs, rdi, rsi, rbp, rbx, rdx, rcx, rax... are 0 from memset
  regs->rip = (uint64_t)entry;
  regs->cs = 0x08;      // Kernel Code
  regs->ss = 0x10;      // Kernel Data
  regs->rflags = 0x202; // IF set
  regs->rsp = rsp + sizeof(registers_t);

  t->rsp = rsp;
  t->stack_base = stack_phys;
  t->priority = 1;
  t->time_slice = 2; // priority * 2
  t->slice_remaining = 2;
  t->cpu_ticks = 0;
  t->cpu_usage_percent = 0;

  // Initialize FPU state for ISR restore
  uintptr_t fpu_area = (rsp - 512) & ~15;
  __asm__ volatile("fxsave (%0)" : : "r"(fpu_area) : "memory");

  // Run as root by default
  t->uid = 0;
  t->gid = 0;

  extern pml4_table_t *kernel_pml4;
  t->pagedir = kernel_pml4;

  t->msg_queue.head = 0;
  t->msg_queue.tail = 0;
  spinlock_irq_init(&t->msg_queue.lock);

  __asm__ volatile("cli");
  spinlock_acquire(&task_queue_lock);
  task_t *cur = get_current();
  t->next = cur->next;
  cur->next = t;
  spinlock_release(&task_queue_lock);
  __asm__ volatile("sti");

  print_serial("Task '");
  print_serial(name);
  print_serial("' linked. List order: ");
  task_t *tmp = cur;
  do {
    print_serial(tmp->name);
    print_serial(" -> ");
    tmp = tmp->next;
  } while (tmp != cur);
  print_serial("DONE\n");

  return t;
}

uint64_t task_switch(uint64_t current_rsp) {
  int this_core = get_core_id();
  task_t *current_task = cpu_cores[this_core].current_task;

  if (!current_task) {
    spinlock_acquire(&task_queue_lock);
    task_t *next = task_queue;
    if (next) {
      task_t *start = next;
      do {
        if (next->state == TASK_READY) {
          next->state = TASK_RUNNING;
          cpu_cores[this_core].current_task = next;
          spinlock_release(&task_queue_lock);
          extern void tss_set_stack_cpu(int cpu_num, uint64_t rsp0);
          tss_set_stack_cpu(this_core, next->stack_base + 128 * 1024);
          cpu_state[this_core].kernel_stack = next->stack_base + 128 * 1024;
          if (next->pagedir) {
            switch_page_directory(next->pagedir);
          }
          return next->rsp;
        }
        next = next->next;
      } while (next != start);
    }
    spinlock_release(&task_queue_lock);
    return current_rsp;
  }

  current_task->rsp = current_rsp;

  spinlock_acquire(&task_queue_lock);

  // --- Reaper: Clean up ZOMBIE tasks ---
  task_t *prev = current_task;
  task_t *t_reap = current_task->next;
  while (t_reap != current_task) {
    task_t *next_reap = t_reap->next;
    if (t_reap->state == TASK_ZOMBIE) {
      prev->next = next_reap;
      if (task_queue == t_reap) {
        task_queue = next_reap;
      }
      if (t_reap->stack_base) {
        kfree((void *)t_reap->stack_base);
      }
      kfree(t_reap);
    } else {
      prev = t_reap;
    }
    t_reap = next_reap;
  }

  // --- Fair Priority Scheduling ---
  if (current_task->slice_remaining > 0)
    current_task->slice_remaining--;

  int highest_prio = -1;
  task_t *t = current_task->next;
  do {
    if (t->state == TASK_READY) {
      if ((int)t->priority > highest_prio)
        highest_prio = t->priority;
    }
    t = t->next;
  } while (t != current_task->next);

  if ((current_task->state == TASK_RUNNING) &&
      (int)current_task->priority > highest_prio)
    highest_prio = current_task->priority;

  if (current_task->state == TASK_RUNNING &&
      current_task->slice_remaining > 0 &&
      (int)current_task->priority >= highest_prio) {
    spinlock_release(&task_queue_lock);
    return current_task->rsp;
  }

  task_t *next_task = 0;
  t = current_task->next;
  do {
    if (t->state == TASK_READY && (int)t->priority == highest_prio) {
      if (t->slice_remaining > 0) {
        next_task = t;
        break;
      }
      if (!next_task)
        next_task = t;
    }
    t = t->next;
  } while (t != current_task->next);

  if (next_task && next_task->slice_remaining == 0) {
    t = current_task->next;
    do {
      if (t->state == TASK_READY && (int)t->priority == highest_prio) {
        t->slice_remaining = t->time_slice;
      }
      t = t->next;
    } while (t != current_task->next);
    if ((int)current_task->priority == highest_prio &&
        current_task->state == TASK_RUNNING)
      current_task->slice_remaining = current_task->time_slice;
  }

  if (!next_task) {
    if (current_task->state == TASK_RUNNING) {
      next_task = current_task;
    } else {
      t = task_queue;
      if (t) {
        task_t *start = t;
        do {
          if (t->state == TASK_READY) {
            next_task = t;
            break;
          }
          t = t->next;
        } while (t != start);
      }
      if (!next_task) {
        t = task_queue;
        if (t) {
          task_t *start = t;
          do {
            if (t->state == TASK_READY) {
              next_task = t;
              break;
            }
            t = t->next;
          } while (t != start);
        }
        // If we still didn't find one, we are in trouble. 
        // But with 16 idle tasks, we should always find one.
      }
    }
  }

  if (next_task && next_task != current_task) {
    if (current_task->state == TASK_RUNNING) {
      current_task->state = TASK_READY;
    }

    current_task = next_task;
    current_task->state = TASK_RUNNING;

    cpu_cores[this_core].current_task = current_task;

    extern void tss_set_stack_cpu(int cpu_num, uint64_t rsp0);
    tss_set_stack_cpu(this_core, current_task->stack_base + 128 * 1024);
    cpu_state[this_core].kernel_stack = current_task->stack_base + 128 * 1024;

    if (current_task->pagedir) {
      switch_page_directory(current_task->pagedir);
    }
  } else {
    if (current_task->slice_remaining == 0)
      current_task->slice_remaining = current_task->time_slice;
  }

  spinlock_release(&task_queue_lock);
  return current_task->rsp;
}

// Phase 4: Jump to User Mode
void jump_to_user_mode(uint64_t rip) {
  // 64-bit iretq stack frame:
  // [SS] [RSP] [RFLAGS] [CS] [RIP]

  __asm__ volatile(
      "cli\n"
      "mov $0x23, %%ax\n" // User Data Segment (0x23 assumes DS=0x20|3)
      "mov %%ax, %%ds\n"
      "mov %%ax, %%es\n"
      "mov %%ax, %%fs\n"
      "mov %%ax, %%gs\n"
      "pushq $0x23\n"       // SS
      "pushq $0x7FFFFFF0\n" // RSP (test stack top)
      "pushfq\n"            // RFLAGS
      "popq %%rax\n"
      "orq $0x200, %%rax\n"
      "pushq %%rax\n"
      "pushq $0x1B\n" // CS (User Code = 0x18 | 3)
      "pushq %0\n"    // RIP
      "iretq\n"
      :
      : "r"(rip)
      : "rax");
}

task_t *create_user_process(const char *name, void *entry) {
  task_t *t = (task_t *)kmalloc(sizeof(task_t));
  t->id = atomic_fetch_inc(&next_pid);
  strcpy(t->name, (char *)name);
  t->state = TASK_READY;
  t->is_user = 1;
  memset(t->files, 0, sizeof(t->files));

  // 1. Create Address Space
  t->pagedir = paging_create_user_address_space();

  // 2. Allocate and Map User Stack (at 0x70000000, 64KB)
  uint64_t stack_vaddr = 0x70000000;
  uint32_t stack_phys;
  void *stack_ptr = kmalloc_ap(65536, &stack_phys);
  for (int i = 0; i < 16; i++) {
    paging_map_user_page(t->pagedir, stack_vaddr + i * 4096,
                         stack_phys + i * 4096, 0x3); // User, RW
  }
  t->user_stack_base = (uint64_t)stack_ptr;
  t->user_rsp = stack_vaddr + 65536;

  // 3. Allocate Kernel Stack for this task (interrupted state)
  uint32_t kstack_size = 128 * 1024;
  uint64_t kstack_phys = (uint64_t)kmalloc(kstack_size + 16);
  uint64_t rsp = (kstack_phys + kstack_size + 15) & ~0xF;

  // Write guard canary at kernel stack base
  for (int i = 0; i < STACK_GUARD_SIZE / 8; i++) {
    ((uint64_t *)kstack_phys)[i] = STACK_GUARD_MAGIC;
  }

  // 4. Set up initial stack state for the ASM stub to restore (iretq frame)
  rsp -= sizeof(registers_t);
  registers_t *regs = (registers_t *)rsp;
  memset(regs, 0, sizeof(registers_t));

  regs->rip = (uint64_t)entry;
  regs->cs = 0x1B;      // User Code
  regs->ss = 0x23;      // User Data
  regs->rflags = 0x202; // IF set
  regs->rsp = t->user_rsp;

  t->rsp = rsp;
  t->stack_base = kstack_phys;
  t->priority = 1;
  t->time_slice = 2; // priority * 2
  t->slice_remaining = 2;
  t->cpu_ticks = 0;
  t->cpu_usage_percent = 0;

  // Initialize FPU state for ISR restore
  uintptr_t fpu_area = (rsp - 512) & ~15;
  __asm__ volatile("fxsave (%0)" : : "r"(fpu_area) : "memory");

  t->uid = 1000; // User ID

  return t;
}

int task_fork(registers_t *regs) {
  __asm__ volatile("cli");
  task_t *parent = get_current();

  // 1. Deep copy page directory
  pml4_table_t *child_pml4 = copy_page_directory(parent->pagedir);

  // 2. Create new task structure
  task_t *child = (task_t *)kmalloc(sizeof(task_t));
  memset(child, 0, sizeof(task_t));
  child->id = atomic_fetch_inc(&next_pid);
  strcpy(child->name, parent->name);
  strcat(child->name, "_child");
  child->pagedir = child_pml4;
  child->state = TASK_READY;
  child->priority = parent->priority;
  child->time_slice = parent->time_slice;
  child->slice_remaining = child->time_slice;
  child->is_user = parent->is_user;
  child->uid = parent->uid;
  child->gid = parent->gid;
  child->cpu_ticks = 0;
  child->cpu_usage_percent = 0;

  extern void vfs_dup_fd_table(void **parent_files, void **child_files);
  vfs_dup_fd_table(parent->files, child->files);

  // 3. Duplicate Kernel Stack
  uint32_t kstack_size = 64 * 1024;
  uint64_t kstack_phys = (uint64_t)kmalloc(kstack_size + 16);
  child->stack_base = kstack_phys;

  // Copy the entire kernel stack content
  memcpy((void *)kstack_phys, (void *)parent->stack_base, kstack_size);

  // Calculate the offset of the current stack pointer from the stack base
  uint64_t stack_offset = parent->rsp - parent->stack_base;
  child->rsp = kstack_phys + stack_offset;

  // 4. Modify Child's return value (RAX)
  // The registers are on the stack at the 'rsp' we just calculated
  registers_t *child_regs = (registers_t *)child->rsp;
  child_regs->rax = 0; // Return 0 in child

  // 5. Link to queue
  spinlock_acquire(&task_queue_lock);
  task_t *cur_fork = get_current();
  child->next = cur_fork->next;
  cur_fork->next = child;
  spinlock_release(&task_queue_lock);

  __asm__ volatile("sti");
  return child->id; // Return child PID in parent
}

void task_set_priority(int pid, int priority) {
  __asm__ volatile("cli");
  task_t *t = task_queue;
  if (!t) {
    __asm__ volatile("sti");
    return;
  }
  do {
    if (t->id == pid) {
      t->priority = priority;
      t->time_slice = (uint32_t)(priority < 1 ? 2 : priority * 2);
      t->slice_remaining = t->time_slice;
      break;
    }
    t = t->next;
  } while (t != task_queue);
  __asm__ volatile("sti");
}

void exit(int exit_code) {
  __asm__ volatile("cli");
  task_t *cur_exit = get_current();
  if (cur_exit) {
    extern void vfs_close_all_fds(void **files);
    vfs_close_all_fds(cur_exit->files);
    cur_exit->state = TASK_ZOMBIE;
    print_serial(" [TASK EXIT: ");
    print_serial(cur_exit->name);
    print_serial("] ");
  }
  __asm__ volatile("sti");

  // Force a switch immediately
  __asm__ volatile("int $49");

  while (1)
    __asm__ volatile("hlt");
}

void task_kill(int pid) {
  __asm__ volatile("cli");
  task_t *cur_kill = get_current();
  task_t *t = task_queue;
  if (!t) {
    __asm__ volatile("sti");
    return;
  }

  do {
    if (t->id == pid) {
      if (t == cur_kill) {
        __asm__ volatile("sti");
        exit(0);
        return;
      }
      extern void vfs_close_all_fds(void **files);
      vfs_close_all_fds(t->files);
      t->state = TASK_STOPPED;
      print_serial(" [TASK KILLED: ");
      print_serial(t->name);
      print_serial("] ");
      break;
    }
    t = t->next;
  } while (t != task_queue);
  __asm__ volatile("sti");
}

#include "elf.h"

int task_execve(const char *path, registers_t *regs) {
  task_t *t = get_current();

  // 1. Clear current user address space
  // For now, let's just create a fresh one
  pml4_table_t *old_pml4 = t->pagedir;
  t->pagedir = paging_create_user_address_space();
  switch_page_directory(t->pagedir);

  // 2. Load the new ELF
  if (!elf_load_into_process(t, path)) {
    // Failed to load — this is a problem since we already cleared the old
    // space. In a real OS, we'd check the file before clearing.
    print_serial("EXECVE: Failed to load ");
    print_serial(path);
    print_serial("\n");
    t->pagedir = old_pml4; // Try to restore
    switch_page_directory(old_pml4);
    return -1;
  }

  // 3. Reset registers for new entry point
  // We don't need to change cs, ss, rflags typically
  regs->rip = t->rip;

  // 4. Reset User Stack
  // (We currently re-allocate a stack in elf_load_into_process? No, it only
  // maps segments) Actually create_user_process allocates a stack at
  // 0x70000000. We should probably re-map a fresh stack.
  uint64_t stack_vaddr = 0x70000000;
  uint32_t stack_phys;
  void *stack_ptr = kmalloc_ap(65536, &stack_phys);
  for (int i = 0; i < 16; i++) {
    paging_map_user_page(t->pagedir, stack_vaddr + i * 4096,
                         stack_phys + i * 4096, 0x3); // User, RW
  }
  regs->rsp = stack_vaddr + 65536;

  // Update task name
  strcpy(t->name, (char *)path);

  return 0; // Success (though return value in RAX will be set to 0)
}

task_t *get_task_by_pid(int pid) {
  if (!task_queue)
    return 0;
  task_t *t = task_queue;
  do {
    if (t->id == pid)
      return t;
    t = t->next;
  } while (t != task_queue);
  return 0;
}

task_t *get_task_by_name(const char *name) {
  if (!task_queue)
    return 0;
  task_t *t = task_queue;
  do {
    if (strcmp(t->name, name) == 0)
      return t;
    t = t->next;
  } while (t != task_queue);
  return 0;
}

// IPC Functions
int msg_send(int pid, msg_t *msg) {
  if (!msg)
    return -1;
  uint64_t rflags;
  __asm__ volatile("pushfq; pop %0; cli" : "=r"(rflags));

  task_t *target = get_task_by_pid(pid);
  if (!target) {
    __asm__ volatile("push %0; popfq" ::"r"(rflags));
    return -1;
  }

  msg_queue_t *q = &target->msg_queue;
  spinlock_irq_acquire(&q->lock);

  int next_head = (q->head + 1) % TASK_MSG_QUEUE_SIZE;
  if (next_head == q->tail) {
    // Queue full
    spinlock_irq_release(&q->lock);
    __asm__ volatile("push %0; popfq" ::"r"(rflags));
    return -1; // Or block? For now just drop
  }

  q->queue[q->head] = *msg;
  q->head = next_head;

  /* Wake the target if it was sleeping (waiting for messages)
     CRITICAL: Only wake if it's TASK_STOPPED.
     DO NOT wake if it's TASK_ZOMBIE (exiting/dead). */
  if (target->state == TASK_STOPPED) {
    target->state = TASK_READY;
  }

  spinlock_irq_release(&q->lock);
  __asm__ volatile("push %0; popfq" ::"r"(rflags));
  return 0;
}

int msg_send_to_name(const char *name, msg_t *msg) {
  task_t *t = get_task_by_name(name);
  if (!t)
    return -1;
  return msg_send(t->id, msg);
}

int msg_receive(msg_t *msg) {
  task_t *cur_recv = get_current();
  if (!cur_recv || !msg)
    return -1;

  msg_queue_t *q = &cur_recv->msg_queue;
  spinlock_irq_acquire(&q->lock);

  if (q->head == q->tail) {
    // Queue empty
    spinlock_irq_release(&q->lock);
    return 0; // Return 0 messages read
  }

  *msg = q->queue[q->tail];
  q->tail = (q->tail + 1) % TASK_MSG_QUEUE_SIZE;

  spinlock_irq_release(&q->lock);
  return 1; // 1 message read
}

int msg_peek(msg_t *msg) {
  task_t *cur_peek = get_current();
  if (!cur_peek || !msg)
    return -1;

  msg_queue_t *q = &cur_peek->msg_queue;
  spinlock_irq_acquire(&q->lock);

  if (q->head == q->tail) {
    // Queue empty
    spinlock_irq_release(&q->lock);
    return 0;
  }

  *msg = q->queue[q->tail];

  spinlock_irq_release(&q->lock);
  return 1;
}

// Check stack guard canary integrity for a given task
int task_check_stack(task_t *t) {
  if (!t || !t->stack_base)
    return -1;
  uint64_t *guard = (uint64_t *)t->stack_base;
  for (int i = 0; i < STACK_GUARD_SIZE / 8; i++) {
    if (guard[i] != STACK_GUARD_MAGIC) {
      print_serial("STACK OVERFLOW DETECTED in task '");
      print_serial(t->name);
      print_serial("'!\n");
      return 0; // Corrupted
    }
  }
  return 1; // OK
}

uint32_t task_get_uid(void) {
  task_t *cur = get_current();
  if (cur)
    return cur->uid;
  return 0;
}

uint32_t task_get_gid(void) {
  task_t *cur = get_current();
  if (cur)
    return cur->gid;
  return 0;
}

void task_set_uid(uint32_t uid) {
  task_t *cur = get_current();
  if (cur)
    cur->uid = uid;
}

void task_set_gid(uint32_t gid) {
  task_t *cur = get_current();
  if (cur)
    cur->gid = gid;
}

void update_cpu_stats(void) {
  spinlock_acquire(&task_queue_lock);
  if (!task_queue) {
    spinlock_release(&task_queue_lock);
    return;
  }
  uint64_t total_ticks = 0;
  task_t *t = task_queue;
  do {
    total_ticks += t->cpu_ticks;
    t = t->next;
  } while (t != task_queue);

  if (total_ticks == 0) {
    spinlock_release(&task_queue_lock);
    return;
  }

  t = task_queue;
  do {
    int pct = (t->cpu_ticks * 100) / total_ticks;
    t->cpu_usage_percent = pct;
    t->cpu_ticks = 0; // reset for next window
    t = t->next;
  } while (t != task_queue);
  
  spinlock_release(&task_queue_lock);
}

uint32_t get_idle_cpu_usage(void) {
  spinlock_acquire(&task_queue_lock);
  if (!task_queue) {
    spinlock_release(&task_queue_lock);
    return 0;
  }
  uint32_t idle_sum = 0;
  task_t *t = task_queue;
  do {
    if (strcmp(t->name, "idle") == 0) {
      idle_sum += t->cpu_usage_percent;
    }
    t = t->next;
  } while (t != task_queue);
  spinlock_release(&task_queue_lock);
  return idle_sum;
}
