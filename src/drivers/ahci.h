#ifndef AHCI_H
#define AHCI_H

#include <stdint.h>

#define SATA_SIG_ATA 0x00000101   // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101    // Port multiplier

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

typedef enum {
  FIS_TYPE_REG_H2D = 0x27,   // Register FIS - host to device
  FIS_TYPE_REG_D2H = 0x34,   // Register FIS - device to host
  FIS_TYPE_DMA_ACT = 0x39,   // DMA activate FIS - device to host
  FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
  FIS_TYPE_DATA = 0x46,      // Data FIS - bidirectional
  FIS_TYPE_BIST = 0x58,      // BIST activate FIS - bidirectional
  FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
  FIS_TYPE_DEV_BITS = 0xA1,  // Set device bits FIS - device to host
} FIS_TYPE;

typedef struct {
  uint32_t clb;       // 0x00, command list base address, 1K-byte aligned
  uint32_t clbu;      // 0x04, command list base address upper 32 bits
  uint32_t fb;        // 0x08, FIS base address, 256-byte aligned
  uint32_t fbu;       // 0x0C, FIS base address upper 32 bits
  uint32_t is;        // 0x10, interrupt status
  uint32_t ie;        // 0x14, interrupt enable
  uint32_t cmd;       // 0x18, command and status
  uint32_t rsv0;      // 0x1C, reserved
  uint32_t tfd;       // 0x20, task file data
  uint32_t sig;       // 0x24, signature
  uint32_t ssts;      // 0x28, SATA status (SCR0:SStatus)
  uint32_t sctl;      // 0x2C, SATA control (SCR2:SControl)
  uint32_t serr;      // 0x30, SATA error (SCR1:SError)
  uint32_t sact;      // 0x34, SATA active (SCR3:SActive)
  uint32_t ci;        // 0x38, command issue
  uint32_t sntf;      // 0x3C, SATA notification (SCR4:SNotification)
  uint32_t fbs;       // 0x40, FIS-based switch control
  uint32_t rsv1[11];  // 0x44 ~ 0x6F, reserved
  uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific
} hba_port_t;

typedef struct {
  uint32_t cap;        // 0x00, Host capability
  uint32_t ghc;        // 0x04, Global host control
  uint32_t is;         // 0x08, Interrupt status
  uint32_t pi;         // 0x0C, Port implemented
  uint32_t vs;         // 0x10, Version
  uint32_t ccc_ctl;    // 0x14, Command completion coalescing control
  uint32_t ccc_pts;    // 0x18, Command completion coalescing ports
  uint32_t em_loc;     // 0x1C, Enclosure management location
  uint32_t em_ctl;     // 0x20, Enclosure management control
  uint32_t cap2;       // 0x24, Host capabilities extended
  uint32_t bohc;       // 0x28, BIOS/OS handoff control and status
  uint32_t rsv[29];    // 0x2C ~ 0x9F, Reserved
  uint32_t vendor[15]; // 0xA0 ~ 0xFF, Vendor specific registers
  hba_port_t
      ports[1]; // 0x100 ~ 0x10FF, Port control registers (actually up to 32)
} hba_mem_t;

typedef struct {
  uint8_t cfl : 5; // Command FIS length in DWORDS, 2 ~ 16
  uint8_t a : 1;   // ATAPI
  uint8_t w : 1;   // Write, 1: H2D, 0: D2H
  uint8_t p : 1;   // Prefetchable

  uint8_t r : 1;    // Reset
  uint8_t b : 1;    // BIST
  uint8_t c : 1;    // Clear busy upon R_OK
  uint8_t rsv0 : 1; // Reserved
  uint8_t pmp : 4;  // Port multiplier port

  uint16_t prdtl; // Physical region descriptor table length in entries

  uint32_t prdbc; // Physical region descriptor byte count transferred

  uint32_t ctba;  // Command table descriptor base address
  uint32_t ctbau; // Command table descriptor base address upper 32 bits

  uint32_t rsv1[4]; // Reserved
} hba_cmd_header_t;

typedef struct {
  uint32_t dba;      // Data base address
  uint32_t dbau;     // Data base address upper 32 bits
  uint32_t rsv0;     // Reserved
  uint32_t dbc : 22; // Byte count, 4M max
  uint32_t rsv1 : 9; // Reserved
  uint32_t i : 1;    // Interrupt on completion
} hba_prdt_entry_t;

typedef struct {
  uint8_t cfis[64]; // Command FIS
  uint8_t acmd[16]; // ATAPI command, 12 or 16 bytes
  uint8_t rsv[48];  // Reserved
  hba_prdt_entry_t
      prdt_entry[1]; // Physical region descriptor table entries, 0 ~ 65535
} hba_cmd_table_t;

// H2D Register FIS
typedef struct {
  uint8_t fis_type; // FIS_TYPE_REG_H2D
  uint8_t pmport : 4;
  uint8_t rsv0 : 3;
  uint8_t c : 1; // 1: Command, 0: Control
  uint8_t command;
  uint8_t featurel;

  uint8_t lba0;
  uint8_t lba1;
  uint8_t lba2;
  uint8_t device;

  uint8_t lba3;
  uint8_t lba4;
  uint8_t lba5;
  uint8_t featureh;

  uint8_t countl;
  uint8_t counth;
  uint8_t icc;
  uint8_t control;

  uint8_t rsv1[4];
} __attribute__((packed)) fis_reg_h2d_t;

// ATA Commands
#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35
#define ATA_CMD_IDENTIFY 0xEC

// ATA Status bits
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

// HBA PxCMD bits
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_FR 0x4000
#define HBA_PxCMD_CR 0x8000

// HBA PxIS bits
#define HBA_PxIS_TFES (1 << 30)

void ahci_init(uint32_t bar5);
int ahci_read_sectors(int portno, uint64_t lba, uint32_t count, void *buffer);
int ahci_write_sectors(int portno, uint64_t lba, uint32_t count,
                       const void *buffer);
int ahci_get_sata_port(void);

#endif
