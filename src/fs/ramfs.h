#ifndef RAMFS_H
#define RAMFS_H

#include "fs.h"

int ramfs_init();
int ramfs_read(const char* name, uint8_t* buffer);
int ramfs_write(const char* name, const uint8_t* content, uint32_t size);
int ramfs_list(char* buffer, int max_len);
int ramfs_delete(const char* name);
int ramfs_list_files(FileInfo* buffer, int max_files);
void ramfs_add_binary_file(const char* name, const unsigned char* data, unsigned int size);

#endif
