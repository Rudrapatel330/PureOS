#ifndef NE2000_H
#define NE2000_H

#include "../kernel/hal/isr.h"
#include <stdint.h>

#define NE2000_IO_BASE 0x300

// Memory Map Constants (NE2000 NIC internal buffer pages)
#define NE2000_TX_START 0x40
#define NE2000_RX_START 0x46
#define NE2000_RX_STOP 0x80

// Standard I/O helpers
static inline uint16_t inw(uint16_t port) {
  uint16_t v;
  __asm__ volatile("inw %1, %0" : "=a"(v) : "Nd"(port));
  return v;
}

#define NE2000_CMD 0x00
#define NE2000_DATAPORT 0x10
#define NE2000_RESET 0x1F

#define NE2000_CR 0x00
// Page 0 (Write)
#define NE2000_PSTART 0x01
#define NE2000_PSTOP 0x02
#define NE2000_BNRY 0x03
#define NE2000_TPSR 0x04
#define NE2000_TBCR0 0x05
#define NE2000_TBCR1 0x06
#define NE2000_ISR 0x07
#define NE2000_RSAR0 0x08
#define NE2000_RSAR1 0x09
#define NE2000_RBCR0 0x0A
#define NE2000_RBCR1 0x0B
#define NE2000_RCR 0x0C
#define NE2000_TCR 0x0D
#define NE2000_DCR 0x0E
#define NE2000_IMR 0x0F

// Page 0 (Read)
#define NE2000_CLDA0 0x01
#define NE2000_CLDA1 0x02
#define NE2000_TSR 0x04
#define NE2000_NCR 0x05
#define NE2000_FIFO 0x06
#define NE2000_RSR 0x0C

// Page 1 (Read/Write)
#define NE2000_PAR0 0x01
#define NE2000_CURR 0x07
#define NE2000_MAR0 0x08

#define NE2000_CR_STP 0x01
#define NE2000_CR_STA 0x02
#define NE2000_CR_TXP 0x04
#define NE2000_CR_RDMA 0x08
#define NE2000_CR_PG0 0x00
#define NE2000_CR_PG1 0x40

#define NE2000_ISR_PRX 0x01
#define NE2000_ISR_PTX 0x02
#define NE2000_ISR_RXE 0x04
#define NE2000_ISR_TXE 0x08
#define NE2000_ISR_OVW 0x10
#define NE2000_ISR_CNT 0x20

#define NE2000_RSR_PRX 0x01
#define NE2000_RSR_CRC 0x02
#define NE2000_RSR_FAE 0x04
#define NE2000_RSR_FO 0x08
#define NE2000_RSR_PAE 0x20

#define NE2000_NUM_PAGES 32
#define NE2000_MEM_SIZE 32768
#define NE2000_TX_BUF_SIZE 1536

typedef struct {
  uint8_t mac[6];
  uint8_t initialized;
  uint16_t tx_len;
} ne2000_device_t;

extern ne2000_device_t ne2000_dev;

void ne2000_init(void);
int ne2000_send(const uint8_t *data, uint16_t len);
int ne2000_poll(uint8_t *buffer, uint16_t *len);
uint32_t ne2000_handler(registers_t *regs);

#endif
