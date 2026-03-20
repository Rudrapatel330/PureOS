#include "heap.h"
#include "../kernel/string.h"
#include "spinlock.h"
#include "types.h"

extern void print_serial(const char *);

static spinlock_irq_t heap_lock = {0};
static uint32_t heap_used_bytes = 0; // Running counter for O(1) usage queries

// Block Header
typedef struct block_t {
  uint32_t magic;       // BLOCK_MAGIC_USED or BLOCK_MAGIC_FREE
  uint32_t free;        // 1 = free, 0 = used
  struct block_t *next; // Phys next
  struct block_t *prev; // Phys prev (for O(1) coalescing)
  size_t size;          // Size of data part
  uint64_t reserved[2]; // Padding for 16-byte alignment (Total = 48 bytes)
} block_t;

// Free node structure (stored in data area of free blocks)
typedef struct free_node_t {
  struct block_t *next;
  struct block_t *prev;
} free_node_t;

#define NUM_BUCKETS 8
static block_t *buckets[NUM_BUCKETS];
static block_t *physical_head = (block_t *)HEAP_START;

// Helper: check if a pointer is within valid heap range
static int is_heap_ptr(void *ptr) {
  uintptr_t p = (uintptr_t)ptr;
  return (p >= (uintptr_t)HEAP_START &&
          p < (uintptr_t)(HEAP_START + HEAP_SIZE));
}

// Helper: get bucket index for a given size
static int get_bucket(size_t size) {
  if (size <= 32)
    return 0;
  if (size <= 64)
    return 1;
  if (size <= 128)
    return 2;
  if (size <= 256)
    return 3;
  if (size <= 512)
    return 4;
  if (size <= 1024)
    return 5;
  if (size <= 2048)
    return 6;
  return 7; // 4096 and up
}

#include "assert.h"

#define BLOCK_CANARY 0xC0FFEE42

// Helper: validate a block's magic value
static int block_valid(block_t *b) {
  if (b->magic != BLOCK_MAGIC_USED && b->magic != BLOCK_MAGIC_FREE) {
    print_serial("HEAP: Invalid magic!\n");
    return 0;
  }
  if (!b->free) {
    uint32_t *canary = (uint32_t *)((uint8_t *)b + sizeof(block_t) + b->size -
                                    sizeof(uint32_t));
    if (*canary != BLOCK_CANARY) {
      print_serial("HEAP: Canary corruption detected at block 0x");
      char hex[20];
      k_itoa_hex((uint64_t)b, hex);
      print_serial(hex);
      print_serial("\n");
      return 0; // Will trigger panic if checked
    }
  }
  return 1;
}

void heap_audit() {
  spinlock_irq_acquire(&heap_lock);
  block_t *current = physical_head;
  while (current) {
    if (!block_valid(current)) {
      KASSERT(0);
    }
    current = current->next;
  }
  spinlock_irq_release(&heap_lock);
}

// Helper: Add a FREE block to the appropriate segregated list
static void add_to_free_list(block_t *b) {
  int bucket = get_bucket(b->size);
  if (bucket < 0 || bucket >= NUM_BUCKETS) {
    print_serial("HEAP: Invalid bucket index in add!\n");
    return;
  }

  free_node_t *node = (free_node_t *)((uint8_t *)b + sizeof(block_t));

  node->next = buckets[bucket];
  node->prev = 0;

  if (buckets[bucket]) {
    if (!is_heap_ptr(buckets[bucket])) {
      print_serial(
          "HEAP: add_to_free_list clobbered by non-heap bucket head!\n");
      buckets[bucket] = 0;
      return;
    }
    free_node_t *next_node =
        (free_node_t *)((uint8_t *)buckets[bucket] + sizeof(block_t));
    next_node->prev = b;
  }
  buckets[bucket] = b;
}

// Helper: Remove a block from its segregated list
static void remove_from_free_list(block_t *b) {
  int bucket = get_bucket(b->size);
  if (bucket < 0 || bucket >= NUM_BUCKETS) {
    print_serial("HEAP: Invalid bucket index in remove!\n");
    return;
  }

  free_node_t *node = (free_node_t *)((uint8_t *)b + sizeof(block_t));

  if (node->prev) {
    if (!is_heap_ptr(node->prev)) {
      print_serial("HEAP: remove_from_free_list: prev is NOT a heap pointer! "
                   "Ignoring.\n");
      return;
    }
    free_node_t *prev_node =
        (free_node_t *)((uint8_t *)node->prev + sizeof(block_t));
    prev_node->next = node->next;
  } else {
    buckets[bucket] = node->next;
  }

  if (node->next) {
    if (!is_heap_ptr(node->next)) {
      print_serial("HEAP: remove_from_free_list: next is NOT a heap pointer! "
                   "Ignoring.\n");
      return;
    }
    free_node_t *next_node =
        (free_node_t *)((uint8_t *)node->next + sizeof(block_t));
    next_node->prev = node->prev;
  }
}

