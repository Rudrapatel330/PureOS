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

#define AC97_BD_FLAGS_IOC 0x8000

#define AC97_BDL_ENTRIES 32
static ac97_bd_t *ac97_po_bdl = 0; 
static uint32_t ac97_po_bdl_phys = 0;
static ac97_bd_t *ac97_pi_bdl = 0; 
static uint32_t ac97_pi_bdl_phys = 0;

#define AC97_DMA_BUF_COUNT 32
#define AC97_DMA_BUF_SIZE  16384
#define AC97_PI_BUF_SIZE   1024   
static uint8_t *ac97_pi_buffers[AC97_DMA_BUF_COUNT];
static uint32_t ac97_pi_buffers_phys[AC97_DMA_BUF_COUNT];
static int ac97_pi_last_civ = 0;

// ============== STREAMING PLAYBACK RING BUFFER ==============
#define AC97_PO_RING_SIZE  (48000 * 2 * 2 * 2)  // 2 seconds ring buffer
static uint8_t *ac97_po_ring = 0;
static uint32_t ac97_po_ring_phys = 0;
static volatile uint32_t ac97_po_write_pos = 0;
static int ac97_po_running = 0;

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
    
    // Dump registers for debug
    char rdb[64];
    strcpy(rdb, "AC97: Mixer Master="); k_itoa_hex(ac97_read_mixer(0x02), rdb+20); print_serial(rdb); print_serial("\n");
    strcpy(rdb, "AC97: Mixer PCM   ="); k_itoa_hex(ac97_read_mixer(0x18), rdb+20); print_serial(rdb); print_serial("\n");
    strcpy(rdb, "AC97: Mixer Phone ="); k_itoa_hex(ac97_read_mixer(0x0C), rdb+20); print_serial(rdb); print_serial("\n");
    
    // BDLs and Ring Buffer
    ac97_po_bdl = (ac97_bd_t *)kmalloc_ap(AC97_BDL_ENTRIES * sizeof(ac97_bd_t), &ac97_po_bdl_phys);
    ac97_pi_bdl = (ac97_bd_t *)kmalloc_ap(AC97_BDL_ENTRIES * sizeof(ac97_bd_t), &ac97_pi_bdl_phys);
    ac97_po_ring = (uint8_t *)kmalloc_ap(AC97_PO_RING_SIZE, &ac97_po_ring_phys);
    memset(ac97_po_ring, 0, AC97_PO_RING_SIZE);

    for (int i = 0; i < AC97_DMA_BUF_COUNT; i++) {
        ac97_pi_buffers[i] = (uint8_t *)kmalloc_ap(AC97_PI_BUF_SIZE, &ac97_pi_buffers_phys[i]);
        ac97_pi_bdl[i].addr = ac97_pi_buffers_phys[i];
        ac97_pi_bdl[i].length = AC97_PI_BUF_SIZE / 2;
        ac97_pi_bdl[i].flags = AC97_BD_FLAGS_IOC;
    }

    register_interrupt_handler(32 + irq, (isr_t)ac97_handler);
    irq_unmask(irq);
}

void ac97_start_capture(void *buffer, uint32_t size) {
    (void)buffer; (void)size;
    ac97_pi_last_civ = 0;
    
    // Reset the PCM-In DMA engine
    outb(ac97_nabmbar + AC97_PI_CR, 0x02);
    for(int i=0; i<1000; i++) inb(0x80);
    
    // Clear reset bit explicitly
    outb(ac97_nabmbar + AC97_PI_CR, 0x00);
    for(int i=0; i<200; i++) inb(0x80);
    
    // Clear any pending status bits
    outw(ac97_nabmbar + AC97_PI_SR, 0x1E);
    
    // Set BDL base address (Physical)
    outl(ac97_nabmbar + AC97_PI_BDBA, ac97_pi_bdl_phys);
    
    // Set Last Valid Index to wrap around all buffers
    outb(ac97_nabmbar + AC97_PI_LVI, AC97_DMA_BUF_COUNT - 1);
    
    // Start DMA: Run + IOCE (interrupt on completion enable)
    outb(ac97_nabmbar + AC97_PI_CR, 0x01 | 0x10);
    
    // Debug: print state after start
    char buf[16];
    uint8_t civ = inb(ac97_nabmbar + AC97_PI_CIV);
    uint8_t cr = inb(ac97_nabmbar + AC97_PI_CR);
    uint16_t sr = inw(ac97_nabmbar + AC97_PI_SR);
    print_serial("AC97 PI: Start Capture. CR=0x");
    k_itoa_hex(cr, buf); print_serial(buf);
    print_serial(" SR=0x");
    k_itoa_hex(sr, buf); print_serial(buf);
    print_serial(" CIV=");
    k_itoa(civ, buf); print_serial(buf);
    print_serial(" LVI=");
    k_itoa(AC97_DMA_BUF_COUNT - 1, buf); print_serial(buf);
    print_serial(" BDL=0x");
    k_itoa_hex((uint32_t)(uintptr_t)ac97_pi_bdl, buf); print_serial(buf);
    print_serial("\n");
}

