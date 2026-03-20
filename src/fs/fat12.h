#ifndef FAT12_H
#define FAT12_H

#include "../kernel/types.h"

// BIOS Parameter Block (Standard FAT12/16)
typedef struct {
  uint8_t jump[3];
  char oem[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t fat_count;
  uint16_t root_dir_entries;
  uint16_t total_sectors_small;
  uint8_t media_descriptor;
  uint16_t sectors_per_fat;
  uint16_t sectors_per_track;
  uint16_t head_count;
  uint32_t hidden_sectors;
  uint32_t total_sectors_large;

  // Extended (FAT12/16)
  uint8_t drive_number;
  uint8_t reserved;
  uint8_t signature;
  uint32_t volume_id;
  char volume_label[11];
  char fs_type[8];
} __attribute__((packed)) fat12_bpb_t;

// Directory Entry
typedef struct {
  char name[8];
  char ext[3];
  uint8_t attributes;
  uint8_t reserved;
  uint8_t creation_ms;
  uint16_t creation_time;
  uint16_t creation_date;
  uint16_t last_access_date;
  uint16_t first_cluster_high; // Always 0 for FAT12
  uint16_t last_write_time;
  uint16_t last_write_date;
  uint16_t first_cluster_low;
  uint32_t size;
} __attribute__((packed)) fat_dir_entry_t;

// FileInfo moved to fs.h

#include "fs.h" // For FileInfo

void fs_init();
// void fs_list(); // Renamed/Removed to avoid conflict
int fs_read(const char *filename, uint8_t *buffer);
int fs_write(const char *filename, const uint8_t *buffer, uint32_t size);
int fs_delete(const char *filename);
int fs_rename(const char *old_name, const char *new_name);
int fs_list_files(const char *path, FileInfo *buffer, int max_files);
int fs_mkdir(const char *name);
int fs_cd(const char *path);
void fs_pwd(char *buffer);

#endif
