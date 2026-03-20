// ata.c - Primary and Secondary ATA Channel Support
#include "ata.h"
#include "../kernel/assert.h"
#include "../kernel/string.h"
#include "ports.h" // Use unified port I/O functions
#include <stdint.h>

// ATA Bus Ports (Primary Bus - Floppy/Boot Disk)
#define ATA_DATA 0x1F0
#define ATA_ERROR 0x1F1
#define ATA_SECTOR_CNT 0x1F2
#define ATA_LBA_LO 0x1F3
#define ATA_LBA_MID 0x1F4
#define ATA_LBA_HI 0x1F5
#define ATA_DRIVE_HEAD 0x1F6
#define ATA_STATUS 0x1F7
#define ATA_COMMAND 0x1F7

// ATA Bus Ports (Secondary Bus - Data Disk)
#define ATA2_DATA 0x170
#define ATA2_ERROR 0x171
#define ATA2_SECTOR_CNT 0x172
#define ATA2_LBA_LO 0x173
#define ATA2_LBA_MID 0x174
#define ATA2_LBA_HI 0x175
#define ATA2_DRIVE_HEAD 0x176
#define ATA2_STATUS 0x177
#define ATA2_COMMAND 0x177

// Status Flags
#define ATA_SR_BSY 0x80 // Busy
#define ATA_SR_DRQ 0x08 // Data Request ready
#define ATA_SR_ERR 0x01 // Error

// ======================== PRIMARY CHANNEL ========================

int ata_wait_bsy() {
  int timeout = 100000;
  while (timeout--) {
    uint32_t status = inb(ATA_STATUS);
    if (!(status & ATA_SR_BSY))
      return 1;
    if (status & ATA_SR_ERR)
      return 0;
  }
  return 0; // Timeout
}

int ata_wait_drq() {
  int timeout = 100000;
  while (timeout--) {
    uint32_t status = inb(ATA_STATUS);
    if (status & ATA_SR_DRQ)
      return 1;
    if (status & ATA_SR_ERR)
      return 0;
  }
  return 0; // Timeout
}

int ata_read_sector(uint32_t lba, uint8_t *buffer) {
  KASSERT(buffer != 0);

  int retries = 3;

  while (retries-- > 0) {
    if (!ata_wait_bsy())
      continue;

    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LO, (uint8_t)(lba & 0xFF));
    outb(ATA_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_COMMAND, 0x20);

    // I/O Delay
    for (volatile int i = 0; i < 4; i++)
      inb(ATA_STATUS);

    if (!ata_wait_bsy())
      continue;

    uint8_t status = inb(ATA_STATUS);
    if (status & ATA_SR_ERR) {
      uint8_t err = inb(ATA_ERROR);
      print_serial("ATA: Read Error Register: 0x");
      char hex[16];
      k_itoa_hex(err, hex);
      print_serial(hex);
      print_serial("\n");
      if (retries > 0)
        continue;
      return 0;
    }

    if (!ata_wait_drq())
      continue;
    insw(ATA_DATA, buffer, 256);
    return 1;
  }

  return 0;
}

int ata_write_sector(uint32_t lba, uint8_t *buffer) {
  print_serial("ATA: write lba=");
  char buf[16];
  k_itoa(lba, buf);
  print_serial(buf);
  print_serial("\n");

  KASSERT(buffer != 0);
  int retries = 3;
  uint8_t status;

  while (retries-- > 0) {
    if (!ata_wait_bsy()) {
      print_serial("ATA: wait_bsy timeout\n");
      continue;
    }

    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LO, (uint8_t)(lba & 0xFF));
    outb(ATA_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_COMMAND, 0x30); // Write Sector

    // I/O Delay
    for (volatile int i = 0; i < 4; i++)
      inb(ATA_STATUS);

    if (!ata_wait_bsy()) {
      print_serial("ATA: wait_bsy (post-cmd) timeout\n");
      continue;
    }
    if (!ata_wait_drq()) {
      print_serial("ATA: wait_drq timeout\n");
      continue;
    }

    outsw(ATA_DATA, buffer, 256);

    // Flush cache
    outb(ATA_COMMAND, 0xE7);
    if (!ata_wait_bsy()) {
      print_serial("ATA: flush cache timeout\n");
      continue;
    }

    // Check if write actually worked
    status = inb(ATA_STATUS);
    if (status & ATA_SR_ERR) {
      uint8_t err = inb(ATA_ERROR);
      print_serial("ATA: Write Error Register: 0x");
      char hex[16];
      k_itoa_hex(err, hex);
      print_serial(hex);
      print_serial("\n");
      continue;
    }
    print_serial("ATA: write successful\n");
    return 1;
  }

  extern void print_serial(const char *);
  print_serial("ATA: Write sector failed.\n");
  return 0;
}

