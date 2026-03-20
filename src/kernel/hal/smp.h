#ifndef SMP_H
#define SMP_H

#include "../types.h"

void smp_init(void);
int smp_get_cpu_count(void);
void ap_kernel_entry_common(int cpu_num);

#endif // SMP_H
