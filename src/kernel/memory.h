#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

void mem_init(uint32_t start, uint32_t size);
void* malloc(size_t size);
void free(void* ptr);

#endif
