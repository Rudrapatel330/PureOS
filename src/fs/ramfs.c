#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "fs.h"
#include "vfs.h"

extern const unsigned char wallpaper_bmp_data[];
extern const unsigned int wallpaper_bmp_size;
extern const unsigned char wallpaper_png_data[];
extern const unsigned int wallpaper_png_size;

file_entry_t ramfs_files[FS_MAX_FILES];
int ramfs_count = 0;

void ramfs_add_file(const char *name, const char *content) {
  // Bounds check: ensure we don't exceed max files
  if (ramfs_count >= FS_MAX_FILES) {
    extern void print_serial(const char *);
    print_serial("ERROR: Filesystem full (max files reached)\n");
    return;
  }

  // Validate filename length (prevent buffer overflow)
  int name_len = 0;
  while (name[name_len] && name_len < 64)
    name_len++;
  if (name_len >= 32) { // MAX_FILENAME is typically 32
    extern void print_serial(const char *);
    print_serial("ERROR: Filename too long\n");
    return;
  }

  file_entry_t *f = &ramfs_files[ramfs_count++];
  strncpy(f->name, name, FS_MAX_FILENAME - 1);
  f->name[FS_MAX_FILENAME - 1] = 0;

  // Allocate memory for content
  int len = 0;
  while (content[len])
    len++;
  f->content = (const char *)kmalloc(len + 1);
  strncpy((char *)f->content, content, len);
  ((char *)f->content)[len] = 0;

  f->size = len;
  f->flags = 0;
}

void ramfs_add_binary_file(const char *name, const unsigned char *data,
                           unsigned int size) {
  if (ramfs_count >= FS_MAX_FILES)
    return;

  file_entry_t *f = &ramfs_files[ramfs_count++];
  strncpy(f->name, name, FS_MAX_FILENAME - 1);
  f->name[FS_MAX_FILENAME - 1] = 0;

  f->content = (const char *)kmalloc(size);
  if (!f->content) {
    ramfs_count--;
    return;
  }
  for (unsigned int i = 0; i < size; i++) {
    ((unsigned char *)f->content)[i] = data[i];
  }

  f->size = size;
  f->flags = 0;
}

int ramfs_mkdir(const char *name) {
  if (ramfs_count >= FS_MAX_FILES)
    return 0;

  // Check if exists
  if (fs_find(name))
    return 0;

  file_entry_t *f = &ramfs_files[ramfs_count++];
  strncpy(f->name, name, FS_MAX_FILENAME - 1);
  f->name[FS_MAX_FILENAME - 1] = 0;
  f->size = 0;
  f->content = 0;
  f->flags = 1; // 1 = Directory

  return 1;
}

int ramfs_write(const char *name, const uint8_t *content, uint32_t size) {
  // Validate inputs
  if (!name || !content)
    return 0;

  // Check filename length
  int name_len = 0;
  while (name[name_len] && name_len < 64)
    name_len++;
  if (name_len >= 32)
    return 0;

  file_entry_t *f = fs_find(name);
  if (f) {
    // CRITICAL FIX: Free old content to prevent memory leak
    if (f->content) {
      kfree((void *)f->content);
    }

    // Update existing file
    f->content = (const char *)kmalloc(size + 1);
    strncpy((char *)f->content, (const char *)content, size);
    ((char *)f->content)[size] = 0;
    f->size = size;
    return 1;
  } else {
    // Create new file - need to create a null-terminated string first
    char *content_str = (char *)kmalloc(size + 1);
    strncpy(content_str, (const char *)content, size);
    content_str[size] = 0;
    ramfs_add_file(name, content_str);
    kfree(content_str);
    return 1;
  }
  return 0;
}

// VFS Implementations for RAMFS
static int ramfs_vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size,
                          uint8_t *buffer) {
  if (node->impl >= (uint32_t)ramfs_count)
    return -1;
  file_entry_t *f = &ramfs_files[node->impl];
  if (!f->content)
    return 0;

  if (offset >= f->size)
    return 0;
  if (offset + size > f->size)
    size = f->size - offset;

  memcpy(buffer, f->content + offset, size);
  return size;
}

static vfs_node_t *ramfs_get_vfs_node(int index) {
  if (index < 0 || index >= ramfs_count)
    return 0;
  vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
  memset(node, 0, sizeof(vfs_node_t));

  file_entry_t *f = &ramfs_files[index];
  strcpy(node->name, f->name);
  node->length = f->size;
  node->flags = (f->flags == 1) ? VFS_DIRECTORY : VFS_FILE;
  node->impl = index;
  extern vfs_driver_t ramfs_vfs_driver;
  node->driver = &ramfs_vfs_driver;
  return node;
}

