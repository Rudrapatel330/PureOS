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
static int ramfs_vfs_read(vfs_inode_t *inode, file_handle_t *file, uint32_t offset, uint32_t size,
                          uint8_t *buffer) {
  if (inode->impl >= (uint32_t)ramfs_count)
    return -1;
  file_entry_t *f = &ramfs_files[inode->impl];
  if (!f->content)
    return 0;

  if (offset >= f->size)
    return 0;
  if (offset + size > f->size)
    size = f->size - offset;

  memcpy(buffer, f->content + offset, size);
  return size;
}

static vfs_dentry_t *ramfs_get_vfs_node(int index) {
  if (index < 0 || index >= ramfs_count)
    return 0;
  vfs_inode_t *inode = (vfs_inode_t *)kmalloc(sizeof(vfs_inode_t));
  memset(inode, 0, sizeof(vfs_inode_t));
  vfs_dentry_t *dentry = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
  memset(dentry, 0, sizeof(vfs_dentry_t));

  file_entry_t *f = &ramfs_files[index];
  strcpy(dentry->name, f->name);
  inode->size = f->size;
  if (f->flags == 2) {
    // Symlink: content is target path
    inode->mode = VFS_SYMLINK | 0777;
    if (f->content) {
      inode->symlink_target = kmalloc(f->size + 1);
      memcpy(inode->symlink_target, f->content, f->size);
      inode->symlink_target[f->size] = 0;
    }
  } else {
    inode->mode = (f->flags == 1) ? (VFS_DIRECTORY | 0555) : (VFS_FILE | 0444);
  }
  inode->impl = index;
  extern file_operations_t ramfs_file_ops;
  extern inode_operations_t ramfs_inode_ops;
  inode->i_ops = &ramfs_inode_ops;
  inode->f_ops = &ramfs_file_ops;
  dentry->inode = inode;
  return dentry;
}

// Create a symlink in ramfs (flags=2 means symlink, content=target path)
int ramfs_create_symlink(const char *name, vfs_inode_t *link_inode) {
  if (ramfs_count >= FS_MAX_FILES) return -1;
  if (!link_inode || !link_inode->symlink_target) return -1;

  file_entry_t *f = &ramfs_files[ramfs_count++];
  strncpy(f->name, name, FS_MAX_FILENAME - 1);
  f->name[FS_MAX_FILENAME - 1] = 0;

  uint32_t tlen = strlen(link_inode->symlink_target);
  f->content = (const char *)kmalloc(tlen + 1);
  memcpy((char *)f->content, link_inode->symlink_target, tlen);
  ((char *)f->content)[tlen] = 0;
  f->size = tlen;
  f->flags = 2; // 2 = symlink

  // Free the caller's inode (we stored the data ourselves)
  kfree(link_inode->symlink_target);
  kfree(link_inode);
  return 0;
}

static vfs_dentry_t *ramfs_vfs_readdir(vfs_inode_t *inode, uint32_t index) {
  if (!(inode->mode & VFS_DIRECTORY))
    return 0;
  if (index >= (uint32_t)ramfs_count)
    return 0;
  return ramfs_get_vfs_node(index);
}

static vfs_dentry_t *ramfs_vfs_finddir(vfs_inode_t *dir, const char *name) {
  if (!(dir->mode & VFS_DIRECTORY))
    return 0;
  for (int i = 0; i < ramfs_count; i++) {
    if (strcmp(ramfs_files[i].name, name) == 0) {
      return ramfs_get_vfs_node(i);
    }
  }
  return 0;
}

file_operations_t ramfs_file_ops = {
  ramfs_vfs_read,
  0,
  0,
  0,
  ramfs_vfs_readdir
};

inode_operations_t ramfs_inode_ops = {
  ramfs_vfs_finddir,
  0,
  0,
  0,
  0,  // create
  0   // rename
};

int ramfs_init() {
  ramfs_count = 0;

  ramfs_add_file("README.TXT",
                 "Welcome to PureOS 32-bit!\nThis is a virtual filesystem.");
  ramfs_add_file("KERNEL.SRC", "void kernel_main() { ... }");
  ramfs_add_file("TODO.LIST", "- Fix Bugs\n- Add Shell\n- Sleep");
  ramfs_add_file("CREDITS", "Developed by User & Gemini.");
  ramfs_add_binary_file("WALL.BMP", wallpaper_bmp_data, wallpaper_bmp_size);
  ramfs_add_binary_file("WALL.PNG", wallpaper_png_data, wallpaper_png_size);

  // Create Root Node for ramfs
  vfs_inode_t *ram_inode = (vfs_inode_t *)kmalloc(sizeof(vfs_inode_t));
  memset(ram_inode, 0, sizeof(vfs_inode_t));
  ram_inode->mode = VFS_DIRECTORY | 0555;
  ram_inode->i_ops = &ramfs_inode_ops;
  ram_inode->f_ops = &ramfs_file_ops;

  vfs_dentry_t *ram_root = (vfs_dentry_t *)kmalloc(sizeof(vfs_dentry_t));
  memset(ram_root, 0, sizeof(vfs_dentry_t));
  strcpy(ram_root->name, "ram");
  ram_root->flags = VFS_DIRECTORY;
  ram_root->inode = ram_inode;
  vfs_mount("/ram", ram_root);

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
