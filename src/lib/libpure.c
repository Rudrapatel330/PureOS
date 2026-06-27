#include "libpure.h"
#include <stdarg.h>
#include <stdint.h>

// === Process Control ===
void pure_exit(int status) {
  pure_syscall(SYS_EXIT, status, 0, 0, 0, 0);
  while (1)
    ;
}

int pure_fork(void) {
  return (int)pure_syscall(SYS_FORK, 0, 0, 0, 0, 0);
}

int pure_getpid(void) {
  return (int)pure_syscall(SYS_GETPID, 0, 0, 0, 0, 0);
}

int pure_execve(const char *path, char *const argv[], char *const envp[]) {
  (void)argv;
  (void)envp;
  return (int)pure_syscall(SYS_EXECVE, (int64_t)path, 0, 0, 0, 0);
}

// === Memory ===
void *pure_malloc(size_t size) {
  return (void *)pure_syscall(SYS_MALLOC, (int64_t)size, 0, 0, 0, 0);
}

void pure_free(void *ptr) {
  pure_syscall(SYS_FREE, (int64_t)ptr, 0, 0, 0, 0);
}

void *pure_mmap(void *addr, size_t length) {
  return (void *)pure_syscall(SYS_MMAP, (int64_t)addr, (int64_t)length, 0, 0, 0);
}

// === File I/O ===
int pure_open(const char *path, int flags) {
  return (int)pure_syscall(SYS_OPEN, (int64_t)path, (int64_t)flags, 0, 0, 0);
}

int pure_close(int fd) {
  return (int)pure_syscall(SYS_CLOSE, (int64_t)fd, 0, 0, 0, 0);
}

int64_t pure_read(int fd, void *buf, size_t count) {
  return pure_syscall(SYS_READ, (int64_t)fd, (int64_t)buf, (int64_t)count, 0, 0);
}

int64_t pure_write(int fd, const void *buf, size_t count) {
  return pure_syscall(SYS_WRITE, (int64_t)fd, (int64_t)buf, (int64_t)count, 0, 0);
}

int64_t pure_lseek(int fd, int64_t offset, int whence) {
  return pure_syscall(SYS_LSEEK, (int64_t)fd, (int64_t)offset, (int64_t)whence, 0, 0);
}

int pure_creat(const char *path, int mode) {
  return (int)pure_syscall(SYS_CREAT, (int64_t)path, (int64_t)mode, 0, 0, 0);
}

int pure_stat(const char *path, pure_stat_t *buf) {
  return (int)pure_syscall(SYS_STAT, (int64_t)path, (int64_t)buf, 0, 0, 0);
}

int pure_fstat(int fd, pure_stat_t *buf) {
  return (int)pure_syscall(SYS_FSTAT, (int64_t)fd, (int64_t)buf, 0, 0, 0);
}

int pure_mkdir(const char *path) {
  return (int)pure_syscall(SYS_MKDIR, (int64_t)path, 0, 0, 0, 0);
}

int pure_unlink(const char *path) {
  return (int)pure_syscall(SYS_UNLINK, (int64_t)path, 0, 0, 0, 0);
}

int pure_rename(const char *oldpath, const char *newpath) {
  return (int)pure_syscall(SYS_RENAME, (int64_t)oldpath, (int64_t)newpath, 0, 0, 0);
}

int pure_chmod(const char *path, int mode) {
  return (int)pure_syscall(SYS_CHMOD, (int64_t)path, (int64_t)mode, 0, 0, 0);
}

int pure_chown(const char *path, int uid, int gid) {
  return (int)pure_syscall(SYS_CHOWN, (int64_t)path, (int64_t)uid, (int64_t)gid, 0, 0);
}

int pure_pipe(int fds[2]) {
  return (int)pure_syscall(SYS_PIPE, (int64_t)fds, 0, 0, 0, 0);
}

int pure_dup2(int oldfd, int newfd) {
  return (int)pure_syscall(SYS_DUP2, (int64_t)oldfd, (int64_t)newfd, 0, 0, 0);
}

// === Directory ===
pure_dirent_t *pure_readdir(int fd) {
  return (pure_dirent_t *)pure_syscall(SYS_READDIR, (int64_t)fd, 0, 0, 0, 0);
}

// === Identity ===
int pure_getuid(void) {
  return (int)pure_syscall(SYS_GETUID, 0, 0, 0, 0, 0);
}

int pure_getgid(void) {
  return (int)pure_syscall(SYS_GETGID, 0, 0, 0, 0, 0);
}

int pure_setuid(int uid) {
  return (int)pure_syscall(SYS_SETUID, (int64_t)uid, 0, 0, 0, 0);
}