void heap_init() {
  // Clear buckets
  for (int i = 0; i < NUM_BUCKETS; i++)
    buckets[i] = 0;

  // Initialize the first huge block covering the whole heap
  // Ensure the block starts at a 16-byte aligned boundary
  physical_head = (block_t *)((((uintptr_t)HEAP_START) + 15) & ~15);
  physical_head->magic = BLOCK_MAGIC_FREE;
  physical_head->size =
      HEAP_SIZE - ((uintptr_t)physical_head - HEAP_START) - sizeof(block_t);
  physical_head->free = 1;
  physical_head->next = 0;
  physical_head->prev = 0;

  // Add to free list
  add_to_free_list(physical_head);

  print_serial("HEAP INITIALIZED (448MB O(1))\n");
}

void *kmalloc(size_t size) {
  if (size == 0)
    return 0;

  spinlock_irq_acquire(&heap_lock);

  // Align size and account for footer canary
  // We need enough space for the user data + 4 byte canary,
  // and the NEXT block header must still be aligned to MIN_BLOCK_SIZE.
  size_t total_needed =
      (size + sizeof(uint32_t) + MIN_BLOCK_SIZE - 1) & ~(MIN_BLOCK_SIZE - 1);

  // Search each bucket starting from the best fit
  for (int i = get_bucket(total_needed); i < NUM_BUCKETS; i++) {
    block_t *current = buckets[i];

    while (current) {
      KASSERT(block_valid(current));

      if (current->free && current->size >= total_needed) {
        // Found a block!
        remove_from_free_list(current);

        // Check if we can split it
        if (current->size > total_needed + sizeof(block_t) + MIN_BLOCK_SIZE) {
          // Split
          block_t *new_block =
              (block_t *)((uint8_t *)current + sizeof(block_t) + total_needed);
          new_block->magic = BLOCK_MAGIC_FREE;
          new_block->size = current->size - total_needed - sizeof(block_t);
          new_block->free = 1;

          // Phys chain
          new_block->next = current->next;
          new_block->prev = current;
          if (current->next)
            current->next->prev = new_block;
          current->next = new_block;

          current->size = total_needed;

          // Add remainder to free list
          add_to_free_list(new_block);
        }

        current->free = 0;
        current->magic = BLOCK_MAGIC_USED;

        // Write canary at the VERY end of the allocated data portion
        uint32_t *canary = (uint32_t *)((uint8_t *)current + sizeof(block_t) +
                                        current->size - sizeof(uint32_t));
        *canary = BLOCK_CANARY;

        heap_used_bytes += current->size;
        spinlock_irq_release(&heap_lock);
        return (void *)((uint8_t *)current + sizeof(block_t));
      }

      // Follow the free list next
      free_node_t *fn = (free_node_t *)((uint8_t *)current + sizeof(block_t));
      current = fn->next;
    }
  }

  print_serial("MALLOC FAILED: OOM\n");
  spinlock_irq_release(&heap_lock);
  return 0; // OOM
}

void *kmalloc_ap(size_t size, uint32_t *phys) {
  // Allocate extra to ensure we can find a page-aligned region within
  void *ptr = kmalloc(size + 4096);
  if (!ptr)
    return 0;
  uint32_t addr = (uint32_t)(uintptr_t)ptr;
  uint32_t aligned = (addr + 0xFFF) & ~0xFFF;
  if (phys)
    *phys = aligned;
  // Return a pointer to the page-aligned region within the allocation.
  // IMPORTANT: This pointer must NOT be passed to kfree() — only the
  // original kmalloc pointer should be freed. Since kmalloc_ap is only used
  // for kernel-lifetime page tables and DMA buffers, this is safe.
  return (void *)(uintptr_t)aligned;
}

