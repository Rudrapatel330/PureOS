#include "vfs.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"

extern void print_serial(const char *);

vfs_node_t *vfs_root = 0;
static file_handle_t fd_table[MAX_FD];

typedef struct vfs_mount {
  char path[64];
  vfs_node_t *root;
  struct vfs_mount *next;
} vfs_mount_t;

static vfs_mount_t *mount_list = 0;

void vfs_mount(const char *path, vfs_node_t *root) {
  vfs_mount_t *m = (vfs_mount_t *)kmalloc(sizeof(vfs_mount_t));
  strcpy(m->path, path);
  m->root = root;
  m->next = mount_list;
  mount_list = m;
}

extern void devfs_init(void);

void vfs_init() {
  memset(fd_table, 0, sizeof(fd_table));
  devfs_init();
  print_serial("VFS INITIALIZED\n");
}

vfs_node_t *vfs_resolve_path(const char *path) {
  if (!vfs_root || !path)
    return 0;

  if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
    // Return a clone/reference to root
    vfs_node_t *root_ref = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    memcpy(root_ref, vfs_root, sizeof(vfs_node_t));
    return root_ref;
  }

  // Check absolute path
  char path_copy[256];
  strncpy(path_copy, path, 255);
  path_copy[255] = 0;

  // Check mount points first (simple longest prefix match would be better, but
  // for now just check exact match or subdirectory)
  vfs_mount_t *m = mount_list;
  vfs_node_t *current = vfs_root;
  char *remaining_path = path_copy;

  while (m) {
    int mlen = strlen(m->path);
    if (strncmp(path, m->path, mlen) == 0) {
      if (path[mlen] == '/' || path[mlen] == 0) {
        current = m->root;
        remaining_path = path_copy + mlen;
        if (remaining_path[0] == '/')
          remaining_path++;
        break;
      }
    }
    m = m->next;
  }

  if (strlen(remaining_path) == 0) {
    vfs_node_t *ref = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    memcpy(ref, current, sizeof(vfs_node_t));
    return ref;
  }

  char *token = strtok(remaining_path, "/");
  vfs_node_t *next = 0;

  while (token) {
    if (!current->driver || !current->driver->finddir) {
      if (next)
        kfree(next);
      return 0;
    }

    next = current->driver->finddir(current, token);

    // If we just resolved a part of the path, and it's not the first part,
    // we should free the intermediate node 'current' IF it's not the global
    // vfs_root.
    if (current != vfs_root) {
      kfree(current);
    }

    if (!next)
      return 0;

    current = next;
    token = strtok(0, "/");
  }

  return current;
}

int vfs_open_node(vfs_node_t *node, int flags) {
  if (!node)
    return -1;

  // Permission check
  extern void *get_current_task(void);
  void *curr_task = get_current_task();
  uint32_t current_uid = 0; // Default to root
  uint32_t current_gid = 0;

  if (curr_task) {
    // Cast to task_t to read uid/gid
    // (Assuming task_t has uid at specific offset, will include task.h)
    uint32_t *t = (uint32_t *)curr_task;
    // We'll define uid/gid at the end of task_t to be safe, but actually let's
    // just call a helper we can add in task.c later: `extern uint32_t
    // task_get_uid();`
  }

  // Find free FD
  for (int i = 0; i < MAX_FD; i++) {
    if (fd_table[i].node == 0) {
      fd_table[i].node = node;
      fd_table[i].offset = 0;
      fd_table[i].flags = flags;
      if (node->driver->open)
        node->driver->open(node);
      return i;
    }
  }
  return -1;
}

int vfs_open(const char *path, int flags) {
  vfs_node_t *node = vfs_resolve_path(path);
  if (!node)
    return -1;

  int fd = vfs_open_node(node, flags);
  if (fd < 0) {
    // If we resolved it but couldn't open it, we should theoretically free
    // the duplicated node if it was a ref, but right now vfs_resolve_path
    // always returns a dynamically allocated node or ref.
    kfree(node);
  }
  return fd;
}

int vfs_read(int fd, uint8_t *buffer, uint32_t size) {
  if (fd < 0 || fd >= MAX_FD || !fd_table[fd].node)
    return -1;
  vfs_node_t *node = fd_table[fd].node;
  if (!node->driver->read)
    return -1;

  int bytes = node->driver->read(node, fd_table[fd].offset, size, buffer);
  if (bytes > 0) {
    fd_table[fd].offset += bytes;
  }
  return bytes;
}

int vfs_write(int fd, const uint8_t *buffer, uint32_t size) {
  if (fd < 0 || fd >= MAX_FD || !fd_table[fd].node)
    return -1;
  vfs_node_t *node = fd_table[fd].node;
  if (!node->driver->write)
    return -1;

  int bytes = node->driver->write(node, fd_table[fd].offset, size, buffer);
  if (bytes > 0) {
    fd_table[fd].offset += bytes;
  }
  return bytes;
}

void vfs_close(int fd) {
  if (fd < 0 || fd >= MAX_FD || !fd_table[fd].node)
    return;
  if (fd_table[fd].node->driver->close) {
    fd_table[fd].node->driver->close(fd_table[fd].node);
  }
  fd_table[fd].node = 0;
}

uint32_t vfs_lseek(int fd, uint32_t offset, int whence) {
  if (fd < 0 || fd >= MAX_FD || !fd_table[fd].node)
    return 0;

  // Basic lseek implementation
  if (whence == 0) { // SEEK_SET
    fd_table[fd].offset = offset;
  } else if (whence == 1) { // SEEK_CUR
    fd_table[fd].offset += offset;
  } else if (whence == 2) { // SEEK_END
    fd_table[fd].offset = fd_table[fd].node->length + offset;
  }

  return fd_table[fd].offset;
}

vfs_node_t *vfs_readdir(int fd, uint32_t index) {
  if (fd < 0 || fd >= MAX_FD || !fd_table[fd].node)
    return 0;
  vfs_node_t *node = fd_table[fd].node;
  if (!node->driver->readdir)
    return 0;
  return node->driver->readdir(node, index);
}
