#include "devfs.h"
#include "../kernel/heap.h"
#include "../kernel/random.h"
#include "../kernel/string.h"
#include "../drivers/camera.h"

static vfs_dentry_t *devfs_root = 0;
static vfs_dentry_t *devfs_nodes[4]; // null, zero, random, video0

static int devfs_read(vfs_inode_t *inode, file_handle_t *file, uint32_t offset, uint32_t size,
                      uint8_t *buffer) {
  (void)offset; // Unused for these devices

  if (inode->impl == 0) { // null
    return 0; // EOF immediately
  } else if (inode->impl == 1) { // zero
    memset(buffer, 0, size);
    return size;
  } else if (inode->impl == 2) { // random
    get_entropy(buffer, size);
    return size;
  } else if (inode->impl == 3) { // video0
    camera_ctx_t *ctx = camera_get_ctx();
    if (!ctx || !ctx->is_active) return -1;
    
    uint32_t frame_size = ctx->width * ctx->height * 4;
    if (offset >= frame_size) return 0;
    
    uint32_t to_read = size;
    if (offset + to_read > frame_size) to_read = frame_size - offset;
    
    memcpy(buffer, ((uint8_t *)ctx->frame_buffer) + offset, to_read);
    return to_read;
  }
  return -1;
}

static int devfs_write(vfs_inode_t *inode, file_handle_t *file, uint32_t offset, uint32_t size,
                       const uint8_t *buffer) {
  (void)offset; // Unused
  (void)buffer; // Unused

  if (inode->impl == 0) {
    return size; // Discard and say we wrote it all
  } else if (inode->impl == 1) {
    return size; // Discard and say we wrote it all
  } else if (inode->impl == 2) {
    // Technically could mix into entropy pool, but for now just discard
    return size;
  }
  return -1;
}

static vfs_dentry_t *devfs_readdir(vfs_inode_t *inode, uint32_t index) {
  (void)inode; // root
  if (index < 4) {
    return devfs_nodes[index];
  }
  return 0;
}

static vfs_dentry_t *devfs_finddir(vfs_inode_t *inode, const char *name) {
  (void)inode;
  for (int i = 0; i < 4; i++) {
    if (strcmp(devfs_nodes[i]->name, name) == 0) {
      vfs_dentry_t *ret = kmalloc(sizeof(vfs_dentry_t));
      memcpy(ret, devfs_nodes[i], sizeof(vfs_dentry_t));
      ret->inode->refcount++;
      ret->refcount = 1;
      return ret;
    }
  }
  return 0;
}

extern file_operations_t devfs_file_ops;
extern inode_operations_t devfs_inode_ops;

static vfs_dentry_t *create_dev_node(const char *name, int impl) {
  vfs_inode_t *inode = kmalloc(sizeof(vfs_inode_t));
  memset(inode, 0, sizeof(vfs_inode_t));
  inode->mode = VFS_FILE | 0666;
  inode->impl = impl;
  inode->i_ops = &devfs_inode_ops;
  inode->f_ops = &devfs_file_ops;

  vfs_dentry_t *dentry = kmalloc(sizeof(vfs_dentry_t));
  memset(dentry, 0, sizeof(vfs_dentry_t));
  strcpy(dentry->name, name);
  dentry->inode = inode;
  return dentry;
}

file_operations_t devfs_file_ops = {
  devfs_read,
  devfs_write,
  0,
  0,
  devfs_readdir
};

inode_operations_t devfs_inode_ops = {
  devfs_finddir,
  0,
  0,
  0
};

void devfs_init(void) {
  // Create root inode
  vfs_inode_t *root_inode = kmalloc(sizeof(vfs_inode_t));
  memset(root_inode, 0, sizeof(vfs_inode_t));
  root_inode->mode = VFS_DIRECTORY | 0755;
  root_inode->i_ops = &devfs_inode_ops;
  root_inode->f_ops = &devfs_file_ops;

  // Create root dentry
  devfs_root = kmalloc(sizeof(vfs_dentry_t));
  memset(devfs_root, 0, sizeof(vfs_dentry_t));
  strcpy(devfs_root->name, "dev");
  devfs_root->inode = root_inode;

  // Create device nodes
  devfs_nodes[0] = create_dev_node("null", 0);
  devfs_nodes[1] = create_dev_node("zero", 1);
  devfs_nodes[2] = create_dev_node("random", 2);
  devfs_nodes[3] = create_dev_node("video0", 3);

  // Register in the character device registry
  extern int register_chrdev(int major, const char *name, file_operations_t *fops);
  register_chrdev(1, "null", &devfs_file_ops);
  register_chrdev(2, "zero", &devfs_file_ops);
  register_chrdev(3, "random", &devfs_file_ops);
  register_chrdev(4, "video0", &devfs_file_ops);

  // Mount at /dev
  vfs_mount("/dev", devfs_root);
}