static int ac97_capture_dbg_count = 0;

uint32_t ac97_read_capture(void *buffer, uint32_t size) {
    uint8_t civ = inb(ac97_nabmbar + AC97_PI_CIV);
    
    // Debug print every 200 calls to see if DMA is advancing
    if (ac97_capture_dbg_count++ % 200 == 0) {
        char buf[16];
        uint16_t sr = inw(ac97_nabmbar + AC97_PI_SR);
        print_serial("AC97 PI poll: CIV=");
        k_itoa(civ, buf); print_serial(buf);
        print_serial(" last=");
        k_itoa(ac97_pi_last_civ, buf); print_serial(buf);
        print_serial(" SR=0x");
        k_itoa_hex(sr, buf); print_serial(buf);
        print_serial("\n");
    }
    
    if (civ == ac97_pi_last_civ) return 0;
    
    uint8_t *src = ac97_pi_buffers[ac97_pi_last_civ];
    uint32_t to_copy = (size < AC97_PI_BUF_SIZE) ? size : AC97_PI_BUF_SIZE;
    memcpy(buffer, src, to_copy);
    
    ac97_pi_last_civ = (ac97_pi_last_civ + 1) % AC97_DMA_BUF_COUNT;
    
    // Clear all pending status bits (IOC, FPI, LVBCI)
    outw(ac97_nabmbar + AC97_PI_SR, 0x1E); 
    
    // Advance LVI to keep the DMA running
    outb(ac97_nabmbar + AC97_PI_LVI, (ac97_pi_last_civ + 31) % AC97_DMA_BUF_COUNT);
    return to_copy;
}

void ac97_stop_capture() {
    outb(ac97_nabmbar + AC97_PI_CR, 0); 
}

// Rebuild BDL to map the entire ring buffer, split into 32 entries
static void ac97_po_setup_bdl(void) {
    uint32_t chunk_size = AC97_PO_RING_SIZE / AC97_BDL_ENTRIES;
    for (int i = 0; i < AC97_BDL_ENTRIES; i++) {
        ac97_po_bdl[i].addr = ac97_po_ring_phys + i * chunk_size;
        ac97_po_bdl[i].length = chunk_size / 2;  // in 16-bit samples
        ac97_po_bdl[i].flags = AC97_BD_FLAGS_IOC;
    }
}

