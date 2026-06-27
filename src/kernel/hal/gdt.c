// gdt.c
#include "gdt.h"
#include "../acpi.h"
#include "../heap.h"
#include "../smp.h"
#include "../string.h"
#include <stdint.h>

#define GDT_ENTRIES (5 + 2 * MAX_CPUS)

struct gdt_entry gdt[GDT_ENTRIES];
struct gdt_ptr gp;

typedef struct tss_entry {
  uint32_t reserved0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

static tss_entry_t bsp_tss;
static tss_entry_t *cpu_tss_entries[MAX_CPUS];

void gdt_set_gate(int num, uint64_t base, uint64_t limit, uint8_t access,
                  uint8_t gran);
extern void tss_flush();
extern uint64_t kernel_stack_top;

static void init_tss_common(tss_entry_t *t, uint64_t rsp0) {
  memset(t, 0, sizeof(tss_entry_t));
  t->rsp0 = rsp0;
  t->iomap_base = sizeof(tss_entry_t);
}

static void set_tss_descriptor(int gdt_index, uint64_t tss_base) {
  uint32_t tss_limit = sizeof(tss_entry_t) - 1;

  gdt_set_gate(gdt_index, tss_base, tss_limit, 0x89, 0x00);

  struct gdt_entry *upper = &gdt[gdt_index + 1];
  uint32_t base_high = (uint32_t)(tss_base >> 32);
  upper->limit_low = (uint16_t)(base_high & 0xFFFF);
  upper->base_low = (uint16_t)(base_high >> 16);
  upper->base_middle = 0;
  upper->access = 0;
  upper->granularity = 0;
  upper->base_high = 0;
}

static inline void ltr_selector(uint16_t sel) {
  __asm__ volatile("ltr %0" : : "r"(sel));
}

void gdt_init_cpu(int cpu_num) {
  if (cpu_num == 0) {
    gdt_init();
    return;
  }

  tss_entry_t *t = (tss_entry_t *)kmalloc(sizeof(tss_entry_t));
  uint64_t stack = (uint64_t)kmalloc(16384) + 16384;
  init_tss_common(t, stack);
  cpu_tss_entries[cpu_num] = t;
  cpu_cores[cpu_num].tss = t;

  int tss_idx = 5 + 2 * cpu_num;
  set_tss_descriptor(tss_idx, (uint64_t)t);

  gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
  gp.base = (uint64_t)&gdt;

  gdt_flush((uint64_t)&gp);

  uint16_t sel = (uint16_t)(tss_idx * 8);
  ltr_selector(sel);
}

void gdt_init() {
  gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
  gp.base = (uint64_t)&gdt;

  gdt_set_gate(0, 0, 0, 0, 0);
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF);
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xAF);
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xAF);
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xAF);

  init_tss_common(&bsp_tss, (uint64_t)&kernel_stack_top);
  cpu_tss_entries[0] = &bsp_tss;
  cpu_cores[0].tss = &bsp_tss;

  set_tss_descriptor(5, (uint64_t)&bsp_tss);

  gdt_flush((uint64_t)&gp);
  tss_flush();
}

void gdt_set_gate(int num, uint64_t base, uint64_t limit, uint8_t access,
                  uint8_t gran) {
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;

  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = ((limit >> 16) & 0x0F);
  gdt[num].granularity |= (gran & 0xF0);
  gdt[num].access = access;
}

void tss_set_stack(uint64_t rsp0) {
  bsp_tss.rsp0 = rsp0;
}

void tss_set_stack_cpu(int cpu_num, uint64_t rsp0) {
  if (cpu_num >= 0 && cpu_num < MAX_CPUS && cpu_tss_entries[cpu_num]) {
    cpu_tss_entries[cpu_num]->rsp0 = rsp0;
  }
}
