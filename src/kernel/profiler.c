#include "profiler.h"
#include "debug.h"
#include "hal/hal.h"
#include "string.h"

extern void print_serial(const char *str);

#define MAX_UNIQUE_IPS 512

typedef struct {
  uint64_t rip;
  uint32_t count;
} profile_entry_t;

static profile_entry_t profile_buffer[MAX_UNIQUE_IPS];
static int profile_count = 0;
static int enabled = 0;
static uint32_t total_samples = 0;
static uint32_t kernel_samples = 0;
static uint32_t user_samples = 0;

void profiler_init(void) {
  memset(profile_buffer, 0, sizeof(profile_buffer));
  profile_count = 0;
  total_samples = 0;
  kernel_samples = 0;
  user_samples = 0;
  enabled = 0;
}

void profiler_enable(int enable) {
  enabled = enable;
  if (enable) {
    print_serial("Profiler ENABLED\n");
  } else {
    print_serial("Profiler DISABLED\n");
  }
}

void profiler_sample(registers_t *regs) {
  if (!enabled || !regs)
    return;

  uint64_t rip = regs->rip;
  total_samples++;

  if ((regs->cs & 0x3) == 3) {
    user_samples++;
  } else {
    kernel_samples++;
  }

  // Simple linear search for now.
  // In a high-frequency timer (1000Hz), this might be slow if MAX_UNIQUE_IPS is
  // large. But at 250Hz it should be fine.
  for (int i = 0; i < profile_count; i++) {
    if (profile_buffer[i].rip == rip) {
      profile_buffer[i].count++;
      return;
    }
  }

  if (profile_count < MAX_UNIQUE_IPS) {
    profile_buffer[profile_count].rip = rip;
    profile_buffer[profile_count].count = 1;
    profile_count++;
  }
}

void profiler_dump(void) {
  print_serial("\n=== KERNEL PROFILER REPORT ===\n");
  char buf[32];

  print_serial("Total Samples: ");
  k_itoa(total_samples, buf);
  print_serial(buf);
  print_serial("\n");

  print_serial("Kernel: ");
  k_itoa(kernel_samples, buf);
  print_serial(buf);
  print_serial(" (");
  if (total_samples > 0) {
    k_itoa((kernel_samples * 100) / total_samples, buf);
    print_serial(buf);
    print_serial("%)");
  }
  print_serial("\n");

  print_serial("User:   ");
  k_itoa(user_samples, buf);
  print_serial(buf);
  print_serial(" (");
  if (total_samples > 0) {
    k_itoa((user_samples * 100) / total_samples, buf);
    print_serial(buf);
    print_serial("%)");
  }
  print_serial("\n");

  print_serial("\n--- Top Hits ---\n");
  // We don't sort here to keep it simple and avoid long stalls in interrupt
  // context or dump call. But for a report, it's better to find top ones.
  for (int i = 0; i < profile_count; i++) {
    if (profile_buffer[i].count > (total_samples / 100)) { // Show anything > 1%
      print_serial("0x");
      k_itoa_hex(profile_buffer[i].rip, buf);
      print_serial(buf);
      print_serial(": ");
      k_itoa(profile_buffer[i].count, buf);
      print_serial(buf);
      print_serial(" hits\n");
    }
  }
  print_serial("==============================\n");
}