void *krealloc(void *ptr, size_t new_size) {
  if (!ptr)
    return kmalloc(new_size);
  if (new_size == 0) {
    kfree(ptr);
    return 0;
  }

  uint64_t rflags;
  __asm__ volatile("pushfq; popq %0; cli" : "=r"(rflags));

  // Get old block header
  block_t *block = (block_t *)((uint8_t *)ptr - sizeof(block_t));
  if (!block_valid(block)) {
    print_serial("HEAP: krealloc on corrupted block!\n");
    __asm__ volatile("pushq %0; popfq" : : "r"(rflags));
    return 0;
  }

  size_t old_size = block->size;

  // We must calculate the total block size needed for the new request
  size_t new_total = (new_size + sizeof(uint32_t) + MIN_BLOCK_SIZE - 1) &
                     ~(MIN_BLOCK_SIZE - 1);

  // If the new requirements still fit in the current physical block, just
  // return it. This avoids unnecessary copies and fragmentation.
  if (new_total <= old_size) {
    __asm__ volatile("pushq %0; popfq" : : "r"(rflags));
    return ptr;
  }

  __asm__ volatile("pushq %0; popfq" : : "r"(rflags));

  // Allocate new, copy, free old
  void *new_ptr = kmalloc(new_size);
  if (!new_ptr)
    return 0;

  // Copy old data - only up to either the old size or the new size
  size_t copy_size = (old_size < new_size) ? old_size : new_size;
  memcpy(new_ptr, ptr, copy_size);

  kfree(ptr);
  return new_ptr;
}

void kfree(void *ptr) {
  if (!ptr)
    return;

  spinlock_irq_acquire(&heap_lock);

  // Get block header
  block_t *block = (block_t *)((uint8_t *)ptr - sizeof(block_t));

  // Validate magic and footer canary before freeing
  if (block->magic == BLOCK_MAGIC_FREE) {
    print_serial("HEAP: CRITICAL: Double free detected at 0x");
    char hex[20];
    k_itoa_hex((uint64_t)block, hex);
    print_serial(hex);
    print_serial("\n");
    KASSERT(0);
  }
  KASSERT(block_valid(block));
  KASSERT(block->magic == BLOCK_MAGIC_USED);

  if (block->size > sizeof(free_node_t)) {
    // Heap Scrubbing: Clear data with 0xDE to catch use-after-free
    memset((uint8_t *)ptr, 0xDE, block->size);
  }

  block->free = 1;
  block->magic = BLOCK_MAGIC_FREE;
  heap_used_bytes -= block->size;

  // Coalesce with NEXT physical block if free
  if (block->next && block->next->free) {
    if (block_valid(block->next)) {
      remove_from_free_list(block->next);
      block->size += sizeof(block_t) + block->next->size;
      block->next = block->next->next;
      if (block->next)
        block->next->prev = block;
    }
  }

  // Coalesce with PREVIOUS physical block if free
  if (block->prev && block->prev->free) {
    if (block_valid(block->prev)) {
      block_t *p = block->prev;
      remove_from_free_list(p);
      p->size += sizeof(block_t) + block->size;
      p->next = block->next;
      if (block->next)
        block->next->prev = p;
      block = p;
    }
  }

  add_to_free_list(block);

  spinlock_irq_release(&heap_lock);
}

void *malloc(size_t size) { return kmalloc(size); }
void *realloc(void *ptr, size_t new_size) { return krealloc(ptr, new_size); }
void *calloc(size_t nmemb, size_t size) {
  size_t total = nmemb * size;
  void *ptr = kmalloc(total);
  if (ptr)
    memset(ptr, 0, total);
  return ptr;
}
void free(void *ptr) { kfree(ptr); }

static void print_serial_num(uint32_t n) {
  char buf[12];
  int i = 10;
  buf[11] = 0;
  if (n == 0) {
    buf[10] = '0';
    i = 9;
  } else {
    while (n > 0 && i >= 0) {
      buf[i--] = '0' + (n % 10);
      n /= 10;
    }
  }
  print_serial(&buf[i + 1]);
}

void heap_stats() {
  spinlock_irq_acquire(&heap_lock);
  block_t *current = physical_head;
  uint32_t total_blocks = 0;
  uint32_t free_blocks = 0;
  uint32_t used_bytes = 0;
  uint32_t free_bytes = 0;
  uint32_t corrupted = 0;

  while (current) {
    if (!block_valid(current)) {
      corrupted++;
      break;
    }
    total_blocks++;
    if (current->free) {
      free_blocks++;
      free_bytes += current->size;
    } else {
      used_bytes += current->size;
    }
    current = current->next;
  }

  print_serial("HEAP: blocks=");
  print_serial_num(total_blocks);
  print_serial(" used=");
  print_serial_num(used_bytes / 1024);
  print_serial("KB free=");
  print_serial_num(free_bytes / 1024);
  print_serial("KB");
  if (corrupted) {
    print_serial(" CORRUPTED!");
  }
  print_serial("\n");
  spinlock_irq_release(&heap_lock);
}

uint32_t heap_get_used_bytes() { return heap_used_bytes; }

uint32_t heap_get_total_bytes() { return HEAP_SIZE; }