static vfs_node_t *ramfs_vfs_readdir(vfs_node_t *node, uint32_t index) {
  if (!(node->flags & VFS_DIRECTORY))
    return 0;
  if (index >= (uint32_t)ramfs_count)
    return 0;
  return ramfs_get_vfs_node(index);
}

static vfs_node_t *ramfs_vfs_finddir(vfs_node_t *node, char *name) {
  if (!(node->flags & VFS_DIRECTORY))
    return 0;
  for (int i = 0; i < ramfs_count; i++) {
    if (strcmp(ramfs_files[i].name, name) == 0) {
      return ramfs_get_vfs_node(i);
    }
  }
  return 0;
}

vfs_driver_t ramfs_vfs_driver = {"ramfs",
                                 ramfs_vfs_read,
                                 0, // write
                                 0, // open
                                 0, // close
                                 ramfs_vfs_readdir,
                                 ramfs_vfs_finddir};

int ramfs_init() {
  ramfs_count = 0;

  ramfs_add_file("README.TXT",
                 "Welcome to PureOS 32-bit!\nThis is a virtual filesystem.");
  ramfs_add_file("KERNEL.SRC", "void kernel_main() { ... }");
  ramfs_add_file("TODO.LIST", "- Fix Bugs\n- Add Shell\n- Sleep");
  ramfs_add_file("CREDITS", "Developed by User & Gemini.");
  ramfs_add_binary_file("WALL.BMP", wallpaper_bmp_data, wallpaper_bmp_size);
  ramfs_add_binary_file("WALL.PNG", wallpaper_png_data, wallpaper_png_size);

  // Create Root Node
  vfs_root = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
  memset(vfs_root, 0, sizeof(vfs_node_t));
  strcpy(vfs_root->name, "/");
  vfs_root->flags = VFS_DIRECTORY;
  vfs_root->driver = &ramfs_vfs_driver;

  return 1;
}

// Lists files into a buffer (New-line separated)
int ramfs_list(char *buffer, int max_len) {
  int pos = 0;
  buffer[0] = 0;

  for (int i = 0; i < ramfs_count; i++) {
    // Safe concat? Minimal check
    const char *name = ramfs_files[i].name;
    while (*name && pos < max_len - 2) {
      buffer[pos++] = *name++;
    }
    buffer[pos++] = '\n';
  }
  buffer[pos] = 0;
  return ramfs_count;
}

file_entry_t *fs_find(const char *name) {
  for (int i = 0; i < ramfs_count; i++) {
    if (strcmp(ramfs_files[i].name, name) == 0) {
      return &ramfs_files[i];
    }
  }
  return 0;
}

int ramfs_read(const char *name, uint8_t *buffer) {
  file_entry_t *file = fs_find(name);
  if (!file)
    return 0;

  const char *content = file->content;
  if (!content) {
    buffer[0] = 0;
    return 0;
  }
  int size = file->size;
  for (int i = 0; i < size; i++) {
    buffer[i] = (uint8_t)content[i];
  }
  return size;
}

int ramfs_list_files(FileInfo *buffer, int max_files) {
  if (!buffer || max_files <= 0)
    return 0;

  memset(buffer, 0, max_files * sizeof(FileInfo));

  int count = 0;
  for (int i = 0; i < ramfs_count && count < max_files; i++) {
    // Copy filename (truncate to 12 chars for FAT compatibility)
    int j = 0;
    while (ramfs_files[i].name[j] && j < 31) {
      buffer[count].name[j] = ramfs_files[i].name[j];
      j++;
    }
    buffer[count].name[j] = 0;

    buffer[count].size = ramfs_files[i].size;
    buffer[count].is_dir = ramfs_files[i].flags;
    count++;
  }
  return count;
}

int ramfs_delete(const char *name) {
  if (!name)
    return 0;

  int index = -1;
  for (int i = 0; i < ramfs_count; i++) {
    if (strcmp(ramfs_files[i].name, name) == 0) {
      index = i;
      break;
    }
  }

  if (index == -1)
    return 0; // Not found

  // Free content
  if (ramfs_files[index].content) {
    kfree((void *)ramfs_files[index].content);
  }

  // Shift remaining files
  for (int i = index; i < ramfs_count - 1; i++) {
    ramfs_files[i] = ramfs_files[i + 1];
  }

  ramfs_count--;
  return 1;
}
