#include "vfs.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/task.h"

extern void print_serial(const char *);

vfs_dentry_t *vfs_root = 0;
static file_handle_t *boot_fd_table[MAX_FD] = {0};

// ======================== CACHING LAYER (Sprint 4) ========================
#define DCACHE_SIZE 128
typedef struct {
  char name[128];
  vfs_dentry_t *parent;
  vfs_dentry_t *dentry;
  uint32_t valid;
  uint32_t last_used;
} dcache_entry_t;

static dcache_entry_t dcache[DCACHE_SIZE];
static uint32_t dcache_clock = 0;

void dcache_add(vfs_dentry_t *parent, vfs_dentry_t *dentry) {
  if (!parent || !dentry) return;
  dcache_clock++;
  for (int i = 0; i < DCACHE_SIZE; i++) {
    if (dcache[i].valid && dcache[i].parent == parent && strcmp(dcache[i].name, dentry->name) == 0) {
      dcache[i].dentry = dentry;
      dcache[i].last_used = dcache_clock;
      return;
    }
  }
  int lru = 0;
  uint32_t oldest = 0xFFFFFFFF;
  for (int i = 0; i < DCACHE_SIZE; i++) {
    if (!dcache[i].valid) { lru = i; break; }
    if (dcache[i].last_used < oldest) { oldest = dcache[i].last_used; lru = i; }
  }
  strcpy(dcache[lru].name, dentry->name);
  dcache[lru].parent = parent;
  
  // Keep an original copy in cache to prevent caller from freeing it
  vfs_dentry_t *cached_dentry = kmalloc(sizeof(vfs_dentry_t));
  memcpy(cached_dentry, dentry, sizeof(vfs_dentry_t));
  if (cached_dentry->inode) cached_dentry->inode->refcount++;
  
  dcache[lru].dentry = cached_dentry;
  dcache[lru].valid = 1;
  dcache[lru].last_used = dcache_clock;
}

vfs_dentry_t *dcache_lookup(vfs_dentry_t *parent, const char *name) {
  dcache_clock++;
  for (int i = 0; i < DCACHE_SIZE; i++) {
    if (dcache[i].valid && dcache[i].parent == parent && strcmp(dcache[i].name, name) == 0) {
      dcache[i].last_used = dcache_clock;
      
      // Clone dentry for caller to own (since vfs_resolve_path frees intermediate nodes)
      vfs_dentry_t *ref = kmalloc(sizeof(vfs_dentry_t));
      memcpy(ref, dcache[i].dentry, sizeof(vfs_dentry_t));
      ref->refcount = 1;
      if (ref->inode) ref->inode->refcount++;
      return ref;
    }
  }
  return 0;
}

#define ICACHE_SIZE 128
typedef struct {
  void *i_ops;
  uint32_t inode_no;
  vfs_inode_t *inode;
  uint32_t valid;
  uint32_t last_used;
} icache_entry_t;

static icache_entry_t icache[ICACHE_SIZE];
static uint32_t icache_clock = 0;

void icache_add(void *i_ops, vfs_inode_t *inode) {
  if (!i_ops || !inode) return;
  icache_clock++;
  for (int i = 0; i < ICACHE_SIZE; i++) {
    if (icache[i].valid && icache[i].i_ops == i_ops && icache[i].inode_no == inode->inode_no) {
      icache[i].inode = inode;
      icache[i].last_used = icache_clock;
      return;
    }
  }
  int lru = 0;
  uint32_t oldest = 0xFFFFFFFF;
  for (int i = 0; i < ICACHE_SIZE; i++) {
    if (!icache[i].valid) { lru = i; break; }
    if (icache[i].last_used < oldest) { oldest = icache[i].last_used; lru = i; }
  }
  icache[lru].i_ops = i_ops;
  icache[lru].inode_no = inode->inode_no;
  icache[lru].inode = inode;
  icache[lru].valid = 1;
  icache[lru].last_used = icache_clock;
}

vfs_inode_t *icache_lookup(void *i_ops, uint32_t inode_no) {
  icache_clock++;
  for (int i = 0; i < ICACHE_SIZE; i++) {
    if (icache[i].valid && icache[i].i_ops == i_ops && icache[i].inode_no == inode_no) {
      icache[i].last_used = icache_clock;
      return icache[i].inode;
    }
  }
  return 0;
}

