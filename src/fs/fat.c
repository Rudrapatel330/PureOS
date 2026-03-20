#include "fat.h"
#include "../drivers/ata.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "vfs.h"

static fat_bpb_t bpb;
static uint32_t first_data_sector;
static uint32_t root_dir_sectors;
static uint32_t first_root_dir_sector;
static int use_secondary_ata = 0; // 1 = data disk found on ATA2
uint32_t fat_partition_offset = 0;
static int is_fat32 = 0;
static uint32_t sectors_per_fat;

extern void print_serial(const char *);

static uint32_t get_entry_cluster(fat_dir_entry_t *entry) {
  return (uint32_t)entry->first_cluster_low |
         ((uint32_t)entry->first_cluster_high << 16);
}

static void set_entry_cluster(fat_dir_entry_t *entry, uint32_t cluster) {
  entry->first_cluster_low = (uint16_t)(cluster & 0xFFFF);
  entry->first_cluster_high = (uint16_t)((cluster >> 16) & 0xFFFF);
}

// === Block Cache for Disk Reads ===
#define BLOCK_CACHE_SIZE 128

typedef struct {
  uint32_t lba;
  uint8_t data[512];
  int valid;
  uint32_t last_used; // Simple LRU counter
} cache_entry_t;

static cache_entry_t block_cache[BLOCK_CACHE_SIZE];
static uint32_t cache_access_counter = 0;

// Wrapper: Read from whichever disk FAT is mounted on (with caching)
static int fat_disk_read(uint32_t lba, uint8_t *buffer) {
  // Check cache first
  for (int i = 0; i < BLOCK_CACHE_SIZE; i++) {
    if (block_cache[i].valid && block_cache[i].lba == lba) {
      // Cache hit
      block_cache[i].last_used = ++cache_access_counter;
      memcpy(buffer, block_cache[i].data, 512);
      return 0;
    }
  }

  // Cache miss — read from disk
  int result;
  if (use_secondary_ata)
    result = ata2_read_sector(lba, buffer);
  else
    result = ata_read_sector(lba + fat_partition_offset, buffer);

  if (result == 0) {
    // Find an empty slot or the LRU slot
    int slot = 0;
    uint32_t oldest = 0xFFFFFFFF;
    for (int i = 0; i < BLOCK_CACHE_SIZE; i++) {
      if (!block_cache[i].valid) {
        slot = i;
        break;
      }
      if (block_cache[i].last_used < oldest) {
        oldest = block_cache[i].last_used;
        slot = i;
      }
    }
    block_cache[slot].lba = lba;
    memcpy(block_cache[slot].data, buffer, 512);
    block_cache[slot].valid = 1;
    block_cache[slot].last_used = ++cache_access_counter;
  }
  return result;
}

// Wrapper: Write to whichever disk FAT is mounted on
static int fat_disk_write(uint32_t lba, uint8_t *buffer) {
  print_serial("DISK: write lba=");
  char buf[16];
  extern void k_itoa(int, char *);
  k_itoa(lba, buf);
  print_serial(buf);
  print_serial("\n");

  // Invalidate cache for this sector on write
  for (int i = 0; i < BLOCK_CACHE_SIZE; i++) {
    if (block_cache[i].valid && block_cache[i].lba == lba) {
      block_cache[i].valid = 0;
      break;
    }
  }
  if (use_secondary_ata) {
    print_serial("DISK: using secondary ATA\n");
    return ata2_write_sector(lba, buffer);
  }
  print_serial("DISK: using primary ATA\n");
  return ata_write_sector(lba + fat_partition_offset, buffer);
}

