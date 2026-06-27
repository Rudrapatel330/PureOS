#include "ext2.h"
#include "../drivers/ata.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "vfs.h"

extern void print_serial(const char *);
extern void k_itoa(int, char *);
extern void k_itoa_hex(uint32_t, char *);

static ext2_superblock_t *sb = 0;
static ext2_bgroup_desc_t *bg_table = 0;
static uint32_t block_size = 0;
static int ext2_drive = -1; // 0 for ATA0_MASTER, 1 for ATA0_SLAVE, etc.

// Read an Ext2 block (can span multiple ATA sectors)
static int ext2_read_block(uint32_t block, uint8_t *buf) {
  if (!sb)
    return 0;
  uint32_t sectors_per_block = block_size / 512;
  uint32_t start_sector = block * sectors_per_block;

  // ext2_drive is an index into ata.c drives, assuming standard ATA access
  for (uint32_t i = 0; i < sectors_per_block; i++) {
    if (ext2_drive == 1) {
      extern int ata2_read_sector(uint32_t lba, uint8_t *buffer);
      ata2_read_sector(start_sector + i, buf + (i * 512));
    } else {
      extern int ata_read_sector(uint32_t lba, uint8_t *buffer);
      ata_read_sector(start_sector + i, buf + (i * 512));
    }
  }
  return 1;
}

static int ext2_read_inode(uint32_t inode_num, ext2_inode_t *out_inode) {
  if (inode_num == 0 || inode_num > sb->total_inodes)
    return 0;

  uint32_t bg_index = (inode_num - 1) / sb->inodes_per_bgroup;
  uint32_t index_in_bg = (inode_num - 1) % sb->inodes_per_bgroup;
  uint32_t inode_table_block = bg_table[bg_index].inode_table;

  uint32_t inode_size = 128; // Standard Ext2 inode size
  // For Ext2 v1+, it might be defined in extended SB. Hardcode 128 for now.

  uint32_t block_index = (index_in_bg * inode_size) / block_size;
  uint32_t offset_in_block = (index_in_bg * inode_size) % block_size;

  uint8_t *buf = kmalloc(block_size);
  if (!buf)
    return 0;

  if (!ext2_read_block(inode_table_block + block_index, buf)) {
    kfree(buf);
    return 0;
  }

  memcpy(out_inode, buf + offset_in_block, sizeof(ext2_inode_t));
  kfree(buf);
  return 1;
}

// Map a logical file block to a physical disk block
static uint32_t ext2_get_data_block(ext2_inode_t *inode,
                                    uint32_t logical_block) {
  uint32_t pointers_per_block = block_size / 4;

  // Direct blocks
  if (logical_block < 12) {
    return inode->dbp[logical_block];
  }
  logical_block -= 12;

  // Singly indirect
  if (logical_block < pointers_per_block) {
    if (!inode->sibp)
      return 0;
    uint32_t *buf = kmalloc(block_size);
    ext2_read_block(inode->sibp, (uint8_t *)buf);
    uint32_t ret = buf[logical_block];
    kfree(buf);
    return ret;
  }
  logical_block -= pointers_per_block;

  // Doubly indirect
  if (logical_block < pointers_per_block * pointers_per_block) {
    if (!inode->dibp)
      return 0;
    uint32_t *buf = kmalloc(block_size);
    ext2_read_block(inode->dibp, (uint8_t *)buf);
    uint32_t next_block = buf[logical_block / pointers_per_block];
    if (!next_block) {
      kfree(buf);
      return 0;
    }

    ext2_read_block(next_block, (uint8_t *)buf);
    uint32_t ret = buf[logical_block % pointers_per_block];
    kfree(buf);
    return ret;
  }

  // Triply indirect not supported for simplicity
  return 0;
}