static file_handle_t **get_current_fd_table() {
  task_t *curr_task = get_current_task();
  if (curr_task) {
    return (file_handle_t **)curr_task->files;
  }
  return boot_fd_table;
}

// Mount struct is defined in vfs.h
vfs_mount_t *mount_list = 0;

uint32_t vfs_get_time(void) {
  extern uint32_t get_timer_ticks(void);
  return get_timer_ticks() / 250; // 250Hz PIT -> seconds
}

void vfs_mount(const char *path, vfs_dentry_t *root) {
  vfs_mount_t *m = (vfs_mount_t *)kmalloc(sizeof(vfs_mount_t));
  strcpy(m->path, path);
  m->fstype[0] = 0; // Will be set by caller if needed
  m->root = root;
  m->next = mount_list;
  mount_list = m;
}

int vfs_get_mount_count(void) {
  int count = 0;
  vfs_mount_t *m = mount_list;
  while (m) { count++; m = m->next; }
  return count;
}

// Resolve parent directory and extract basename from path
// Returns parent dentry (caller must free), puts basename into basename_out
vfs_dentry_t *vfs_resolve_parent(const char *path, char *basename_out, int basename_maxlen) {
  if (!path || !basename_out) return 0;
  
  // Find last slash
  int last_slash = -1;
  for (int i = 0; path[i]; i++) {
    if (path[i] == '/') last_slash = i;
  }
  
  if (last_slash < 0) {
    // No slash — parent is cwd (treat as root for now)
    strncpy(basename_out, path, basename_maxlen - 1);
    basename_out[basename_maxlen - 1] = 0;
    return vfs_resolve_path("/");
  }
  
  // Extract basename
  strncpy(basename_out, path + last_slash + 1, basename_maxlen - 1);
  basename_out[basename_maxlen - 1] = 0;
  
  // Extract parent path
  char parent_path[256];
  if (last_slash == 0) {
    parent_path[0] = '/';
    parent_path[1] = 0;
  } else {
    strncpy(parent_path, path, last_slash);
    parent_path[last_slash] = 0;
  }
  
  return vfs_resolve_path(parent_path);
}

extern void devfs_init(void);
extern void procfs_init(void);

void vfs_init() {
  memset(boot_fd_table, 0, sizeof(boot_fd_table));
  devfs_init();
  procfs_init();
  print_serial("VFS INITIALIZED\n");
}

