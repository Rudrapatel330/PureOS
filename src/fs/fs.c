#include "fs.h"
#include "../kernel/string.h"
#include "fat.h"
#include "ramfs.h"

#include "vfs.h"

void fs_init() {
  vfs_init();
  // Initialize both
  ramfs_init();
  fat_init();
  extern int ext2_init(void);
  ext2_init();
}

file_entry_t *fs_find(const char *name) {
  if (!name) return 0;
  
  extern void print_serial(const char *str);
  print_serial("fs_find: ");
  print_serial(name);
  print_serial("\n");

  // 1. Check RamFS
  extern file_entry_t ramfs_files[];
  extern int ramfs_count;
  
  const char *ram_name = name;
  if (name[0] == '/') ram_name = name + 1;

  for (int i = 0; i < ramfs_count; i++) {
    if (strcmp(ramfs_files[i].name, ram_name) == 0 || strcmp(ramfs_files[i].name, name) == 0) {
      return &ramfs_files[i];
    }
  }
  
  // 2. Check FAT
  static file_entry_t fat_fe;
  fat_dir_entry_t fat_entry;
  if (fat_find_file(name, &fat_entry)) {
      int len = 0;
      while (name[len]) len++;
      int start = 0;
      // Strip path for the name field in file_entry_t if it's too long, but usually we just want the basename or the full path if it fits
      if (len >= FS_MAX_FILENAME) {
          // Find last slash
          for (int i = len - 1; i >= 0; i--) {
              if (name[i] == '/') {
                  start = i + 1;
                  break;
              }
          }
      }
      strncpy(fat_fe.name, name + start, FS_MAX_FILENAME - 1);
      fat_fe.name[FS_MAX_FILENAME - 1] = 0;
      fat_fe.size = fat_entry.file_size;
      fat_fe.content = 0; // Not cached in memory yet
      fat_fe.flags = (fat_entry.attributes & 0x10) ? 1 : 0; // 0x10 is FAT_ATTR_DIRECTORY
      return &fat_fe;
  }

  return 0;
}

int fs_list(const char *path, char *buffer, int max_len) {
  int fd = vfs_open(path, 0);
  if (fd < 0) return -1;
  vfs_dentry_t *node;
  int idx = 0;
  int total = 0;
  buffer[0] = 0;
  while ((node = vfs_readdir(fd, idx++)) != 0) {
    int len = 0;
    while (node->name[len]) {
      if (total < max_len - 2) buffer[total++] = node->name[len];
      len++;
    }
    if (total < max_len - 1) buffer[total++] = '\n';
  }
  buffer[total] = 0;
  vfs_close(fd);
  return total;
}

int fs_list_files(const char *path, FileInfo *buffer, int max_files) {
  int fd = vfs_open(path, 0);
  if (fd < 0) return -1;
  vfs_dentry_t *node;
  int count = 0;
  while ((node = vfs_readdir(fd, count)) != 0 && count < max_files) {
    int j = 0;
    while (node->name[j] && j < 31) {
      buffer[count].name[j] = node->name[j];
      j++;
    }
    buffer[count].name[j] = 0;
    buffer[count].size = node->inode ? node->inode->size : 0;
    buffer[count].is_dir = (node->inode && (node->inode->mode & VFS_DIRECTORY)) ? 1 : 0;
    count++;
  }
  vfs_close(fd);
  return count;
}

int fs_read(const char *filename, uint8_t *buffer) {
  vfs_stat_t st;
  if (vfs_stat(filename, &st) < 0) return 0;
  int fd = vfs_open(filename, 0);
  if (fd < 0) return 0;
  int bytes = vfs_read(fd, buffer, st.size);
  vfs_close(fd);
  return bytes;
}

int fs_write(const char *filename, const uint8_t *buffer, uint32_t size) {
  if (strncmp(filename, "/ram/", 5) == 0 || strcmp(filename, "/ram") == 0) {
    const char *name = filename;
    if (strncmp(filename, "/ram/", 5) == 0) name = filename + 5;
    return ramfs_write(name, buffer, size);
  }
  return fat_write_file(filename, buffer, size);
}

int fs_delete(const char *filename) { return vfs_unlink(filename); }

int fs_mkdir(const char *path) { return vfs_mkdir(path); }

uint32_t fs_get_total_size() {
  extern uint32_t fat_get_total_size();
  return fat_get_total_size();
}

uint32_t fs_get_used_size() {
  extern uint32_t fat_get_used_size();
  return fat_get_used_size();
}
