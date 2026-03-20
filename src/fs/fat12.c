#include "fat12.h"
#include "../drivers/ata.h"
#include "../drivers/ports.h" // For print_serial
#include "../kernel/heap.h"
#include "../kernel/memory.h"
#include "../kernel/string.h"
#include "fs.h"
#include <stdint.h>


// Debug wrappers
#define print_str print_serial
void print_char(char c) {
  char s[2] = {c, 0};
  print_serial(s);
}

// Globals
static uint8_t disk_buffer_sector[512];
static fat12_bpb_t bpb;
uint32_t fat_start_sector;
uint32_t root_start_sector;
uint32_t data_start_sector;
uint32_t root_sectors;

int fs_ready = 0;
uint16_t cwd_cluster = 0; // 0 = Root
char current_path_str[256] = "/";

void fs_pwd(char *buffer) { strcpy(buffer, current_path_str); }

// Helper: Get next cluster from FAT
uint16_t fat12_get_next_cluster(uint16_t cluster) {
  // Offset in FAT (1.5 bytes per entry)
  uint32_t fat_offset = cluster + (cluster / 2);
  uint32_t fat_sector = fat_start_sector + (fat_offset / 512);
  uint32_t fat_ent_offset = fat_offset % 512;

  ata_read_sector(fat_sector, disk_buffer_sector);

  uint16_t next_cluster;
  if (cluster & 1) {
    next_cluster = (disk_buffer_sector[fat_ent_offset] >> 4);
    if (fat_ent_offset + 1 < 512)
      next_cluster |= (disk_buffer_sector[fat_ent_offset + 1] << 4);
    // Handle split across sectors? (Simplified: Ignore for now)
  } else {
    next_cluster = disk_buffer_sector[fat_ent_offset];
    if (fat_ent_offset + 1 < 512)
      next_cluster |= ((disk_buffer_sector[fat_ent_offset + 1] & 0x0F) << 8);
  }
  return next_cluster;
}

void fs_init() {
  if (fs_ready)
    return;

  // Read Sector 0 (BPB)
  if (!ata_read_sector(0, disk_buffer_sector)) {
    print_str("[FS] Disk Read Error on Sector 0\n");
    return;
  }

  // Copy BPB
  fat12_bpb_t *read_bpb = (fat12_bpb_t *)disk_buffer_sector;

  // Check Signature (0x55 0xAA at end of sector, valid boot sector)
  if (disk_buffer_sector[510] != 0x55 || disk_buffer_sector[511] != 0xAA) {
    print_str("[FS] No Boot Signature found.\n");
    return;
  }

  // Calculate Offsets
  fat_start_sector = read_bpb->reserved_sectors;
  root_start_sector =
      fat_start_sector + (read_bpb->fat_count * read_bpb->sectors_per_fat);

  // Root Data Size
  root_sectors = (read_bpb->root_dir_entries * 32) / 512;
  if ((read_bpb->root_dir_entries * 32) % 512 != 0)
    root_sectors++;

  data_start_sector = root_start_sector + root_sectors;

  // print_str("[FS] FAT12 Init OK.\n"); // Silent
  fs_ready = 1;
}

// Rename conflicting fs_list
void fs_print_debug() {
  if (!fs_ready)
    fs_init();
  if (!fs_ready)
    return;

  print_str("Directory of ");
  print_str(current_path_str);
  print_str("\n");

  int found = 0;

  uint16_t cluster = cwd_cluster;
  int sector_idx = 0;

  while (1) {
    // Read Sector
    if (cwd_cluster == 0) {
      if (sector_idx >= root_sectors)
        break;
      ata_read_sector(root_start_sector + sector_idx, disk_buffer_sector);
      sector_idx++;
    } else {
      // Cluster based
      uint32_t lba = data_start_sector + (cluster - 2);
      ata_read_sector(lba, disk_buffer_sector);
    }

    fat_dir_entry_t *dir = (fat_dir_entry_t *)disk_buffer_sector;

    for (int j = 0; j < 16; j++) {
      if ((unsigned char)dir[j].name[0] == 0x00)
        goto done_list;
      if ((unsigned char)dir[j].name[0] == 0xE5)
        continue;
      if (dir[j].attributes == 0x0F)
        continue;

      // Print Name
      char name[12];
      int len = 0;
      for (int k = 0; k < 8; k++) {
        if (dir[j].name[k] == ' ')
          break;
        name[len++] = dir[j].name[k];
      }
      if (dir[j].ext[0] != ' ') {
        name[len++] = '.';
        for (int k = 0; k < 3; k++) {
          if (dir[j].ext[k] == ' ')
            break;
          name[len++] = dir[j].ext[k];
        }
      }
      name[len] = 0;

      print_str(name);

      // Padding
      int pad = 12 - len;
      for (int p = 0; p < pad; p++)
        print_char(' ');

      if (dir[j].attributes & 0x10)
        print_str("  <DIR>\n");
      else
        print_str("  FILE\n");
      found++;
    }

    // Advance
    if (cwd_cluster != 0) {
      cluster = fat12_get_next_cluster(cluster);
      if (cluster >= 0xFF8)
        break;
    }
  }

done_list:
  if (found == 0)
    print_str("(Empty)\n");
}