vfs_dentry_t *vfs_resolve_path(const char *path) {
  if (!vfs_root || !path)
    return 0;

  if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
    // Return a clone/reference to root
    vfs_dentry_t *root_ref = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
    memcpy(root_ref, vfs_root, sizeof(vfs_dentry_t));
    root_ref->refcount = 1;
    if (root_ref->inode) root_ref->inode->refcount++;
    return root_ref;
  }

  // Check absolute path
  char path_copy[256];
  strncpy(path_copy, path, 255);
  path_copy[255] = 0;

  // Check mount points first (simple longest prefix match would be better, but
  // for now just check exact match or subdirectory)
  vfs_mount_t *m = mount_list;
  vfs_dentry_t *current = vfs_root;
  char *remaining_path = path_copy;
  int longest_match = 0;
  vfs_mount_t *best_mount = 0;

  while (m) {
    int mlen = strlen(m->path);
    if (strncmp(path, m->path, mlen) == 0) {
      if (mlen == 1 && m->path[0] == '/') {
        if (mlen > longest_match) {
          longest_match = mlen;
          best_mount = m;
        }
      } else if (path[mlen] == '/' || path[mlen] == 0) {
        if (mlen > longest_match) {
          longest_match = mlen;
          best_mount = m;
        }
      }
    }
    m = m->next;
  }

  if (best_mount) {
    vfs_dentry_t *m_root = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
    memcpy(m_root, best_mount->root, sizeof(vfs_dentry_t));
    m_root->refcount = 1;
    if (m_root->inode) m_root->inode->refcount++;
    current = m_root;
    
    remaining_path = path_copy + longest_match;
    if (remaining_path[0] == '/')
      remaining_path++;
  } else {
    // Clone vfs_root as well so we can safely free current in the loop
    vfs_dentry_t *root_ref = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
    memcpy(root_ref, vfs_root, sizeof(vfs_dentry_t));
    root_ref->refcount = 1;
    if (root_ref->inode) root_ref->inode->refcount++;
    current = root_ref;
  }

  if (strlen(remaining_path) == 0) {
    return current;
  }

  char *saveptr = remaining_path;
  char *token = 0;
  
  while (*saveptr == '/') saveptr++;
  if (*saveptr) {
    token = saveptr;
    while (*saveptr && *saveptr != '/') saveptr++;
    if (*saveptr == '/') {
      *saveptr = 0;
      saveptr++;
    }
  }

  vfs_dentry_t *next = 0;

  while (token && *token) {
    if (!current->inode || !current->inode->i_ops || !current->inode->i_ops->lookup) {
      if (next)
        kfree(next);
      return 0;
    }

    next = dcache_lookup(current, token);
    if (!next) {
      next = current->inode->i_ops->lookup(current->inode, token);
      if (next) {
        dcache_add(current, next);
      }
    }

    // If we just resolved a part of the path, we should free the intermediate node 'current'
    // since we already found 'next'. current is guaranteed to be a temporary clone here.
    if (current->inode && current->inode->refcount > 0) current->inode->refcount--;
    kfree(current);

    if (!next)
      return 0;

    current = next;

    // Follow symlinks (up to 8 hops to prevent loops)
    int symlink_depth = 0;
    while (current->inode && (current->inode->mode & VFS_SYMLINK) && 
           current->inode->symlink_target && symlink_depth < 8) {
      vfs_dentry_t *resolved = vfs_resolve_path(current->inode->symlink_target);
      if (current->inode && current->inode->refcount > 0) current->inode->refcount--;
      kfree(current);
      if (!resolved) return 0;
      current = resolved;
      symlink_depth++;
    }

    while (*saveptr == '/') saveptr++;
    if (*saveptr == 0) {
      token = 0;
    } else {
      token = saveptr;
      while (*saveptr && *saveptr != '/') saveptr++;
      if (*saveptr == '/') {
        *saveptr = 0;
        saveptr++;
      }
    }
  }

  return current;
}

int inode_permission(vfs_inode_t *inode, int mask) {
  if (!inode) return 0;
  task_t *curr_task = get_current_task();
  uint32_t current_uid = curr_task ? curr_task->uid : 0;
  
  // Root bypasses all permissions
  if (current_uid == 0) return 1;
  
  int mode = inode->mode & 0777;
  
  if (inode->uid == current_uid) {
    mode >>= 6;
  } else if (inode->gid == (curr_task ? curr_task->gid : 0)) {
    mode >>= 3;
  }
  
  if ((mode & mask) == mask) return 1;
  return 0; // Permission denied
}

int vfs_open_node(vfs_dentry_t *dentry, int flags) {
  if (!dentry || !dentry->inode)
    return -1;

  // Permission check: flags O_RDONLY=0, O_WRONLY=1, O_RDWR=2
  int req_mask = 4; // default to read
  if (flags == 1) req_mask = 2; // write
  if (flags == 2) req_mask = 6; // read+write
  
  if (!inode_permission(dentry->inode, req_mask)) {
      extern void print_serial(const char*);
      print_serial("VFS: Permission denied\n");
      return -1;
  }

  file_handle_t **table = get_current_fd_table();

  // Find free FD
  for (int i = 0; i < MAX_FD; i++) {
    if (table[i] == 0) {
      file_handle_t *handle = (file_handle_t *)kmalloc(sizeof(file_handle_t));
      if (!handle) return -1;
      
      handle->dentry = dentry;
      handle->offset = 0;
      handle->flags = flags;
      handle->f_refcount = 1;
      table[i] = handle;
      
      if (dentry->inode->f_ops && dentry->inode->f_ops->open)
        dentry->inode->f_ops->open(dentry->inode, handle);
      return i;
    }
  }
  return -1;
}

int vfs_open(const char *path, int flags) {
  vfs_dentry_t *node = vfs_resolve_path(path);
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
  file_handle_t **table = get_current_fd_table();
  if (fd < 0 || fd >= MAX_FD || !table[fd])
    return -1;
  vfs_dentry_t *dentry = table[fd]->dentry;
  if (!dentry || !dentry->inode || !dentry->inode->f_ops || !dentry->inode->f_ops->read)
    return -1;

  int bytes = dentry->inode->f_ops->read(dentry->inode, table[fd], table[fd]->offset, size, buffer);
  if (bytes > 0) {
    table[fd]->offset += bytes;
  }
  return bytes;
}

