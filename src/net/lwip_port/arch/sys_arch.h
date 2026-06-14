#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include <stdint.h>

#define SYS_MBOX_NULL (NULL)
#define SYS_SEM_NULL  (NULL)

typedef struct sys_sem_t {
    volatile int count;
    volatile int waiting;
} *sys_sem_t;

typedef struct sys_mutex_t {
    volatile int locked;
} *sys_mutex_t;

typedef struct sys_mbox_t {
    void **messages;
    int size;
    volatile int head;
    volatile int tail;
    sys_sem_t sem;
} *sys_mbox_t;

typedef void * sys_thread_t;
typedef uint32_t sys_prot_t;

#endif /* LWIP_ARCH_SYS_ARCH_H */
