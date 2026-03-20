#include "ne2000.h"
#include "../kernel/hal/isr.h"
#include "../kernel/string.h"
#include "ports.h"

extern void print_serial(const char *str);

// Register offsets are defined in ne2000.h
ne2000_device_t ne2000_dev;

// Dynamic I/O base — set during ne2000_init from PCI BAR0
static uint16_t ne2000_io = NE2000_IO_BASE;

static void ne2000_print(const char *str) { print_serial(str); }

// Global interrupt control wrappers
static inline void cli() { __asm__ volatile("cli"); }
static inline void sti() { __asm__ volatile("sti"); }

static void ne2000_write_mem(uint16_t addr, const uint8_t *data, uint16_t len) {
  uint16_t count = (len + 1) & ~1; // Must be even for word mode

  cli();
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA | 0x20); // Abort
  outb(ne2000_io + NE2000_RSAR0, addr & 0xFF);
  outb(ne2000_io + NE2000_RSAR1, (addr >> 8) & 0xFF);
  outb(ne2000_io + NE2000_RBCR0, count & 0xFF);
  outb(ne2000_io + NE2000_RBCR1, (count >> 8) & 0xFF);
  outb(ne2000_io + NE2000_CR,
       NE2000_CR_PG0 | NE2000_CR_STA | 0x12); // Write DMA

  outsw(ne2000_io + NE2000_DATAPORT, data, count / 2);

  // Wait for DMA complete (RDC bit 0x40)
  for (int delay = 0; delay < 1000; delay++) {
    if (inb(ne2000_io + NE2000_ISR) & 0x40)
      break;
  }
  outb(ne2000_io + NE2000_ISR, 0x40); // Clear RDC

  // Abort DMA
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA | 0x20);
  sti();
}

static void ne2000_dump_header(uint8_t *h) {
  char buf[64];
  print_serial("NE2000: PKT HDR: Stat=");
  k_itoa(h[0], buf);
  print_serial(buf);
  print_serial(" Next=");
  k_itoa(h[1], buf);
  print_serial(buf);
  print_serial(" Len=");
  k_itoa(h[2] | (h[3] << 8), buf);
  print_serial(buf);
  print_serial("\n");
}

static void ne2000_read_mem(uint16_t addr, uint8_t *data, uint16_t len) {
  uint16_t count = (len + 1) & ~1; // Must be even for word mode

  cli();
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA | 0x20);
  outb(ne2000_io + NE2000_RSAR0, addr & 0xFF);
  outb(ne2000_io + NE2000_RSAR1, (addr >> 8) & 0xFF);
  outb(ne2000_io + NE2000_RBCR0, count & 0xFF);
  outb(ne2000_io + NE2000_RBCR1, (count >> 8) & 0xFF);
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA | 0x0A); // Read DMA

  insw(ne2000_io + NE2000_DATAPORT, data, count / 2);

  // Wait for DMA complete
  for (int delay = 0; delay < 1000; delay++) {
    if (inb(ne2000_io + NE2000_ISR) & 0x40)
      break;
  }
  outb(ne2000_io + NE2000_ISR, 0x40); // Clear RDC

  // Abort DMA
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA | 0x20);
  sti();
}

uint32_t ne2000_handler(registers_t *regs) {
  // Critical: Always switch to Page 0 before touching ISR
  uint8_t old_cr = inb(ne2000_io + NE2000_CR);
  outb(ne2000_io + NE2000_CR, (old_cr & 0x3F) | NE2000_CR_PG0);

  uint8_t isr = inb(ne2000_io + NE2000_ISR);
  if (!isr) {
    outb(ne2000_io + NE2000_CR, old_cr);
    return (uint32_t)regs;
  }

  outb(ne2000_io + NE2000_ISR, isr); // Clear handled flags

  if (isr & NE2000_ISR_OVW)
    ne2000_print("NE2000: Ring buffer overflow!\n");

  outb(ne2000_io + NE2000_CR, old_cr);
  return (uint32_t)regs;
}

