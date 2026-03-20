#include "paging.h"
#include "../drivers/ports.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "isr.h"
#include "syscall.h"
#include "task.h"

extern void print_serial(const char *);

pml4_table_t *kernel_pml4 = 0;
pml4_table_t *current_pml4 = 0;

static uint64_t kernel_rsp;
static uint64_t kernel_rbp;
static uint8_t user_stack_array[4096] __attribute__((aligned(16)));
int in_user_mode = 0;

static void pat_init() {
  uint32_t pat_lo, pat_hi;
  // Read IA32_PAT MSR
  __asm__ volatile("rdmsr" : "=a"(pat_lo), "=d"(pat_hi) : "c"(0x277));

  // Set PAT4 to WC (01)
  // PAT register is 8 bits per entry. Entry 4 starts at bit 0 of pat_hi.
  pat_hi &= ~0xFF;
  pat_hi |= 0x01; // Write-Combining

  __asm__ volatile("wrmsr" : : "a"(pat_lo), "d"(pat_hi), "c"(0x277));
  print_serial("PAGING: IA32_PAT configured (PAT4=WC)\n");
}

page_t *get_page(uint64_t address, int make, pml4_table_t *pml4) {
  address /= 0x1000;
  uint64_t pml4_idx = (address >> 27) & 0x1FF;
  uint64_t pdpt_idx = (address >> 18) & 0x1FF;
  uint64_t pd_idx = (address >> 9) & 0x1FF;
  uint64_t pt_idx = address & 0x1FF;

  if (!pml4->pdpts[pml4_idx]) {
    if (!make)
      return 0;
    uint32_t phys;
    page_directory_pointer_t *pdpt = (page_directory_pointer_t *)kmalloc_ap(
        sizeof(page_directory_pointer_t), &phys);
    memset(pdpt, 0, sizeof(page_directory_pointer_t));
    pml4->pdpts[pml4_idx] = pdpt;
    pml4->pdpt_physical[pml4_idx] = phys | 0x7;
  }

  page_directory_pointer_t *pdpt = pml4->pdpts[pml4_idx];

  if (!pdpt->directories[pdpt_idx]) {
    if (!make)
      return 0;
    uint32_t phys;
    page_directory_t *pd =
        (page_directory_t *)kmalloc_ap(sizeof(page_directory_t), &phys);
    memset(pd, 0, sizeof(page_directory_t));
    pdpt->directories[pdpt_idx] = pd;
    pdpt->directories_physical[pdpt_idx] = phys | 0x7;
  }

  page_directory_t *pd = pdpt->directories[pdpt_idx];

  if (!pd->tables[pd_idx]) {
    if (!make)
      return 0;
    uint32_t phys;
    page_table_t *pt = (page_table_t *)kmalloc_ap(sizeof(page_table_t), &phys);
    memset(pt, 0, sizeof(page_table_t));
    pd->tables[pd_idx] = pt;
    pd->tables_physical[pd_idx] = phys | 0x7;
  }

  page_table_t *pt = pd->tables[pd_idx];
  return &pt->pages[pt_idx];
}

