#ifndef OHCI_H
#define OHCI_H

#include "../../kernel/types.h"

// OHCI Register Offsets
#define OH_REVISION          0x00
#define OH_CONTROL           0x04
#define OH_COMMAND_STATUS    0x08
#define OH_INTERRUPT_STATUS  0x0C
#define OH_INTERRUPT_ENABLE  0x10
#define OH_INTERRUPT_DISABLE 0x14
#define OH_HCCA              0x18
#define OH_PERIOD_CURRENT_ED 0x1C
#define OH_CONTROL_HEAD_ED   0x20
#define OH_CONTROL_CURR_ED   0x24
#define OH_BULK_HEAD_ED      0x28
#define OH_BULK_CURR_ED      0x2C
#define OH_DONE_HEAD         0x30
#define OH_FM_INTERVAL       0x34
#define OH_FM_REMAINING      0x38
#define OH_FM_NUMBER         0x3C
#define OH_PERIOD_START      0x40
#define OH_LS_THRESHOLD      0x44
#define OH_RH_DESCRIPTOR_A   0x48
#define OH_RH_DESCRIPTOR_B   0x4C
#define OH_RH_STATUS         0x50
#define OH_RH_PORT_STATUS    0x54

// Endpoint Descriptor (ED) - 16 bytes
typedef struct ohci_ed {
    uint32_t flags;
    uint32_t tail_td;
    uint32_t head_td;
    uint32_t next_ed;
} __attribute__((packed)) ohci_ed_t;

// General Transfer Descriptor (TD) - 16 bytes
typedef struct ohci_td {
    uint32_t flags;
    uint32_t cbp; // Current Buffer Pointer
    uint32_t next_td;
    uint32_t bep; // Buffer End Pointer
} __attribute__((packed)) ohci_td_t;

// Isochronous Transfer Descriptor (ISO TD) - 32 bytes
typedef struct ohci_iso_td {
    uint32_t flags;        // CC, FC, DI, SF (Starting Frame)
    uint32_t bp0;          // Buffer Page 0
    uint32_t next_td;      // Next ISO TD
    uint32_t bep;          // Buffer End
    uint16_t psw[8];       // Packet Status Words (offset/size + CC)
} __attribute__((packed)) ohci_iso_td_t;

// HCCA - 256 bytes
typedef struct ohci_hcca {
    uint32_t interrupt_table[32];
    uint16_t frame_number;
    uint16_t pad1;
    uint32_t done_head;
    uint8_t  reserved[116];
} __attribute__((packed)) ohci_hcca_t;

void ohci_init(uintptr_t base_addr);

#endif
