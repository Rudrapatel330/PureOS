#ifndef FS_H
#define FS_H

#include <stdint.h>

#define FS_MAX_FILENAME 32
#define FS_MAX_FILES 64

typedef struct {
  char name[FS_MAX_FILENAME];
  int size;
  const char *content; // For RamFS
  int flags;           // 0=File, 1=Directory
} file_entry_t;

// File Info struct for GUI
typedef struct {
  char name[32]; // Increased from 12 to prevent overflow with 8.3+dot+null
  int size;
  int is_dir;
} __attribute__((packed)) FileInfo;

// FS Interface
void fs_init();
int fs_list(const char *path, char *buffer, int max_len);
int fs_list_files(const char *path, FileInfo *buffer, int max_files);
file_entry_t *fs_find(const char *name);
int fs_read(const char *filename, uint8_t *buffer);
void ramfs_add_binary_file(const char *name, const unsigned char *data,
                           unsigned int size);

int fs_write(const char *filename, const uint8_t *buffer, uint32_t size);
int fs_delete(const char *filename);
int fs_mkdir(const char *path);

#endif