int fat_init() {
  print_serial("FAT: Initializing driver...\n");
  uint8_t boot_sector[512];

  // Try secondary ATA first (persistent data disk)
  if (ata2_detect()) {
    print_serial("FAT: Data disk detected on ATA2!\n");
    use_secondary_ata = 1;
    if (!ata2_read_sector(0, boot_sector)) {
      print_serial("FAT: ATA2 Read Error, falling back...\n");
      use_secondary_ata = 0;
    }
  }

  // Fallback to primary ATA
  if (!use_secondary_ata) {
    if (!ata_read_sector(0, boot_sector)) {
      print_serial("FAT: Disk Read Error\n");
      return 0;
    }

    // Check if sector 0 looks like a BPB.
    // Standard FAT boot sectors have 0x55AA at 510.
    // BUT we must also check bps=512 and fat_count=1 or 2 to avoid bootloader
    // code.
    int looks_like_vbr = (boot_sector[510] == 0x55 && boot_sector[511] == 0xAA);
    if (looks_like_vbr) {
      uint16_t bps = *(uint16_t *)&boot_sector[11];
      uint8_t count = boot_sector[16];
      if (bps != 512 || (count != 1 && count != 2)) {
        looks_like_vbr = 0;
      }
    }

    if (!looks_like_vbr) {
      print_serial("FAT: No valid BPB at Sector 0 (likely MBR/Bootloader), "
                   "checking 50MB offset...\n");
      if (ata_read_sector(102400, boot_sector)) {
        if (boot_sector[510] == 0x55 && boot_sector[511] == 0xAA) {
          uint16_t bps = *(uint16_t *)&boot_sector[11];
          if (bps == 512) {
            print_serial("FAT: Internal Partition found at Sector 102400!\n");
            fat_partition_offset = 102400;
          }
        }
      }
    }
  }

  for (int i = 0; i < sizeof(fat_bpb_t); i++)
    ((uint8_t *)&bpb)[i] = boot_sector[i];

  print_serial("FAT: BPB - bps=");
  char b1[12];
  k_itoa(bpb.bytes_per_sector, b1);
  print_serial(b1);
  print_serial(" spc=");
  k_itoa(bpb.sectors_per_cluster, b1);
  print_serial(b1);
  print_serial(" res=");
  k_itoa(bpb.reserved_sectors, b1);
  print_serial(b1);
  print_serial(" fat_cnt=");
  k_itoa(bpb.fat_count, b1);
  print_serial(b1);
  print_serial(" root_ent=");
  k_itoa(bpb.root_entries, b1);
  print_serial(b1);
  print_serial("\n");

  if (bpb.bytes_per_sector == 0 || bpb.sectors_per_cluster == 0) {
    print_serial("FAT: No valid BPB found.\n");
    return 0;
  }

  sectors_per_fat = (bpb.sectors_per_fat != 0)
                        ? bpb.sectors_per_fat
                        : bpb.ext.fat32.sectors_per_fat32;

  print_serial("FAT: sectors_per_fat=");
  k_itoa(sectors_per_fat, b1);
  print_serial(b1);
  print_serial("\n");

  root_dir_sectors = ((bpb.root_entries * 32) + (bpb.bytes_per_sector - 1)) /
                     bpb.bytes_per_sector;

  first_root_dir_sector =
      bpb.reserved_sectors + (bpb.fat_count * sectors_per_fat);
  first_data_sector = first_root_dir_sector + root_dir_sectors;

  uint32_t total_sectors = (bpb.total_sectors_short != 0)
                               ? bpb.total_sectors_short
                               : bpb.total_sectors_long;
  uint32_t data_sectors = total_sectors - first_data_sector;
  uint32_t total_clusters = data_sectors / bpb.sectors_per_cluster;

  if (total_clusters >= 65525) {
    is_fat32 = 1;
    print_serial("FAT: Detected FAT32\n");
  } else {
    is_fat32 = 0;
    print_serial("FAT: Detected FAT12/16\n");
  }

  uint32_t root_cluster =
      is_fat32 ? bpb.ext.fat32.root_cluster : FAT_ROOT_CLUSTER;

  if (use_secondary_ata)
    print_serial("FAT: Persistent data disk ready (C:)\n");
  else
    print_serial("FAT: Ready (boot disk)\n");

  // Mount FAT at /disk
  vfs_node_t *fat_root = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
  memset(fat_root, 0, sizeof(vfs_node_t));
  strcpy(fat_root->name, "disk");
  fat_root->flags = VFS_DIRECTORY;
  fat_root->inode = root_cluster;
  extern vfs_driver_t fat_vfs_driver;
  fat_root->driver = &fat_vfs_driver;
  vfs_mount("/disk", fat_root);

  return 1;
}

uint32_t fat_get_root_cluster() {
  return is_fat32 ? bpb.ext.fat32.root_cluster : FAT_ROOT_CLUSTER;
}

static void to_fat_name(const char *filename, char *fat_name) {
  for (int i = 0; i < 11; i++)
    fat_name[i] = ' ';

  // Special case for . and ..
  if (strcmp(filename, ".") == 0) {
    fat_name[0] = '.';
    return;
  }
  if (strcmp(filename, "..") == 0) {
    fat_name[0] = '.';
    fat_name[1] = '.';
    return;
  }

  int p = 0;
  for (int i = 0; i < 8 && filename[p] && filename[p] != '.'; i++) {
    char c = filename[p++];
    if (c >= 'a' && c <= 'z')
      c -= 32;
    fat_name[i] = c;
  }
  while (filename[p] && filename[p] != '.')
    p++;
  if (filename[p] == '.') {
    p++;
    for (int i = 0; i < 3 && filename[p]; i++) {
      char c = filename[p++];
      if (c >= 'a' && c <= 'z')
        c -= 32;
      fat_name[i + 8] = c;
    }
  }
}

static uint32_t fat_get_next_cluster(uint32_t cluster) {
  uint32_t fat_offset;
  if (is_fat32) {
    fat_offset = cluster * 4;
  } else {
    fat_offset = cluster * 2;
  }

  uint8_t buffer[512];
  uint32_t fat_sector = bpb.reserved_sectors + (fat_offset / 512);
  if (!fat_disk_read(fat_sector, buffer))
    return is_fat32 ? 0x0FFFFFFF : 0xFFFF;

  if (is_fat32) {
    return (*(uint32_t *)&buffer[fat_offset % 512]) & 0x0FFFFFFF;
  } else {
    return *(uint16_t *)&buffer[fat_offset % 512];
  }
}

static void fat_set_next_cluster(uint32_t cluster, uint32_t next) {
  uint32_t fat_offset;
  if (is_fat32) {
    fat_offset = cluster * 4;
  } else {
    fat_offset = cluster * 2;
  }

  uint8_t buffer[512];
  uint32_t fat_sector = bpb.reserved_sectors + (fat_offset / 512);

  if (!fat_disk_read(fat_sector, buffer))
    return;

  if (is_fat32) {
    *(uint32_t *)&buffer[fat_offset % 512] = next;
  } else {
    *(uint16_t *)&buffer[fat_offset % 512] = (uint16_t)next;
  }

  fat_disk_write(fat_sector, buffer);
}