// VFS driver: Read
static int ext2_vfs_read(vfs_inode_t *inode_obj, file_handle_t *file, uint32_t offset, uint32_t size,
                         uint8_t *buffer) {
  ext2_inode_t inode;
  if (!ext2_read_inode(inode_obj->inode_no, &inode))
    return -1;

  if (offset >= inode.size_lower)
    return 0;
  if (offset + size > inode.size_lower) {
    size = inode.size_lower - offset;
  }

  uint8_t *bounce_buf = kmalloc(block_size);
  uint32_t bytes_read = 0;

  while (bytes_read < size) {
    uint32_t logical_block = (offset + bytes_read) / block_size;
    uint32_t off_in_block = (offset + bytes_read) % block_size;

    uint32_t phys_block = ext2_get_data_block(&inode, logical_block);
    if (phys_block) {
      ext2_read_block(phys_block, bounce_buf);
    } else {
      // Sparse block, zero it out
      memset(bounce_buf, 0, block_size);
    }

    uint32_t to_copy = block_size - off_in_block;
    if (to_copy > size - bytes_read)
      to_copy = size - bytes_read;

    memcpy(buffer + bytes_read, bounce_buf + off_in_block, to_copy);
    bytes_read += to_copy;
  }

  kfree(bounce_buf);
  return bytes_read;
}

// Parse directory to find a name or return entry by index
static vfs_dentry_t *ext2_parse_dir(vfs_inode_t *dir_inode, const char *find_name,
                                  uint32_t get_index) {
  ext2_inode_t inode;
  if (!ext2_read_inode(dir_inode->inode_no, &inode))
    return 0;

  if ((inode.type_perm & EXT2_S_IFDIR) == 0)
    return 0;

  uint8_t *dir_buf = kmalloc(block_size);
  uint32_t offset = 0;
  uint32_t logical_block = 0;
  uint32_t current_index = 0;

  vfs_dentry_t *ret_dentry = 0;

  while (offset < inode.size_lower) {
    if (offset % block_size == 0) {
      uint32_t pb = ext2_get_data_block(&inode, logical_block++);
      if (pb)
        ext2_read_block(pb, dir_buf);
      else {
        offset += block_size;
        continue;
      }
    }

    ext2_dirent_t *ent = (ext2_dirent_t *)(dir_buf + (offset % block_size));
    if (ent->inode == 0 || ent->entry_size == 0) {
      offset += block_size - (offset % block_size); // jump to next block
      continue;
    }

    char name[256];
    memcpy(name, dir_buf + (offset % block_size) + 8, ent->name_len);
    name[ent->name_len] = 0;

    // Match condition
    int match = 0;
    if (find_name) {
      if (strcmp(name, find_name) == 0)
        match = 1;
    } else {
      if (current_index == get_index)
        match = 1;
    }

    if (match) {
      ret_dentry = kmalloc(sizeof(vfs_dentry_t));
      memset(ret_dentry, 0, sizeof(vfs_dentry_t));
      extern inode_operations_t ext2_inode_ops;
      extern file_operations_t ext2_file_ops;
      
      vfs_inode_t *cached_inode = icache_lookup(&ext2_inode_ops, ent->inode);
      
      strcpy(ret_dentry->name, name);

      vfs_inode_t *ret_inode = 0;
      if (cached_inode) {
        ret_inode = cached_inode;
        ret_inode->refcount++;
      } else {
        ret_inode = kmalloc(sizeof(vfs_inode_t));
        memset(ret_inode, 0, sizeof(vfs_inode_t));
        ret_inode->inode_no = ent->inode;

        // Need to read the inode to get size and type
        ext2_inode_t t_inode;
        ext2_read_inode(ent->inode, &t_inode);
        ret_inode->size = t_inode.size_lower;
        ret_inode->uid = t_inode.uid;
        ret_inode->gid = t_inode.gid;
        ret_inode->mode = t_inode.type_perm & 0xFFF;

        if (t_inode.type_perm & EXT2_S_IFDIR) {
          ret_inode->mode |= VFS_DIRECTORY;
        } else {
          ret_inode->mode |= VFS_FILE;
        }

        ret_inode->i_ops = &ext2_inode_ops;
        ret_inode->f_ops = &ext2_file_ops;
        ret_inode->refcount = 1;
        
        icache_add(&ext2_inode_ops, ret_inode);
      }
      
      ret_dentry->inode = ret_inode;
      break;
    }

    current_index++;
    offset += ent->entry_size;
  }

  kfree(dir_buf);
  return ret_dentry;
}

static vfs_dentry_t *ext2_vfs_finddir(vfs_inode_t *inode, const char *name) {
  return ext2_parse_dir(inode, name, 0);
}