int fs_list_files(const char *path, FileInfo *buffer, int max_files) {
  (void)path; // Ignore for now as FAT12 uses global CWD cluster
  if (!fs_ready)
    fs_init();
  if (!fs_ready)
    return 0;

  int count = 0;
  uint16_t cluster = cwd_cluster;
  int sector_idx = 0;

  while (1) {
    if (cwd_cluster == 0) {
      if (sector_idx >= root_sectors)
        break;
      ata_read_sector(root_start_sector + sector_idx, disk_buffer_sector);
      sector_idx++;
    } else {
      uint32_t lba = data_start_sector + (cluster - 2);
      ata_read_sector(lba, disk_buffer_sector);
    }

    fat_dir_entry_t *directory = (fat_dir_entry_t *)disk_buffer_sector;

    for (int j = 0; j < 16; j++) {
      if ((unsigned char)directory[j].name[0] == 0x00)
        return count; // End of Dir
      if ((unsigned char)directory[j].name[0] == 0xE5)
        continue; // Deleted
      if (directory[j].attributes == 0x0F)
        continue; // LFN
      if (directory[j].attributes & 0x08)
        continue; // Volume Label

      // Limit check
      if (count >= max_files)
        return count;

      // Copy Name
      char name[13];
      int len = 0;
      for (int k = 0; k < 8; k++) {
        char c = directory[j].name[k];
        if (c == ' ')
          continue;
        // Basic Alphanumeric + common symbols check
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ||
            c == '-') {
          name[len++] = c;
        }
      }
      // If empty name here, skip
      if (len == 0 && directory[j].name[0] != '.')
        continue;

      if (directory[j].ext[0] != ' ') {
        name[len++] = '.';
        for (int k = 0; k < 3; k++) {
          char c = directory[j].ext[k];
          if (c == ' ')
            break;
          if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            name[len++] = c;
          }
        }
      }
      name[len] = 0;

      // Fill Buffer
      int ni = 0;
      while (name[ni]) {
        buffer[count].name[ni] = name[ni];
        ni++;
      }
      buffer[count].name[ni] = 0;

      buffer[count].size = directory[j].size;
      buffer[count].is_dir = (directory[j].attributes & 0x10) ? 1 : 0;

      count++;
    }

    if (cwd_cluster != 0) {
      cluster = fat12_get_next_cluster(cluster);
      if (cluster >= 0xFF8)
        break;
    }
  }
  return count;
}

