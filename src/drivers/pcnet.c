#include "pcnet.h"
#include "../kernel/hal/isr.h"
#include "../kernel/string.h"
#include "ports.h"

extern void print_serial(const char *str);
extern uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys);

pcnet_device_t pcnet_dev;

static void pcnet_print(const char *str) { print_serial(str); }

static inline void cli() { __asm__ volatile("cli"); }
static inline void sti() { __asm__ volatile("sti"); }

// 32-bit DWIO access helpers
static void pcnet_write_csr(uint32_t csr, uint32_t val) {
  outl(pcnet_dev.io_base + PCNET_RAP, csr);
  outl(pcnet_dev.io_base + PCNET_RDP, val);
}

static uint32_t pcnet_read_csr(uint32_t csr) {
  outl(pcnet_dev.io_base + PCNET_RAP, csr);
  return inl(pcnet_dev.io_base + PCNET_RDP);
}

static void pcnet_write_bcr(uint32_t bcr, uint32_t val) {
  outl(pcnet_dev.io_base + PCNET_RAP, bcr);
  outl(pcnet_dev.io_base + PCNET_BDP, val);
}

void pcnet_init(void) {
  pcnet_dev.initialized = 0;
  pcnet_print("PCnet: Initializing Driver...\n");

  extern uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func,
                                        uint8_t offset);
  extern int pci_find_device(uint16_t vendor, uint16_t device, uint8_t *bus_out,
                             uint8_t *slot_out, uint8_t *func_out);

  uint8_t bus, slot, func;
  if (!pci_find_device(PCNET_VENDOR_ID, PCNET_DEVICE_ID, &bus, &slot, &func)) {
    pcnet_print("PCnet: AMD Am79C973 not found on PCI bus.\n");
    return;
  }

  pcnet_print("PCnet: AMD PCnet-FAST III (Am79C973) found.\n");

  uint32_t bar0 = pci_config_read_dword(bus, slot, func, 0x10);
  if (!(bar0 & 1)) {
    pcnet_print("PCnet: BAR0 is not I/O space. Aborting.\n");
    return;
  }

  pcnet_dev.io_base = bar0 & 0xFFFFFFFC;

  // Reset the device from any state (read from 16-bit and 32-bit reset ports)
  inw(pcnet_dev.io_base + 0x14);
  inl(pcnet_dev.io_base + 0x18);

  // Switch to 32-bit DWIO mode (Writing 0 to RDP at 0x10)
  outl(pcnet_dev.io_base + 0x10, 0);

  // Read MAC from APROM (safest byte-by-byte)
  char buf[32];
  pcnet_print("PCnet: MAC: ");
  for (int i = 0; i < 6; i++) {
    pcnet_dev.mac[i] = inb(pcnet_dev.io_base + i);
    k_itoa_hex(pcnet_dev.mac[i], buf);
    pcnet_print(buf);
    if (i < 5)
      pcnet_print(":");
  }
  pcnet_print("\n");

  // Set Software Style 3 (32-bit structures)
  pcnet_write_bcr(20, 0x0102);

  // Allocate Initialization Block and Rings
  uint32_t init_block_phys;
  pcnet_init_block_t *init_block = (pcnet_init_block_t *)kmalloc_ap(
      sizeof(pcnet_init_block_t), &init_block_phys);

  memset(init_block, 0, sizeof(pcnet_init_block_t));
  init_block->mode = 0x0000; // Promiscuous = 0
  init_block->rlen = 4 << 4; // log2(16) = 4
  init_block->tlen = 4 << 4;

  for (int i = 0; i < 6; i++) {
    init_block->mac[i] = pcnet_dev.mac[i];
  }

  uint32_t tx_phys;
  pcnet_dev.tx_ring = (pcnet_tdes_t *)kmalloc_ap(
      sizeof(pcnet_tdes_t) * PCNET_TX_BUFFERS, &tx_phys);

  uint32_t rx_phys;
  pcnet_dev.rx_ring = (pcnet_rdes_t *)kmalloc_ap(
      sizeof(pcnet_rdes_t) * PCNET_RX_BUFFERS, &rx_phys);

  init_block->tx_ring_phys = tx_phys;
  init_block->rx_ring_phys = rx_phys;

  // Initialize ring buffers
  for (int i = 0; i < PCNET_TX_BUFFERS; i++) {
    uint32_t buf_phys;
    pcnet_dev.tx_buffers[i] = (uint8_t *)kmalloc_ap(2048, &buf_phys);
    pcnet_dev.tx_ring[i].buffer_phys = buf_phys;
    pcnet_dev.tx_ring[i].length_flags = 0; // Host owns it initially
    pcnet_dev.tx_ring[i].status = 0;
  }

  for (int i = 0; i < PCNET_RX_BUFFERS; i++) {
    uint32_t buf_phys;
    pcnet_dev.rx_buffers[i] = (uint8_t *)kmalloc_ap(2048, &buf_phys);
    pcnet_dev.rx_ring[i].buffer_phys = buf_phys;
    // Length is 2's complement of 2048 (-2048 = 0xF800).
    // Set OWN bit (0x80000000)
    pcnet_dev.rx_ring[i].length_flags = PCNET_OWN_CARD | 0xF800;
    pcnet_dev.rx_ring[i].message_len = 0;
  }

  pcnet_dev.tx_current = 0;
  pcnet_dev.rx_current = 0;

  // Send Initializaton Block address to device
  pcnet_write_csr(1, init_block_phys & 0xFFFF);
  pcnet_write_csr(2, (init_block_phys >> 16) & 0xFFFF);

  // Set INIT bit in CSR0
  pcnet_write_csr(0, 0x0001);

  // Wait for initialization to complete (IDON bit 0x0100)
  for (int i = 0; i < 100000; i++) {
    if (pcnet_read_csr(0) & 0x0100) {
      break;
    }
  }

  if (!(pcnet_read_csr(0) & 0x0100)) {
    pcnet_print("PCnet: Initialization failed (timeout waiting for IDON).\n");
    return;
  }

  // Clear IDON, set STRT (Start) and INEA (Interrupt Enable)
  pcnet_write_csr(0, 0x0142);

  pcnet_dev.initialized = 1;
  pcnet_print("PCnet: Initialized OK.\n");
}

