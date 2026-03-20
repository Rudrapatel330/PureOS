#ifndef MEM_H
#define MEM_H

#include "types.h"
#include <stddef.h>

void mem_init(uint32_t start, uint32_t size);
void *malloc(size_t size);
void free(void *ptr);

uint32_t get_used_memory();
uint32_t get_total_memory();

void memory_copy(char *source, char *dest, int nbytes);

#endif