void ne2000_init(void) {
  ne2000_print("NE2000: Initializing...\n");

  // --- Step 1: Discover PCI I/O base ---
  // VirtualBox NE2000 is an AMD PCnet at 0x1022:0x2000.
  // Fallback to generic NE2000 at 0x10EC:0x8029 (Realtek).
  extern uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func,
                                        uint8_t offset);
  extern int pci_find_device(uint16_t vendor, uint16_t device, uint8_t *bus_out,
                             uint8_t *slot_out, uint8_t *func_out);

  uint8_t bus = 0, slot = 0, func = 0;
  int found = 0;

  if (pci_find_device(0x1022, 0x2000, &bus, &slot, &func) == 0) {
    ne2000_print("NE2000: Found AMD PCnet (0x1022:0x2000)\n");
    found = 1;
  } else if (pci_find_device(0x10EC, 0x8029, &bus, &slot, &func) == 0) {
    ne2000_print("NE2000: Found Realtek RTL8029 (0x10EC:0x8029)\n");
    found = 1;
  } else if (pci_find_device(0x1050, 0x0940, &bus, &slot, &func) == 0) {
    ne2000_print("NE2000: Found Winbond NE2000 (0x1050:0x0940)\n");
    found = 1;
  }

  if (found) {
    // BAR0 (offset 0x10): lower bit set = I/O space, mask off bit 0
    uint32_t bar0 = pci_config_read_dword(bus, slot, func, 0x10);
    if (bar0 & 1) {
      ne2000_io = (uint16_t)(bar0 & 0xFFFC);
      char buf[64];
      strcpy(buf, "NE2000: PCI BAR0 I/O base = 0x");
      char hex[8];
      k_itoa_hex(ne2000_io, hex);
      strcat(buf, hex);
      strcat(buf, "\n");
      ne2000_print(buf);
    } else {
      ne2000_print("NE2000: BAR0 is MMIO, not I/O — using default 0x300\n");
    }
  } else {
    ne2000_print("NE2000: No PCI NIC found, trying hardcoded 0x300\n");
  }

  // --- Step 2: Reset the NIC ---
  uint8_t b = inb(ne2000_io + NE2000_RESET);
  outb(ne2000_io + NE2000_RESET, b);
  for (volatile int i = 0; i < 20000; i++)
    ; // Wait for reset

  // Poll ISR for RST bit (0x80) to confirm reset complete
  int reset_ok = 0;
  for (int i = 0; i < 5000; i++) {
    if (inb(ne2000_io + NE2000_ISR) & 0x80) {
      reset_ok = 1;
      break;
    }
  }
  if (!reset_ok) {
    // If we never got 0x80, check if port is at all responsive (not 0xFF)
    if (inb(ne2000_io + NE2000_ISR) == 0xFF) {
      ne2000_print("NE2000: Hardware not detected (0xFF) — check PCI\n");
      ne2000_dev.initialized = 0;
      return;
    }
  }

  // Clear ISR
  outb(ne2000_io + NE2000_ISR, 0xFF);

  // --- Step 3: Read MAC from PROM (NE2000 stores MAC at bytes 0,2,4,6,8,10)
  // ---
  uint8_t mac[6];

  // To read PROM: do a 32-byte DMA read at address 0
  outb(ne2000_io + NE2000_CR, 0x21);  // page 0, stop, abort DMA
  outb(ne2000_io + NE2000_DCR, 0x49); // word mode
  outb(ne2000_io + NE2000_RBCR0, 32);
  outb(ne2000_io + NE2000_RBCR1, 0);
  outb(ne2000_io + NE2000_RSAR0, 0);
  outb(ne2000_io + NE2000_RSAR1, 0);
  outb(ne2000_io + NE2000_CR, 0x0A); // page 0, start, remote read

  uint8_t prom[32];
  for (int i = 0; i < 32; i += 2) {
    uint16_t w = inw(ne2000_io + NE2000_DATAPORT);
    prom[i] = w & 0xFF;
    prom[i + 1] = (w >> 8) & 0xFF;
  }

  // PROM bytes 0,2,4,6,8,10 hold the MAC (byte-duplicated in word reads)
  mac[0] = prom[0];
  mac[1] = prom[2];
  mac[2] = prom[4];
  mac[3] = prom[6];
  mac[4] = prom[8];
  mac[5] = prom[10];

  {
    char mbuf[64] = "NE2000: MAC: ";
    for (int i = 0; i < 6; i++) {
      char hb[4];
      k_itoa_hex(mac[i], hb);
      strcat(mbuf, hb);
      if (i < 5)
        strcat(mbuf, ":");
    }
    strcat(mbuf, "\n");
    ne2000_print(mbuf);
  }

  // --- Step 4: Normal NIC init ---
  outb(ne2000_io + NE2000_CR, 0x21);  // page 0, stop, abort DMA
  outb(ne2000_io + NE2000_DCR, 0x49); // word mode
  outb(ne2000_io + NE2000_RBCR0, 0);
  outb(ne2000_io + NE2000_RBCR1, 0);
  outb(ne2000_io + NE2000_RCR, 0x1E); // Accept broadcast + multicast
  outb(ne2000_io + NE2000_TCR, 0x02); // Internal loopback during init
  outb(ne2000_io + NE2000_PSTART, NE2000_RX_START);
  outb(ne2000_io + NE2000_PSTOP, NE2000_RX_STOP);
  outb(ne2000_io + NE2000_BNRY, NE2000_RX_START);
  outb(ne2000_io + NE2000_TPSR, NE2000_TX_START);
  outb(ne2000_io + NE2000_ISR, 0xFF);
  outb(ne2000_io + NE2000_IMR, 0x00);

  // Page 1: set CURR and MAC
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG1 | 0x21);
  outb(ne2000_io + NE2000_CURR, NE2000_RX_START + 1);
  for (int i = 0; i < 6; i++)
    outb(ne2000_io + NE2000_PAR0 + i, mac[i]);
  for (int i = 0; i < 8; i++)
    outb(ne2000_io + NE2000_MAR0 + i, 0xFF);

  // Page 0: start
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | 0x22); // stop->start
  outb(ne2000_io + NE2000_TCR, 0x00);                // Normal mode
  outb(ne2000_io + NE2000_ISR, 0xFF);
  outb(ne2000_io + NE2000_IMR, 0x00); // Polling mode

  for (int i = 0; i < 6; i++)
    ne2000_dev.mac[i] = mac[i];
  ne2000_dev.initialized = 1;

  ne2000_print("NE2000: Initialized OK.\n");
}

