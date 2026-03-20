#ifndef EXT2_H
#define EXT2_H

#include "../kernel/types.h"
#include "vfs.h"

// Ext2 Superblock (at offset 1024)
typedef struct {
  uint32_t total_inodes;
  uint32_t total_blocks;
  uint32_t reserved_blocks;
  uint32_t unallocated_blocks;
  uint32_t unallocated_inodes;
  uint32_t
      superblock_block; // Block number of the block containing the superblock
  uint32_t log2_block_size; // Block size = 1024 << log2_block_size
  uint32_t log2_frag_size;
  uint32_t blocks_per_bgroup;
  uint32_t frags_per_bgroup;
  uint32_t inodes_per_bgroup;
  uint32_t last_mount_time;
  uint32_t last_written_time;
  uint16_t mount_count_since_fsck;
  uint16_t mounts_allowed_before_fsck;
  uint16_t ext2_signature; // Must be 0xEF53
  uint16_t fs_state;
  uint16_t error_handling_method;
  uint16_t minor_version;
  uint32_t last_fsck_time;
  uint32_t forced_fsck_interval;
  uint32_t creator_os_id;
  uint32_t major_version;
  uint16_t res_uid;
  uint16_t res_gid;
  // Extended Superblock fields omitted for simplicity since we are read-only
} __attribute__((packed)) ext2_superblock_t;

// Block Group Descriptor
typedef struct {
  uint32_t block_usage_bitmap;
  uint32_t inode_usage_bitmap;
  uint32_t inode_table;
  uint16_t unallocated_blocks;
  uint16_t unallocated_inodes;
  uint16_t dirs_count;
  uint8_t padding[14];
} __attribute__((packed)) ext2_bgroup_desc_t;

// Inode
typedef struct {
  uint16_t type_perm;
  uint16_t uid;
  uint32_t size_lower;
  uint32_t last_access;
  uint32_t creation_time;
  uint32_t last_mod;
  uint32_t deletion_time;
  uint16_t gid;
  uint16_t hard_links;
  uint32_t disk_sectors;
  uint32_t flags;
  uint32_t os_val1;
  uint32_t dbp[12]; // Direct block pointers
  uint32_t sibp;    // Singly indirect
  uint32_t dibp;    // Doubly indirect
  uint32_t tibp;    // Triply indirect
  uint32_t gen_num;
  uint32_t extended_attr;
  uint32_t size_upper;
  uint32_t frag_addr;
  uint8_t os_val2[12];
} __attribute__((packed)) ext2_inode_t;

// Directory Entry
typedef struct {
  uint32_t inode;
  uint16_t entry_size;
  uint8_t name_len;
  uint8_t type;
  // name follows (variable length)
} __attribute__((packed)) ext2_dirent_t;

// File Types in Directory Entry
#define EXT2_FT_UNKNOWN 0
#define EXT2_FT_FILE 1
#define EXT2_FT_DIR 2
#define EXT2_FT_CHRDEV 3
#define EXT2_FT_BLKDEV 4
#define EXT2_FT_FIFO 5
#define EXT2_FT_SOCK 6
#define EXT2_FT_SYMLINK 7

// Inode Type/Perm Flags
#define EXT2_S_IFMT 0xF000
#define EXT2_S_IFSOCK 0xC000
#define EXT2_S_IFLNK 0xA000
#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFBLK 0x6000
#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFCHR 0x2000
#define EXT2_S_IFIFO 0x1000

int ext2_init(void);

#endif