int fs_read(const char *filename, uint8_t *buffer) {
  if (!fs_ready)
    fs_init();
  if (!fs_ready)
    return 0;

  // 1. Search Root Dir for Filename
  // Need to format local filename to 8.3 space padded to match
  // Simplified: Just naive substring match or implement parser.
  // For now: Assume exact match of mapped standard name?
  // Let's iterate and reconstruct name to compare.

  // 1. Search in current dir
  uint16_t cluster = cwd_cluster;
  int sector_idx = 0;

  while (1) {
    if (cwd_cluster == 0) {
      if (sector_idx >= root_sectors)
        break;
      ata_read_sector(root_start_sector + sector_idx, disk_buffer_sector);
      sector_idx++;
    } else {
      uint32_t lba = data_start_sector + (cluster - 2);
      ata_read_sector(lba, disk_buffer_sector);
    }

    fat_dir_entry_t *dir = (fat_dir_entry_t *)disk_buffer_sector;

    for (int j = 0; j < 16; j++) {
      if ((unsigned char)dir[j].name[0] == 0x00)
        return 0;
      if ((unsigned char)dir[j].name[0] == 0xE5)
        continue;
      if (dir[j].attributes == 0x0F)
        continue;

      // Reconstruct Name
      char name[13];
      int len = 0;
      for (int k = 0; k < 8; k++) {
        if (dir[j].name[k] == ' ')
          break;
        name[len++] = dir[j].name[k];
      }
      if (dir[j].ext[0] != ' ') {
        name[len++] = '.';
        for (int k = 0; k < 3; k++) {
          if (dir[j].ext[k] == ' ')
            break;
          name[len++] = dir[j].ext[k];
        }
      }
      name[len] = 0;

      // Compare
      int match = 1;
      char *input = (char *)filename;
      char *entry = name;
      while (*input && *entry) {
        char a = *input;
        char b = *entry;
        if (a >= 'a' && a <= 'z')
          a -= 32;
        if (b >= 'a' && b <= 'z')
          b -= 32;
        if (a != b) {
          match = 0;
          break;
        }
        input++;
        entry++;
      }
      if (match && (*input == 0 && *entry == 0)) {
        // FOUND!
        uint16_t cluster = dir[j].first_cluster_low;
        uint32_t cluster_lba = data_start_sector + (cluster - 2);
        ata_read_sector(cluster_lba, buffer);
        return 1;
      }
    }

    if (cwd_cluster != 0) {
      cluster = fat12_get_next_cluster(cluster);
      if (cluster >= 0xFF8)
        break;
    }
  }
  return 0; // Not Found
}
// Helper to write FAT entry
void fat_write_entry(uint16_t cluster, uint16_t value) {
  // Calculate offset
  uint32_t fat_offset = cluster + (cluster / 2); // 1.5 bytes per entry
  uint32_t fat_sector = fat_start_sector + (fat_offset / 512);
  uint32_t fat_ent_offset = fat_offset % 512;

  // Read FAT sector
  ata_read_sector(fat_sector, disk_buffer_sector);

  // Update Entry
  if (cluster & 1) { // Odd
    disk_buffer_sector[fat_ent_offset] =
        (disk_buffer_sector[fat_ent_offset] & 0x0F) | ((value << 4) & 0xF0);
    // We might span sectors, but ignoring that edge case for now (simplicity)
    if (fat_ent_offset + 1 < 512)
      disk_buffer_sector[fat_ent_offset + 1] = (value >> 4) & 0xFF;
  } else { // Even
    disk_buffer_sector[fat_ent_offset] = value & 0xFF;
    if (fat_ent_offset + 1 < 512)
      disk_buffer_sector[fat_ent_offset + 1] =
          (disk_buffer_sector[fat_ent_offset + 1] & 0xF0) |
          ((value >> 8) & 0x0F);
  }

  // Write back
  ata_write_sector(fat_sector, disk_buffer_sector);
  // TODO: Write to FAT2 as well? Skipping for speed/simplicity.
}

