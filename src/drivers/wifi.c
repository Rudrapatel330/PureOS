#include "../kernel/types.h"
#include "pci.h"
#include "ports.h"

extern void print_serial(const char *);

#define RTL8185_VENDOR_ID 0x10EC
#define RTL8185_DEVICE_ID 0x8185

void wifi_init(void) {
  print_serial("WIFI: Searching for RTL8185 PCI card...\n");

  uint8_t bus, slot, func;
  if (pci_find_device(RTL8185_VENDOR_ID, RTL8185_DEVICE_ID, &bus, &slot,
                      &func)) {
    print_serial("WIFI: Found RTL8185 at PCI ");
    // Future: Map BARs and initialize hardware
    print_serial("WIFI: Driver initialized (Stub).\n");
  } else {
    print_serial("WIFI: No RTL8185 card found.\n");
  }
}
