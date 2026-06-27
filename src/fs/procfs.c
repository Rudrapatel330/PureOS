#include "vfs.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/task.h"

extern void print_serial(const char *);

static vfs_dentry_t *procfs_root = 0;
extern inode_operations_t procfs_inode_ops;
extern file_operations_t procfs_file_ops;

static int procfs_read(vfs_inode_t *inode, file_handle_t *file, uint32_t offset,
                       uint32_t size, uint8_t *buffer) {
  (void)file;

  if (inode->impl == 1) { // meminfo
    char info[512];
    extern uint32_t get_total_memory();
    extern uint32_t get_used_memory();
    extern void k_itoa(int, char *);

    uint32_t total = get_total_memory();
    uint32_t used = get_used_memory();
    uint32_t free_mem = total - used;

    char total_s[16], used_s[16], free_s[16];
    k_itoa(total / 1024, total_s);
    k_itoa(used / 1024, used_s);
    k_itoa(free_mem / 1024, free_s);

    strcpy(info, "MemTotal:  ");
    strcat(info, total_s);
    strcat(info, " kB\n");
    strcat(info, "MemUsed:   ");
    strcat(info, used_s);
    strcat(info, " kB\n");
    strcat(info, "MemFree:   ");
    strcat(info, free_s);
    strcat(info, " kB\n");

    uint32_t len = strlen(info);
    if (offset >= len)
      return 0;
    if (offset + size > len)
      size = len - offset;
    memcpy(buffer, info + offset, size);
    return size;
  } else if (inode->impl == 2) { // cpuinfo
    char info[] = "processor\t: 0\n"
                  "vendor_id\t: PureOS_CPU\n"
                  "cpu family\t: 6\n"
                  "model\t\t: 1\n"
                  "model name\t: PureOS Virtual CPU\n";
    uint32_t len = strlen(info);
    if (offset >= len)
      return 0;
    if (offset + size > len)
      size = len - offset;
    memcpy(buffer, info + offset, size);
    return size;
  } else if (inode->impl == 3) { // uptime
    extern uint32_t get_timer_ticks();
    extern void k_itoa(int, char *);
    char info[64];
    uint32_t ticks = get_timer_ticks();
    uint32_t seconds = ticks / 250; // 250Hz PIT timer
    char sec_s[16];
    k_itoa(seconds, sec_s);
    strcpy(info, sec_s);
    strcat(info, " seconds\n");

    uint32_t len = strlen(info);
    if (offset >= len)
      return 0;
    if (offset + size > len)
      size = len - offset;
    memcpy(buffer, info + offset, size);
    return size;
  } else if (inode->impl == 4) { // version
    char info[] = "PureOS version 1.0 (VFS Sprint 6)\n";
    uint32_t len = strlen(info);
    if (offset >= len)
      return 0;
    if (offset + size > len)
      size = len - offset;
    memcpy(buffer, info + offset, size);
    return size;
  }
  return 0;
}

static vfs_dentry_t *create_proc_node(const char *name, int impl) {
  vfs_inode_t *inode = kmalloc(sizeof(vfs_inode_t));
  memset(inode, 0, sizeof(vfs_inode_t));
  inode->mode = VFS_FILE | 0444;
  inode->impl = impl;
  inode->i_ops = &procfs_inode_ops;
  inode->f_ops = &procfs_file_ops;
  inode->size = 4096; // virtual file, size is dynamic

  vfs_dentry_t *dentry = kmalloc(sizeof(vfs_dentry_t));
  memset(dentry, 0, sizeof(vfs_dentry_t));
  strcpy(dentry->name, name);
  dentry->inode = inode;
  return dentry;
}

#define PROCFS_ENTRY_COUNT 4

static vfs_dentry_t *procfs_readdir(vfs_inode_t *inode, uint32_t index) {
  if (inode->impl == 0) { // root
    if (index == 0)
      return create_proc_node("meminfo", 1);
    if (index == 1)
      return create_proc_node("cpuinfo", 2);
    if (index == 2)
      return create_proc_node("uptime", 3);
    if (index == 3)
      return create_proc_node("version", 4);
  }
  return 0;
}

static vfs_dentry_t *procfs_finddir(vfs_inode_t *inode, const char *name) {
  if (inode->impl == 0) {
    if (strcmp(name, "meminfo") == 0)
      return create_proc_node("meminfo", 1);
    if (strcmp(name, "cpuinfo") == 0)
      return create_proc_node("cpuinfo", 2);
    if (strcmp(name, "uptime") == 0)
      return create_proc_node("uptime", 3);
    if (strcmp(name, "version") == 0)
      return create_proc_node("version", 4);
  }
  return 0;
}

file_operations_t procfs_file_ops = {procfs_read, 0, // no write
                                     0, 0, procfs_readdir};

inode_operations_t procfs_inode_ops = {procfs_finddir, 0, 0, 0, 0, 0};

void procfs_init() {
  vfs_inode_t *root_inode = kmalloc(sizeof(vfs_inode_t));
  memset(root_inode, 0, sizeof(vfs_inode_t));
  root_inode->mode = VFS_DIRECTORY | 0555;
  root_inode->impl = 0; // root node ID
  root_inode->i_ops = &procfs_inode_ops;
  root_inode->f_ops = &procfs_file_ops;

  procfs_root = kmalloc(sizeof(vfs_dentry_t));
  memset(procfs_root, 0, sizeof(vfs_dentry_t));
  strcpy(procfs_root->name, "proc");
  procfs_root->inode = root_inode;

  vfs_mount("/proc", procfs_root);
  print_serial("PROCFS: mounted at /proc\n");
}