void paging_init() {
  print_serial("PAGING: Initializing 4-level paging...\n");

  // Enable NX bit via EFER MSR (IA32_EFER, MSR 0xC0000080, bit 11)
  uint32_t efer_lo, efer_hi;
  __asm__ volatile("rdmsr" : "=a"(efer_lo), "=d"(efer_hi) : "c"(0xC0000080));
  efer_lo |= (1 << 11); // Set NXE bit
  __asm__ volatile("wrmsr" : : "a"(efer_lo), "d"(efer_hi), "c"(0xC0000080));
  print_serial("PAGING: NX bit enabled via EFER\n");

  uint32_t phys_pml4;
  kernel_pml4 = (pml4_table_t *)kmalloc_ap(sizeof(pml4_table_t), &phys_pml4);
  memset(kernel_pml4, 0, sizeof(pml4_table_t));
  kernel_pml4->phys_addr = phys_pml4;

  // ---- Allocate PDPT for PML4[0] (covers 0x0 - 0x3FFFFFFFFF) ----
  uint32_t phys_pdpt0;
  page_directory_pointer_t *pdpt0 = (page_directory_pointer_t *)kmalloc_ap(
      sizeof(page_directory_pointer_t), &phys_pdpt0);
  memset(pdpt0, 0, sizeof(page_directory_pointer_t));
  kernel_pml4->pdpts[0] = pdpt0;
  kernel_pml4->pdpt_physical[0] = phys_pdpt0 | 0x7;

  // ==== Identity map first 1GB using 2MB large pages (PDPT[0]) ====
  // 512 x 2MB = 1GB, using PS bit (bit 7) in PDE for 2MB pages
  print_serial("PAGING: Identity mapping first 1GB (2MB pages)...\n");
  uint32_t phys_pd_id;
  print_serial("PAGING: About to kmalloc_ap for pd_id...\n");
  page_directory_t *pd_id =
      (page_directory_t *)kmalloc_ap(sizeof(page_directory_t), &phys_pd_id);
  print_serial("PAGING: kmalloc_ap pd_id done, about to memset...\n");
  memset(pd_id, 0, sizeof(page_directory_t));
  print_serial("PAGING: memset done, about to set pdpt0...\n");
  pdpt0->directories[0] = pd_id;
  pdpt0->directories_physical[0] = phys_pd_id | 0x7;
  print_serial("PAGING: pdpt0 set, about to enter loop...\n");

  for (int i = 0; i < 512; i++) {
    // Present(0) | RW(1) | User(2) | PS(7) = 0x87
    pd_id->tables_physical[i] = ((uint64_t)i * 0x200000) | 0x87;
    pd_id->tables[i] = 0; // No page table - 2MB large page
  }
  print_serial("PAGING: 1GB identity map done.\n");

  // ==== Higher-half mappings (all in PDPT[3], covers 0xC0000000-0xFFFFFFFF)
  // ====
  print_serial("PAGING: Mapping kernel + APIC/MMIO (2MB pages)...\n");
  uint32_t phys_pd_hi;
  page_directory_t *pd_hi =
      (page_directory_t *)kmalloc_ap(sizeof(page_directory_t), &phys_pd_hi);
  memset(pd_hi, 0, sizeof(page_directory_t));
  pdpt0->directories[3] = pd_hi;
  pdpt0->directories_physical[3] = phys_pd_hi | 0x7;
  print_serial("PAGING: pd_hi allocated.\n");

  // 0xC0000000-0xCFFFFFFF: Kernel higher-half (256MB = 128 x 2MB, PD[0..127])
  for (int i = 0; i < 128; i++) {
    // Present(0) | RW(1) | PS(7) = 0x83 (no User)
    pd_hi->tables_physical[i] = ((uint64_t)i * 0x200000) | 0x83;
    pd_hi->tables[i] = 0;
  }

  // 0xF0000000-0xFFFFFFFF: APIC/MMIO (256MB = 128 x 2MB, PD[384..511])
  for (int i = 384; i < 512; i++) {
    // Virtual 0xC0000000 + i*2MB = physical address (identity-mapped MMIO)
    pd_hi->tables_physical[i] = (0xC0000000ULL + (uint64_t)i * 0x200000) | 0x83;
    pd_hi->tables[i] = 0;
  }

  // ==== Framebuffer (4MB, uses get_page since address is dynamic) ====
  uint32_t *fb_ptr = (uint32_t *)0x6000;
  uint64_t lfb_phys = *fb_ptr;
  if (lfb_phys == 0 || lfb_phys == 0xFFFFFFFF) {
    lfb_phys = 0xE0000000;
  }
  print_serial("PAGING: Mapping LFB (4MB)...\n");

  // Only 1024 pages - negligible cost, keeps get_page() for dynamic LFB address
  for (uint64_t i = 0; i < 0x400000; i += 0x1000) {
    page_t *page = get_page(lfb_phys + i, 1, kernel_pml4);
    if (!page) {
      print_serial("PAGING: get_page failed during LFB map!\n");
      break; // Stop immediately to avoid crash
    }
    page->present = 1;
    page->rw = 1;
    page->user = 1;
    page->nx = 1;  // Framebuffer is data only, not executable
    page->pat = 1; // PAT=1, PCD=0, PWT=0 selects PAT4 (WC)
    page->frame = (lfb_phys + i) >> 12;
  }
  print_serial("PAGING: LFB Map complete.\n");

  pat_init();

  print_serial("PAGING: Done.\n");
  current_pml4 = kernel_pml4;
  __asm__ volatile("mov %0, %%cr3" : : "r"((uint64_t)phys_pml4));
}

void switch_page_directory(pml4_table_t *new_dir) {
  current_pml4 = new_dir;
  __asm__ volatile("mov %0, %%cr3" : : "r"(new_dir->phys_addr));
}

pml4_table_t *copy_page_directory(pml4_table_t *src) {
  uint32_t phys;
  pml4_table_t *dir = (pml4_table_t *)kmalloc_ap(sizeof(pml4_table_t), &phys);
  memset(dir, 0, sizeof(pml4_table_t));
  dir->phys_addr = phys;

  for (int i = 0; i < 512; i++) {
    dir->pdpts[i] = src->pdpts[i];
    dir->pdpt_physical[i] = src->pdpt_physical[i];
  }
  return dir;
}

pml4_table_t *paging_create_user_address_space() {
  uint32_t phys;
  pml4_table_t *pml4 = (pml4_table_t *)kmalloc_ap(sizeof(pml4_table_t), &phys);
  memset(pml4, 0, sizeof(pml4_table_t));
  pml4->phys_addr = phys;

  // Copy kernel mappings (PDPT[0] contains identity and higher-half)
  pml4->pdpts[0] = kernel_pml4->pdpts[0];
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
    exit(-1);
  }

  print_serial("Kernel page fault at 0x");
  char hex[20];
  k_itoa_hex(fault_addr, hex);
  print_serial(hex);
  print_serial(" - halting\n");
  while (1) {
    __asm__ volatile("hlt");
  }
}
