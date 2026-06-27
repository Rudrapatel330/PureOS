#ifndef IPC_H
#define IPC_H

#include "task.h"
#include <stddef.h>
#include <stdint.h>

// === Shared Memory Segments ===
#define SHM_MAX_SEGMENTS 64
#define SHM_MAX_SIZE 0x100000 // 1MB max per segment

typedef struct {
  int id;
  size_t size;
  uint32_t phys_addr;
  int refcount;
  int creator_pid;
  int is_free;
} shm_segment_t;

typedef struct {
  int shmid;
  size_t size;
  int creator_pid;
  int refcount;
} shm_info_t;

int shm_create(size_t size);
uint64_t shm_attach(task_t *t, int shmid);
int shm_detach(task_t *t, uint64_t vaddr);
int shm_stat(int shmid, shm_info_t *info);
void shm_init(void);

// === Unix Domain Sockets (Stream Pipes) ===
#define IPC_MAX_PAIRS 32
#define IPC_BUFFER_SIZE 4096

typedef struct {
  int used;
  int read_fd;
  int write_fd;
  uint8_t buffer[IPC_BUFFER_SIZE];
  int head;
  int tail;
  int read_open;
  int write_open;
} ipc_pair_t;

int ipc_socketpair(int fds[2]);
int ipc_send(int fd, const void *data, size_t len);
int ipc_recv(int fd, void *buf, size_t len);
void ipc_init(void);

// === Window Server Protocol ===
#define WM_MAX_CLIENTS 16
#define WM_MAX_WINDOWS_PER_CLIENT 8

typedef struct {
  int type; // 0=key, 1=mouse_move, 2=mouse_click, 3=close, 4=resize
  int d1;
  int d2;
  int d3;
} wm_event_t;

typedef struct {
  int client_id;
  task_t *task;
  int window_ids[WM_MAX_WINDOWS_PER_CLIENT];
  int window_count;
} wm_client_t;

typedef struct {
  int win_id;
  int client_id;
  int x, y, w, h;
  uint64_t shm_vaddr; // Shared memory framebuffer
  int shm_id;         // Shared memory segment ID
  int active;
} wm_window_t;

int wm_connect(task_t *t);
int wm_create_window(task_t *t, int w, int h);
int wm_submit_frame(task_t *t, int win_id, void *fb);
int wm_get_event(task_t *t, wm_event_t *ev);
int wm_get_mouse(task_t *t, int out[3]);
void wm_init(void);
void wm_dispatch_input(int type, int d1, int d2, int d3);

#endif
