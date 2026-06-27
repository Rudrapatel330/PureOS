#include "ipc.h"
#include "heap.h"
#include "string.h"
#include "syscall.h"

extern void print_serial(const char *);
extern pml4_table_t *kernel_pml4;

// ==== Shared Memory Implementation ====
static shm_segment_t shm_segments[SHM_MAX_SEGMENTS];
static int shm_next_id = 1;

void shm_init(void) {
  memset(shm_segments, 0, sizeof(shm_segments));
  print_serial("IPC: Shared memory initialized.\n");
}

int shm_create(size_t size) {
  if (size == 0 || size > SHM_MAX_SIZE)
    return -1;

  // Find free slot
  int slot = -1;
  for (int i = 0; i < SHM_MAX_SEGMENTS; i++) {
    if (shm_segments[i].is_free) {
      slot = i;
      break;
    }
  }
  if (slot < 0)
    return -1;

  // Allocate physical pages
  uint32_t num_pages = (size + 4095) / 4096;
  uint32_t phys;
  void *buf = kmalloc_ap(num_pages * 4096, &phys);
  if (!buf)
    return -1;
  memset(buf, 0, num_pages * 4096);

  shm_segments[slot].id = shm_next_id++;
  shm_segments[slot].size = size;
  shm_segments[slot].phys_addr = phys;
  shm_segments[slot].refcount = 0;
  shm_segments[slot].creator_pid = get_current_task()->id;
  shm_segments[slot].is_free = 0;

  print_serial("IPC: SHM created id=");
  char buf2[16];
  extern void k_itoa(int, char *);
  k_itoa(shm_segments[slot].id, buf2);
  print_serial(buf2);
  print_serial(" size=");
  k_itoa((int)size, buf2);
  print_serial(buf2);
  print_serial("\n");

  return shm_segments[slot].id;
}

uint64_t shm_attach(task_t *t, int shmid) {
  // Find segment
  int slot = -1;
  for (int i = 0; i < SHM_MAX_SEGMENTS; i++) {
    if (!shm_segments[i].is_free && shm_segments[i].id == shmid) {
      slot = i;
      break;
    }
  }
  if (slot < 0)
    return 0;

  shm_segment_t *seg = &shm_segments[slot];
  uint32_t num_pages = (seg->size + 4095) / 4096;

  // Pick a virtual address for the mapping
  uint64_t vaddr = 0x50000000; // Fixed region for shared memory
  // In production, we'd scan for free space

  for (uint32_t i = 0; i < num_pages; i++) {
    paging_map_user_page(t->pagedir, vaddr + i * 4096,
                         seg->phys_addr + i * 4096, 0x3);
  }

  seg->refcount++;

  print_serial("IPC: SHM attached id=");
  char buf[16];
  extern void k_itoa(int, char *);
  k_itoa(shmid, buf);
  print_serial(buf);
  print_serial(" vaddr=0x");
  k_itoa((int)vaddr, buf);
  print_serial(buf);
  print_serial("\n");

  return vaddr;
}

int shm_detach(task_t *t, uint64_t vaddr) {
  (void)t;
  (void)vaddr;
  // Find which segment this vaddr belongs to
  int slot = -1;
  for (int i = 0; i < SHM_MAX_SEGMENTS; i++) {
    if (shm_segments[i].is_free)
      continue;
    slot = i;
    break;
  }
  if (slot < 0)
    return -1;

  shm_segments[slot].refcount--;

  // If no more references, free the physical pages
  if (shm_segments[slot].refcount <= 0) {
    shm_segments[slot].is_free = 1;
    print_serial("IPC: SHM freed id=");
    char buf[16];
    extern void k_itoa(int, char *);
    k_itoa(shm_segments[slot].id, buf);
    print_serial(buf);
    print_serial("\n");
  }

  return 0;
}