void pcnet_send(const uint8_t *data, uint16_t len) {
  if (!pcnet_dev.initialized)
    return;

  cli();

  uint8_t curr = pcnet_dev.tx_current;

  // Check if Card still owns this descriptor
  if (pcnet_dev.tx_ring[curr].length_flags & PCNET_OWN_CARD) {
    pcnet_print("PCnet: TX Ring full! Dropping packet.\n");
    sti();
    return;
  }

  // Copy data
  memcpy(pcnet_dev.tx_buffers[curr], data, len);

  // 2's complement of length in bits 0-11
  // According to AM79C973 manual, bits 12-15 MUST be 1s (ONES field)
  uint32_t len_2s = (-len) & 0x0FFF;

  // Set ownership to card, flag Start of Packet (STP) and End of Packet (ENP)
  pcnet_dev.tx_ring[curr].length_flags =
      PCNET_OWN_CARD | PCNET_STP | PCNET_ENP | 0xF000 | len_2s;
  pcnet_dev.tx_ring[curr].status = 0;

  pcnet_dev.tx_current = (pcnet_dev.tx_current + 1) % PCNET_TX_BUFFERS;

  // Tell chip to read descriptor (TDMD in CSR0)
  uint32_t csr0 = pcnet_read_csr(0);
  pcnet_write_csr(0, csr0 | 0x0008);

  sti();
}

int pcnet_poll(uint8_t *buf, uint16_t *len_out) {
  if (!pcnet_dev.initialized)
    return 0;

  cli();

  uint8_t curr = pcnet_dev.rx_current;

  // If the host owns the current buffer, a packet is ready
  if ((pcnet_dev.rx_ring[curr].length_flags & PCNET_OWN_CARD) == 0) {
    // Check for errors (ERR bit 0x40000000)
    if (pcnet_dev.rx_ring[curr].length_flags & PCNET_ERR) {
      // Packet corrupted, discard it
    } else {
      // Valid packet!
      uint16_t pkt_len = pcnet_dev.rx_ring[curr].message_len & 0xFFF;
      // Truncate FCS (4 bytes)
      if (pkt_len > 4)
        pkt_len -= 4;

      if (pkt_len > 2048)
        pkt_len = 2048;

      memcpy(buf, pcnet_dev.rx_buffers[curr], pkt_len);
      *len_out = pkt_len;

      // Flip ownership back to card
      pcnet_dev.rx_ring[curr].length_flags = PCNET_OWN_CARD | 0xF800;
      pcnet_dev.rx_ring[curr].message_len = 0;

      pcnet_dev.rx_current = (pcnet_dev.rx_current + 1) % PCNET_RX_BUFFERS;

      // Clear interrupt flags
      pcnet_write_csr(0, 0x0400);

      sti();
      return 1;
    }

    // Packet was bad, reset descriptor anyway
    pcnet_dev.rx_ring[curr].length_flags = PCNET_OWN_CARD | 0xF800;
    pcnet_dev.rx_ring[curr].message_len = 0;
    pcnet_dev.rx_current = (pcnet_dev.rx_current + 1) % PCNET_RX_BUFFERS;

    // Clear interrupt flags
    pcnet_write_csr(0, 0x0400);
  }

  sti();
  return 0;
}