void ac97_stream_pcm(const void *data, uint32_t size, uint32_t sample_rate, uint8_t bits, uint8_t channels) {
    (void)sample_rate; (void)bits; (void)channels;
    const uint8_t *src = (const uint8_t *)data;

    uint32_t chunk_size = AC97_PO_RING_SIZE / AC97_BDL_ENTRIES;
    uint8_t civ = inb(ac97_nabmbar + AC97_PO_CIV);
    uint32_t read_pos = civ * chunk_size;

    // 1. Manage Synchronization
    if (!ac97_po_running) {
        // Start 4 chunks ahead (~125ms)
        ac97_po_write_pos = (read_pos + chunk_size * 4) % AC97_PO_RING_SIZE;
        memset(ac97_po_ring, 0, AC97_PO_RING_SIZE);
    } else {
        uint32_t ahead = (ac97_po_write_pos >= read_pos) ? 
                         (ac97_po_write_pos - read_pos) : 
                         (AC97_PO_RING_SIZE - read_pos + ac97_po_write_pos);
        
        // CATCH-UP LOGIC:
        // Underrun threshold: chunk_size (approx 31ms) - tolerant of normal jitter
        // Lag threshold: chunk_size * 6 (approx 187ms) - catch real drift sooner
        if (ahead > chunk_size * 12 || ahead < chunk_size / 2) {
            ac97_po_write_pos = (read_pos + chunk_size * 4) % AC97_PO_RING_SIZE;
            
            uint32_t clear_ptr = ac97_po_write_pos;
            uint32_t clear_size = chunk_size * 2; // ~62 ms
            for (uint32_t c = 0; c < clear_size; c++) {
                ac97_po_ring[(clear_ptr + c) % AC97_PO_RING_SIZE] = 0;
            }
        }
    }

    // 2. Copy data into ring buffer
    for (uint32_t i = 0; i < size; i++) {
        ac97_po_ring[ac97_po_write_pos] = src[i];
        ac97_po_write_pos = (ac97_po_write_pos + 1) % AC97_PO_RING_SIZE;
    }

    // 3. Keep Hardware Running
    if (!ac97_po_running) {
        ac97_po_setup_bdl();
        
        // Reset Mixer to max volume
        ac97_write_mixer(0x02, 0x0000); 
        ac97_write_mixer(0x18, 0x0000); 
        
        // Reset Controller
        outb(ac97_nabmbar + AC97_PO_CR, 0x02); // Reset
        for(int i=0; i<1000; i++) inb(0x80);
        outb(ac97_nabmbar + AC97_PO_CR, 0x00);
        for(int i=0; i<500; i++) inb(0x80);
        outw(ac97_nabmbar + AC97_PO_SR, 0x1E); 
        
        outl(ac97_nabmbar + AC97_PO_BDBA, ac97_po_bdl_phys);
        outb(ac97_nabmbar + AC97_PO_LVI, AC97_BDL_ENTRIES - 1);
        outb(ac97_nabmbar + AC97_PO_CR, 0x01 | 0x10); // RUN + IOCE
        
        ac97_po_running = 1;
        print_serial("AC97: Streaming playback started\n");
    } else {
        // Continuous maintenance: update LVI and ensure RUN bit
        outb(ac97_nabmbar + AC97_PO_LVI, (civ + 31) % AC97_BDL_ENTRIES);
        outb(ac97_nabmbar + AC97_PO_CR, inb(ac97_nabmbar + AC97_PO_CR) | 0x01);
    }
}

void ac97_play_pcm(const void *data, uint32_t size, uint32_t sample_rate, uint8_t bits, uint8_t channels) {
    (void)sample_rate; (void)bits; (void)channels;
    ac97_po_running = 0; // Mutually exclusive with streaming

    uint32_t total_samples = size / 2; // assuming 16-bit
    uint32_t samples_remaining = total_samples;
    const uint16_t *src_ptr = (const uint16_t *)data;
    int bdl_idx = 0;

    while (samples_remaining > 0 && bdl_idx < AC97_BDL_ENTRIES) {
        uint32_t chunk_samples = samples_remaining;
        if (chunk_samples > 65535) chunk_samples = 65535;

        ac97_po_bdl[bdl_idx].addr = (uint32_t)(uintptr_t)src_ptr;
        ac97_po_bdl[bdl_idx].length = chunk_samples;
        
        samples_remaining -= chunk_samples;
        src_ptr += chunk_samples;
        
        if (samples_remaining == 0) {
            ac97_po_bdl[bdl_idx].flags = 0x8000; // IOC for last entry
        } else {
            ac97_po_bdl[bdl_idx].flags = 0;
        }
        bdl_idx++;
    }

    outb(ac97_nabmbar + AC97_PO_CR, 0x02); // Reset
    for(int i=0; i<500; i++) inb(0x80);
    outb(ac97_nabmbar + AC97_PO_CR, 0x00); // Clear reset
    for(int i=0; i<200; i++) inb(0x80);
    outw(ac97_nabmbar + AC97_PO_SR, 0x1E); // Clear status

    outl(ac97_nabmbar + AC97_PO_BDBA, (uint32_t)(uintptr_t)ac97_po_bdl);
    outb(ac97_nabmbar + AC97_PO_LVI, bdl_idx - 1);
    outb(ac97_nabmbar + AC97_PO_CR, 0x01 | 0x10); // Run + IOCE
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

void ac97_stop_playback(void) {
    if (ac97_po_running) {
        // Stop the DMA engine
        outb(ac97_nabmbar + AC97_PO_CR, 0); 
        ac97_po_running = 0;
        print_serial("AC97: Streaming playback stopped\n");
    }
    // Clear ring buffer unconditionally so we don't play stale audio later
    memset(ac97_po_ring, 0, AC97_PO_RING_SIZE);
    ac97_po_write_pos = 0;
}