int ne2000_send(const uint8_t *data, uint16_t len) {
  if (!ne2000_dev.initialized) {
    return -1;
  }

  if (len > NE2000_TX_BUF_SIZE) {
    ne2000_print("NE2000: Packet too large\n");
    return -1;
  }

  uint8_t padded[64];
  if (len < 60) {
    memcpy(padded, data, len);
    memset(padded + len, 0, 60 - len);
    data = padded;
    len = 60; // Some docs say 60, some 64. 64 is safest, but 60 is standard
              // without FCS. Let's use 64.
  }
  if (len < 64) {
    memcpy(padded, data, len);
    memset(padded + len, 0, 64 - len);
    data = padded;
    len = 64;
  }

  // Write data to NIC buffer (0x4000)
  ne2000_write_mem(0x4000, data, len);

  // Set Transmit Page Start and Transmit Byte Count
  cli();
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA | 0x20);
  outb(ne2000_io + NE2000_TPSR, 0x40);
  outb(ne2000_io + NE2000_TBCR0, len & 0xFF);
  outb(ne2000_io + NE2000_TBCR1, (len >> 8) & 0xFF);
  outb(ne2000_io + NE2000_CR,
       NE2000_CR_PG0 | NE2000_CR_STA | NE2000_CR_TXP | 0x20);
  sti();

  extern uint32_t get_timer_ticks(void);
  extern void kernel_poll_events(void);

  uint32_t tstart = get_timer_ticks();
  while ((get_timer_ticks() - tstart) < 100) {
    kernel_poll_events();
    uint8_t isr = inb(ne2000_io + NE2000_ISR);
    if (isr & NE2000_ISR_PTX) {
      outb(ne2000_io + NE2000_ISR, NE2000_ISR_PTX);
      return 0; // Success
    }
    if (isr & NE2000_ISR_TXE) {
      print_serial("NE2000: Transmit error bit set\n");
      outb(ne2000_io + NE2000_ISR, NE2000_ISR_TXE);
      return -1;
    }
  }

  uint8_t timeout_isr = inb(ne2000_io + NE2000_ISR);
  char diag[64];
  strcpy(diag, "NE2000: Send timeout! ISR=");
  char hex[16];
  k_itoa(timeout_isr, hex);
  strcat(diag, hex);
  strcat(diag, "\n");
  ne2000_print(diag);
  return -1;
}