static uint32_t fat_allocate_cluster() {
  uint8_t buffer[512];
  for (uint32_t sector = 0; sector < sectors_per_fat; sector++) {
    if (!fat_disk_read(bpb.reserved_sectors + sector, buffer))
      break;

    if (is_fat32) {
      uint32_t *fat = (uint32_t *)buffer;
      for (int i = 0; i < 128; i++) {
        uint32_t cluster = sector * 128 + i;
        if (cluster < 2)
          continue;
        if ((fat[i] & 0x0FFFFFFF) == 0) {
          fat[i] = 0x0FFFFFFF;
          fat_disk_write(bpb.reserved_sectors + sector, buffer);
          return cluster;
        }
      }
    } else {
      uint16_t *fat = (uint16_t *)buffer;
      for (int i = 0; i < 256; i++) {
        uint32_t cluster = sector * 256 + i;
        if (cluster < 2)
          continue;
        if (fat[i] == 0) {
          fat[i] = 0xFFFF;
          fat_disk_write(bpb.reserved_sectors + sector, buffer);
          return cluster;
        }
      }
    }
  }
  return 0;
}

static int raw_find_in_dir(uint32_t dir_cluster, const char *name,
                           fat_dir_entry_t *out_entry) {
  uint8_t buffer[512];
  char fat_name[11];
  to_fat_name(name, fat_name);

  if (!is_fat32 && dir_cluster == FAT_ROOT_CLUSTER) {
    for (int sector = 0; sector < root_dir_sectors; sector++) {
      if (!fat_disk_read(first_root_dir_sector + sector, buffer))
        break;
      fat_dir_entry_t *entries = (fat_dir_entry_t *)buffer;
      for (int i = 0; i < 512 / sizeof(fat_dir_entry_t); i++) {
        if ((uint8_t)entries[i].filename[0] == 0x00)
          continue;
        if ((uint8_t)entries[i].filename[0] == 0xE5)
          continue;
        if (memcmp(entries[i].filename, fat_name, 11) == 0) {
          if (out_entry)
            *out_entry = entries[i];
          return 1;
        }
      }
    }
  } else {
    // For FAT32, the root is handled like any other cluster chain
    uint32_t cluster = dir_cluster;
    uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;

    while (cluster >= 2 && cluster < eof) {
      uint32_t sector =
          first_data_sector + (cluster - 2) * bpb.sectors_per_cluster;
      for (int s = 0; s < bpb.sectors_per_cluster; s++) {
        if (!fat_disk_read(sector + s, buffer))
          return 0;
        fat_dir_entry_t *entries = (fat_dir_entry_t *)buffer;
        for (int i = 0; i < 512 / sizeof(fat_dir_entry_t); i++) {
          if ((uint8_t)entries[i].filename[0] == 0x00)
            continue;
          if ((uint8_t)entries[i].filename[0] == 0xE5)
            continue;
          if (memcmp(entries[i].filename, fat_name, 11) == 0) {
            if (out_entry)
              *out_entry = entries[i];
            return 1;
          }
        }
      }
      cluster = fat_get_next_cluster(cluster);
    }
  }
  return 0;
}

uint32_t fat_resolve_path(const char *path, uint32_t start_cluster) {
  uint32_t root_cluster =
      is_fat32 ? bpb.ext.fat32.root_cluster : FAT_ROOT_CLUSTER;

  if (!path || path[0] == 0)
    return start_cluster;
  if (strcmp(path, "/") == 0)
    return root_cluster;

  uint32_t current_dir = start_cluster;
  if (path[0] == '/') {
    current_dir = root_cluster;
    while (*path == '/')
      path++;
  }
  if (*path == 0)
    return current_dir;

  char component[32];
  const char *p = path;
  while (*p) {
    int i = 0;
    while (*p && *p != '/') {
      if (i < 31)
        component[i++] = *p;
      p++;
    }
    component[i] = 0;

    if (i > 0) {
      fat_dir_entry_t entry;
      if (raw_find_in_dir(current_dir, component, &entry)) {
        if (entry.attributes & FAT_ATTR_DIRECTORY) {
          uint32_t next = get_entry_cluster(&entry);
          current_dir = (next == 0 && !is_fat32) ? FAT_ROOT_CLUSTER : next;
        } else {
          if (*p == '/')
            return 0xFFFFFFFF;
        }
      } else {
        return 0xFFFFFFFF;
      }
    }

    while (*p == '/')
      p++;
  }
  return current_dir;
}

int fat_find_file(const char *path, fat_dir_entry_t *out_entry) {
  uint32_t root_cluster =
      is_fat32 ? bpb.ext.fat32.root_cluster : FAT_ROOT_CLUSTER;
  char dir_path[128];
  strcpy(dir_path, path);
  char *last_slash = 0;
  for (int i = 0; dir_path[i]; i++)
    if (dir_path[i] == '/')
      last_slash = dir_path + i;

  uint32_t dir_cluster;
  const char *filename;

  if (last_slash) {
    if (last_slash == dir_path) {
      dir_cluster = root_cluster;
    } else {
      *last_slash = 0;
      dir_cluster = fat_resolve_path(dir_path, root_cluster);
    }
    filename = last_slash + 1;
  } else {
    dir_cluster = root_cluster;
    filename = path;
  }

  if (dir_cluster == 0xFFFFFFFF)
    return 0;
  return raw_find_in_dir(dir_cluster, filename, out_entry);
}

