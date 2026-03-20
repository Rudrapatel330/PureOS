#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func,
                               uint8_t offset);
uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func,
                              uint8_t offset);

void pci_init();

int pci_find_device(uint16_t vendor, uint16_t device, uint8_t *bus_out,
                    uint8_t *slot_out, uint8_t *func_out);

#endif
