#ifndef ES1370_H
#define ES1370_H

#include <stdint.h>

// ES1370 PCI ID: Vendor 0x1274, Device 0x5000

// Register Offsets
#define ES1370_REG_CONTROL 0x00
#define ES1370_REG_STATUS 0x04
#define ES1370_REG_UART_DATA 0x08
#define ES1370_REG_UART_RES 0x09
#define ES1370_REG_UART_STAT 0x09
#define ES1370_REG_UART_CTRL 0x09
#define ES1370_REG_MEM_PAGE 0x0C
#define ES1370_REG_CODEC 0x10
#define ES1370_REG_SERIAL_CTRL 0x20
#define ES1370_REG_DAC1_SCOUNT 0x24
#define ES1370_REG_DAC1_FRAMEADR 0x28
#define ES1370_REG_DAC1_FRAMECNT 0x2C
#define ES1370_REG_DAC2_SCOUNT 0x24   // Page 1
#define ES1370_REG_DAC2_FRAMEADR 0x28 // Page 1
#define ES1370_REG_DAC2_FRAMECNT 0x2C // Page 1
#define ES1370_REG_ADC_SCOUNT 0x30
#define ES1370_REG_ADC_FRAMEADR 0x34
#define ES1370_REG_ADC_FRAMECNT 0x38

// Serial Control Bits
#define SCTRL_P1_LOOP_SEL (1 << 0)
#define SCTRL_P1_PAUSE (1 << 1)
#define SCTRL_P1_S_INTR_EN (1 << 2)
#define SCTRL_P1_S_EB (1 << 3)
#define SCTRL_P2_LOOP_SEL (1 << 4)
#define SCTRL_P2_PAUSE (1 << 5)
#define SCTRL_P2_S_INTR_EN (1 << 6)
#define SCTRL_P2_S_EB (1 << 7)
#define SCTRL_R1_LOOP_SEL (1 << 8)
#define SCTRL_R1_PAUSE (1 << 9)
#define SCTRL_R1_S_INTR_EN (1 << 10)
#define SCTRL_R1_S_EB (1 << 11)
#define SCTRL_P1_FORMAT_8BIT (0 << 12)
#define SCTRL_P1_FORMAT_16BIT (1 << 12)
#define SCTRL_P1_FORMAT_MONO (0 << 13)
#define SCTRL_P1_FORMAT_STEREO (1 << 13)
#define SCTRL_P2_FORMAT_8BIT (0 << 14)
#define SCTRL_P2_FORMAT_16BIT (1 << 14)
#define SCTRL_P2_FORMAT_MONO (0 << 15)
#define SCTRL_P2_FORMAT_STEREO (1 << 15)
#define SCTRL_R1_FORMAT_8BIT (0 << 16)
#define SCTRL_R1_FORMAT_16BIT (1 << 16)
#define SCTRL_R1_FORMAT_MONO (0 << 17)
#define SCTRL_R1_FORMAT_STEREO (1 << 17)

// Control Register Bits
#define CTRL_ADC_EN (1 << 0)
#define CTRL_DAC1_EN (1 << 1)
#define CTRL_DAC2_EN (1 << 2)
#define CTRL_UART_EN (1 << 3)
#define CTRL_JYSTK_EN (1 << 4)
#define CTRL_CDC_EN (1 << 5)
#define CTRL_PDRBMERS (1 << 6) // Port reset for AC'97 (ES1371)
#define CTRL_MSF_SEL (1 << 7)

// Status Register Bits
#define STAT_INTR (1 << 31)
#define STAT_DAC1 (1 << 2)
#define STAT_DAC2 (1 << 3)
#define STAT_ADC (1 << 0)

// Sample Rate selection (for ES1370)
#define CTRL_WTSRSEL_5KHZ (0 << 12)
#define CTRL_WTSRSEL_11KHZ (1 << 12)
#define CTRL_WTSRSEL_22KHZ (2 << 12)
#define CTRL_WTSRSEL_44KHZ (3 << 12)

typedef struct {
  uint32_t io_base;
  uint32_t irq;
} es1370_device_t;

void es1370_init(uint32_t io_base, uint8_t irq);
void es1370_play_test_beep();
void es1370_play_pcm(const uint8_t *data, uint32_t size, uint32_t sample_rate,
                     int bits, int channels);
void es1370_set_volume(uint8_t left, uint8_t right);
void es1370_stop(void);
void es1370_start_capture(uint32_t sample_rate, int bits, int channels);
void es1370_stop_capture(void);
int es1370_read_capture(uint8_t *buffer, uint32_t max_size);

#endif
