#ifndef FAT_H
#define FAT_H

#include "fs.h"
#include <stdint.h>

#define FAT_ATTR_DIRECTORY 0x10
#define FAT_ATTR_VOLUME_ID 0x08
#define FAT_ROOT_CLUSTER 0 // Custom sentinel for root dir in FAT16

// FAT32 Extended BPB
typedef struct {
  uint32_t sectors_per_fat32;
  uint16_t flags;
  uint16_t version;
  uint32_t root_cluster;
  uint16_t fs_info;
  uint16_t backup_boot_sector;
  uint8_t reserved[12];
  uint8_t drive_num;
  uint8_t reserved1;
  uint8_t boot_sig;
  uint32_t volume_id;
  char volume_label[11];
  char fs_type[8];
} __attribute__((packed)) fat32_ext_bpb_t;

// Unified FAT BPB
typedef struct {
  uint8_t jmp[3];
  char oem[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t fat_count;
  uint16_t root_entries;
  uint16_t total_sectors_short;
  uint8_t media_type;
  uint16_t sectors_per_fat;
  uint16_t sectors_per_track;
  uint16_t head_count;
  uint32_t hidden_sectors;
  uint32_t total_sectors_long;

  union {
    struct {
      uint8_t drive_num;
      uint8_t reserved;
      uint8_t boot_sig;
      uint32_t volume_id;
      char volume_label[11];
      char fs_type[8];
    } __attribute__((packed)) fat16;
    fat32_ext_bpb_t fat32;
  } ext;
} __attribute__((packed)) fat_bpb_t;

// FAT Directory Entry
typedef struct {
  char filename[8];
  char ext[3];
  uint8_t attributes;
  uint8_t reserved;
  uint8_t creation_time_ms;
  uint16_t creation_time;
  uint16_t creation_date;
  uint16_t last_access_date;
  uint16_t first_cluster_high; // FAT32: high 16 bits of cluster
  uint16_t last_write_time;
  uint16_t last_write_date;
  uint16_t first_cluster_low; // low 16 bits of cluster
  uint32_t file_size;
} __attribute__((packed)) fat_dir_entry_t;

// FS Status
typedef struct {
  uint32_t total_clusters;
  uint32_t free_clusters;
  uint32_t bytes_per_cluster;
} fat_info_t;

// API
int fat_init();
int fat_find_file(const char *path, fat_dir_entry_t *out_entry);
int fat_read_file(const char *path, uint8_t *buffer);
int fat_write_file(const char *path, const uint8_t *data, uint32_t size);
int fat_delete_file(const char *path);
int fat_copy_file(const char *src, const char *dst);
int fat_move_file(const char *src, const char *dst);

int fat_mkdir(const char *path);
int fat_list_files_str_dir(uint32_t dir_cluster, char *buffer, int max_len);
int fat_list_files_gui_dir(uint32_t dir_cluster, FileInfo *buffer,
                           int max_files);

// For shell/explorer state
uint32_t fat_get_root_cluster();
uint32_t fat_resolve_path(const char *path, uint32_t start_cluster);

#endif
