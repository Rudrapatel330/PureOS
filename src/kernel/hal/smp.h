#ifndef SMP_H
#define SMP_H

#include "../types.h"
#include "acpi.h"
#include "cpu.h"

struct task;

typedef struct {
  uint8_t apic_id;
  uint32_t cpu_number;
  struct task *current_task;
  void *tss;
  uint32_t lapic_timer_freq;
} cpu_core_t;

extern cpu_core_t cpu_cores[MAX_CPUS];
extern volatile int active_cpus;

void smp_init(void);
void smp_start_aps(void);
int smp_get_cpu_count(void);
void ap_kernel_entry_common(int cpu_num);

#endif // SMP_H
