#ifndef LIBPURE_H
#define LIBPURE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Syscall numbers (must match kernel's syscall.h)
#define SYS_EXIT 0
#define SYS_PRINT 1
#define SYS_READKEY 2
#define SYS_GETMOUSE 3
#define SYS_SLEEP 4
#define SYS_MALLOC 5
#define SYS_FREE 6
#define SYS_GETTIME 7

#define SYS_OPEN 10
#define SYS_CLOSE 11
#define SYS_READ 12
#define SYS_WRITE 13
#define SYS_LSEEK 14
#define SYS_STAT 15
#define SYS_MKDIR 16
#define SYS_UNLINK 17
#define SYS_LISTDIR 18

#define SYS_FORK 20
#define SYS_EXECVE 21
#define SYS_WAITPID 22
#define SYS_GETPID 23
#define SYS_MMAP 24
#define SYS_MUNMAP 25
#define SYS_PIPE 26
#define SYS_DUP2 27
#define SYS_CHMOD 28
#define SYS_CHOWN 29
#define SYS_RENAME 30
#define SYS_READDIR 31
#define SYS_FSTAT 32
#define SYS_CREAT 33
#define SYS_SYMLINK 34
#define SYS_READLINK 35

#define SYS_GETUID 40
#define SYS_GETGID 41
#define SYS_SETUID 42
#define SYS_SETGID 43
#define SYS_UMASK 44

#define SYS_SHM_CREATE 50
#define SYS_SHM_ATTACH 51
#define SYS_SHM_DETACH 52
#define SYS_SHM_STAT 53

#define SYS_SOCKETPAIR 60
#define SYS_SENDMSG 61
#define SYS_RECVMSG 62

#define SYS_WM_CONNECT 80
#define SYS_WM_CREATE_WINDOW 81
#define SYS_WM_SUBMIT_FRAME 82
#define SYS_WM_GET_EVENT 83
#define SYS_WM_GET_MOUSE 84

// Time structure
typedef struct {
  int second;
  int minute;
  int hour;
  int day;
  int month;
  int year;
} pure_time_t;

// Stat structure
typedef struct {
  int st_size;
  int st_mode;
  int st_uid;
  int st_gid;
  int st_atime;
  int st_mtime;
  int st_ctime;
} pure_stat_t;

// Directory entry
typedef struct {
  char name[256];
  int type; // 0=file, 1=directory
  int size;
} pure_dirent_t;

// Window manager event
typedef struct {
  int type;
  int d1;
  int d2;
  int d3;
} wm_event_t;

// SHM info
typedef struct {
  int shmid;
  int size;
  int creator_pid;
  int refcount;
} shm_info_t;

// Raw syscall (int 0x80 fast path)
static inline int64_t pure_syscall(int num, int64_t a1, int64_t a2,
                                   int64_t a3, int64_t a4, int64_t a5) {
  int64_t ret;
  __asm__ volatile("mov %1, %%rax\n"
                   "mov %2, %%rbx\n"
                   "mov %3, %%rcx\n"
                   "mov %4, %%rdx\n"
                   "mov %5, %%rsi\n"
                   "mov %6, %%rdi\n"
                   "int $0x80\n"
                   "mov %%rax, %0"
                   : "=r"(ret)
                   : "r"((int64_t)num), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                     "r"(a5)
                   : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory");
  return ret;
}

// === Standard C Library Functions ===

// Process control
void pure_exit(int status);
int pure_fork(void);
int pure_getpid(void);
int pure_execve(const char *path, char *const argv[], char *const envp[]);

// Memory
void *pure_malloc(size_t size);
void pure_free(void *ptr);
void *pure_mmap(void *addr, size_t length);

// File I/O
int pure_open(const char *path, int flags);
int pure_close(int fd);
int64_t pure_read(int fd, void *buf, size_t count);
int64_t pure_write(int fd, const void *buf, size_t count);
int64_t pure_lseek(int fd, int64_t offset, int whence);
int pure_creat(const char *path, int mode);
int pure_stat(const char *path, pure_stat_t *buf);
int pure_fstat(int fd, pure_stat_t *buf);
int pure_mkdir(const char *path);
int pure_unlink(const char *path);
int pure_rename(const char *oldpath, const char *newpath);
int pure_chmod(const char *path, int mode);
int pure_chown(const char *path, int uid, int gid);
int pure_pipe(int fds[2]);
int pure_dup2(int oldfd, int newfd);

// Directory
pure_dirent_t *pure_readdir(int fd);

// Identity
int pure_getuid(void);
int pure_getgid(void);
int pure_setuid(int uid);
int pure_setgid(int gid);

// Time
int pure_gettime(pure_time_t *t);

// Input
int pure_readkey(void);

// Shared Memory IPC
int pure_shm_create(size_t size);
void *pure_shm_attach(int shmid);
int pure_shm_detach(void *addr);
int pure_shm_stat(int shmid, shm_info_t *info);

// Unix Domain Sockets
int pure_socketpair(int fds[2]);
int pure_sendmsg(int fd, const void *data, size_t len);
int pure_recvmsg(int fd, void *buf, size_t len);

// Printf (freestanding, no libc dependency)
int pure_printf(const char *fmt, ...);

// Window Server
int pure_wm_connect(void);
int pure_wm_create_window(int w, int h);
int pure_wm_submit_frame(int win_id, void *fb);
int pure_wm_get_event(wm_event_t *ev);
int pure_wm_get_mouse(int out[3]);

#ifdef __cplusplus
}
#endif

#endif // LIBPURE_H
