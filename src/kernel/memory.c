#include "memory.h"

// Simple Linked List Allocator
struct block_header {
    size_t size;
    uint8_t is_free;
    struct block_header* next;
};

#define HEADER_SIZE sizeof(struct block_header)

static struct block_header* head = NULL;

void mem_init(uint32_t start, uint32_t size) {
    head = (struct block_header*)start;
    head->size = size - HEADER_SIZE;
    head->is_free = 1;
    head->next = NULL;
}

void* malloc(size_t size) {
    struct block_header* curr = head;
    
    while(curr) {
        if (curr->is_free && curr->size >= size) {
            // Found a block
            // Split if large enough
            if (curr->size > size + HEADER_SIZE + 32) { // Minimum split size
                struct block_header* new_block = (struct block_header*)((uint8_t*)curr + HEADER_SIZE + size);
                new_block->size = curr->size - size - HEADER_SIZE;
                new_block->is_free = 1;
                new_block->next = curr->next;
                
                curr->size = size;
                curr->next = new_block;
            }
            
            curr->is_free = 0;
            return (void*)((uint8_t*)curr + HEADER_SIZE);
        }
        curr = curr->next;
    }
    
    return NULL; // OOM
}

void free(void* ptr) {
    if (!ptr) return;
    
    struct block_header* header = (struct block_header*)((uint8_t*)ptr - HEADER_SIZE);
    header->is_free = 1;
    
    // Coalesce (Merge) free blocks
    struct block_header* curr = head;
    while(curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += curr->next->size + HEADER_SIZE;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}
