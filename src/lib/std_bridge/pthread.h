#ifndef PTHREAD_H
#define PTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int pthread_mutex_t;
typedef int pthread_mutexattr_t;

#define PTHREAD_MUTEX_INITIALIZER 0

static inline int pthread_mutex_init(pthread_mutex_t* m, const pthread_mutexattr_t* a) { (void)m; (void)a; return 0; }
static inline int pthread_mutex_lock(pthread_mutex_t* m) { (void)m; return 0; }
static inline int pthread_mutex_unlock(pthread_mutex_t* m) { (void)m; return 0; }
static inline int pthread_mutex_destroy(pthread_mutex_t* m) { (void)m; return 0; }

#ifdef __cplusplus
}
#endif

#endif
