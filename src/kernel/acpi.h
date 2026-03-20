#ifndef ACPI_H
#define ACPI_H

#include "types.h"

// RSDP (Root System Description Pointer)
typedef struct {
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_address;
} __attribute__((packed)) rsdp_descriptor_t;

// XSDP (Extended)
typedef struct {
  rsdp_descriptor_t first_part;
  uint32_t length;
  uint64_t xsdt_address;
  uint8_t extended_checksum;
  uint8_t reserved[3];
} __attribute__((packed)) rsdp_descriptor20_t;

typedef struct {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed)) acpi_header_t;

typedef struct {
  acpi_header_t header;
  uint32_t pointer_to_other_sdt[];
} __attribute__((packed)) rsdt_t;

typedef struct {
  acpi_header_t header;
  uint64_t pointer_to_other_sdt[];
} __attribute__((packed)) xsdt_t;

// MADT (Multiple APIC Description Table)
typedef struct {
  acpi_header_t header;
  uint32_t local_apic_address;
  uint32_t flags;
} __attribute__((packed)) madt_t;

typedef struct {
  uint8_t type;
  uint8_t length;
} __attribute__((packed)) madt_record_header_t;

// Type 0: Processor Local APIC
typedef struct {
  madt_record_header_t header;
  uint8_t acpi_processor_id;
  uint8_t apic_id;
  uint32_t flags;
} __attribute__((packed)) madt_local_apic_t;

// Type 1: I/O APIC
typedef struct {
  madt_record_header_t header;
  uint8_t io_apic_id;
  uint8_t reserved;
  uint32_t io_apic_address;
  uint32_t global_system_interrupt_base;
} __attribute__((packed)) madt_io_apic_t;

// Type 2: Interrupt Source Override
typedef struct {
  madt_record_header_t header;
  uint8_t bus_source;
  uint8_t irq_source;
  uint32_t global_system_interrupt;
  uint16_t flags;
} __attribute__((packed)) madt_iso_t;

#define MAX_CPUS 32

typedef struct {
  uint8_t apic_id;
  uint8_t is_bsp;
} cpu_info_t;

extern cpu_info_t cpus[MAX_CPUS];
extern int num_cpus;
extern uint32_t local_apic_phys_addr;
extern uint32_t io_apic_phys_addr;

// FADT (Fixed ACPI Description Table) — subset of fields we need
typedef struct {
  acpi_header_t header;
  uint32_t firmware_ctrl;
  uint32_t dsdt;
  uint8_t reserved;
  uint8_t preferred_pm_profile;
  uint16_t sci_interrupt;
  uint32_t smi_command_port;
  uint8_t acpi_enable;
  uint8_t acpi_disable;
  uint8_t s4bios_req;
  uint8_t pstate_control;
  uint32_t pm1a_event_block;
  uint32_t pm1b_event_block;
  uint32_t pm1a_control_block;
  uint32_t pm1b_control_block;
  uint32_t pm2_control_block;
  uint32_t pm_timer_block;
  uint32_t gpe0_block;
  uint32_t gpe1_block;
  uint8_t pm1_event_length;
  uint8_t pm1_control_length;
  uint8_t pm2_control_length;
  uint8_t pm_timer_length;
  // ... more fields exist but we only need up to here
} __attribute__((packed)) fadt_t;

// S5 sleep type values (parsed from DSDT)
#define ACPI_SLP_EN (1 << 13)

void acpi_init(void);
void acpi_shutdown(void);
void acpi_reboot(void);

#endif // ACPI_H