int fat_read_file(const char *path, uint8_t *buffer) {
  fat_dir_entry_t entry;
  if (!fat_find_file(path, &entry))
    return 0;
  uint32_t cluster = get_entry_cluster(&entry);
  uint32_t bytes_left = entry.file_size;
  uint32_t offset = 0;
  uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;

  while (cluster >= 2 && cluster < eof && bytes_left > 0) {
    uint32_t sector =
        first_data_sector + (cluster - 2) * bpb.sectors_per_cluster;
    for (int i = 0; i < bpb.sectors_per_cluster && bytes_left > 0; i++) {
      uint8_t sbuf[512];
      if (!fat_disk_read(sector + i, sbuf)) {
        buffer[offset] = 0;
        return offset;
      }
      uint32_t chunk = (bytes_left > 512) ? 512 : bytes_left;
      memcpy(buffer + offset, sbuf, chunk);
      offset += chunk;
      bytes_left -= chunk;
    }
    cluster = fat_get_next_cluster(cluster);
  }
  buffer[offset] = 0;
  return offset;
}

int fat_write_file(const char *path, const uint8_t *data, uint32_t size) {
  print_serial("FAT: write_file called with path=");
  print_serial(path);
  print_serial("\n");

  uint32_t root_cluster =
      is_fat32 ? bpb.ext.fat32.root_cluster : FAT_ROOT_CLUSTER;
  char dir_path[128];
  strcpy(dir_path, path);
  char *last_slash = 0;
  for (int i = 0; dir_path[i]; i++)
    if (dir_path[i] == '/')
      last_slash = dir_path + i;

  uint32_t dir_cluster;
  const char *filename;

  if (last_slash) {
    if (last_slash == dir_path) {
      dir_cluster = root_cluster;
    } else {
      *last_slash = 0;
      dir_cluster = fat_resolve_path(dir_path, root_cluster);
    }
    filename = last_slash + 1;
  } else {
    dir_cluster = root_cluster;
    filename = path;
  }

  if (dir_cluster == 0xFFFFFFFF) {
    print_serial("FAT: resolve_path failed\n");
    return 0;
  }
  char fat_name[11];
  to_fat_name(filename, fat_name);

  uint8_t buffer[512];
  uint32_t entry_sector = 0;
  int entry_index = -1;
  fat_dir_entry_t entry;
  int found = 0;

  if (!is_fat32 && dir_cluster == FAT_ROOT_CLUSTER) {
    for (int s = 0; s < root_dir_sectors; s++) {
      uint32_t slba = first_root_dir_sector + s;
      fat_disk_read(slba, buffer);
      fat_dir_entry_t *es = (fat_dir_entry_t *)buffer;
      for (int i = 0; i < 16; i++) {
        if ((uint8_t)es[i].filename[0] == 0x00 ||
            (uint8_t)es[i].filename[0] == 0xE5) {
          if (entry_index == -1) {
            entry_sector = slba;
            entry_index = i;
          }
          if ((uint8_t)es[i].filename[0] == 0x00) {
            found = -1;
            break;
          }
          continue;
        }
        if (memcmp(es[i].filename, fat_name, 11) == 0) {
          entry = es[i];
          entry_sector = slba;
          entry_index = i;
          found = 1;
          break;
        }
      }
      if (found == 1 || found == -1)
        break;
    }
  } else {
    uint32_t cluster = dir_cluster;
    uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;
    uint32_t last_cluster = cluster;
    while (cluster >= 2 && cluster < eof) {
      last_cluster = cluster;
      uint32_t base_sec =
          first_data_sector + (cluster - 2) * bpb.sectors_per_cluster;
      for (int s = 0; s < bpb.sectors_per_cluster; s++) {
        fat_disk_read(base_sec + s, buffer);
        fat_dir_entry_t *es = (fat_dir_entry_t *)buffer;
        for (int i = 0; i < 16; i++) {
          if ((uint8_t)es[i].filename[0] == 0x00 ||
              (uint8_t)es[i].filename[0] == 0xE5) {
            if (entry_index == -1) {
              entry_sector = base_sec + s;
              entry_index = i;
            }
            if ((uint8_t)es[i].filename[0] == 0x00) {
              found = -1;
              break;
            }
            continue;
          }
          if (memcmp(es[i].filename, fat_name, 11) == 0) {
            entry = es[i];
            entry_sector = base_sec + s;
            entry_index = i;
            found = 1;
            break;
          }
        }
        if (found == 1 || found == -1)
          break;
      }
      if (found == 1 || found == -1)
        break;
      cluster = fat_get_next_cluster(cluster);
    }

    if (entry_index == -1) {
      uint32_t new_cluster = fat_allocate_cluster();
      if (new_cluster >= 2) {
        fat_set_next_cluster(last_cluster, new_cluster);
        fat_set_next_cluster(new_cluster, is_fat32 ? 0x0FFFFFFF : 0xFFFF);

        uint8_t zero_buf[512];
        for (int j = 0; j < 512; j++)
          zero_buf[j] = 0;

        uint32_t new_sec =
            first_data_sector + (new_cluster - 2) * bpb.sectors_per_cluster;
        for (int i = 0; i < bpb.sectors_per_cluster; i++) {
          fat_disk_write(new_sec + i, zero_buf);
        }

        entry_sector = new_sec;
        entry_index = 0;
      }
    }
  }

  if (entry_index == -1)
    return 0;

  if (found != 1) {
    memset(&entry, 0, sizeof(entry));
    memcpy(entry.filename, fat_name, 11);
    set_entry_cluster(&entry, fat_allocate_cluster());
  }

  uint32_t cluster = get_entry_cluster(&entry);
  uint32_t left = size;
  uint32_t off = 0;
  uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;

  while (left > 0) {
    uint32_t sec = first_data_sector + (cluster - 2) * bpb.sectors_per_cluster;
    for (int i = 0; i < bpb.sectors_per_cluster && left > 0; i++) {
      uint8_t sbuf[512] = {0};
      uint32_t chunk = (left > 512) ? 512 : left;
      memcpy(sbuf, data + off, chunk);

      print_serial("FAT: writing sector LBA=");
      char buf[16];
      extern void k_itoa(int, char *);
      k_itoa(sec + i, buf);
      print_serial(buf);
      print_serial("\n");

      if (!fat_disk_write(sec + i, sbuf)) {
        print_serial("FAT: disk write failed\n");
        return 0;
      }
      off += chunk;
      left -= chunk;
    }
    if (left > 0) {
      uint32_t next = fat_get_next_cluster(cluster);
      if (next >= eof) {
        next = fat_allocate_cluster();
        fat_set_next_cluster(cluster, next);
        fat_set_next_cluster(next, is_fat32 ? 0x0FFFFFFF : 0xFFFF);
      }
      cluster = next;
    }
  }

  entry.file_size = size;
  fat_disk_read(entry_sector, buffer);
  ((fat_dir_entry_t *)buffer)[entry_index] = entry;
  if (!fat_disk_write(entry_sector, buffer)) {
    print_serial("FAT: directory entry write failed\n");
    return 0;
  }

  print_serial("FAT: write_file succeeded\n");
  return 1;
}

