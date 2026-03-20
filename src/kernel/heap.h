#ifndef HEAP_H
#define HEAP_H

#include "types.h"

// Define size_t if not available
#include <stddef.h>

// Heap Configuration
#define HEAP_START 0x4000000 // Start at 64MB (Safe for 34MB+ kernel)
#define HEAP_SIZE 0x1C000000 // 448MB Size (Safe for VMs, ends at 464MB)
#define MIN_BLOCK_SIZE 16    // Alignment

// Magic numbers for corruption detection
#define BLOCK_MAGIC_USED 0xDEADBEEF
#define BLOCK_MAGIC_FREE 0xFEEDFACE

void heap_init();
void *kmalloc(size_t size);
void *kmalloc_ap(size_t size, uint32_t *phys);
void *krealloc(void *ptr, size_t new_size);
void kfree(void *ptr);

// Standard C wrappers
void *malloc(size_t size);
void *realloc(void *ptr, size_t new_size);
void free(void *ptr);

// Debug - prints heap stats to serial
void heap_stats();
void heap_audit();
uint32_t heap_get_used_bytes();
uint32_t heap_get_total_bytes();

#endif
