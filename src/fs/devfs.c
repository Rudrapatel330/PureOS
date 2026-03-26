#include "devfs.h"
#include "../kernel/heap.h"
#include "../kernel/random.h"
#include "../kernel/string.h"
#include "../drivers/camera.h"

static vfs_driver_t devfs_driver;
static vfs_node_t *devfs_root = 0;
static vfs_node_t *devfs_nodes[4]; // null, zero, random, video0

static int devfs_read(vfs_node_t *node, uint32_t offset, uint32_t size,
                      uint8_t *buffer) {
  (void)offset; // Unused for these devices

  if (strcmp(node->name, "null") == 0) {
    return 0; // EOF immediately
  } else if (strcmp(node->name, "zero") == 0) {
    memset(buffer, 0, size);
    return size;
  } else if (strcmp(node->name, "random") == 0) {
    get_entropy(buffer, size);
    return size;
  } else if (strcmp(node->name, "video0") == 0) {
    camera_ctx_t *ctx = camera_get_ctx();
    if (!ctx || !ctx->is_active) return -1;
    
    uint32_t frame_size = ctx->width * ctx->height * 4;
    // For video devices, we often treat reads as frame-based or linear.
    // If offset is greater than frame size, wrap or return EOF.
    // Here we treat it as a linear "file" of one frame.
    if (offset >= frame_size) return 0;
    
    uint32_t to_read = size;
    if (offset + to_read > frame_size) to_read = frame_size - offset;
    
    memcpy(buffer, ((uint8_t *)ctx->frame_buffer) + offset, to_read);
    return to_read;
  }
  return -1;
}

static int devfs_write(vfs_node_t *node, uint32_t offset, uint32_t size,
                       const uint8_t *buffer) {
  (void)offset; // Unused
  (void)buffer; // Unused

  if (strcmp(node->name, "null") == 0) {
    return size; // Discard and say we wrote it all
  } else if (strcmp(node->name, "zero") == 0) {
    return size; // Discard and say we wrote it all
  } else if (strcmp(node->name, "random") == 0) {
    // Technically could mix into entropy pool, but for now just discard
    return size;
  }
  return -1;
}

static vfs_node_t *devfs_readdir(vfs_node_t *node, uint32_t index) {
  (void)node; // root
  if (index < 4) {
    return devfs_nodes[index];
  }
  return 0;
}

static vfs_node_t *devfs_finddir(vfs_node_t *node, char *name) {
  (void)node;
  for (int i = 0; i < 4; i++) {
    if (strcmp(devfs_nodes[i]->name, name) == 0) {
      vfs_node_t *ret = kmalloc(sizeof(vfs_node_t));
      memcpy(ret, devfs_nodes[i], sizeof(vfs_node_t));
      return ret;
    }
  }
  return 0;
}

static vfs_node_t *create_dev_node(const char *name) {
  vfs_node_t *n = kmalloc(sizeof(vfs_node_t));
  memset(n, 0, sizeof(vfs_node_t));
  strcpy(n->name, name);
  n->flags = VFS_FILE;
  n->mask = 0666; // Readable/writable by all
  n->driver = &devfs_driver;
  return n;
}

void devfs_init(void) {
  // Setup driver
  memset(&devfs_driver, 0, sizeof(vfs_driver_t));
  strcpy(devfs_driver.name, "devfs");
  devfs_driver.read = devfs_read;
  devfs_driver.write = devfs_write;
  devfs_driver.readdir = devfs_readdir;
  devfs_driver.finddir = devfs_finddir;

  // Create root node
  devfs_root = kmalloc(sizeof(vfs_node_t));
  memset(devfs_root, 0, sizeof(vfs_node_t));
  strcpy(devfs_root->name, "dev");
  devfs_root->flags = VFS_DIRECTORY;
  devfs_root->mask = 0755;
  devfs_root->driver = &devfs_driver;

  // Create device nodes
  devfs_nodes[0] = create_dev_node("null");
  devfs_nodes[1] = create_dev_node("zero");
  devfs_nodes[2] = create_dev_node("random");
  devfs_nodes[3] = create_dev_node("video0");

  // Mount at /dev
  vfs_mount("/dev", devfs_root);
}