int pure_setgid(int gid) {
  return (int)pure_syscall(SYS_SETGID, (int64_t)gid, 0, 0, 0, 0);
}

// === Time ===
int pure_gettime(pure_time_t *t) {
  return (int)pure_syscall(SYS_GETTIME, (int64_t)t, 0, 0, 0, 0);
}

// === Input ===
int pure_readkey(void) {
  return (int)pure_syscall(SYS_READKEY, 0, 0, 0, 0, 0);
}

// === Shared Memory IPC ===
int pure_shm_create(size_t size) {
  return (int)pure_syscall(SYS_SHM_CREATE, (int64_t)size, 0, 0, 0, 0);
}

void *pure_shm_attach(int shmid) {
  return (void *)pure_syscall(SYS_SHM_ATTACH, (int64_t)shmid, 0, 0, 0, 0);
}

int pure_shm_detach(void *addr) {
  return (int)pure_syscall(SYS_SHM_DETACH, (int64_t)addr, 0, 0, 0, 0);
}

int pure_shm_stat(int shmid, shm_info_t *info) {
  return (int)pure_syscall(SYS_SHM_STAT, (int64_t)shmid, (int64_t)info, 0, 0, 0);
}

// === Unix Domain Sockets ===
int pure_socketpair(int fds[2]) {
  return (int)pure_syscall(SYS_SOCKETPAIR, (int64_t)fds, 0, 0, 0, 0);
}

int pure_sendmsg(int fd, const void *data, size_t len) {
  return (int)pure_syscall(SYS_SENDMSG, (int64_t)fd, (int64_t)data, (int64_t)len, 0, 0);
}

int pure_recvmsg(int fd, void *buf, size_t len) {
  return (int)pure_syscall(SYS_RECVMSG, (int64_t)fd, (int64_t)buf, (int64_t)len, 0, 0);
}

// === Window Server ===
int pure_wm_connect(void) {
  return (int)pure_syscall(SYS_WM_CONNECT, 0, 0, 0, 0, 0);
}

int pure_wm_create_window(int w, int h) {
  return (int)pure_syscall(SYS_WM_CREATE_WINDOW, (int64_t)w, (int64_t)h, 0, 0, 0);
}

int pure_wm_submit_frame(int win_id, void *fb) {
  return (int)pure_syscall(SYS_WM_SUBMIT_FRAME, (int64_t)win_id, (int64_t)fb, 0, 0, 0);
}

int pure_wm_get_event(wm_event_t *ev) {
  return (int)pure_syscall(SYS_WM_GET_EVENT, (int64_t)ev, 0, 0, 0, 0);
}

int pure_wm_get_mouse(int out[3]) {
  return (int)pure_syscall(SYS_WM_GET_MOUSE, (int64_t)out, 0, 0, 0, 0);
}

// === printf-like helper ===
int pure_printf(const char *fmt, ...) {
  char buf[2048];
  va_list args;
  va_start(args, fmt);

  // Simple format string processing
  int i = 0, bi = 0;
  while (fmt[i] && bi < (int)sizeof(buf) - 1) {
    if (fmt[i] == '%') {
      i++;
      switch (fmt[i]) {
      case 'd': {
        int val = va_arg(args, int);
        char tmp[32];
        int neg = 0, ti = 0;
        if (val < 0) {
          neg = 1;
          val = -val;
        }
        if (val == 0)
          tmp[ti++] = '0';
        while (val > 0) {
          tmp[ti++] = '0' + (val % 10);
          val /= 10;
        }
        if (neg)
          tmp[ti++] = '-';
        while (ti > 0)
          buf[bi++] = tmp[--ti];
        break;
      }
      case 's': {
        const char *s = va_arg(args, const char *);
        while (*s && bi < (int)sizeof(buf) - 1)
          buf[bi++] = *s++;
        break;
      }
      case 'x': {
        unsigned int val = va_arg(args, unsigned int);
        char tmp[16];
        int ti = 0;
        if (val == 0)
          tmp[ti++] = '0';
        while (val > 0) {
          int d = val % 16;
          tmp[ti++] = d < 10 ? '0' + d : 'a' + d - 10;
          val /= 16;
        }
        while (ti > 0)
          buf[bi++] = tmp[--ti];
        break;
      }
      case 'c':
        buf[bi++] = (char)va_arg(args, int);
        break;
      case '%':
        buf[bi++] = '%';
        break;
      default:
        buf[bi++] = '%';
        buf[bi++] = fmt[i];
        break;
      }
      i++;
    } else {
      buf[bi++] = fmt[i++];
    }
  }
  va_end(args);
  buf[bi] = '\0';

  pure_write(1, buf, bi);
  return bi;
}
