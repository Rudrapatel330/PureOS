#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "types.h"

typedef struct {
  volatile uint32_t locked;
} __attribute__((packed)) spinlock_t;

static inline void spinlock_init(spinlock_t *lock) { lock->locked = 0; }

static inline void spinlock_acquire(spinlock_t *lock) {
  // Use atomic exchange to set locked to 1
  // If the old value was already 1, keep spinning
  uint32_t old_val;
  do {
    __asm__ volatile("lock xchgl %0, %1"
                     : "=r"(old_val), "+m"(lock->locked)
                     : "0"(1)
                     : "memory");
    if (old_val == 0)
      break;
    __asm__ volatile("pause");
  } while (1);
}

static inline void spinlock_release(spinlock_t *lock) {
  __asm__ volatile("movl $0, %0" : "+m"(lock->locked) : : "memory");
}

// Spinlock with interrupt safety
typedef struct {
  spinlock_t lock;
  uint64_t rflags;
} __attribute__((packed)) spinlock_irq_t;

static inline void spinlock_irq_init(spinlock_irq_t *lock) {
  spinlock_init(&lock->lock);
  lock->rflags = 0;
}

static inline void spinlock_irq_acquire(spinlock_irq_t *lock) {
  // Save rflags and disable interrupts
  uint64_t rflags;
  __asm__ volatile("pushfq\n"
                   "pop %0\n"
                   "cli"
                   : "=r"(rflags)
                   :
                   : "memory");

  spinlock_acquire(&lock->lock);
  lock->rflags = rflags;
}

static inline void spinlock_irq_release(spinlock_irq_t *lock) {
  uint64_t rflags = lock->rflags;
  spinlock_release(&lock->lock);

  // Restore rflags
  __asm__ volatile("push %0\n"
                   "popfq"
                   :
                   : "r"(rflags)
                   : "memory");
}

#endif // SPINLOCK_H