int fat_delete_file(const char *path) {
  uint32_t root_cluster =
      is_fat32 ? bpb.ext.fat32.root_cluster : FAT_ROOT_CLUSTER;
  char dir_path[128];
  strcpy(dir_path, path);
  char *last_slash = 0;
  for (int i = 0; dir_path[i]; i++)
    if (dir_path[i] == '/')
      last_slash = dir_path + i;

  uint32_t dir_cluster;
  const char *filename;

  if (last_slash) {
    if (last_slash == dir_path) {
      dir_cluster = root_cluster;
    } else {
      *last_slash = 0;
      dir_cluster = fat_resolve_path(dir_path, root_cluster);
    }
    filename = last_slash + 1;
  } else {
    dir_cluster = root_cluster;
    filename = path;
  }

  if (dir_cluster == 0xFFFFFFFF)
    return 0;
  char fat_name[11];
  to_fat_name(filename, fat_name);

  uint8_t buffer[512];
  uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;

  if (!is_fat32 && dir_cluster == FAT_ROOT_CLUSTER) {
    for (int s = 0; s < root_dir_sectors; s++) {
      uint32_t slba = first_root_dir_sector + s;
      fat_disk_read(slba, buffer);
      fat_dir_entry_t *es = (fat_dir_entry_t *)buffer;
      for (int i = 0; i < 16; i++) {
        if (memcmp(es[i].filename, fat_name, 11) == 0) {
          uint32_t c = get_entry_cluster(&es[i]);
          es[i].filename[0] = 0xE5;
          fat_disk_write(slba, buffer);
          while (c >= 2 && c < eof) {
            uint32_t next = fat_get_next_cluster(c);
            fat_set_next_cluster(c, 0);
            c = next;
          }
          return 1;
        }
      }
    }
  } else {
    uint32_t cluster = dir_cluster;
    while (cluster >= 2 && cluster < eof) {
      uint32_t base_sec =
          first_data_sector + (cluster - 2) * bpb.sectors_per_cluster;
      for (int s = 0; s < bpb.sectors_per_cluster; s++) {
        fat_disk_read(base_sec + s, buffer);
        fat_dir_entry_t *es = (fat_dir_entry_t *)buffer;
        for (int i = 0; i < 16; i++) {
          if (memcmp(es[i].filename, fat_name, 11) == 0) {
            uint32_t c = get_entry_cluster(&es[i]);
            es[i].filename[0] = 0xE5;
            fat_disk_write(base_sec + s, buffer);
            while (c >= 2 && c < eof) {
              uint32_t next = fat_get_next_cluster(c);
              fat_set_next_cluster(c, 0);
              c = next;
            }
            return 1;
          }
        }
      }
      cluster = fat_get_next_cluster(cluster);
    }
  }
  return 0;
}

