#ifndef VFS_H
#define VFS_H

#include "../kernel/types.h"
#include <stddef.h>

struct vfs_inode;
struct vfs_dentry;
struct file_handle;

typedef struct {
  uint32_t size;
  uint32_t flags;
} vfs_stat_t;

typedef struct {
  int (*read)(struct vfs_inode *inode, struct file_handle *file, uint32_t offset, uint32_t size, uint8_t *buffer);
  int (*write)(struct vfs_inode *inode, struct file_handle *file, uint32_t offset, uint32_t size, const uint8_t *buffer);
  int (*open)(struct vfs_inode *inode, struct file_handle *file);
  int (*close)(struct vfs_inode *inode, struct file_handle *file);
  struct vfs_dentry *(*readdir)(struct vfs_inode *inode, uint32_t index);
} file_operations_t;

typedef struct {
  struct vfs_dentry *(*lookup)(struct vfs_inode *dir, const char *name);
  int (*mkdir)(struct vfs_inode *dir, const char *name, uint32_t mode);
  int (*unlink)(struct vfs_inode *dir, const char *name);
  int (*stat)(struct vfs_inode *inode, vfs_stat_t *st);
} inode_operations_t;

typedef struct vfs_inode {
  uint32_t inode_no;
  uint32_t mode;   // Permissions and flags (VFS_FILE, VFS_DIRECTORY, VFS_SYMLINK)
  uint32_t uid;
  uint32_t gid;
  uint32_t size;
  uint32_t refcount;
  uint32_t impl;   // Implementation-defined number
  void *fs_data;   // FS-specific data
  char *symlink_target; // Target path for symlinks (NULL if not symlink)
  inode_operations_t *i_ops;
  file_operations_t *f_ops;
} vfs_inode_t;

typedef struct vfs_dentry {
  char name[128];
  vfs_inode_t *inode;
  struct vfs_dentry *parent;
  struct vfs_dentry *next;
  struct vfs_dentry *child;
  uint32_t refcount;
  uint32_t flags; // VFS_MOUNTPOINT
  struct vfs_dentry *mount_root;
} vfs_dentry_t;

// File Handle (Stateful open file)
typedef struct file_handle {
  vfs_dentry_t *dentry;
  uint32_t offset;
  uint32_t flags;
  uint32_t f_refcount;
} file_handle_t;

// Global Root
extern vfs_dentry_t *vfs_root;

// VFS API
void vfs_init();
void vfs_mount(const char *path, vfs_dentry_t *root);
vfs_dentry_t *vfs_resolve_path(const char *path);
vfs_dentry_t *vfs_readdir(int fd, uint32_t index);

// Caching Interface
void icache_add(void *i_ops, vfs_inode_t *inode);
vfs_inode_t *icache_lookup(void *i_ops, uint32_t inode_no);

// File Descriptor Interface
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2

int vfs_open(const char *path, int flags);
int vfs_open_node(vfs_dentry_t *dentry, int flags);
int vfs_read(int fd, uint8_t *buffer, uint32_t size);
int vfs_write(int fd, const uint8_t *buffer, uint32_t size);
void vfs_close(int fd);
uint32_t vfs_lseek(int fd, uint32_t offset, int whence);
void vfs_dup_fd_table(void **parent_files, void **child_files);
void vfs_close_all_fds(void **files);
int vfs_dup2(int oldfd, int newfd);

int vfs_stat(const char *path, vfs_stat_t *st);
int vfs_mkdir(const char *path);
int vfs_unlink(const char *path);
int vfs_copy_file(const char *src, const char *dst);
int vfs_move_file(const char *src, const char *dst);
int vfs_chmod(const char *path, uint32_t mode);
int vfs_chown(const char *path, uint32_t uid, uint32_t gid);
int inode_permission(vfs_inode_t *inode, int mask);

// Symlink API
int vfs_symlink(const char *target, const char *linkpath);
int vfs_readlink(const char *path, char *buf, uint32_t bufsiz);

// Device Registry
#define MAX_CHRDEV 32
typedef struct {
  const char *name;
  file_operations_t *fops;
} chrdev_entry_t;

int register_chrdev(int major, const char *name, file_operations_t *fops);
file_operations_t *get_chrdev_fops(int major);

#define VFS_FILE 0x01
#define VFS_DIRECTORY 0x02
#define VFS_MOUNTPOINT 0x04
#define VFS_SYMLINK 0x08

#define MAX_FD 64

#endif
