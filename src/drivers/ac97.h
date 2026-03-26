#ifndef AC97_H
#define AC97_H

#include <stdint.h>

// AC97 Mixer Registers (NAMBAR)
#define AC97_RESET              0x00
#define AC97_MASTER_VOLUME      0x02
#define AC97_AUX_VOLUME         0x04
#define AC97_MONO_VOLUME        0x06
#define AC97_MIC_VOLUME         0x0E
#define AC97_LINEIN_VOLUME      0x10
#define AC97_CD_VOLUME          0x12
#define AC97_PCM_OUT_VOLUME     0x18
#define AC97_REC_SELECT         0x1A
#define AC97_REC_GAIN           0x1C
#define AC97_GENERAL_PURPOSE    0x20

// AC97 Bus Master Registers (NABMBAR)
#define AC97_PI_BDBA            0x00 // PCM In Buffer Descriptor Base Address
#define AC97_PI_CIV             0x04 // PCM In Current Index Value
#define AC97_PI_LVI             0x05 // PCM In Last Valid Index
#define AC97_PI_SR              0x06 // PCM In Status Register
#define AC97_PI_PICB            0x08 // PCM In Position In Current Buffer
#define AC97_PI_PIV             0x0A // PCM In Prefetched Index Value
#define AC97_PI_CR              0x0B // PCM In Control Register

#define AC97_PO_BDBA            0x10 // PCM Out Buffer Descriptor Base Address
#define AC97_PO_CIV             0x14
#define AC97_PO_LVI             0x15
#define AC97_PO_SR              0x16
#define AC97_PO_PICB            0x18
#define AC97_PO_PIV             0x1A
#define AC97_PO_CR              0x1B

// Control Register Bits
#define AC97_CR_RPBM            0x01 // Run/Pause Bus Master
#define AC97_CR_RR              0x02 // Reset Registers
#define AC97_CR_LVBIE           0x04 // Last Valid Buffer Interrupt Enable
#define AC97_CR_FEIE            0x08 // FIFO Error Interrupt Enable
#define AC97_CR_IOCE            0x10 // Interrupt On Completion Enable

// Status Register Bits
#define AC97_SR_DCH             0x01 // DMA Controller Halted
#define AC97_SR_CELV            0x02 // Current Equals Last Valid
#define AC97_SR_LVBCI           0x04 // Last Valid Buffer Completion Interrupt
#define AC97_SR_BCIS            0x08 // Buffer Completion Interrupt Status
#define AC97_SR_FIFOE           0x10 // FIFO Error

typedef struct {
    uint32_t addr;
    uint16_t length; // in samples
    uint16_t flags;  // bit 15: IOC, bit 14: BUP
} __attribute__((packed)) ac97_bd_t;

void ac97_init(uint32_t nambar, uint32_t nabmbar, uint8_t irq);
void ac97_start_capture(void *buffer, uint32_t size);
uint32_t ac97_read_capture(void *buffer, uint32_t size);
void ac97_stop_capture();

void ac97_play_pcm(const void *data, uint32_t size, uint32_t sample_rate, uint8_t bits, uint8_t channels);
void ac97_stream_pcm(const void *data, uint32_t size, uint32_t sample_rate, uint8_t bits, uint8_t channels);
void ac97_get_playback_status(uint8_t *civ, uint16_t *picb, uint16_t *sr);
int ac97_is_playback_done(void);
void ac97_stop_playback(void);

#endif
