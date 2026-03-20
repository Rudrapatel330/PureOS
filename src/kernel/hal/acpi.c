#include "acpi.h"
#include "../../drivers/ports.h"
#include "../memory.h"
#include "../string.h"

extern void print_serial(const char *);
extern void k_itoa(int n, char *s);
extern void k_itoa_hex(uint32_t n, char *s);

cpu_info_t cpus[MAX_CPUS];
int num_cpus = 0;
uint32_t local_apic_phys_addr = 0;
uint32_t io_apic_phys_addr = 0;

// FADT fields for shutdown
static uint32_t acpi_pm1a_cnt = 0;
static uint32_t acpi_smi_cmd = 0;
static uint8_t acpi_enable_val = 0;

static int checksum_valid(uint8_t *ptr, int len) {
  uint8_t sum = 0;
  for (int i = 0; i < len; i++) {
    sum += ptr[i];
  }
  return sum == 0;
}

static rsdp_descriptor_t *find_rsdp(void) {
  uint16_t ebda_base_ptr = *(uint16_t *)0x40E;
  uint32_t ebda_start = ebda_base_ptr << 4;
  for (uint32_t addr = ebda_start; addr < ebda_start + 1024; addr += 16) {
    if (memcmp((void *)(uintptr_t)addr, "RSD PTR ", 8) == 0 &&
        checksum_valid((uint8_t *)(uintptr_t)addr, 20)) {
      return (rsdp_descriptor_t *)(uintptr_t)addr;
    }
  }

  for (uint32_t addr = 0xE0000; addr < 0x100000; addr += 16) {
    if (memcmp((void *)(uintptr_t)addr, "RSD PTR ", 8) == 0 &&
        checksum_valid((uint8_t *)(uintptr_t)addr, 20)) {
      return (rsdp_descriptor_t *)(uintptr_t)addr;
    }
  }

  return 0;
}

static void parse_madt(madt_t *madt) {
  print_serial("ACPI: Parsing MADT...\n");

  local_apic_phys_addr = madt->local_apic_address;

  uint8_t *ptr = (uint8_t *)madt + sizeof(madt_t);
  uint8_t *end = (uint8_t *)madt + madt->header.length;

  while (ptr < end) {
    madt_record_header_t *record = (madt_record_header_t *)ptr;

    if (record->type == 0) {
      madt_local_apic_t *lapic = (madt_local_apic_t *)record;
      if (lapic->flags & 1) {
        if (num_cpus < MAX_CPUS) {
          cpus[num_cpus].apic_id = lapic->apic_id;
          cpus[num_cpus].is_bsp = (num_cpus == 0) ? 1 : 0;
          num_cpus++;
        }
      }
    } else if (record->type == 1) {
      madt_io_apic_t *ioapic = (madt_io_apic_t *)record;
      io_apic_phys_addr = ioapic->io_apic_address;
    }

    ptr += record->length;
  }
}

void acpi_init(void) {
  print_serial("ACPI: Initializing...\n");
  num_cpus = 0;

  rsdp_descriptor_t *rsdp = find_rsdp();
  if (!rsdp) {
    print_serial("ACPI: RSDP not found!\n");
    return;
  }

  rsdt_t *rsdt = (rsdt_t *)(uintptr_t)rsdp->rsdt_address;
  if (!checksum_valid((uint8_t *)rsdt, rsdt->header.length)) {
    print_serial("ACPI: RSDT checksum invalid!\n");
    return;
  }

  int entries = (rsdt->header.length - sizeof(acpi_header_t)) / 4;
  for (int i = 0; i < entries; i++) {
    acpi_header_t *header =
        (acpi_header_t *)(uintptr_t)rsdt->pointer_to_other_sdt[i];
    if (memcmp(header->signature, "APIC", 4) == 0 &&
        checksum_valid((uint8_t *)header, header->length)) {
      parse_madt((madt_t *)header);
    }
    if (memcmp(header->signature, "FACP", 4) == 0 &&
        checksum_valid((uint8_t *)header, header->length)) {
      fadt_t *fadt = (fadt_t *)header;
      acpi_pm1a_cnt = fadt->pm1a_control_block;
      acpi_smi_cmd = fadt->smi_command_port;
      acpi_enable_val = fadt->acpi_enable;
      print_serial("ACPI: FADT found. PM1a_CNT=0x");
      char hex[16];
      k_itoa_hex(acpi_pm1a_cnt, hex);
      print_serial(hex);
      print_serial("\n");
    }
  }

  print_serial("ACPI: Detected CPUs: ");
  char buf[16];
  k_itoa(num_cpus, buf);
  print_serial(buf);
  print_serial("\n");
}

void acpi_shutdown(void) {
  print_serial("ACPI: Initiating shutdown...\n");

  if (acpi_pm1a_cnt) {
    if (acpi_smi_cmd && acpi_enable_val) {
      outb(acpi_smi_cmd, acpi_enable_val);
      for (volatile int i = 0; i < 1000000; i++)
        ;
    }
    outw(acpi_pm1a_cnt, (5 << 10) | ACPI_SLP_EN);
  }

  outw(0xB004, 0x2000);
  outw(0x604, 0x2000);

  print_serial("ACPI: Shutdown failed, halting.\n");
  __asm__ volatile("cli; hlt");
}

void acpi_reboot(void) {
  print_serial("ACPI: Initiating reboot...\n");

  uint8_t good = 0x02;
  while (good & 0x02)
    good = inb(0x64);
  outb(0x64, 0xFE);

  __asm__ volatile("cli");
  struct {
    uint16_t limit;
    uint64_t base;
  } __attribute__((packed)) null_idt = {0, 0};
  __asm__ volatile("lidt %0" : : "m"(null_idt));
  __asm__ volatile("int $3");

  while (1)
    __asm__ volatile("hlt");
}