static vfs_dentry_t *ext2_vfs_readdir(vfs_inode_t *inode, uint32_t index) {
  return ext2_parse_dir(inode, 0, index);
}

file_operations_t ext2_file_ops = {
  ext2_vfs_read,
  0,
  0,
  0,
  ext2_vfs_readdir
};

inode_operations_t ext2_inode_ops = {
  ext2_vfs_finddir,
  0,
  0,
  0,
  0,  // create
  0   // rename
};

int ext2_init(void) {
  print_serial("EXT2: Initializing read-only driver...\n");

  // Check ATA drives for Ext2 signature
  for (int d = 0; d < 2; d++) {
    uint8_t buffer[1024];
    print_serial("EXT2: Checking ATA drive ");
    char d_str[4];
    k_itoa(d, d_str);
    print_serial(d_str);
    print_serial("...\n");
    if (d == 1) {
      extern int ata2_detect(void);
      if (!ata2_detect()) {
        print_serial("EXT2: ATA2 not detected.\n");
        continue;
      }
      extern int ata2_read_sector(uint32_t lba, uint8_t *buffer);
      ata2_read_sector(2, buffer);
      ata2_read_sector(3, buffer + 512);
    } else {
      extern int ata_read_sector(uint32_t lba, uint8_t *buffer);
      ata_read_sector(2, buffer);
      ata_read_sector(3, buffer + 512);
    }
    // Ext2 sb is 1024 bytes, spans LBA 2 and 3 potentially

    // Wait, normally SB is at offset 1024. If sector size is 512:
    // LBA 0 (0-511): MBR or Boot block
    // LBA 1 (512-1023): Boot block part 2
    // LBA 2 (1024-1535): Superblock part 1
    // LBA 3 (1536-2047): Superblock part 2
    ext2_superblock_t *tsb = (ext2_superblock_t *)buffer;

    if (tsb->ext2_signature == 0xEF53) {
      print_serial("EXT2: Found valid filesystem on ATA drive ");
      char num[16];
      k_itoa(d, num);
      print_serial(num);
      print_serial("\n");

      ext2_drive = d;
      sb = kmalloc(sizeof(ext2_superblock_t));
      memcpy(sb, tsb, sizeof(ext2_superblock_t));

      block_size = 1024 << sb->log2_block_size;

      // Read Block Group Descriptor Table
      // It starts at the block following the superblock.
      uint32_t bgdt_block =
          (sb->superblock_block == 0) ? 2 : (sb->superblock_block + 1);
      if (block_size > 1024)
        bgdt_block = 1; // if block size > 1KB, SB is in block 0

      uint32_t total_bgroups = (sb->total_blocks + sb->blocks_per_bgroup - 1) /
                               sb->blocks_per_bgroup;

      uint32_t bgdt_size = total_bgroups * sizeof(ext2_bgroup_desc_t);
      bg_table = kmalloc(bgdt_size);

      uint32_t bgdt_blocks = (bgdt_size + block_size - 1) / block_size;
      uint8_t *bgdt_buf = kmalloc(bgdt_blocks * block_size);

      for (uint32_t b = 0; b < bgdt_blocks; b++) {
        ext2_read_block(bgdt_block + b, bgdt_buf + (b * block_size));
      }

      memcpy(bg_table, bgdt_buf, bgdt_size);
      kfree(bgdt_buf);

      // Mount Ext2 root
      vfs_inode_t *root_inode = kmalloc(sizeof(vfs_inode_t));
      memset(root_inode, 0, sizeof(vfs_inode_t));
      root_inode->inode_no = 2; // Ext2 root is always inode 2
      root_inode->mode = VFS_DIRECTORY | 0755;
      root_inode->i_ops = &ext2_inode_ops;
      root_inode->f_ops = &ext2_file_ops;

      vfs_dentry_t *root_dentry = kmalloc(sizeof(vfs_dentry_t));
      memset(root_dentry, 0, sizeof(vfs_dentry_t));
      strcpy(root_dentry->name, "ext2");
      root_dentry->flags = VFS_DIRECTORY;
      root_dentry->inode = root_inode;

      vfs_mount("/ext2", root_dentry);
      return 1;
    }
  }

  print_serial("EXT2: No filesystem found on any ATA drive.\n");
  return 0;
}