// ======================== SECONDARY CHANNEL (Data Disk)
// ========================

static int ata2_wait_bsy() {
  int timeout = 100000;
  while ((inb(ATA2_STATUS) & ATA_SR_BSY) && timeout--)
    ;
  if (timeout <= 0)
    return 0;
  return 1;
}

static int ata2_wait_drq() {
  int timeout = 100000;
  while (!(inb(ATA2_STATUS) & ATA_SR_DRQ) && timeout--) {
    if (inb(ATA2_STATUS) & ATA_SR_ERR)
      return 0;
  }
  if (timeout <= 0)
    return 0;
  return 1;
}

int ata2_detect() {
  // Try to detect a drive on the secondary channel
  if (!ata2_wait_bsy())
    return 0;
  outb(ATA2_DRIVE_HEAD, 0xE0); // Select master drive

  // Small delay
  for (volatile int i = 0; i < 1000; i++)
    ;

  uint8_t status = inb(ATA2_STATUS);
  // If status is 0xFF or 0x00, no drive present
  if (status == 0xFF || status == 0x00)
    return 0;

  // Try to read sector 0 to verify
  uint8_t test_buf[512];
  if (ata2_read_sector(0, test_buf)) {
    // Check for FAT BPB signature
    if (test_buf[510] == 0x55 && test_buf[511] == 0xAA) {
      return 1; // Valid disk found!
    }
  }
  return 0;
}

int ata2_read_sector(uint32_t lba, uint8_t *buffer) {
  KASSERT(buffer != 0);

  int retries = 3;

  while (retries-- > 0) {
    if (!ata2_wait_bsy())
      continue;

    outb(ATA2_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA2_SECTOR_CNT, 1);
    outb(ATA2_LBA_LO, (uint8_t)(lba & 0xFF));
    outb(ATA2_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA2_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA2_COMMAND, 0x20); // Read Sectors

    // I/O Delay
    for (volatile int i = 0; i < 4; i++)
      inb(ATA2_STATUS);

    if (!ata2_wait_bsy())
      continue;

    uint8_t status = inb(ATA2_STATUS);
    if (status & ATA_SR_ERR) {
      uint8_t err = inb(ATA2_ERROR);
      print_serial("ATA2: Read Error Register: 0x");
      char hex[16];
      k_itoa_hex(err, hex);
      print_serial(hex);
      print_serial("\n");
      if (retries > 0)
        continue;
      return 0;
    }

    if (!ata2_wait_drq())
      continue;
    insw(ATA2_DATA, buffer, 256);
    return 1;
  }

  return 0;
}

int ata2_write_sector(uint32_t lba, uint8_t *buffer) {
  print_serial("ATA2: write lba=");
  char buf[16];
  k_itoa(lba, buf);
  print_serial(buf);
  print_serial("\n");

  KASSERT(buffer != 0);

  int retries = 3;
  uint8_t status;
  while (retries-- > 0) {
    if (!ata2_wait_bsy()) {
      print_serial("ATA2: wait_bsy timeout\n");
      continue;
    }

    outb(ATA2_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA2_SECTOR_CNT, 1);
    outb(ATA2_LBA_LO, (uint8_t)(lba & 0xFF));
    outb(ATA2_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA2_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA2_COMMAND, 0x30); // Write Sectors

    // I/O Delay
    for (volatile int i = 0; i < 4; i++)
      inb(ATA2_STATUS);

    if (!ata2_wait_bsy()) {
      print_serial("ATA2: wait_bsy (post-cmd) timeout\n");
      continue;
    }
    if (!ata2_wait_drq()) {
      print_serial("ATA2: wait_drq timeout\n");
      continue;
    }

    outsw(ATA2_DATA, buffer, 256);

    // Flush cache
    outb(ATA2_COMMAND, 0xE7);
    if (!ata2_wait_bsy()) {
      print_serial("ATA2: flush cache timeout\n");
      continue;
    }

    status = inb(ATA2_STATUS);
    if (status & ATA_SR_ERR) {
      uint8_t err = inb(ATA2_ERROR);
      print_serial("ATA2: Write Error Register: 0x");
      char hex[16];
      k_itoa_hex(err, hex);
      print_serial(hex);
      print_serial("\n");
      continue;
    }
    print_serial("ATA2: write successful\n");
    return 1;
  }

  extern void print_serial(const char *);
  print_serial("ATA2: Write sector failed.\n");
  return 0;
}
