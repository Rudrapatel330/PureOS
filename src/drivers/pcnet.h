#ifndef PCNET_H
#define PCNET_H

#include "../kernel/types.h"
#include <stdint.h>


// Am79C973 (PCnet-FAST III) PCI identifiers
#define PCNET_VENDOR_ID 0x1022
#define PCNET_DEVICE_ID 0x2000

// 32-bit DWIO mode register offsets
#define PCNET_APROM 0x00 // MAC address (16 bytes)
#define PCNET_RDP 0x10   // Register Data Port
#define PCNET_RAP 0x14   // Register Address Port
#define PCNET_RESET 0x18 // Reset Port
#define PCNET_BDP 0x1C   // Bus Data Port

// Number of descriptors in the rings
// Max is 512, but we use a smaller memory footprint for simplicity
#define PCNET_TX_BUFFERS 16
#define PCNET_RX_BUFFERS 16

// Ownership bits for descriptors
#define PCNET_OWN_CARD 0x80000000 // Card owns the descriptor buffer
#define PCNET_OWN_HOST 0x00000000 // Host owns the descriptor buffer

// Descriptor Flags
#define PCNET_ENP 0x01000000 // End of Packet
#define PCNET_STP 0x02000000 // Start of Packet
#define PCNET_ERR 0x40000000 // Error

// Initialization Block (must be word aligned, but 32-bit dwio demands 32-bit
// boundaries)
typedef struct __attribute__((packed)) {
  uint16_t mode;
  uint8_t rlen; // Log2 of RX ring size << 4
  uint8_t tlen; // Log2 of TX ring size << 4
  uint8_t mac[6];
  uint16_t reserved;
  uint8_t ladr[8]; // Logical Address Filter (Multicast mask)
  uint32_t rx_ring_phys;
  uint32_t tx_ring_phys;
} pcnet_init_block_t;

// Transmit Descriptor (TMD)
typedef struct __attribute__((packed)) {
  uint32_t buffer_phys;
  uint32_t length_flags; // lower 12 bits = 2's complement length | upper bits =
                         // flags (STP/ENP/OWN)
  uint32_t status;       // Error flags after transmission
  uint32_t reserved;
} pcnet_tdes_t;

// Receive Descriptor (RMD)
typedef struct __attribute__((packed)) {
  uint32_t buffer_phys;
  uint32_t length_flags; // lower 12 bits = 2's complement of buffer size |
                         // upper bits = flags (OWN)
  uint32_t message_len;  // length of received message
  uint32_t reserved;
} pcnet_rdes_t;

// Driver State
typedef struct {
  uint32_t io_base;
  uint8_t mac[6];
  uint8_t initialized;

  // Pointers to the descriptor rings
  pcnet_tdes_t *tx_ring;
  pcnet_rdes_t *rx_ring;

  // Pointers to the payload buffers
  uint8_t *tx_buffers[PCNET_TX_BUFFERS];
  uint8_t *rx_buffers[PCNET_RX_BUFFERS];

  // Current indices
  uint8_t tx_current;
  uint8_t rx_current;
} pcnet_device_t;

// Global instance
extern pcnet_device_t pcnet_dev;

// Function prototypes
void pcnet_init(void);
void pcnet_send(const uint8_t *data, uint16_t len);
int pcnet_poll(uint8_t *buf, uint16_t *len_out);

#endif