int fs_write(const char *filename, const uint8_t *buffer, uint32_t size) {
  if (!fs_ready)
    fs_init();
  if (!fs_ready)
    return 0;

  // 0. Check if file exists -> Overwrite
  // Reuse fs_read logic to find entry? Or just iterate.
  // Iterating is safer to get the exact entry index and cluster.

  uint16_t search_cluster = cwd_cluster;
  int sector_idx = 0;

  // Search Loop
  while (1) {
    uint32_t lba;
    if (cwd_cluster == 0) {
      if (sector_idx >= root_sectors)
        break;
      lba = root_start_sector + sector_idx;
    } else {
      lba = data_start_sector + (search_cluster - 2);
    }

    ata_read_sector(lba, disk_buffer_sector);
    fat_dir_entry_t *dir = (fat_dir_entry_t *)disk_buffer_sector;

    for (int j = 0; j < 16; j++) {
      if ((unsigned char)dir[j].name[0] == 0x00)
        break; // End of Dir
      if ((unsigned char)dir[j].name[0] == 0xE5)
        continue; // Deleted
      if (dir[j].attributes == 0x0F)
        continue; // LFN
      if (dir[j].attributes & 0x10)
        continue; // Skip Dirs

      char name[13];
      int len = 0;
      for (int k = 0; k < 8; k++) {
        if (dir[j].name[k] == ' ')
          break;
        name[len++] = dir[j].name[k];
      }
      if (dir[j].ext[0] != ' ') {
        name[len++] = '.';
        for (int k = 0; k < 3; k++) {
          if (dir[j].ext[k] == ' ')
            break;
          name[len++] = dir[j].ext[k];
        }
      }
      name[len] = 0;

      int match = 1;
      const char *a = filename;
      const char *b = name;
      while (*a && *b) {
        char ca = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
        char cb = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
        if (ca != cb) {
          match = 0;
          break;
        }
        a++;
        b++;
      }
      if (match && !*a && !*b) {
        // FOUND EXISTING FILE!
        // Overwrite logic
        uint16_t cluster = dir[j].first_cluster_low;
        uint32_t cluster_lba = data_start_sector + (cluster - 2);

        // Write Data (Single Cluster limit for now)
        static uint8_t write_buf[512];
        memset(write_buf, 0, 512);
        uint32_t copy_size = size > 512 ? 512 : size;
        for (uint32_t k = 0; k < copy_size; k++)
          write_buf[k] = buffer[k];

        ata_write_sector(cluster_lba, write_buf);

        // Update Size
        dir[j].size = size;
        ata_write_sector(lba, disk_buffer_sector); // Write Dir Entry back
        return 1;
      }
    }

    if (cwd_cluster == 0) {
      sector_idx++;
    } else {
      search_cluster = fat12_get_next_cluster(search_cluster);
      if (search_cluster >= 0xFF8)
        break;
    }
  }

  // 1. Find Free Entry in CWD (Create New)
  uint32_t dir_lba = 0;
  int dir_entry_idx = -1;

  uint16_t lp_cluster = cwd_cluster;
  int lp_sector = 0;

  while (1) {
    if (cwd_cluster == 0) {
      if (lp_sector >= root_sectors)
        break;
      dir_lba = root_start_sector + lp_sector;
    } else {
      dir_lba = data_start_sector + (lp_cluster - 2);
    }

    ata_read_sector(dir_lba, disk_buffer_sector);
    fat_dir_entry_t *dir = (fat_dir_entry_t *)disk_buffer_sector;

    for (int j = 0; j < 16; j++) {
      if ((unsigned char)dir[j].name[0] == 0x00 ||
          (unsigned char)dir[j].name[0] == 0xE5) {
        dir_entry_idx = j;
        goto found_slot;
      }
    }

    if (cwd_cluster == 0) {
      lp_sector++;
    } else {
      lp_cluster = fat12_get_next_cluster(lp_cluster);
      if (lp_cluster >= 0xFF8)
        break;
    }
  }

found_slot:
  if (dir_entry_idx == -1) {
    print_str("FS: Directory Full.\n");
    return 0;
  }

  // 2. Find Free Cluster (Scan FAT)
  uint16_t free_cluster = 0;
  ata_read_sector(fat_start_sector, disk_buffer_sector);

  // Skip 0 and 1
  for (int c = 2; c < 128; c++) {
    // Read entry c
    uint32_t offset = c + (c / 2);
    uint16_t val;
    if (c & 1) {
      val = (disk_buffer_sector[offset] >> 4) |
            (disk_buffer_sector[offset + 1] << 4);
    } else {
      val = disk_buffer_sector[offset] |
            ((disk_buffer_sector[offset + 1] & 0x0F) << 8);
    }

    if ((val & 0xFFF) == 0) {
      free_cluster = c;
      break;
    }
  }

  if (free_cluster == 0) {
    print_str("FS: Disk Full.\n");
    return 0;
  }

  // 3. Write Data to Cluster
  uint32_t cluster_lba = data_start_sector + (free_cluster - 2);
  static uint8_t write_buf[512];
  memset(write_buf, 0, 512);
  int copy_size = size > 512 ? 512 : size;
  for (int k = 0; k < copy_size; k++)
    write_buf[k] = buffer[k];

  ata_write_sector(cluster_lba, write_buf);

  // 4. Update FAT
  fat_write_entry(free_cluster, 0xFFF);

  // 5. Update Directory
  ata_read_sector(dir_lba, disk_buffer_sector);
  fat_dir_entry_t *dir = (fat_dir_entry_t *)disk_buffer_sector;

  // Parse Filename (Simple: "NAME    EXT")
  memset(dir[dir_entry_idx].name, ' ', 8);
  memset(dir[dir_entry_idx].ext, ' ', 3);

  int ni = 0, di = 0;
  // Name
  while (filename[ni] && filename[ni] != '.' && di < 8) {
    char c = filename[ni++];
    if (c >= 'a' && c <= 'z')
      c -= 32;
    dir[dir_entry_idx].name[di++] = c;
  }
  // Skip to dot
  while (filename[ni] && filename[ni] != '.')
    ni++;
  if (filename[ni] == '.')
    ni++;
  // Ext
  di = 0;
  while (filename[ni] && di < 3) {
    char c = filename[ni++];
    if (c >= 'a' && c <= 'z')
      c -= 32;
    dir[dir_entry_idx].ext[di++] = c;
  }

  dir[dir_entry_idx].attributes = 0x20; // Archive
  dir[dir_entry_idx].first_cluster_low = free_cluster;
  dir[dir_entry_idx].size = size;

  // Write Dir Sector Back
  ata_write_sector(dir_lba, disk_buffer_sector);

  return 1;
}
// DELETE FILE
int fs_delete(const char *filename) {
  if (!fs_ready)
    fs_init();

  uint16_t search_cluster = cwd_cluster;
  int sector_idx = 0;

  while (1) {
    uint32_t lba;
    if (cwd_cluster == 0) {
      if (sector_idx >= root_sectors)
        break;
      lba = root_start_sector + sector_idx;
      sector_idx++;
    } else {
      lba = data_start_sector + (search_cluster - 2);
    }

    ata_read_sector(lba, disk_buffer_sector);
    fat_dir_entry_t *dir = (fat_dir_entry_t *)disk_buffer_sector;

    for (int j = 0; j < 16; j++) {
      if ((unsigned char)dir[j].name[0] == 0x00)
        return 0;
      if ((unsigned char)dir[j].name[0] == 0xE5)
        continue;

      char name[13];
      int len = 0;
      for (int k = 0; k < 8; k++) {
        if (dir[j].name[k] == ' ')
          break;
        name[len++] = dir[j].name[k];
      }
      if (dir[j].ext[0] != ' ') {
        name[len++] = '.';
        for (int k = 0; k < 3; k++) {
          if (dir[j].ext[k] == ' ')
            break;
          name[len++] = dir[j].ext[k];
        }
      }
      name[len] = 0;

      int match = 1;
      const char *a = filename;
      const char *b = name;
      while (*a && *b) {
        char ca = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
        char cb = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
        if (ca != cb) {
          match = 0;
          break;
        }
        a++;
        b++;
      }
      if (match && !*a && !*b) {
        uint16_t cluster = dir[j].first_cluster_low;
        dir[j].name[0] = 0xE5;
        ata_write_sector(lba, disk_buffer_sector);

        while (cluster >= 2 && cluster < 0xFF0) {
          uint16_t next_cluster = fat12_get_next_cluster(cluster);
          fat_write_entry(cluster, 0x000);
          cluster = next_cluster;
        }
        return 1;
      }
    }

    if (cwd_cluster != 0) {
      search_cluster = fat12_get_next_cluster(search_cluster);
      if (search_cluster >= 0xFF8)
        break;
    }
  }
  return 0; // Not Found
}

