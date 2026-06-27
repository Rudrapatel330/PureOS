#ifndef VFS_H
#define VFS_H

#include "../kernel/types.h"
#include <stddef.h>

struct vfs_inode;
struct vfs_dentry;
struct file_handle;

// ======================== POSIX-STYLE STAT ========================
typedef struct {
  uint32_t st_ino;     // Inode number
  uint32_t st_mode;    // File mode (permissions + type)
  uint32_t st_nlink;   // Number of hard links
  uint32_t st_uid;     // Owner UID
  uint32_t st_gid;     // Owner GID
  uint32_t st_size;    // File size in bytes
  uint32_t st_blksize; // Block size for filesystem I/O
  uint32_t st_blocks;  // Number of 512B blocks allocated
  uint32_t st_atime;   // Last access time
  uint32_t st_mtime;   // Last modification time
  uint32_t st_ctime;   // Last status change time
  uint32_t st_dev;     // Device ID
  // Legacy compat fields
  uint32_t size;       // Alias for st_size
  uint32_t flags;      // Alias for st_mode (legacy)
} vfs_stat_t;

// ======================== FILE TYPE BITS (S_IF*) ========================
#define S_IFMT   0170000  // File type mask
#define S_IFREG  0100000  // Regular file
#define S_IFDIR  0040000  // Directory
#define S_IFLNK  0120000  // Symbolic link
#define S_IFCHR  0020000  // Character device
#define S_IFBLK  0060000  // Block device
#define S_IFIFO  0010000  // FIFO/pipe

// Permission bits
#define S_IRWXU  0700   // Owner: rwx
#define S_IRUSR  0400   // Owner: read
#define S_IWUSR  0200   // Owner: write
#define S_IXUSR  0100   // Owner: execute
#define S_IRWXG  0070   // Group: rwx
#define S_IRGRP  0040   // Group: read
#define S_IWGRP  0020   // Group: write
#define S_IXGRP  0010   // Group: execute
#define S_IRWXO  0007   // Others: rwx
#define S_IROTH  0004   // Others: read
#define S_IWOTH  0002   // Others: write
#define S_IXOTH  0001   // Others: execute

#define S_ISUID  04000  // Set-user-ID
#define S_ISGID  02000  // Set-group-ID
#define S_ISVTX  01000  // Sticky bit

// Type test macros
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)

// ======================== OPERATIONS ========================
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
  int (*create)(struct vfs_inode *dir, const char *name, uint32_t mode);
  int (*rename)(struct vfs_inode *old_dir, const char *old_name,
                struct vfs_inode *new_dir, const char *new_name);
} inode_operations_t;

// ======================== CORE STRUCTURES ========================
typedef struct vfs_inode {
  uint32_t inode_no;
  uint32_t mode;   // Permissions and type (S_IF* | permission bits)
  uint32_t uid;
  uint32_t gid;
  uint32_t size;
  uint32_t refcount;
  uint32_t nlink;     // Hard link count
  uint32_t impl;      // Implementation-defined number
  void *fs_data;      // FS-specific data
  char *symlink_target; // Target path for symlinks (NULL if not symlink)
  uint32_t atime;     // Last access time (seconds since boot)
  uint32_t mtime;     // Last modification time
  uint32_t ctime;     // Last status change time
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

// ======================== VFS API ========================
void vfs_init(void);
void vfs_mount(const char *path, vfs_dentry_t *root);
vfs_dentry_t *vfs_resolve_path(const char *path);
vfs_dentry_t *vfs_readdir(int fd, uint32_t index);

// Caching Interface
void icache_add(void *i_ops, vfs_inode_t *inode);
vfs_inode_t *icache_lookup(void *i_ops, uint32_t inode_no);

// ======================== OPEN FLAGS ========================
#define O_RDONLY   0x0000
#define O_WRONLY   0x0001
#define O_RDWR     0x0002
#define O_ACCMODE  0x0003  // Mask for access mode
#define O_CREAT    0x0040  // Create file if it doesn't exist
#define O_TRUNC    0x0200  // Truncate to zero length
#define O_APPEND   0x0400  // Append mode

// Seek whence
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// ======================== FILE DESCRIPTOR API ========================
int vfs_open(const char *path, int flags);
int vfs_open_node(vfs_dentry_t *dentry, int flags);
int vfs_read(int fd, uint8_t *buffer, uint32_t size);
int vfs_write(int fd, const uint8_t *buffer, uint32_t size);
void vfs_close(int fd);
uint32_t vfs_lseek(int fd, uint32_t offset, int whence);
void vfs_dup_fd_table(void **parent_files, void **child_files);
void vfs_close_all_fds(void **files);
int vfs_dup2(int oldfd, int newfd);

// ======================== PATH-BASED API ========================
int vfs_stat(const char *path, vfs_stat_t *st);
int vfs_fstat(int fd, vfs_stat_t *st);
int vfs_mkdir(const char *path);
int vfs_unlink(const char *path);
int vfs_rename(const char *oldpath, const char *newpath);
int vfs_copy_file(const char *src, const char *dst);
int vfs_move_file(const char *src, const char *dst);
int vfs_chmod(const char *path, uint32_t mode);
int vfs_chown(const char *path, uint32_t uid, uint32_t gid);
int inode_permission(vfs_inode_t *inode, int mask);
int vfs_create_file(const char *path, uint32_t mode);

// Symlink API
int vfs_symlink(const char *target, const char *linkpath);
int vfs_readlink(const char *path, char *buf, uint32_t bufsiz);

// Mount info
typedef struct vfs_mount {
  char path[64];
  char fstype[16];
  vfs_dentry_t *root;
  struct vfs_mount *next;
} vfs_mount_t;

extern vfs_mount_t *mount_list;
int vfs_get_mount_count(void);

// ======================== DEVICE REGISTRY ========================
#define MAX_CHRDEV 32
typedef struct {
  const char *name;
  file_operations_t *fops;
} chrdev_entry_t;

int register_chrdev(int major, const char *name, file_operations_t *fops);
file_operations_t *get_chrdev_fops(int major);

// ======================== NODE TYPE FLAGS (Legacy compat) ========================
#define VFS_FILE       0x10000
#define VFS_DIRECTORY  0x20000
#define VFS_MOUNTPOINT 0x40000
#define VFS_SYMLINK    0x80000

#define MAX_FD 64

// Helper: get current time (seconds since boot)
uint32_t vfs_get_time(void);

// Helper: resolve parent directory and extract basename
vfs_dentry_t *vfs_resolve_parent(const char *path, char *basename_out, int basename_maxlen);

#endif
