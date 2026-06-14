#include "arch/sys_arch.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/err.h"

extern uint32_t kmalloc(uint32_t size);
extern void kfree(void *ptr);
extern uint64_t get_timer_ms_hires();
extern void print_serial(const char *);

static void yield(void) {
    __asm__ volatile("int $32");
}

u32_t sys_now(void) {
    return (u32_t)get_timer_ms_hires();
}

void sys_init(void) {
    print_serial("LWIP: sys_init\n");
}

sys_prot_t sys_arch_protect(void) {
    uint64_t flags;
    __asm__ volatile("pushfq\n\tpopq %0\n\tcli" : "=r"(flags));
    return (sys_prot_t)flags;
}

void sys_arch_unprotect(sys_prot_t pval) {
    if (pval & (1 << 9)) {
        __asm__ volatile("sti");
    }
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count) {
    *sem = (sys_sem_t)kmalloc(sizeof(struct sys_sem_t));
    if (*sem == NULL) return ERR_MEM;
    (*sem)->count = count;
    (*sem)->waiting = 0;
    return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem) {
    if (sem && *sem) {
        kfree(*sem);
        *sem = NULL;
    }
}

void sys_sem_signal(sys_sem_t *sem) {
    __atomic_add_fetch(&((*sem)->count), 1, __ATOMIC_SEQ_CST);
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout) {
    u32_t start_time = sys_now();
    __atomic_add_fetch(&((*sem)->waiting), 1, __ATOMIC_SEQ_CST);
    
    while ((*sem)->count == 0) {
        if (timeout != 0) {
            if ((sys_now() - start_time) >= timeout) {
                __atomic_sub_fetch(&((*sem)->waiting), 1, __ATOMIC_SEQ_CST);
                return SYS_ARCH_TIMEOUT;
            }
        }
        yield();
    }
    
    __atomic_sub_fetch(&((*sem)->waiting), 1, __ATOMIC_SEQ_CST);
    __atomic_sub_fetch(&((*sem)->count), 1, __ATOMIC_SEQ_CST);
    
    return sys_now() - start_time;
}

err_t sys_mutex_new(sys_mutex_t *mutex) {
    *mutex = (sys_mutex_t)kmalloc(sizeof(struct sys_mutex_t));
    if (*mutex == NULL) return ERR_MEM;
    (*mutex)->locked = 0;
    return ERR_OK;
}

void sys_mutex_free(sys_mutex_t *mutex) {
    if (mutex && *mutex) {
        kfree(*mutex);
        *mutex = NULL;
    }
}

void sys_mutex_lock(sys_mutex_t *mutex) {
    while (__atomic_test_and_set(&((*mutex)->locked), __ATOMIC_ACQUIRE)) {
        yield();
    }
}

void sys_mutex_unlock(sys_mutex_t *mutex) {
    __atomic_clear(&((*mutex)->locked), __ATOMIC_RELEASE);
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size) {
    *mbox = (sys_mbox_t)kmalloc(sizeof(struct sys_mbox_t));
    if (*mbox == NULL) return ERR_MEM;
    
    (*mbox)->size = size;
    (*mbox)->head = 0;
    (*mbox)->tail = 0;
    (*mbox)->messages = (void**)kmalloc(sizeof(void*) * size);
    
    sys_sem_new(&((*mbox)->sem), 0);
    
    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox) {
    if (mbox && *mbox) {
        sys_sem_free(&((*mbox)->sem));
        kfree((*mbox)->messages);
        kfree(*mbox);
        *mbox = NULL;
    }
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg) {
    while ((((*mbox)->head + 1) % (*mbox)->size) == (*mbox)->tail) {
        yield(); 
    }
    
    (*mbox)->messages[(*mbox)->head] = msg;
    (*mbox)->head = ((*mbox)->head + 1) % (*mbox)->size;
    
    sys_sem_signal(&((*mbox)->sem));
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg) {
    if ((((*mbox)->head + 1) % (*mbox)->size) == (*mbox)->tail) {
        return ERR_MEM;
    }
    
    (*mbox)->messages[(*mbox)->head] = msg;
    (*mbox)->head = ((*mbox)->head + 1) % (*mbox)->size;
    
    sys_sem_signal(&((*mbox)->sem));
    return ERR_OK;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout) {
    u32_t time_waited = sys_arch_sem_wait(&((*mbox)->sem), timeout);
    if (time_waited == SYS_ARCH_TIMEOUT) {
        return SYS_ARCH_TIMEOUT;
    }
    
    if (msg != NULL) {
        *msg = (*mbox)->messages[(*mbox)->tail];
    }
    (*mbox)->tail = ((*mbox)->tail + 1) % (*mbox)->size;
    
    return time_waited;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg) {
    if ((*mbox)->head == (*mbox)->tail) {
        return SYS_MBOX_EMPTY;
    }
    
    if (msg != NULL) {
        *msg = (*mbox)->messages[(*mbox)->tail];
    }
    (*mbox)->tail = ((*mbox)->tail + 1) % (*mbox)->size;
    
    __atomic_sub_fetch(&(((*mbox)->sem)->count), 1, __ATOMIC_SEQ_CST);
    
    return 0;
}

int sys_mbox_valid(sys_mbox_t *mbox) {
    return (mbox != NULL && *mbox != NULL);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox) {
    if (mbox) *mbox = NULL;
}

int sys_sem_valid(sys_sem_t *sem) {
    return (sem != NULL && *sem != NULL);
}

void sys_sem_set_invalid(sys_sem_t *sem) {
    if (sem) *sem = NULL;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg) {
    return sys_mbox_trypost(mbox, msg);
}

extern void *create_task(void (*entry)(), char *name);

// Global argument pointer for lwIP tcpip_thread hack
void *g_lwip_thread_arg = NULL;
lwip_thread_fn g_lwip_thread_func = NULL;

static void lwip_thread_wrapper() {
    if (g_lwip_thread_func) {
        g_lwip_thread_func(g_lwip_thread_arg);
    }
    while(1) yield();
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio) {
    (void)stacksize;
    (void)prio;
    
    g_lwip_thread_func = thread;
    g_lwip_thread_arg = arg;
    
    print_serial("LWIP: Creating thread ");
    print_serial(name);
    print_serial("\n");
    
    void *t = create_task(lwip_thread_wrapper, (char*)name);
    return (sys_thread_t)t;
}
