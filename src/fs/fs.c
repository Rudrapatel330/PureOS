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
  int total = 0;
  uint32_t cluster = fat_resolve_path(path, fat_get_root_cluster());
  if (cluster != 0xFFFFFFFF) {
    total = fat_list_files_str_dir(cluster, buffer, max_len);
  } else {
    // Not in FAT. Check if it's root for RamFS
    if (!(!path || path[0] == 0 || (path[0] == '/' && path[1] == 0))) {
      return -1; // Path not found
    }
  }

  // Combine with RamFS for root
  if (!path || path[0] == 0 || (path[0] == '/' && path[1] == 0)) {
    if (total < max_len - 1) {
      // Append RamFS listing
      total += ramfs_list(buffer + total, max_len - total);
    }
  }
  return total;
}

int fs_list_files(const char *path, FileInfo *buffer, int max_files) {
  int count = 0;
  uint32_t cluster = fat_resolve_path(path, fat_get_root_cluster());
  if (cluster != 0xFFFFFFFF) {
    count = fat_list_files_gui_dir(cluster, buffer, max_files);
  } else {
    // Not in FAT. Check if it's root for RamFS
    if (!(!path || path[0] == 0 || (path[0] == '/' && path[1] == 0))) {
      return -1; // Path not found
    }
  }

  if (!path || path[0] == 0 || (path[0] == '/' && path[1] == 0)) {
    if (count < max_files) {
      count += ramfs_list_files(buffer + count, max_files - count);
    }
  }
  return count;
}

int fs_read(const char *filename, uint8_t *buffer) {
  int bytes = fat_read_file(filename, buffer);
  if (bytes > 0) {
    extern void print_serial(const char *str);
    print_serial("fs_read: read ");
    print_serial(filename);
    print_serial(" OK\n");
    return bytes;
  }
  return ramfs_read(filename, buffer);
}

int fs_write(const char *filename, const uint8_t *buffer, uint32_t size) {
  int ok = fat_write_file(filename, buffer, size);
  if (ok)
    return ok;
  return ramfs_write(filename, buffer, size);
}

int fs_delete(const char *filename) {
  int ok = fat_delete_file(filename);
  if (ok)
    return ok;
  return ramfs_delete(filename);
}

int fs_mkdir(const char *path) { return fat_mkdir(path); }

uint32_t fs_get_total_size() {
  extern uint32_t fat_get_total_size();
  return fat_get_total_size();
}

uint32_t fs_get_used_size() {
  extern uint32_t fat_get_used_size();
  return fat_get_used_size();
}