int vfs_write(int fd, const uint8_t *buffer, uint32_t size) {
  file_handle_t **table = get_current_fd_table();
  if (fd < 0 || fd >= MAX_FD || !table[fd])
    return -1;
  vfs_dentry_t *dentry = table[fd]->dentry;
  if (!dentry || !dentry->inode || !dentry->inode->f_ops || !dentry->inode->f_ops->write)
    return -1;

  int bytes = dentry->inode->f_ops->write(dentry->inode, table[fd], table[fd]->offset, size, buffer);
  if (bytes > 0) {
    table[fd]->offset += bytes;
  }
  return bytes;
}

void vfs_close(int fd) {
  file_handle_t **table = get_current_fd_table();
  if (fd < 0 || fd >= MAX_FD || !table[fd])
    return;
  
  file_handle_t *handle = table[fd];
  table[fd] = 0;
  
  if (handle->f_refcount > 0) {
    handle->f_refcount--;
  }
  
  if (handle->f_refcount == 0) {
    if (handle->dentry && handle->dentry->inode && handle->dentry->inode->f_ops && handle->dentry->inode->f_ops->close) {
      handle->dentry->inode->f_ops->close(handle->dentry->inode, handle);
    }
    if (handle->dentry) {
      if (handle->dentry->inode && handle->dentry->inode->refcount > 0) handle->dentry->inode->refcount--;
      kfree(handle->dentry);
    }
    kfree(handle);
  }
}

uint32_t vfs_lseek(int fd, uint32_t offset, int whence) {
  file_handle_t **table = get_current_fd_table();
  if (fd < 0 || fd >= MAX_FD || !table[fd])
    return 0;

  // Basic lseek implementation
  if (whence == 0) { // SEEK_SET
    table[fd]->offset = offset;
  } else if (whence == 1) { // SEEK_CUR
    table[fd]->offset += offset;
  } else if (whence == 2) { // SEEK_END
    table[fd]->offset = table[fd]->dentry->inode->size + offset;
  }

  return table[fd]->offset;
}

vfs_dentry_t *vfs_readdir(int fd, uint32_t index) {
  file_handle_t **table = get_current_fd_table();
  if (fd < 0 || fd >= MAX_FD || !table[fd])
    return 0;
  vfs_dentry_t *dentry = table[fd]->dentry;
  if (!dentry || !dentry->inode || !dentry->inode->f_ops || !dentry->inode->f_ops->readdir)
    return 0;
  return dentry->inode->f_ops->readdir(dentry->inode, index);
}

void vfs_dup_fd_table(void **parent_files, void **child_files) {
  file_handle_t **p_table = (file_handle_t **)parent_files;
  file_handle_t **c_table = (file_handle_t **)child_files;
  for (int i = 0; i < MAX_FD; i++) {
    if (p_table[i]) {
      c_table[i] = p_table[i];
      c_table[i]->f_refcount++;
    } else {
      c_table[i] = 0;
    }
  }
}

void vfs_close_all_fds(void **files) {
  file_handle_t **table = (file_handle_t **)files;
  for (int i = 0; i < MAX_FD; i++) {
    if (table[i]) {
      file_handle_t *handle = table[i];
      table[i] = 0;
      if (handle->f_refcount > 0) {
        handle->f_refcount--;
      }
      if (handle->f_refcount == 0) {
        if (handle->dentry && handle->dentry->inode && handle->dentry->inode->f_ops && handle->dentry->inode->f_ops->close) {
          handle->dentry->inode->f_ops->close(handle->dentry->inode, handle);
        }
        if (handle->dentry) {
          if (handle->dentry->inode && handle->dentry->inode->refcount > 0) handle->dentry->inode->refcount--;
          kfree(handle->dentry);
        }
        kfree(handle);
      }
    }
  }
}

