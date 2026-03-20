// paging.h - 64-bit 4-level paging
#ifndef PAGING_H
#define PAGING_H

#include "isr.h"
#include <stdint.h>

typedef struct page {
  uint64_t present : 1;
  uint64_t rw : 1;
  uint64_t user : 1;
  uint64_t pwt : 1;
  uint64_t pcd : 1;
  uint64_t accessed : 1;
  uint64_t dirty : 1;
  uint64_t pat : 1;
  uint64_t global : 1;
  uint64_t unused : 3;
  uint64_t frame : 40;
  uint64_t unused2 : 11;
  uint64_t nx : 1;
} page_t;

typedef struct page_table {
  page_t pages[512];
} page_table_t;

typedef struct page_directory {
  uint64_t tables_physical[512]; // PD entries
} __attribute__((aligned(4096))) page_directory_t;

typedef struct page_directory_pointer {
  uint64_t directories_physical[512]; // PDPT entries
} __attribute__((aligned(4096))) page_directory_pointer_t;

typedef struct pml4_table {
  uint64_t pdpt_physical[512]; // PML4 entries
} __attribute__((aligned(4096))) pml4_table_t;

void paging_init();
void switch_page_directory(pml4_table_t *new_dir);
pml4_table_t *copy_page_directory(pml4_table_t *src);
void enter_user_mode(void *entry_point);
void return_to_kernel();
void page_fault_handler(registers_t *regs);
page_t *get_page(uint64_t address, int make, pml4_table_t *pml4);
pml4_table_t *paging_create_user_address_space();
void paging_map_user_page(pml4_table_t *pml4, uint64_t virtual_addr,
                          uint64_t physical_addr, int flags);

extern int in_user_mode;

#endif
