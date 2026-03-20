#ifndef UHCI_H
#define UHCI_H

#include "../../kernel/types.h"

// UHCI Register Offsets
#define UHCI_USBCMD 0x00
#define UHCI_USBSTS 0x02
#define UHCI_USBINTR 0x04
#define UHCI_FRNUM 0x06
#define UHCI_FRBASEADD 0x08
#define UHCI_SOFMOD 0x0C
#define UHCI_PORTSC1 0x10
#define UHCI_PORTSC2 0x12

// UHCI Command Bits
#define USBCMD_RS (1 << 0)      // Run/Stop
#define USBCMD_HCRESET (1 << 1) // Host Controller Reset
#define USBCMD_GRESET (1 << 2)  // Global Reset

// UHCI Transfer Descriptor (TD) - 16 bytes
typedef struct uhci_td {
  uint32_t link_ptr;
  uint32_t control_status;
  uint32_t token;
  uint32_t buffer_ptr;
} __attribute__((packed)) uhci_td_t;

// UHCI Queue Head (QH) - 16 bytes
typedef struct uhci_qh {
  uint32_t head_link_ptr;
  uint32_t element_link_ptr;
} __attribute__((packed)) uhci_qh_t;

void uhci_init(uint32_t base_io);

#endif // UHCI_H
