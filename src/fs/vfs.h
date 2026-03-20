#ifndef VFS_H
#define VFS_H

#include "../kernel/types.h"
#include <stddef.h>

struct vfs_node;

// VFS Driver Interface
typedef struct {
  char name[16];
  int (*read)(struct vfs_node *node, uint32_t offset, uint32_t size,
              uint8_t *buffer);
  int (*write)(struct vfs_node *node, uint32_t offset, uint32_t size,
               const uint8_t *buffer);
  void (*open)(struct vfs_node *node);
  void (*close)(struct vfs_node *node);
  struct vfs_node *(*readdir)(struct vfs_node *node, uint32_t index);
  struct vfs_node *(*finddir)(struct vfs_node *node, char *name);
} vfs_driver_t;

// VFS Node (In-memory representation of a file/dir)
typedef struct vfs_node {
  char name[128];
  uint32_t mask;   // Permissions
  uint32_t uid;    // User ID
  uint32_t gid;    // Group ID
  uint32_t flags;  // 1=File, 2=Directory, 4=Mountpoint, etc.
  uint32_t inode;  // Driver-specific identifier
  uint32_t length; // Size in bytes
  uint32_t impl;   // Implementation-defined number
  vfs_driver_t *driver;
  struct vfs_node
      *ptr; // Used by mountpoints to point to the root of the mounted FS
} vfs_node_t;

// File Handle (Stateful open file)
typedef struct {
  vfs_node_t *node;
  uint32_t offset;
  uint32_t flags;
} file_handle_t;

// Global Root
extern vfs_node_t *vfs_root;

// VFS API
void vfs_init();
void vfs_mount(const char *path, vfs_node_t *root);
vfs_node_t *vfs_resolve_path(const char *path);
vfs_node_t *vfs_readdir(int fd, uint32_t index);

// File Descriptor Interface
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2

int vfs_open(const char *path, int flags);
int vfs_open_node(vfs_node_t *node, int flags);
int vfs_read(int fd, uint8_t *buffer, uint32_t size);
int vfs_write(int fd, const uint8_t *buffer, uint32_t size);
void vfs_close(int fd);
uint32_t vfs_lseek(int fd, uint32_t offset, int whence);

#define VFS_FILE 0x01
#define VFS_DIRECTORY 0x02
#define VFS_MOUNTPOINT 0x04

#define MAX_FD 32

#endif