int vfs_dup2(int oldfd, int newfd) {
  file_handle_t **table = get_current_fd_table();
  if (oldfd < 0 || oldfd >= MAX_FD || !table[oldfd]) return -1;
  if (newfd < 0 || newfd >= MAX_FD) return -1;
  
  if (oldfd == newfd) return newfd;
  
  if (table[newfd]) {
    vfs_close(newfd);
  }
  
  table[newfd] = table[oldfd];
  table[newfd]->f_refcount++;
  return newfd;
}

int vfs_stat(const char *path, vfs_stat_t *st) {
  vfs_dentry_t *node = vfs_resolve_path(path);
  if (!node) return -1;
  
  memset(st, 0, sizeof(vfs_stat_t));
  
  // Try FS-specific stat first
  if (node->inode && node->inode->i_ops && node->inode->i_ops->stat) {
    int ret = node->inode->i_ops->stat(node->inode, st);
    if (node->inode && node->inode->refcount > 0) node->inode->refcount--;
    kfree(node);
    return ret;
  }
  
  // Fill from inode
  if (node->inode) {
    st->st_ino = node->inode->inode_no;
    st->st_mode = node->inode->mode;
    st->st_nlink = node->inode->nlink > 0 ? node->inode->nlink : 1;
    st->st_uid = node->inode->uid;
    st->st_gid = node->inode->gid;
    st->st_size = node->inode->size;
    st->st_blksize = 512;
    st->st_blocks = (node->inode->size + 511) / 512;
    st->st_atime = node->inode->atime;
    st->st_mtime = node->inode->mtime;
    st->st_ctime = node->inode->ctime;
    // Legacy compat
    st->size = node->inode->size;
    st->flags = node->inode->mode;
  }
  
  if (node->inode && node->inode->refcount > 0) node->inode->refcount--;
  kfree(node);
  return 0;
}

int vfs_fstat(int fd, vfs_stat_t *st) {
  file_handle_t **table = get_current_fd_table();
  if (fd < 0 || fd >= MAX_FD || !table[fd]) return -1;
  vfs_dentry_t *dentry = table[fd]->dentry;
  if (!dentry || !dentry->inode) return -1;
  
  memset(st, 0, sizeof(vfs_stat_t));
  vfs_inode_t *inode = dentry->inode;
  st->st_ino = inode->inode_no;
  st->st_mode = inode->mode;
  st->st_nlink = inode->nlink > 0 ? inode->nlink : 1;
  st->st_uid = inode->uid;
  st->st_gid = inode->gid;
  st->st_size = inode->size;
  st->st_blksize = 512;
  st->st_blocks = (inode->size + 511) / 512;
  st->st_atime = inode->atime;
  st->st_mtime = inode->mtime;
  st->st_ctime = inode->ctime;
  st->size = inode->size;
  st->flags = inode->mode;
  return 0;
}

int vfs_mkdir(const char *path) {
  // Try proper VFS dispatch via parent inode
  char basename[128];
  vfs_dentry_t *parent = vfs_resolve_parent(path, basename, sizeof(basename));
  
  if (parent && parent->inode && parent->inode->i_ops && parent->inode->i_ops->mkdir) {
    int ret = parent->inode->i_ops->mkdir(parent->inode, basename, S_IFDIR | 0755);
    if (parent->inode && parent->inode->refcount > 0) parent->inode->refcount--;
    kfree(parent);
    return ret;
  }
  if (parent) {
    if (parent->inode && parent->inode->refcount > 0) parent->inode->refcount--;
    kfree(parent);
  }
  
  // Fallback: direct FS dispatch for backward compat
  if (strncmp(path, "/ram/", 5) == 0 || strcmp(path, "/ram") == 0) {
    extern int ramfs_mkdir(const char *);
    const char *name = path;
    if (strncmp(path, "/ram/", 5) == 0) name = path + 5;
    return ramfs_mkdir(name);
  }
  extern int fat_mkdir(const char *);
  return fat_mkdir(path);
}