int shm_stat(int shmid, shm_info_t *info) {
  for (int i = 0; i < SHM_MAX_SEGMENTS; i++) {
    if (!shm_segments[i].is_free && shm_segments[i].id == shmid) {
      info->shmid = shm_segments[i].id;
      info->size = shm_segments[i].size;
      info->creator_pid = shm_segments[i].creator_pid;
      info->refcount = shm_segments[i].refcount;
      return 0;
    }
  }
  return -1;
}

// ==== Unix Domain Socket IPC ====
static ipc_pair_t ipc_pairs[IPC_MAX_PAIRS];

void ipc_init(void) {
  memset(ipc_pairs, 0, sizeof(ipc_pairs));
  print_serial("IPC: Unix domain sockets initialized.\n");
}

int ipc_socketpair(int fds[2]) {
  // Find a free pair
  int slot = -1;
  for (int i = 0; i < IPC_MAX_PAIRS; i++) {
    if (!ipc_pairs[i].used) {
      slot = i;
      break;
    }
  }
  if (slot < 0)
    return -1;

  ipc_pairs[slot].used = 1;
  ipc_pairs[slot].head = 0;
  ipc_pairs[slot].tail = 0;
  ipc_pairs[slot].read_open = 1;
  ipc_pairs[slot].write_open = 1;

  // Use a unique FD scheme: read_fd = pair index, write_fd = pair index + 1000
  ipc_pairs[slot].read_fd = slot;
  ipc_pairs[slot].write_fd = slot + 1000;

  fds[0] = ipc_pairs[slot].read_fd;
  fds[1] = ipc_pairs[slot].write_fd;

  return 0;
}

int ipc_send(int fd, const void *data, size_t len) {
  // Determine which pair this fd belongs to
  int slot = -1;
  if (fd >= 1000) {
    // Write end
    slot = fd - 1000;
  } else {
    // Read end (can also write)
    slot = fd;
  }

  if (slot < 0 || slot >= IPC_MAX_PAIRS || !ipc_pairs[slot].used)
    return -1;
  if (!ipc_pairs[slot].write_open)
    return -1;

  ipc_pair_t *pair = &ipc_pairs[slot];

  // Write to buffer
  size_t written = 0;
  while (written < len) {
    int next_head = (pair->head + 1) % IPC_BUFFER_SIZE;
    if (next_head == pair->tail) {
      // Buffer full
      break;
    }
    pair->buffer[pair->head] = ((const uint8_t *)data)[written];
    pair->head = next_head;
    written++;
  }

  return (int)written;
}

int ipc_recv(int fd, void *buf, size_t len) {
  // Determine which pair this fd belongs to
  int slot = -1;
  if (fd >= 1000) {
    slot = fd - 1000;
  } else {
    slot = fd;
  }

  if (slot < 0 || slot >= IPC_MAX_PAIRS || !ipc_pairs[slot].used)
    return -1;
  if (!ipc_pairs[slot].read_open)
    return 0;

  ipc_pair_t *pair = &ipc_pairs[slot];

  // Read from buffer
  size_t read = 0;
  while (read < len && pair->tail != pair->head) {
    ((uint8_t *)buf)[read] = pair->buffer[pair->tail];
    pair->tail = (pair->tail + 1) % IPC_BUFFER_SIZE;
    read++;
  }

  return (int)read;
}

// ==== Window Server Protocol ====
static wm_client_t wm_clients[WM_MAX_CLIENTS];
static wm_window_t wm_windows[WM_MAX_CLIENTS * WM_MAX_WINDOWS_PER_CLIENT];
int wm_mouse_x = 400;
int wm_mouse_y = 300;
int wm_mouse_buttons = 0;

void wm_init(void) {
  memset(wm_clients, 0, sizeof(wm_clients));
  memset(wm_windows, 0, sizeof(wm_windows));
  print_serial("IPC: Window server protocol initialized.\n");
}

