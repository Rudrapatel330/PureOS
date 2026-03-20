// paging.c - 64-bit
#include "paging.h"
#include "../../drivers/ports.h"
#include "../heap.h"
#include "../string.h"
#include "../syscall.h"
#include "../task.h"
#include "isr.h"

extern void print_serial(const char *);

pml4_table_t *kernel_pml4 = 0;
uint64_t kernel_pml4_phys = 0;
pml4_table_t *current_pml4 = 0;

static uint64_t kernel_rsp;
static uint64_t kernel_rbp;
static uint8_t user_stack_array[4096] __attribute__((aligned(16)));
int in_user_mode = 0;

page_t *get_page(uint64_t address, int make, pml4_table_t *pml4) {
  address /= 0x1000;
  uint64_t pml4_idx = (address >> 27) & 0x1FF;
  uint64_t pdpt_idx = (address >> 18) & 0x1FF;
  uint64_t pd_idx = (address >> 9) & 0x1FF;
  uint64_t pt_idx = address & 0x1FF;

  if (!(pml4->pdpt_physical[pml4_idx] & 0x1)) {
    if (!make)
      return 0;
    uint32_t phys;
    page_directory_pointer_t *pdpt = (page_directory_pointer_t *)kmalloc_ap(
        sizeof(page_directory_pointer_t), &phys);
    memset(pdpt, 0, sizeof(page_directory_pointer_t));
    pml4->pdpt_physical[pml4_idx] = phys | 0x7;
  }

  // Identity Map (0-1GB) allows using physical address as virtual pointer
  page_directory_pointer_t *pdpt =
      (page_directory_pointer_t *)(uintptr_t)(pml4->pdpt_physical[pml4_idx] &
                                              ~0xFFF);

  if (!(pdpt->directories_physical[pdpt_idx] & 0x1)) {
    if (!make)
      return 0;
    uint32_t phys;
    page_directory_t *pd =
        (page_directory_t *)kmalloc_ap(sizeof(page_directory_t), &phys);
    memset(pd, 0, sizeof(page_directory_t));
    pdpt->directories_physical[pdpt_idx] = phys | 0x7;
  }

  page_directory_t *pd =
      (page_directory_t *)(uintptr_t)(pdpt->directories_physical[pdpt_idx] &
                                      ~0xFFF);

  if (!(pd->tables_physical[pd_idx] & 0x1)) {
    if (!make)
      return 0;
    uint32_t phys;
    page_table_t *pt = (page_table_t *)kmalloc_ap(sizeof(page_table_t), &phys);
    memset(pt, 0, sizeof(page_table_t));
    pd->tables_physical[pd_idx] = phys | 0x7;
  }

  page_table_t *pt =
      (page_table_t *)(uintptr_t)(pd->tables_physical[pd_idx] & ~0xFFF);
  return &pt->pages[pt_idx];
}

void paging_init() {
  print_serial("PAGING FIX V2: Initializing 4-level paging...\n");

  uint32_t phys_pml4;
  kernel_pml4 = (pml4_table_t *)kmalloc_ap(sizeof(pml4_table_t), &phys_pml4);
  memset(kernel_pml4, 0, sizeof(pml4_table_t));
  kernel_pml4_phys = phys_pml4;

  print_serial("PAGING: Identity mapping first 1GB...\n");
  for (uint64_t i = 0; i < 0x40000000; i += 0x1000) {
    if ((i % 0x2000000) == 0) { // Every 32MB
      print_serial(".");
    }
    page_t *page = get_page(i, 1, kernel_pml4);
    page->present = 1;
    page->rw = 1;
    page->user = 1;
    page->frame = i >> 12;
  }
  print_serial(" Done.\n");

  print_serial("PAGING: Mapping kernel to 0xC0000000 (128MB)...\n");
  for (uint64_t i = 0; i < 0x8000000; i += 0x1000) {
    page_t *page = get_page(0xC0000000 + i, 1, kernel_pml4);
    page->present = 1;
    page->rw = 1;
    page->user = 0;
    page->frame = i >> 12;
  }

  print_serial("PAGING: Mapping APIC/MMIO (0xF0000000+)...\n");
  for (uint64_t i = 0xF0000000; i < 0x100000000ULL; i += 0x1000) {
    page_t *page = get_page(i, 1, kernel_pml4);
    page->present = 1;
    page->rw = 1;
    page->user = 0;
    page->frame = i >> 12;
  }

  // Map common LFB ranges to prevent Page Faults during BGA/PCI detection
  // 1. Map the address provided by bootloader (at 0x6000)
  uint32_t *fb_ptr = (uint32_t *)0x6000;
  uint64_t lfb_phys = *fb_ptr;
  if (lfb_phys != 0 && lfb_phys != 0xFFFFFFFF) {
    print_serial("PAGING: Mapping Bootloader LFB...\n");
    for (uint64_t i = 0; i < 0x2000000; i += 0x1000) { // Map 32MB
      page_t *page = get_page(lfb_phys + i, 1, kernel_pml4);
      page->present = 1;
      page->rw = 1;
      page->user = 1;
      page->frame = (lfb_phys + i) >> 12;
    }
  }

  // 2. Map default VBOX/QEMU LFB range (0xE0000000) just in case
  print_serial("PAGING: Mapping Default High VRAM Range (128MB)...\n");
  for (uint64_t i = 0; i < 0x8000000; i += 0x1000) { // Map 128MB
    page_t *page = get_page(0xE0000000 + i, 1, kernel_pml4);
    page->present = 1;
    page->rw = 1;
    page->user = 1;
    page->frame = (0xE0000000 + i) >> 12;
  }

  current_pml4 = kernel_pml4;
  __asm__ volatile("mov %0, %%cr3" : : "r"((uint64_t)phys_pml4));
  print_serial("[POST-CR3] PAGING ENABLED\n");
}