int fat_mkdir(const char *path) {
  uint32_t root_cluster =
      is_fat32 ? bpb.ext.fat32.root_cluster : FAT_ROOT_CLUSTER;
  char dir_path[128];
  strcpy(dir_path, path);
  char *last_slash = 0;
  for (int i = 0; dir_path[i]; i++)
    if (dir_path[i] == '/')
      last_slash = dir_path + i;

  uint32_t parent_cluster;
  const char *dirname;

  if (last_slash) {
    if (last_slash == dir_path) {
      parent_cluster = root_cluster;
    } else {
      *last_slash = 0;
      parent_cluster = fat_resolve_path(dir_path, root_cluster);
    }
    dirname = last_slash + 1;
  } else {
    parent_cluster = root_cluster;
    dirname = path;
  }

  if (parent_cluster == 0xFFFFFFFF || !dirname || dirname[0] == 0)
    return 0;

  // Check if already exists
  if (raw_find_in_dir(parent_cluster, dirname, 0)) {
    print_serial("FAT: mkdir failed, name already exists\n");
    return 0;
  }

  uint32_t new_cluster = fat_allocate_cluster();
  if (new_cluster == 0)
    return 0;
  fat_set_next_cluster(new_cluster, is_fat32 ? 0x0FFFFFFF : 0xFFFF);

  uint8_t dir_buf[512];
  memset(dir_buf, 0, 512);
  fat_dir_entry_t *des = (fat_dir_entry_t *)dir_buf;
  memcpy(des[0].filename, ".          ", 11);
  des[0].attributes = FAT_ATTR_DIRECTORY;
  set_entry_cluster(&des[0], new_cluster);
  memcpy(des[1].filename, "..         ", 11);
  des[1].attributes = FAT_ATTR_DIRECTORY;
  set_entry_cluster(&des[1], (parent_cluster == root_cluster && !is_fat32)
                                 ? 0
                                 : parent_cluster);

  uint32_t dir_sec =
      first_data_sector + (new_cluster - 2) * bpb.sectors_per_cluster;
  fat_disk_write(dir_sec, dir_buf);
  memset(dir_buf, 0, 512);
  for (int i = 1; i < bpb.sectors_per_cluster; i++) {
    fat_disk_write(dir_sec + i, dir_buf);
  }

  char fat_name[11];
  to_fat_name(dirname, fat_name);

  uint8_t sbuf[512];
  if (!is_fat32 && parent_cluster == FAT_ROOT_CLUSTER) {
    for (int s = 0; s < root_dir_sectors; s++) {
      uint32_t slba = first_root_dir_sector + s;
      fat_disk_read(slba, sbuf);
      fat_dir_entry_t *entries = (fat_dir_entry_t *)sbuf;
      for (int i = 0; i < 16; i++) {
        if ((uint8_t)entries[i].filename[0] == 0x00 ||
            (uint8_t)entries[i].filename[0] == 0xE5) {
          memset(&entries[i], 0, sizeof(fat_dir_entry_t));
          memcpy(entries[i].filename, fat_name, 11);
          entries[i].attributes = FAT_ATTR_DIRECTORY;
          set_entry_cluster(&entries[i], new_cluster);
          fat_disk_write(slba, sbuf);
          return 1;
        }
      }
    }
  } else {
    uint32_t pc = parent_cluster;
    uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;
    while (pc >= 2 && pc < eof) {
      uint32_t base = first_data_sector + (pc - 2) * bpb.sectors_per_cluster;
      for (int s = 0; s < bpb.sectors_per_cluster; s++) {
        fat_disk_read(base + s, sbuf);
        fat_dir_entry_t *entries = (fat_dir_entry_t *)sbuf;
        for (int i = 0; i < 16; i++) {
          if ((uint8_t)entries[i].filename[0] == 0x00 ||
              (uint8_t)entries[i].filename[0] == 0xE5) {
            memset(&entries[i], 0, sizeof(fat_dir_entry_t));
            memcpy(entries[i].filename, fat_name, 11);
            entries[i].attributes = FAT_ATTR_DIRECTORY;
            set_entry_cluster(&entries[i], new_cluster);
            fat_disk_write(base + s, sbuf);
            return 1;
          }
        }
      }
      pc = fat_get_next_cluster(pc);
    }
  }
  fat_set_next_cluster(new_cluster, 0);
  return 0;
}

int fat_list_files_str_dir(uint32_t dir_cluster, char *buffer, int max_len) {
  uint8_t sbuf[512];
  int pos = 0;
  if (!is_fat32 && dir_cluster == FAT_ROOT_CLUSTER) {
    for (int s = 0; s < root_dir_sectors && pos < max_len - 15; s++) {
      fat_disk_read(first_root_dir_sector + s, sbuf);
      fat_dir_entry_t *es = (fat_dir_entry_t *)sbuf;
      for (int i = 0; i < 16 && pos < max_len - 15; i++) {
        if ((uint8_t)es[i].filename[0] == 0x00) {
          buffer[pos] = 0;
          return pos;
        }
        if ((uint8_t)es[i].filename[0] == 0xE5 ||
            (es[i].attributes & FAT_ATTR_VOLUME_ID))
          continue;
        for (int j = 0; j < 8; j++)
          if (es[i].filename[j] != ' ')
            buffer[pos++] = es[i].filename[j];
        if (es[i].ext[0] != ' ') {
          buffer[pos++] = '.';
          for (int j = 0; j < 3; j++)
            if (es[i].ext[j] != ' ')
              buffer[pos++] = es[i].ext[j];
        }
        if (es[i].attributes & FAT_ATTR_DIRECTORY)
          buffer[pos++] = '/';
        buffer[pos++] = '\n';
      }
    }
  } else {
    uint32_t c = dir_cluster;
    uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;
    while (c >= 2 && c < eof && pos < max_len - 15) {
      uint32_t base = first_data_sector + (c - 2) * bpb.sectors_per_cluster;
      for (int s = 0; s < bpb.sectors_per_cluster && pos < max_len - 15; s++) {
        fat_disk_read(base + s, sbuf);
        fat_dir_entry_t *es = (fat_dir_entry_t *)sbuf;
        for (int i = 0; i < 16 && pos < max_len - 15; i++) {
          if ((uint8_t)es[i].filename[0] == 0x00) {
            buffer[pos] = 0;
            return pos;
          }
          if ((uint8_t)es[i].filename[0] == 0xE5 ||
              (es[i].attributes & FAT_ATTR_VOLUME_ID))
            continue;
          for (int j = 0; j < 8; j++)
            if (es[i].filename[j] != ' ')
              buffer[pos++] = es[i].filename[j];
          if (es[i].ext[0] != ' ') {
            buffer[pos++] = '.';
            for (int j = 0; j < 3; j++)
              if (es[i].ext[j] != ' ')
                buffer[pos++] = es[i].ext[j];
          }
          if (es[i].attributes & FAT_ATTR_DIRECTORY)
            buffer[pos++] = '/';
          buffer[pos++] = '\n';
        }
      }
      c = fat_get_next_cluster(c);
    }
  }
  buffer[pos] = 0;
  return pos;
}