int wm_connect(task_t *t) {
  for (int i = 0; i < WM_MAX_CLIENTS; i++) {
    if (!wm_clients[i].client_id && !wm_clients[i].task) {
      wm_clients[i].client_id = i + 1;
      wm_clients[i].task = t;
      wm_clients[i].window_count = 0;
      print_serial("IPC: WM client connected id=");
      char buf[16];
      extern void k_itoa(int, char *);
      k_itoa(wm_clients[i].client_id, buf);
      print_serial(buf);
      print_serial("\n");
      return wm_clients[i].client_id;
    }
  }
  return -1;
}

int wm_create_window(task_t *t, int w, int h) {
  // Find client
  wm_client_t *client = 0;
  for (int i = 0; i < WM_MAX_CLIENTS; i++) {
    if (wm_clients[i].task == t) {
      client = &wm_clients[i];
      break;
    }
  }
  if (!client)
    return -1;
  if (client->window_count >= WM_MAX_WINDOWS_PER_CLIENT)
    return -1;

  // Allocate shared memory for the framebuffer
  size_t fb_size = (size_t)(w * h * 4);
  int shmid = shm_create(fb_size);
  if (shmid < 0)
    return -1;

  // Attach to the client's address space
  uint64_t vaddr = shm_attach(t, shmid);
  if (!vaddr)
    return -1;

  // Find a free window slot
  for (int i = 0; i < WM_MAX_CLIENTS * WM_MAX_WINDOWS_PER_CLIENT; i++) {
    if (!wm_windows[i].active) {
      int win_id = i + 1;
      wm_windows[i].win_id = win_id;
      wm_windows[i].client_id = client->client_id;
      wm_windows[i].x = 100 + client->window_count * 30;
      wm_windows[i].y = 100 + client->window_count * 30;
      wm_windows[i].w = w;
      wm_windows[i].h = h;
      wm_windows[i].shm_vaddr = vaddr;
      wm_windows[i].shm_id = shmid;
      wm_windows[i].active = 1;

      client->window_ids[client->window_count++] = win_id;
      return win_id;
    }
  }
  return -1;
}

int wm_submit_frame(task_t *t, int win_id, void *fb) {
  (void)t;
  (void)fb;
  for (int i = 0; i < WM_MAX_CLIENTS * WM_MAX_WINDOWS_PER_CLIENT; i++) {
    if (wm_windows[i].active && wm_windows[i].win_id == win_id) {
      return 0;
    }
  }
  return -1;
}

int wm_get_event(task_t *t, wm_event_t *ev) {
  (void)t;
  if (!ev) return -1;
  // Check if there are pending messages for this task
  msg_t msg;
  while (msg_receive(&msg)) {
    if (msg.type == MSG_WINDOW_EVENT && msg.d1 >= 0) {
      ev->type = msg.d1;
      ev->d1 = msg.d2;
      ev->d2 = msg.d3;
      ev->d3 = msg.d4;
      return 1;
    }
  }
  return 0;
}

int wm_get_mouse(task_t *t, int out[3]) {
  (void)t;
  if (!out) return -1;
  out[0] = wm_mouse_x;
  out[1] = wm_mouse_y;
  out[2] = wm_mouse_buttons;
  return 0;
}

void wm_dispatch_input(int type, int d1, int d2, int d3) {
  wm_event_t ev;
  ev.type = type;
  ev.d1 = d1;
  ev.d2 = d2;
  ev.d3 = d3;

  // Send to all connected clients
  for (int i = 0; i < WM_MAX_CLIENTS; i++) {
    if (wm_clients[i].task) {
      msg_t msg;
      msg.type = MSG_WINDOW_EVENT;
      msg.d1 = ev.type;
      msg.d2 = ev.d1;
      msg.d3 = ev.d2;
      msg.d4 = ev.d3;
      msg.ptr = 0;
      msg_send(wm_clients[i].task->id, &msg);
    }
  }
}