int vfs_unlink(const char *path) {
  // Permission check
  vfs_dentry_t *node = vfs_resolve_path(path);
  if (!node) return -1;
  
  if (node->inode && !inode_permission(node->inode, 2)) {
    if (node->inode->refcount > 0) node->inode->refcount--;
    kfree(node);
    print_serial("VFS: Permission denied on unlink\n");
    return -1;
  }
  if (node->inode && node->inode->refcount > 0) node->inode->refcount--;
  kfree(node);

  // Try proper VFS dispatch via parent inode
  char basename[128];
  vfs_dentry_t *parent = vfs_resolve_parent(path, basename, sizeof(basename));
  
  if (parent && parent->inode && parent->inode->i_ops && parent->inode->i_ops->unlink) {
    int ret = parent->inode->i_ops->unlink(parent->inode, basename);
    if (parent->inode && parent->inode->refcount > 0) parent->inode->refcount--;
    kfree(parent);
    return ret;
  }
  if (parent) {
    if (parent->inode && parent->inode->refcount > 0) parent->inode->refcount--;
    kfree(parent);
  }

  // Fallback: direct FS dispatch
  if (strncmp(path, "/ram/", 5) == 0 || strcmp(path, "/ram") == 0) {
    extern int ramfs_delete(const char *);
    const char *name = path;
    if (strncmp(path, "/ram/", 5) == 0) name = path + 5;
    return ramfs_delete(name);
  }
  extern int fat_delete_file(const char *);
  return fat_delete_file(path);
}

// Cross-filesystem copy via VFS read/write
int vfs_copy_file(const char *src, const char *dst) {
  // Try FAT fast-path first (same filesystem)
  extern int fat_copy_file(const char *src, const char *dst);
  
  // Check if both are on same FS (FAT root)
  int src_is_fat = (strncmp(src, "/ram/", 5) != 0 && strncmp(src, "/dev/", 5) != 0 && strncmp(src, "/proc/", 6) != 0);
  int dst_is_fat = (strncmp(dst, "/ram/", 5) != 0 && strncmp(dst, "/dev/", 5) != 0 && strncmp(dst, "/proc/", 6) != 0);
  
  if (src_is_fat && dst_is_fat) {
    return fat_copy_file(src, dst);
  }
  
  // Cross-filesystem: read source, write to destination
  vfs_stat_t st;
  if (vfs_stat(src, &st) < 0) return -1;
  
  uint32_t file_size = st.st_size;
  if (file_size == 0) file_size = st.size; // legacy compat
  
  uint8_t *buf = kmalloc(file_size + 1);
  if (!buf) return -1;
  
  int src_fd = vfs_open(src, O_RDONLY);
  if (src_fd < 0) { kfree(buf); return -1; }
  
  int bytes_read = vfs_read(src_fd, buf, file_size);
  vfs_close(src_fd);
  
  if (bytes_read <= 0) { kfree(buf); return -1; }
  
  // Write to destination
  extern int fs_write(const char *, const uint8_t *, uint32_t);
  int ret = fs_write(dst, buf, bytes_read) ? 0 : -1;
  kfree(buf);
  return ret;
}

int vfs_move_file(const char *src, const char *dst) {
  // Try FAT fast-path (atomic rename within same FS)
  int src_is_fat = (strncmp(src, "/ram/", 5) != 0 && strncmp(src, "/dev/", 5) != 0 && strncmp(src, "/proc/", 6) != 0);
  int dst_is_fat = (strncmp(dst, "/ram/", 5) != 0 && strncmp(dst, "/dev/", 5) != 0 && strncmp(dst, "/proc/", 6) != 0);
  
  if (src_is_fat && dst_is_fat) {
    extern int fat_move_file(const char *src, const char *dst);
    return fat_move_file(src, dst);
  }
  
  // Cross-filesystem: copy + delete
  if (vfs_copy_file(src, dst) == 0) {
    vfs_unlink(src);
    return 0;
  }
  return -1;
}

int vfs_rename(const char *oldpath, const char *newpath) {
  return vfs_move_file(oldpath, newpath);
}

int vfs_create_file(const char *path, uint32_t mode) {
  // Try proper VFS dispatch via parent inode
  char basename[128];
  vfs_dentry_t *parent = vfs_resolve_parent(path, basename, sizeof(basename));
  
  if (parent && parent->inode && parent->inode->i_ops && parent->inode->i_ops->create) {
    int ret = parent->inode->i_ops->create(parent->inode, basename, S_IFREG | (mode & 0777));
    if (parent->inode && parent->inode->refcount > 0) parent->inode->refcount--;
    kfree(parent);
    return ret;
  }
  if (parent) {
    if (parent->inode && parent->inode->refcount > 0) parent->inode->refcount--;
    kfree(parent);
  }
  
  // Fallback: write empty file
  extern int fs_write(const char *, const uint8_t *, uint32_t);
  uint8_t empty = 0;
  return fs_write(path, &empty, 0) ? 0 : -1;
}