int fat_list_files_gui_dir(uint32_t dir_cluster, FileInfo *buffer,
                           int max_files) {
  uint8_t sbuf[512];
  int count = 0;
  if (buffer) {
    memset(buffer, 0, max_files * sizeof(FileInfo));
    char dbg[64];
    strcpy(dbg, "FAT: Listing dir, FileInfo size=");
    char num[16];
    k_itoa(sizeof(FileInfo), num);
    strcat(dbg, num);
    strcat(dbg, "\n");
    print_serial(dbg);
  }
  if (!is_fat32 && dir_cluster == FAT_ROOT_CLUSTER) {
    for (int s = 0; s < root_dir_sectors && count < max_files; s++) {
      fat_disk_read(first_root_dir_sector + s, sbuf);
      fat_dir_entry_t *es = (fat_dir_entry_t *)sbuf;
      for (int i = 0; i < 16 && count < max_files; i++) {
        if ((uint8_t)es[i].filename[0] == 0x00)
          return count;
        if ((uint8_t)es[i].filename[0] == 0xE5 ||
            (es[i].attributes & FAT_ATTR_VOLUME_ID))
          continue;
        int p = 0;
        for (int j = 0; j < 8; j++)
          if (es[i].filename[j] != ' ')
            buffer[count].name[p++] = es[i].filename[j];
        if (es[i].ext[0] != ' ') {
          buffer[count].name[p++] = '.';
          for (int j = 0; j < 3; j++)
            if (es[i].ext[j] != ' ')
              buffer[count].name[p++] = es[i].ext[j];
        }
        buffer[count].name[p] = 0;
        if (p > 0) {
          buffer[count].size = es[i].file_size;
          buffer[count].is_dir =
              (es[i].attributes & FAT_ATTR_DIRECTORY) ? 1 : 0;
          count++;
        }
      }
    }
  } else {
    uint32_t c = dir_cluster;
    uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;
    while (c >= 2 && c < eof && count < max_files) {
      uint32_t base = first_data_sector + (c - 2) * bpb.sectors_per_cluster;
      for (int s = 0; s < bpb.sectors_per_cluster && count < max_files; s++) {
        fat_disk_read(base + s, sbuf);
        fat_dir_entry_t *es = (fat_dir_entry_t *)sbuf;
        for (int i = 0; i < 16 && count < max_files; i++) {
          if ((uint8_t)es[i].filename[0] == 0x00)
            return count;
          if ((uint8_t)es[i].filename[0] == 0xE5 ||
              (es[i].attributes & FAT_ATTR_VOLUME_ID))
            continue;
          int p = 0;
          for (int j = 0; j < 8; j++)
            if (es[i].filename[j] != ' ')
              buffer[count].name[p++] = es[i].filename[j];
          if (es[i].ext[0] != ' ') {
            buffer[count].name[p++] = '.';
            for (int j = 0; j < 3; j++)
              if (es[i].ext[j] != ' ')
                buffer[count].name[p++] = es[i].ext[j];
          }
          buffer[count].name[p] = 0;
          if (p > 0) {
            buffer[count].is_dir =
                (es[i].attributes & FAT_ATTR_DIRECTORY) ? 1 : 0;
            count++;
          }
        }
      }
      c = fat_get_next_cluster(c);
    }
  }
  return count;
}

static int fat_vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size,
                        uint8_t *buffer) {
  if (offset == 0 && size >= node->length) {
    uint32_t cluster = node->inode;
    uint32_t bytes_left = node->length;
    uint32_t total_read = 0;
    uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;

    while (cluster >= 2 && cluster < eof && bytes_left > 0) {
      uint32_t sector =
          first_data_sector + (cluster - 2) * bpb.sectors_per_cluster;
      for (int i = 0; i < bpb.sectors_per_cluster && bytes_left > 0; i++) {
        uint8_t sbuf[512];
        if (!fat_disk_read(sector + i, sbuf))
          return total_read;
        uint32_t chunk = (bytes_left > 512) ? 512 : bytes_left;
        if (chunk > size - total_read)
          chunk = size - total_read;
        memcpy(buffer + total_read, sbuf, chunk);
        total_read += chunk;
        bytes_left -= chunk;
      }
      cluster = fat_get_next_cluster(cluster);
    }
    return total_read;
  }
  return -1;
}