// RENAME FILE
int fs_rename(const char *old_name, const char *new_name) {
  if (!fs_ready)
    fs_init();

  // Check if new name exists? (Skipped for brevity, usually should check)

  uint16_t search_cluster = cwd_cluster;
  int sector_idx = 0;

  while (1) {
    uint32_t lba;
    if (cwd_cluster == 0) {
      if (sector_idx >= root_sectors)
        break;
      lba = root_start_sector + sector_idx;
      sector_idx++;
    } else {
      lba = data_start_sector + (search_cluster - 2);
    }

    ata_read_sector(lba, disk_buffer_sector);
    fat_dir_entry_t *dir = (fat_dir_entry_t *)disk_buffer_sector;

    for (int j = 0; j < 16; j++) {
      if ((unsigned char)dir[j].name[0] == 0x00)
        return 0;
      if ((unsigned char)dir[j].name[0] == 0xE5)
        continue;

      char name[13];
      int len = 0;
      for (int k = 0; k < 8; k++) {
        if (dir[j].name[k] == ' ')
          break;
        name[len++] = dir[j].name[k];
      }
      if (dir[j].ext[0] != ' ') {
        name[len++] = '.';
        for (int k = 0; k < 3; k++) {
          if (dir[j].ext[k] == ' ')
            break;
          name[len++] = dir[j].ext[k];
        }
      }
      name[len] = 0;

      int match = 1;
      const char *a = old_name;
      const char *b = name;
      while (*a && *b) {
        char ca = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
        char cb = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
        if (ca != cb) {
          match = 0;
          break;
        }
        a++;
        b++;
      }
      if (match && !*a && !*b) {
        // UPDATE NAME
        memset(dir[j].name, ' ', 8);
        memset(dir[j].ext, ' ', 3);

        int ni = 0, di = 0;
        while (new_name[ni] && new_name[ni] != '.' && di < 8) {
          char c = new_name[ni++];
          if (c >= 'a' && c <= 'z')
            c -= 32;
          dir[j].name[di++] = c;
        }
        while (new_name[ni] && new_name[ni] != '.')
          ni++;
        if (new_name[ni] == '.')
          ni++;
        di = 0;
        while (new_name[ni] && di < 3) {
          char c = new_name[ni++];
          if (c >= 'a' && c <= 'z')
            c -= 32;
          dir[j].ext[di++] = c;
        }
        ata_write_sector(lba, disk_buffer_sector);
        return 1;
      }
    }

    if (cwd_cluster != 0) {
      search_cluster = fat12_get_next_cluster(search_cluster);
      if (search_cluster >= 0xFF8)
        break;
    }
  }
  return 0; // Not Found
}