int vfs_chmod(const char *path, uint32_t mode) {
  vfs_dentry_t *node = vfs_resolve_path(path);
  if (!node) return -1;
  
  task_t *curr_task = get_current_task();
  uint32_t current_uid = curr_task ? curr_task->uid : 0;
  
  if (node->inode) {
    // Only owner or root can chmod
    if (current_uid != 0 && current_uid != node->inode->uid) {
      if (node->inode->refcount > 0) node->inode->refcount--;
      kfree(node);
      return -1;
    }
    node->inode->mode = (node->inode->mode & ~0777) | (mode & 0777);
    node->inode->ctime = vfs_get_time();
  }
  
  if (node->inode && node->inode->refcount > 0) node->inode->refcount--;
  kfree(node);
  return 0;
}

int vfs_chown(const char *path, uint32_t uid, uint32_t gid) {
  vfs_dentry_t *node = vfs_resolve_path(path);
  if (!node) return -1;
  
  task_t *curr_task = get_current_task();
  uint32_t current_uid = curr_task ? curr_task->uid : 0;
  
  if (node->inode) {
    if (current_uid != 0) {
      if (node->inode->refcount > 0) node->inode->refcount--;
      kfree(node);
      return -1;
    }
    node->inode->uid = uid;
    node->inode->gid = gid;
    node->inode->ctime = vfs_get_time();
  }
  
  if (node->inode && node->inode->refcount > 0) node->inode->refcount--;
  kfree(node);
  return 0;
}


// ======================== SYMLINK SUPPORT ========================

int vfs_symlink(const char *target, const char *linkpath) {
  if (!target || !linkpath) return -1;
  
  // For now, symlinks are stored in ramfs only
  // Create a new inode+dentry for the symlink
  vfs_inode_t *inode = kmalloc(sizeof(vfs_inode_t));
  memset(inode, 0, sizeof(vfs_inode_t));
  inode->mode = VFS_SYMLINK | 0777;
  inode->size = strlen(target);
  inode->refcount = 1;
  
  // Allocate and copy the target path
  inode->symlink_target = kmalloc(strlen(target) + 1);
  strcpy(inode->symlink_target, target);
  
  // For now, store in ramfs if path starts with /ram/
  if (strncmp(linkpath, "/ram/", 5) == 0) {
    extern int ramfs_create_symlink(const char *name, vfs_inode_t *inode);
    return ramfs_create_symlink(linkpath + 5, inode);
  }
  
  // If we can't place it, free and fail
  kfree(inode->symlink_target);
  kfree(inode);
  return -1;
}

int vfs_readlink(const char *path, char *buf, uint32_t bufsiz) {
  vfs_dentry_t *node = vfs_resolve_path(path);
  if (!node) return -1;
  
  if (!node->inode || !(node->inode->mode & VFS_SYMLINK) || !node->inode->symlink_target) {
    if (node->inode && node->inode->refcount > 0) node->inode->refcount--;
    kfree(node);
    return -1;
  }
  
  uint32_t len = strlen(node->inode->symlink_target);
  if (len > bufsiz) len = bufsiz;
  memcpy(buf, node->inode->symlink_target, len);
  
  if (node->inode && node->inode->refcount > 0) node->inode->refcount--;
  kfree(node);
  return len;
}

// ======================== DEVICE REGISTRY ========================

static chrdev_entry_t chrdev_table[MAX_CHRDEV] = {{0}};

int register_chrdev(int major, const char *name, file_operations_t *fops) {
  if (major < 0 || major >= MAX_CHRDEV) return -1;
  if (chrdev_table[major].fops) return -1; // Already registered
  
  chrdev_table[major].name = name;
  chrdev_table[major].fops = fops;
  
  print_serial("CHRDEV: Registered device '");
  print_serial(name);
  print_serial("'\n");
  return 0;
}

file_operations_t *get_chrdev_fops(int major) {
  if (major < 0 || major >= MAX_CHRDEV) return 0;
  return chrdev_table[major].fops;
}
