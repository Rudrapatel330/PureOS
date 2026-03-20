// ata.h
#ifndef ATA_H
#define ATA_H

#include "../kernel/types.h"

// Primary Channel (Floppy/Boot Disk)
int ata_read_sector(uint32_t lba, uint8_t *buffer);
int ata_write_sector(uint32_t lba, uint8_t *buffer);

// Secondary Channel (Persistent Data Disk)
int ata2_detect();
int ata2_read_sector(uint32_t lba, uint8_t *buffer);
int ata2_write_sector(uint32_t lba, uint8_t *buffer);

#endif
