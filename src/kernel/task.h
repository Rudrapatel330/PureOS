#ifndef TASK_H
#define TASK_H

#include "hal/paging.h"
#include "spinlock.h"
#include <stdint.h>

#define TASK_MSG_QUEUE_SIZE 256

// Message types
typedef enum {
  MSG_NONE,
  MSG_KEYBOARD,
  MSG_MOUSE_MOVE,
  MSG_MOUSE_CLICK,
  MSG_MOUSE_SCROLL,
  MSG_WINDOW_EVENT,

  MSG_QUIT,
  MSG_SCREENSHOT,
  MSG_SHOW_SEARCH,
  MSG_USER
} msg_type_t;

// Standard Message Structure
typedef struct {
  msg_type_t type;
  int d1; // Data 1 (e.g., keycode or mouse X)
  int d2; // Data 2 (e.g., char or mouse Y)
  int d3; // Data 3 (e.g., buttons)
  int d4;
  void *ptr; // Optional pointer for complex payloads
} msg_t;

// Message Queue
typedef struct {
  msg_t queue[TASK_MSG_QUEUE_SIZE];
  int head;
  int tail;
  spinlock_irq_t lock;
} msg_queue_t;

// Process states
typedef enum {
  TASK_RUNNING,
  TASK_READY,
  TASK_STOPPED,
  TASK_ZOMBIE
} task_state_t;

// Task control block
typedef struct task {
  uint8_t fpu_state[512 + 16] __attribute__((aligned(16)));
  int id;                   // Process ID
  char name[32];            // Process Name
  uint64_t rsp;             // Current stack pointer
  uint64_t rbp;             // Base pointer
  uint64_t rip;             // Instruction pointer
  pml4_table_t *pagedir;    // Page directory
  task_state_t state;       // Current state
  uint64_t stack_base;      // Base pointer for stack (to free)
  int priority;             // Process priority (higher = more important)
  uint32_t time_slice;      // Total ticks per scheduling round
  uint32_t slice_remaining; // Ticks left in current round
  msg_queue_t msg_queue;    // IPC Event Queue
  int is_user;              // 1 if user process
  uint64_t user_stack_base;
  uint64_t user_rsp;
  uint32_t uid;          // Security: User ID
  uint32_t gid;          // Security: Group ID
  uint64_t cpu_ticks;    // Total CPU ticks consumed
  int cpu_usage_percent; // Last calculated CPU percentage
  struct task *next;     // Link to next task
} task_t;

// IPC Functions
task_t *get_task_by_name(const char *name);
int msg_send_to_name(const char *name, msg_t *msg);
int msg_send(int pid, msg_t *msg);
int msg_receive(msg_t *msg); // Pops from current task
int msg_peek(msg_t *msg);    // Peeks without popping

// Multitasking Interface
void init_multitasking();
uint64_t task_switch(uint64_t current_rsp);
task_t *create_task(void (*entry)(), char *name);
task_t *create_user_process(const char *name, void *entry);
task_t *get_current_task();
void exit(int exit_code);
void jump_to_user_mode(uint64_t rip);
void task_set_priority(int pid, int priority);
void task_kill(int pid);
task_t *get_task_by_pid(int pid);
int task_check_stack(task_t *t);
int task_fork(registers_t *regs);
int task_execve(const char *path, registers_t *regs);

void print_cpu_stats(void);

#endif
