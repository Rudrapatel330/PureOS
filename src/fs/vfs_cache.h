#ifndef VFS_CACHE_H
#define VFS_CACHE_H

#include "vfs.h"

// --- Dentry Cache (dcache) ---
void dcache_init();
vfs_dentry_t *dcache_lookup(vfs_dentry_t *parent, const char *name);
void dcache_add(vfs_dentry_t *parent, vfs_dentry_t *dentry);
void dget(vfs_dentry_t *dentry);
void dput(vfs_dentry_t *dentry);

// --- Inode Cache (icache) ---
// Since we don't have superblocks, we use driver pointer and inode_no
void icache_init();
vfs_inode_t *icache_lookup(void *driver_ops, uint32_t inode_no);
void icache_add(void *driver_ops, vfs_inode_t *inode);
void iget(vfs_inode_t *inode);
void iput(vfs_inode_t *inode);

// --- Buffer Cache (bcache) ---
void bcache_init();
int bcache_read_block(int drive, uint32_t block_no, uint8_t *buffer, uint32_t block_size);
void bcache_write_block(int drive, uint32_t block_no, const uint8_t *buffer, uint32_t block_size);

#endif
