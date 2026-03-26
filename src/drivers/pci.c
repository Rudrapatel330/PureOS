#include "pci.h"
#include "../kernel/string.h"
#include "ahci.h"
#include "ac97.h"
#include "es1370.h"
#include "ports.h"
#include "usb/uhci.h"

extern void print_serial(const char *str);

uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func,
                               uint8_t offset) {
  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint32_t tmp = 0;

  /* create configuration address */
  address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                       (offset & 0xfc) | ((uint32_t)0x80000000));

  /* write out the address */
  outl(PCI_CONFIG_ADDRESS, address);
  /* read in the data */
  /* (offset & 2) * 8) would be used for non-dword alignment, but here we read
   * dword */
  tmp = inl(PCI_CONFIG_DATA);
  return tmp;
}

void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func,
                            uint8_t offset, uint32_t data) {
  uint32_t address = (uint32_t)((((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) |
                                (((uint32_t)func) << 8) | (offset & 0xfc) |
                                ((uint32_t)0x80000000));
  outl(PCI_CONFIG_ADDRESS, address);
  outl(PCI_CONFIG_DATA, data);
}

void pci_config_write_word(uint8_t bus, uint8_t slot, uint8_t func,
                           uint8_t offset, uint16_t data) {
  uint32_t address = (uint32_t)((((uint32_t)bus) << 16) | (((uint32_t)slot) << 11) |
                                (((uint32_t)func) << 8) | (offset & 0xfc) |
                                ((uint32_t)0x80000000));
  outl(PCI_CONFIG_ADDRESS, address);
  // We need to read-modify-write for word access usually, or just use outw to the data port
  // but let's do the standard shift logic for safety or just outl the mask.
  // Actually on x86 PCI, outw to PCI_CONFIG_DATA + (offset & 2) works.
  outw(PCI_CONFIG_DATA + (offset & 2), data);
}

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func,
                              uint8_t offset) {
  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint16_t tmp = 0;

  /* create configuration address */
  address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                       (offset & 0xfc) | ((uint32_t)0x80000000));

  /* write out the address */
  outl(PCI_CONFIG_ADDRESS, address);
  /* read in the data */
  /* (offset & 2) * 8) is used to find which word within the dword we want */
  tmp = (uint16_t)((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);
  return tmp;
}

void pci_check_device(uint8_t bus, uint8_t device, uint8_t function) {
  uint16_t vendorID = pci_config_read_word(bus, device, function, 0);
  if (vendorID == 0xFFFF)
    return; // Device/Function doesn't exist

  char buf[16];
  print_serial("PCI: Found device - Bus: ");
  k_itoa(bus, buf);
  print_serial(buf);
  print_serial(" Slot: ");
  k_itoa(device, buf);
  print_serial(buf);
  print_serial(" Func: ");
  k_itoa(function, buf);
  print_serial(buf);

  print_serial(" Vendor: 0x");
  k_itoa_hex(vendorID, buf);
  print_serial(buf);

  uint16_t deviceID = pci_config_read_word(bus, device, function, 2);
  print_serial(" Device: 0x");
  k_itoa_hex(deviceID, buf);
  print_serial(buf);

  uint16_t classCode = pci_config_read_word(bus, device, function, 10);
  print_serial(" Class: 0x");
  k_itoa_hex(classCode, buf);
  print_serial(buf);

  if (classCode == 0x0106) {
    print_serial(" [AHCI Controller DETECTED]");
    uint32_t bar5 = pci_config_read_dword(bus, device, function, 0x24);
    ahci_init(bar5);
  }

  if (vendorID == 0x1274 && deviceID == 0x5000) {
    print_serial(" [ES1370 Audio DETECTED]");
    uint32_t bar0 = pci_config_read_dword(bus, device, function, 0x10);
    uint32_t io_base = bar0 & 0xFFFFFFFC;
    uint8_t irq =
        (uint8_t)(pci_config_read_word(bus, device, function, 0x3C) & 0xFF);
    es1370_init(io_base, irq);
  }

  // AC97 Audio (Vendor 0x8086, Device 0x2415 for ICH)
  if (vendorID == 0x8086 && deviceID == 0x2415) {
    print_serial(" [AC97 Audio DETECTED]");
    // AC97 uses BAR0 for Mixer and BAR1 for Bus Master
    uint32_t bar0 = pci_config_read_dword(bus, device, function, 0x10);
    uint32_t bar1 = pci_config_read_dword(bus, device, function, 0x14);
    uint32_t nambar = bar0 & 0xFFFFFFFC;
    uint32_t nabmbar = bar1 & 0xFFFFFFFC;
    uint8_t irq = (uint8_t)(pci_config_read_word(bus, device, function, 0x3C) & 0xFF);
    
    // Enable Bus Mastering
    uint16_t cmd = pci_config_read_word(bus, device, function, 0x04);
    cmd |= 0x07; // I/O, Mem, Master
    pci_config_write_word(bus, device, function, 0x04, cmd);
    
    ac97_init(nambar, nabmbar, irq);
  }

  // USB Controller (Class 0C, Subclass 03)
  if ((classCode >> 8) == 0x0C && (classCode & 0xFF) == 0x03) {
    uint8_t progIF =
        (uint8_t)((pci_config_read_word(bus, device, function, 8) >> 8) & 0xFF);

    // Enable Memory Space, I/O Space, and Bus Mastering (PCI Command Register Offset 0x04)
    uint16_t cmd = pci_config_read_word(bus, device, function, 0x04);
    cmd |= 0x07; // Bit 0 (I/O), 1 (Mem), 2 (Master)
    pci_config_write_word(bus, device, function, 0x04, cmd);

    if (progIF == 0x00) { // UHCI
      print_serial(" [USB UHCI Controller DETECTED]");
      uint32_t bar4 = pci_config_read_dword(bus, device, function, 0x20);
      if (bar4 & 1) { // I/O Space
        uhci_init(bar4 & 0xFFFFFFFC);
      }
    } else if (progIF == 0x10) { // OHCI
      print_serial(" [USB OHCI Controller DETECTED]");
      uint32_t bar0 = pci_config_read_dword(bus, device, function, 0x10);
      if (!(bar0 & 1)) { // Memory Space
        extern void ohci_init(uintptr_t base_addr);
        ohci_init(bar0 & 0xFFFFFFF0);
      }
    } else if (progIF == 0x20) { // EHCI
      print_serial(" [USB EHCI Controller DETECTED - (High Speed but NO DRIVER)]");
    }
  }

  print_serial("\n");
}

void pci_init() {
  char buf[16];
  for (uint16_t bus = 0; bus < 256; bus++) {
    for (uint8_t device = 0; device < 32; device++) {
      // Check if device exists and if it's multi-function
      uint16_t vendorID = pci_config_read_word((uint8_t)bus, device, 0, 0);
      if (vendorID != 0xFFFF) {
        pci_check_device((uint8_t)bus, device, 0);

        // Read header type (at offset 0x0E)
        uint8_t headerType =
            (uint8_t)(pci_config_read_word((uint8_t)bus, device, 0, 0x0E) &
                      0xFF);
        print_serial(" HT: 0x");
        k_itoa_hex(headerType, buf);
        print_serial(buf);
        print_serial("\n");

        if (headerType & 0x80) {
          // Multi-function device, check functions 1-7
          for (uint8_t func = 1; func < 8; func++) {
            pci_check_device((uint8_t)bus, device, func);
          }
        }
      }
    }
  }
}

int pci_find_device(uint16_t vendor, uint16_t device, uint8_t *bus_out,
                    uint8_t *slot_out, uint8_t *func_out) {
  for (uint16_t bus = 0; bus < 256; bus++) {
    for (uint8_t slot = 0; slot < 32; slot++) {
      for (uint8_t func = 0; func < 8; func++) {
        uint16_t vendorID = pci_config_read_word((uint8_t)bus, slot, func, 0);
        if (vendorID != 0xFFFF) {
          uint16_t deviceID = pci_config_read_word((uint8_t)bus, slot, func, 2);
          if (vendor == vendorID && device == deviceID) {
            if (bus_out)
              *bus_out = (uint8_t)bus;
            if (slot_out)
              *slot_out = slot;
            if (func_out)
              *func_out = func;
            return 1;
          }
        }
      }
    }
  }
  return 0; // Not found
}
