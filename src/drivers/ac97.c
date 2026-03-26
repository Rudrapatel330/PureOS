#include "ac97.h"
#include "../kernel/hal/isr.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "ports.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);
extern void k_itoa_hex(uint32_t n, char *s);

static uint32_t ac97_nambar = 0;
static uint32_t ac97_nabmbar = 0;
static uint8_t ac97_irq = 0;

#define AC97_BDL_ENTRIES 32
static ac97_bd_t *ac97_po_bdl = 0; 
static ac97_bd_t *ac97_pi_bdl = 0; 

#define AC97_DMA_BUF_COUNT 32
#define AC97_DMA_BUF_SIZE  16384
static uint8_t *ac97_pi_buffers[AC97_DMA_BUF_COUNT];
static int ac97_pi_last_civ = 0;

static void irq_unmask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    if(irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

uint64_t ac97_handler(registers_t *regs) {
    uint16_t pi_sr = inw(ac97_nabmbar + AC97_PI_SR);
    uint16_t po_sr = inw(ac97_nabmbar + AC97_PO_SR);
    
    if (pi_sr & (AC97_SR_BCIS | AC97_SR_LVBCI | AC97_SR_FIFOE)) {
        outw(ac97_nabmbar + AC97_PI_SR, pi_sr & 0x1E); 
    }
    if (po_sr & (AC97_SR_BCIS | AC97_SR_LVBCI | AC97_SR_FIFOE)) {
        outw(ac97_nabmbar + AC97_PO_SR, po_sr & 0x1E);
    }
    return (uint64_t)regs;
}

void ac97_write_mixer(uint8_t reg, uint16_t data) {
    outw(ac97_nambar + reg, data);
}

uint16_t ac97_read_mixer(uint8_t reg) {
    return inw(ac97_nambar + reg);
}

static void wait_codec_ready() {
    int timeout = 20000;
    while (timeout--) {
        uint32_t gsts = inl(ac97_nabmbar + 0x30);
        // Bit 8 is SDIN0 ready, usually sufficient for ICH
        if (gsts & 0x100) return; 
        for(int i=0; i<10; i++) inb(0x80);
    }
    print_serial("AC97: Codec Ready Timeout!\n");
}

void ac97_init(uint32_t nambar, uint32_t nabmbar, uint8_t irq) {
    print_serial("AC97: Initializing (Ultimate V5)...\n");
    ac97_nambar = nambar;
    ac97_nabmbar = nabmbar;
    ac97_irq = irq;

    // Reset Bus Master
    outl(nabmbar + 0x2C, 0); 
    for(int i=0; i<2000; i++) inb(0x80);
    outl(nabmbar + 0x2C, 0x00000002 | 0x00000001); // Cold Reset + IOCE
    
    wait_codec_ready();
    
    // Mixer Reset
    ac97_write_mixer(AC97_RESET, 0);
    wait_codec_ready();
    
    // Unmute Everything aggressively
    ac97_write_mixer(0x02, 0x0000); // Master
    ac97_write_mixer(0x04, 0x0000); // Headphone
    ac97_write_mixer(0x06, 0x0000); // Mono
    ac97_write_mixer(0x0A, 0x0000); // PC Beep
    ac97_write_mixer(0x0C, 0x0000); // Phone
    ac97_write_mixer(0x0E, 0x0000); // Mic
    ac97_write_mixer(0x10, 0x0000); // Line In
    ac97_write_mixer(0x12, 0x0000); // CD
    ac97_write_mixer(0x14, 0x0000); // Video
    ac97_write_mixer(0x16, 0x0000); // Aux
    ac97_write_mixer(0x18, 0x0000); // PCM Out
    
    // Enable VRA
    ac97_write_mixer(0x2A, ac97_read_mixer(0x2A) | 0x0001);
    
    // Frequencies
    ac97_write_mixer(0x2E, 0xBB80); // Front DAC
    ac97_write_mixer(0x32, 0xBB80); // Record
    
    // Mic Boost
    ac97_write_mixer(AC97_MIC_VOLUME, 0x0040); // 20dB Boost
    ac97_write_mixer(AC97_REC_GAIN, 0x0F0F); 
    ac97_write_mixer(AC97_REC_SELECT, 0x0000); // Mic
    
    // BDLs
    ac97_po_bdl = (ac97_bd_t *)kmalloc_ap(AC97_BDL_ENTRIES * sizeof(ac97_bd_t), 0);
    ac97_pi_bdl = (ac97_bd_t *)kmalloc_ap(AC97_BDL_ENTRIES * sizeof(ac97_bd_t), 0);

    for (int i = 0; i < AC97_DMA_BUF_COUNT; i++) {
        ac97_pi_buffers[i] = (uint8_t *)kmalloc(AC97_DMA_BUF_SIZE);
        ac97_pi_bdl[i].addr = (uint32_t)(uintptr_t)ac97_pi_buffers[i];
        ac97_pi_bdl[i].length = AC97_DMA_BUF_SIZE / 2;
        ac97_pi_bdl[i].flags = 0x8000;
    }

    register_interrupt_handler(32 + irq, (isr_t)ac97_handler);
    irq_unmask(irq);
}

void ac97_start_capture(void *buffer, uint32_t size) {
    (void)buffer; (void)size;
    ac97_pi_last_civ = 0;
    outb(ac97_nabmbar + AC97_PI_CR, 0x02); // Reset
    for(int i=0; i<200; i++) inb(0x80);
    outl(ac97_nabmbar + AC97_PI_BDBA, (uint32_t)(uintptr_t)ac97_pi_bdl);
    outb(ac97_nabmbar + AC97_PI_LVI, AC97_DMA_BUF_COUNT - 1);
    outb(ac97_nabmbar + AC97_PI_CR, 0x01 | 0x10); 
}

uint32_t ac97_read_capture(void *buffer, uint32_t size) {
    uint8_t civ = inb(ac97_nabmbar + AC97_PI_CIV);
    if (civ == ac97_pi_last_civ) return 0;
    
    uint8_t *src = ac97_pi_buffers[ac97_pi_last_civ];
    uint32_t to_copy = (size < AC97_DMA_BUF_SIZE) ? size : AC97_DMA_BUF_SIZE;
    memcpy(buffer, src, to_copy);
    
    ac97_pi_last_civ = (ac97_pi_last_civ + 1) % AC97_DMA_BUF_COUNT;
    outb(ac97_nabmbar + AC97_PI_LVI, (ac97_pi_last_civ + 31) % AC97_DMA_BUF_COUNT);
    return to_copy;
}

void ac97_stop_capture() {
    outb(ac97_nabmbar + AC97_PI_CR, 0); 
}

void ac97_play_pcm(const void *data, uint32_t size, uint32_t sample_rate, uint8_t bits, uint8_t channels) {
    (void)sample_rate; (void)bits; (void)channels;
    
    outb(ac97_nabmbar + AC97_PO_CR, 0x02); // Reset
    for(int i=0; i<200; i++) inb(0x80);
    
    uint32_t offset = 0;
    int entry = 0;
    // Map as much as we can into BDL entries (up to 2MB with 32 entries of 64kb each)
    while (offset < size && entry < AC97_BDL_ENTRIES) {
        uint32_t chunk = (size - offset > 65534) ? 65534 : (size - offset);
        ac97_po_bdl[entry].addr = (uint32_t)(uintptr_t)((uint8_t*)data + offset);
        ac97_po_bdl[entry].length = chunk / 2;
        ac97_po_bdl[entry].flags = 0x8000;
        offset += chunk;
        entry++;
    }
    
    if (entry > 0) {
        outl(ac97_nabmbar + AC97_PO_BDBA, (uint32_t)(uintptr_t)ac97_po_bdl);
        outb(ac97_nabmbar + AC97_PO_LVI, entry - 1);
        outb(ac97_nabmbar + AC97_PO_CR, 0x01 | 0x10); 
        print_serial("AC97: Playback Start. Entries: ");
        char buf[8]; k_itoa(entry, buf); print_serial(buf); print_serial("\n");
    }
}

void ac97_get_playback_status(uint8_t *civ, uint16_t *picb, uint16_t *sr) {
    if (civ) *civ = inb(ac97_nabmbar + AC97_PO_CIV);
    if (picb) *picb = inw(ac97_nabmbar + AC97_PO_PICB);
    if (sr) *sr = inw(ac97_nabmbar + AC97_PO_SR);
}

int ac97_is_playback_done(void) {
    uint16_t sr = inw(ac97_nabmbar + AC97_PO_SR);
    // DCH (halted) + CELV (current equals last valid) = playback finished
    return (sr & (AC97_SR_DCH | AC97_SR_CELV)) == (AC97_SR_DCH | AC97_SR_CELV);
}