void switch_page_directory(pml4_table_t *new_dir) {
  current_pml4 = new_dir;
  // Use identity mapping to get physical address
  uint64_t phys = (uint64_t)(uintptr_t)new_dir;
  __asm__ volatile("mov %0, %%cr3" : : "r"(phys));
}

pml4_table_t *copy_page_directory(pml4_table_t *src) {
  uint32_t phys;
  pml4_table_t *dir = (pml4_table_t *)kmalloc_ap(sizeof(pml4_table_t), &phys);
  memset(dir, 0, sizeof(pml4_table_t));

  // Copy kernel mappings (PDPT[0] in our simplified model contains kernel +
  // basic user) Actually, let's just use paging_create_user_address_space() as
  // a base
  pml4_table_t *new_pml4 = paging_create_user_address_space();

  // Now we need to iterate over the source and duplicate any USER pages.
  // This is a slow, primitive fork but it gets us started.
  // We only care about addresses < 0xC0000000 (kernel start)
  for (uint64_t addr = 0; addr < 0xC0000000; addr += 0x1000) {
    page_t *src_page = get_page(addr, 0, src);
    if (src_page && src_page->present && src_page->user) {
      // Duplicate this page
      uint32_t new_phys;
      void *new_buf = kmalloc_ap(4096, &new_phys);

      // We need to temporarily map the src page to read it, or use the fact
      // that our identity mapping covers the first 1GB? Our current paging_init
      // maps first 1GB identity. Most user apps are in that range or the stack
      // is at 0x70000000. If addr is in our identity range, we can just memcpy.
      memcpy(new_buf, (void *)(uintptr_t)(src_page->frame << 12), 4096);

      paging_map_user_page(new_pml4, addr, new_phys, src_page->rw ? 0x2 : 0);
    }
  }

  return new_pml4;
}

pml4_table_t *paging_create_user_address_space() {
  uint32_t phys;
  pml4_table_t *pml4 = (pml4_table_t *)kmalloc_ap(sizeof(pml4_table_t), &phys);
  memset(pml4, 0, sizeof(pml4_table_t));

  // Copy kernel mappings (PDPT[0] contains identity and higher-half)
  pml4->pdpt_physical[0] = kernel_pml4->pdpt_physical[0];

  return pml4;
}

void paging_map_user_page(pml4_table_t *pml4, uint64_t virtual_addr,
                          uint64_t physical_addr, int flags) {
  page_t *p = get_page(virtual_addr, 1, pml4);
  p->present = 1;
  p->rw = (flags & 0x2) ? 1 : 0;
  p->user = 1;
  p->frame = physical_addr >> 12;
}

void enter_user_mode(void *entry_point) {
  if (in_user_mode)
    return;
  in_user_mode = 1;

  __asm__ volatile("mov %%rsp, %0" : "=r"(kernel_rsp));
  __asm__ volatile("mov %%rbp, %0" : "=r"(kernel_rbp));

  uint64_t user_stack = (uint64_t)user_stack_array + 4096;

  __asm__ volatile("cli\n"
                   "mov $0x23, %%ax\n"
                   "mov %%ax, %%ds\n"
                   "mov %%ax, %%es\n"
                   "mov %%ax, %%fs\n"
                   "mov %%ax, %%gs\n"
                   "push $0x23\n"
                   "push %0\n"
                   "push $0x202\n"
                   "push $0x1B\n"
                   "push %1\n"
                   "iretq\n"
                   :
                   : "r"(user_stack), "r"((uint64_t)entry_point)
                   : "ax", "memory");
  in_user_mode = 0;
}

void return_to_kernel() {
  if (!in_user_mode)
    return;
  in_user_mode = 0;

  __asm__ volatile("cli\n"
                   "mov $0x10, %%ax\n"
                   "mov %%ax, %%ds\n"
                   "mov %%ax, %%es\n"
                   "mov %%ax, %%fs\n"
                   "mov %%ax, %%gs\n"
                   "mov %0, %%rsp\n"
                   "mov %1, %%rbp\n"
                   "sti\n"
                   "ret\n"
                   :
                   : "r"(kernel_rsp), "r"(kernel_rbp)
                   : "ax", "memory");
}

void page_fault_handler(registers_t *regs) {
  uint64_t fault_addr;
  __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));

  if (regs->cs == 0x1B) { // User mode code segment
    print_serial("User process page fault at 0x");
    char hex[20];
    k_itoa_hex(fault_addr, hex);
    print_serial(hex);
    print_serial(" - terminating\n");
    // Forward declare task_kill/get_current_task if needed or include task.h
    extern void exit(int status);
    exit(-1);
  }

  char buf[32];
  print_serial("DEBUG PAGE FAULT at 0x");
  k_itoa_hex(fault_addr, buf);
  print_serial(buf);
  print_serial(" (HALTING)\n");

  while (1) {
    __asm__ volatile("hlt");
  }
}
