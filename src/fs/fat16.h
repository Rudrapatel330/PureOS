#ifndef FAT16_H
#define FAT16_H

#include "../include/types.h"

typedef struct {
    u8  oem_name[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  fat_copies;
    u16 root_dir_entries;
    u16 total_sectors;
    u8  media_descriptor;
    u16 sectors_per_fat;
    u16 sectors_per_track;
    u16 heads;
    u32 hidden_sectors;
    u32 total_sectors_big;
    u8  drive_number;
    u8  unused;
    u8  ext_boot_signature;
    u32 serial_number;
    u8  volume_label[11];
    u8  fs_type[8];
} __attribute__((packed)) fat_bs_t;

typedef struct {
    u8  filename[8];
    u8  ext[3];
    u8  attributes;
    u8  reserved;
    u8  create_ms;
    u16 create_time;
    u16 create_date;
    u16 access_date;
    u16 cluster_high;
    u16 modify_time;
    u16 modify_date;
    u16 cluster_low;
    u32 size;
} __attribute__((packed)) fat_dir_entry_t;

void fat16_init();
void fat16_list_root();

#endif
