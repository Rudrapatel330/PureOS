#include "mem.h"
#include "../kernel/string.h"
#include "heap.h"
#include "types.h"

/* Simple Placement Malloc for now.
 * A real allocator is complex. We'll implement a bump pointer allocator for
 * this stage.
 */

void mem_init(uint32_t start, uint32_t size) {
  // Stats initialization if needed.
  // The actual heap is initialized via heap_init() in kernel.c
  (void)start;
  (void)size;
}

uint32_t get_used_memory() { return heap_get_used_bytes(); }

uint32_t get_total_memory() { return heap_get_total_bytes(); }

void memory_copy(char *source, char *dest, int nbytes) {
  int i;
  for (i = 0; i < nbytes; i++) {
    *(dest + i) = *(source + i);
  }
}
