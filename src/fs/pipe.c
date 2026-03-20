#include "pipe.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/task.h"
#include "vfs.h"

#define PIPE_SIZE 4096

typedef struct {
  uint8_t buffer[PIPE_SIZE];
  uint32_t head;
  uint32_t tail;
  int read_closed;
  int write_closed;
  // We would need wait queues here, but for now we poll/yield
} pipe_state_t;

static int pipe_read(vfs_node_t *node, uint32_t offset, uint32_t size,
                     uint8_t *buffer) {
  (void)offset;
  pipe_state_t *p = (pipe_state_t *)node->impl;

  int read = 0;
  while (read < (int)size) {
    if (p->head == p->tail) {
      // Empty
      if (p->write_closed) {
        break; // EOF
      }
      // Block/Yield if empty but writers exist
      __asm__ volatile("int $32");
      continue;
    }

    buffer[read++] = p->buffer[p->tail];
    p->tail = (p->tail + 1) % PIPE_SIZE;
  }

  return read;
}

static int pipe_write(vfs_node_t *node, uint32_t offset, uint32_t size,
                      const uint8_t *buffer) {
  (void)offset;
  pipe_state_t *p = (pipe_state_t *)node->impl;

  int written = 0;
  while (written < (int)size) {
    if (p->read_closed) {
      // Broken pipe (should signal SIGPIPE, returning error for now)
      return -1;
    }

    uint32_t next_head = (p->head + 1) % PIPE_SIZE;
    if (next_head == p->tail) {
      // Full, yield
      __asm__ volatile("int $32");
      continue;
    }

    p->buffer[p->head] = buffer[written++];
    p->head = next_head;
  }

  return written;
}

static void pipe_close(vfs_node_t *node) {
  pipe_state_t *p = (pipe_state_t *)node->impl;
  if (node->flags & 0x10) { // arbitrary flag for read end
    p->read_closed = 1;
  } else {
    p->write_closed = 1;
  }

  // If both closed, free the state
  if (p->read_closed && p->write_closed) {
    kfree(p);
  }
}

static vfs_driver_t pipe_driver = {"pipe",     pipe_read, pipe_write, 0,
                                   pipe_close, 0,         0};

// Returns 0 on success, fills fds[0] (read) and fds[1] (write)
int pipe(int fds[2]) {
  pipe_state_t *state = kmalloc(sizeof(pipe_state_t));
  memset(state, 0, sizeof(pipe_state_t));

  // Create Read Node
  vfs_node_t *r_node = kmalloc(sizeof(vfs_node_t));
  memset(r_node, 0, sizeof(vfs_node_t));
  strcpy(r_node->name, "pipe_read");
  r_node->flags = VFS_FILE | 0x10; // 0x10 custom flag for read end
  r_node->impl = (uint32_t)state;
  r_node->driver = &pipe_driver;

  // Create Write Node
  vfs_node_t *w_node = kmalloc(sizeof(vfs_node_t));
  memset(w_node, 0, sizeof(vfs_node_t));
  strcpy(w_node->name, "pipe_write");
  w_node->flags = VFS_FILE;
  w_node->impl = (uint32_t)state;
  w_node->driver = &pipe_driver;

  // Open Read End
  int fd0 = -1, fd1 = -1;
  // We bypass vfs_open since these nodes aren't linked in the standard VFS tree
  // We add them directly to the file descriptor table
  extern int vfs_open_node(vfs_node_t * node, int flags);
  fd0 = vfs_open_node(r_node, 0); // O_RDONLY
  fd1 = vfs_open_node(w_node, 1); // O_WRONLY

  if (fd0 < 0 || fd1 < 0) {
    // Cleanup on failure
    if (fd0 >= 0)
      vfs_close(fd0);
    if (fd1 >= 0)
      vfs_close(fd1);
    kfree(state);
    kfree(r_node);
    kfree(w_node);
    return -1;
  }

  fds[0] = fd0;
  fds[1] = fd1;
  return 0;
}