// CHANGE DIRECTORY
int fs_cd(const char *path) {
  if (!fs_ready)
    fs_init();

  // 1. Search in current dir
  uint16_t cluster = cwd_cluster;
  int sector_idx = 0;

  while (1) {
    if (cwd_cluster == 0) {
      if (sector_idx >= root_sectors)
        break;
      ata_read_sector(root_start_sector + sector_idx, disk_buffer_sector);
      sector_idx++;
    } else {
      uint32_t lba = data_start_sector + (cluster - 2);
      ata_read_sector(lba, disk_buffer_sector);
    }

    fat_dir_entry_t *dir = (fat_dir_entry_t *)disk_buffer_sector;
    for (int j = 0; j < 16; j++) {
      if ((unsigned char)dir[j].name[0] == 0x00)
        return 0; // Not Found
      if ((unsigned char)dir[j].name[0] == 0xE5)
        continue;

      // Construct Name
      char name[13];
      int len = 0;
      for (int k = 0; k < 8; k++) {
        if (dir[j].name[k] == ' ')
          break;
        name[len++] = dir[j].name[k];
      }
      if (dir[j].ext[0] != ' ') {
        name[len++] = '.';
        for (int k = 0; k < 3; k++) {
          if (dir[j].ext[k] == ' ')
            break;
          name[len++] = dir[j].ext[k];
        }
      }
      name[len] = 0;

      // Check Match
      int match = 1;
      const char *a = path;
      const char *b = name;
      while (*a && *b) {
        char ca = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
        char cb = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
        if (ca != cb) {
          match = 0;
          break;
        }
        a++;
        b++;
      }
      if (match && !*a && !*b) {
        // Found!
        if (dir[j].attributes & 0x10) {
          // Update CWD
          cwd_cluster = dir[j].first_cluster_low;

          // Update String (Basic)
          if (path[0] == '.' && path[1] == '.' && path[2] == 0) {
            // strip last segment
            int l = strlen(current_path_str);
            while (l > 0 && current_path_str[l] != '/')
              l--;
            if (l == 0)
              l = 1; // keep /
            current_path_str[l] = 0;
          } else if (path[0] != '.') {
            if (strcmp(current_path_str, "/") != 0)
              strcat(current_path_str, "/");
            strcat(current_path_str, name);
          }
          return 1;
        } else {
          print_str("Not a directory.\n");
          return 0;
        }
      }
    }

    if (cwd_cluster != 0) {
      cluster = fat12_get_next_cluster(cluster);
      if (cluster >= 0xFF8)
        break;
    }
  }
  return 0;
}