int ne2000_poll(uint8_t *buffer, uint16_t *len) {
  if (!ne2000_dev.initialized) {
    return -1;
  }

  static int poll_count = 0;
  if (++poll_count > 8) { // Limit packets per poll
    poll_count = 0;
    return 0;
  }

  // Switch to Page 0
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA | 0x20);

  // Check Boundary and Current Page
  cli();
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA);
  uint8_t bnr = inb(ne2000_io + NE2000_BNRY);

  // Read Current Page (from Page 1)
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG1 | NE2000_CR_STA | 0x20);
  uint8_t curr = inb(ne2000_io + 0x07); // CURR
  outb(ne2000_io + NE2000_CR, NE2000_CR_PG0 | NE2000_CR_STA | 0x20);
  sti();

  // Determine the page of the next unread packet
  uint8_t page = bnr + 1;
  if (page >= NE2000_RX_STOP)
    page = NE2000_RX_START;

  // If the next unread packet page aligns with where the NIC will write next,
  // the ring buffer is empty.
  if (page == curr)
    return 0;

  // Read Packet Header (4 bytes) at 'page * 256'
  uint8_t header[4];
  ne2000_read_mem(page * 256, header, 4);

  // Header: [0]=Status, [1]=NextPage, [2]=LowLen, [3]=HighLen
  uint8_t next_page = header[1];
  uint16_t total_len = header[2] | ((uint16_t)header[3] << 8);

  // Validate Header (Stat=255 and Next=255 means hardware failure/garbage)
  if (header[0] == 0xFF || next_page == 0xFF || total_len == 0xFFFF) {
    ne2000_print(
        "NE2000: Invalid packet header (garbage) - Resetting pointers\n");
    outb(ne2000_io + NE2000_BNRY,
         curr - 1 < NE2000_RX_START ? NE2000_RX_STOP - 1 : curr - 1);
    outb(ne2000_io + NE2000_ISR, 0xFF); // Clear all
    return -1;
  }

  ne2000_dump_header(header);

  // Validate Length (Ethernet Frame: 64 to 1522)
  if (total_len < 4 || total_len > 1522) {
    uint8_t flush_bnr = curr - 1;
    if (flush_bnr < NE2000_RX_START)
      flush_bnr = NE2000_RX_STOP - 1;
    outb(ne2000_io + NE2000_BNRY, flush_bnr);
    return -1;
  }

  // The actual data length is total_len - 4
  uint16_t data_len = total_len - 4;

  // Compute ring buffer read
  // The packet data starts at offset 4 in this page.
  uint16_t read_offset = page * 256 + 4;

  // Does the packet wrap around the end of the ring buffer?
  // Available bytes from offset to PSTOP:
  uint16_t bytes_to_end = (NE2000_RX_STOP * 256) - read_offset;

  if (data_len <= bytes_to_end) {
    // Contiguous read
    ne2000_read_mem(read_offset, buffer, data_len);
  } else {
    // Wrapped read
    // 1. Read to end
    ne2000_read_mem(read_offset, buffer, bytes_to_end);

    // 2. Read remainder from PSTART
    uint16_t remainder = data_len - bytes_to_end;
    ne2000_read_mem(NE2000_RX_START * 256, buffer + bytes_to_end, remainder);
  }

  *len = data_len;

  // Update Boundary Pointer
  // It should point to the page BEFORE the next packet.
  uint8_t next_bnr = next_page - 1;
  if (next_bnr < NE2000_RX_START)
    next_bnr = NE2000_RX_STOP - 1;

  outb(ne2000_io + NE2000_BNRY, next_bnr);

  // Clear PRX bit
  outb(ne2000_io + NE2000_ISR, NE2000_ISR_PRX);

  return 1;
}