static vfs_node_t *fat_get_vfs_node(fat_dir_entry_t *entry) {
  vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
  memset(node, 0, sizeof(vfs_node_t));
  int p = 0;
  for (int j = 0; j < 8; j++)
    if (entry->filename[j] != ' ')
      node->name[p++] = entry->filename[j];
  if (entry->ext[0] != ' ') {
    node->name[p++] = '.';
    for (int j = 0; j < 3; j++)
      if (entry->ext[j] != ' ')
        node->name[p++] = entry->ext[j];
  }
  node->name[p] = 0;
  node->length = entry->file_size;
  node->inode = get_entry_cluster(entry);
  node->flags =
      (entry->attributes & FAT_ATTR_DIRECTORY) ? VFS_DIRECTORY : VFS_FILE;
  extern vfs_driver_t fat_vfs_driver;
  node->driver = &fat_vfs_driver;
  return node;
}

static vfs_node_t *fat_vfs_readdir(vfs_node_t *node, uint32_t index) {
  if (!(node->flags & VFS_DIRECTORY))
    return 0;
  uint32_t dir_cluster = node->inode;
  uint8_t sbuf[512];
  uint32_t count = 0;
  if (!is_fat32 && dir_cluster == FAT_ROOT_CLUSTER) {
    for (int s = 0; s < root_dir_sectors; s++) {
      fat_disk_read(first_root_dir_sector + s, sbuf);
      fat_dir_entry_t *es = (fat_dir_entry_t *)sbuf;
      for (int i = 0; i < 16; i++) {
        if ((uint8_t)es[i].filename[0] == 0x00)
          return 0;
        if ((uint8_t)es[i].filename[0] == 0xE5 ||
            (es[i].attributes & FAT_ATTR_VOLUME_ID))
          continue;
        if (count == index)
          return fat_get_vfs_node(&es[i]);
        count++;
      }
    }
  } else {
    uint32_t c = dir_cluster;
    uint32_t eof = is_fat32 ? 0x0FFFFFF8 : 0xFFF8;
    while (c >= 2 && c < eof) {
      uint32_t base = first_data_sector + (c - 2) * bpb.sectors_per_cluster;
      for (int s = 0; s < bpb.sectors_per_cluster; s++) {
        fat_disk_read(base + s, sbuf);
        fat_dir_entry_t *es = (fat_dir_entry_t *)sbuf;
        for (int i = 0; i < 16; i++) {
          if ((uint8_t)es[i].filename[0] == 0x00)
            return 0;
          if ((uint8_t)es[i].filename[0] == 0xE5 ||
              (es[i].attributes & FAT_ATTR_VOLUME_ID))
            continue;
          if (count == index)
            return fat_get_vfs_node(&es[i]);
          count++;
        }
      }
      c = fat_get_next_cluster(c);
    }
  }
  return 0;
}

static vfs_node_t *fat_vfs_finddir(vfs_node_t *node, char *name) {
  if (!(node->flags & VFS_DIRECTORY))
    return 0;
  fat_dir_entry_t entry;
  if (raw_find_in_dir(node->inode, name, &entry)) {
    return fat_get_vfs_node(&entry);
  }
  return 0;
}

vfs_driver_t fat_vfs_driver = {"fat32_adaptive",
                               fat_vfs_read,
                               0, // write
                               0, // open
                               0, // close
                               fat_vfs_readdir,
                               fat_vfs_finddir};

int fat_copy_file(const char *src, const char *dst) {
  fat_dir_entry_t entry;
  if (!fat_find_file(src, &entry)) {
    return -1;
  }

  uint32_t size = entry.file_size;

  // file_data can be empty if size is 0
  uint8_t *file_data = (uint8_t *)kmalloc(size + 1);
  if (!file_data && size > 0) {
    return -1;
  }
  if (size > 0) {
    if (fat_read_file(src, file_data) == 0 && entry.file_size > 0) {
      kfree(file_data);
      return -1;
    }
  }

  if (fat_write_file(dst, file_data ? file_data : (uint8_t *)"", size) == 0) {
    if (file_data)
      kfree(file_data);
    return -1;
  }

  if (file_data)
    kfree(file_data);
  return 0;
}

int fat_move_file(const char *src, const char *dst) {
  // If same directory, we could just rename (not implemented yet)
  // For now, copy and delete
  if (fat_copy_file(src, dst) < 0)
    return -1;
  return fat_delete_file(src) == 1 ? 0 : -1;
}
uint32_t fat_get_total_size() {
  return (bpb.total_sectors_short != 0 ? bpb.total_sectors_short
                                       : bpb.total_sectors_long) *
         512;
}

uint32_t fat_get_used_size() {
  // Basic approximation for now
  static uint32_t cached_used = 0;
  static uint32_t call_count = 0;

  // Refresh every 100 calls to avoid linker issue with timer_ticks for now
  if (cached_used != 0 && (call_count++ % 100 != 0))
    return cached_used;

  // For now, just return a fake increasing value or a fixed percentage
  cached_used = (first_data_sector * 512) + (128 * 1024);
  return cached_used;
}