int fs_mkdir(const char *name) {
  if (!fs_ready)
    fs_init();

  // 1. Check if name exists
  // (Skipped for brevity, assume user knows what they are doing or relying on
  // free entry check) Actually, should check duplication.

  // 2. Find Free Cluster
  uint16_t free_cluster = 0;
  ata_read_sector(fat_start_sector, disk_buffer_sector);
  for (int c = 2; c < 128; c++) {
    // Read entry c
    uint32_t offset = c + (c / 2);
    uint16_t val;
    if (c & 1) {
      val = (disk_buffer_sector[offset] >> 4) |
            (disk_buffer_sector[offset + 1] << 4);
    } else {
      val = disk_buffer_sector[offset] |
            ((disk_buffer_sector[offset + 1] & 0x0F) << 8);
    }
    if ((val & 0xFFF) == 0) {
      free_cluster = c;
      break;
    }
  }
  if (free_cluster == 0) {
    print_str("FS: Disk Full.\n");
    return 0;
  }

  // 3. Initialize New Cluster (Zero out)
  static uint8_t sector_buf[512];
  memset(sector_buf, 0, 512);

  // Add "." and ".." entries
  fat_dir_entry_t *dir = (fat_dir_entry_t *)sector_buf;

  // Entry 0: "."
  memset(dir[0].name, ' ', 11);
  dir[0].name[0] = '.';
  dir[0].attributes = 0x10;
  dir[0].first_cluster_low = free_cluster;

  // Entry 1: ".."
  memset(dir[1].name, ' ', 11);
  dir[1].name[0] = '.';
  dir[1].name[1] = '.';
  dir[1].attributes = 0x10;
  dir[1].first_cluster_low = cwd_cluster; // Parent

  // Write Init Sector
  uint32_t cluster_lba = data_start_sector + (free_cluster - 2);
  ata_write_sector(cluster_lba, sector_buf);

  // Mark FAT
  fat_write_entry(free_cluster, 0xFFF);

  // 4. Add Entry to Parent (CWD)
  uint16_t p_cluster = cwd_cluster;
  int p_sector_idx = 0;
  int entry_found = 0;

  while (1) {
    uint32_t lba;
    if (cwd_cluster == 0) {
      if (p_sector_idx >= root_sectors)
        break;
      lba = root_start_sector + p_sector_idx;
    } else {
      lba = data_start_sector + (p_cluster - 2);
    }

    ata_read_sector(lba, disk_buffer_sector);
    fat_dir_entry_t *p_dir = (fat_dir_entry_t *)disk_buffer_sector;

    for (int j = 0; j < 16; j++) {
      if ((unsigned char)p_dir[j].name[0] == 0x00 ||
          (unsigned char)p_dir[j].name[0] == 0xE5) {
        // Found Free Slot
        memset(p_dir[j].name, ' ', 11);

        // Parse Name (8.3 rule appplies? Directory usually just Name)
        int ni = 0, di = 0;
        while (name[ni] && di < 8) {
          char c = name[ni++];
          if (c >= 'a' && c <= 'z')
            c -= 32;
          p_dir[j].name[di++] = c;
        }
        // No extension for directories usually, but FAT allows it.
        // We'll stick to name-only for mkdir logic.

        p_dir[j].attributes = 0x10;
        p_dir[j].first_cluster_low = free_cluster;
        p_dir[j].size = 0;

        ata_write_sector(lba, disk_buffer_sector);
        return 1;
      }
    }

    if (cwd_cluster == 0) {
      p_sector_idx++;
    } else {
      // Next cluster
      // TODO: If full, allocate new cluster for parent?
      // Skipping simplify.
      p_cluster = fat12_get_next_cluster(p_cluster);
      if (p_cluster >= 0xFF8)
        break;
    }
  }

  print_str("FS: Directory Full.\n");
  return 0;
}
