#include "es1370.h"
#include "../kernel/hal/isr.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "ports.h"

extern void print_serial(const char *str);

static es1370_device_t audio_dev;

#define DMA_BUFFER_SIZE 16384
static uint8_t *dma_buffer = 0;
static uint8_t *adc_dma_buffer = 0;
static uint32_t last_adc_pos = 0;
static int capture_bits = 8;
static int capture_channels = 1;

void es1370_handler(registers_t *regs) {
  (void)regs;
  // Handle ES1370 interrupt (DMA completion, etc.)
  uint32_t status = inl(audio_dev.io_base + ES1370_REG_STATUS);
  if (status & STAT_INTR) {
    // Clear interrupt
    // For ES1370, we might need to clear specific bits in SERIAL_CTRL or STATUS
    // Usually reading/writing STATUS or specific DMA registers clears it.
    // print_serial("S");
  }
}

void es1370_init(uint32_t io_base, uint8_t irq) {
  print_serial("ES1370: Initializing AudioPCI...\n");

  audio_dev.io_base = io_base;
  audio_dev.irq = irq;

  // 1. Reset the chip
  outl(io_base + ES1370_REG_CONTROL, 0);

  // 2. Allocate DMA buffer (kmalloc is identity mapped for low memory)
  dma_buffer = (uint8_t *)kmalloc(DMA_BUFFER_SIZE);
  if (!dma_buffer) {
    print_serial("ES1370: Failed to allocate DMA buffer\n");
    return;
  }
  memset(dma_buffer, 0, DMA_BUFFER_SIZE);

  adc_dma_buffer = (uint8_t *)kmalloc(DMA_BUFFER_SIZE);
  if (!adc_dma_buffer) {
    print_serial("ES1370: Failed to allocate ADC DMA buffer\n");
  } else {
    memset(adc_dma_buffer, 0, DMA_BUFFER_SIZE);
  }

  // 3. Set up DMA for DAC2 (Page 1)
  outl(io_base + ES1370_REG_MEM_PAGE, 1); // Page 1 = DAC2
  outl(io_base + ES1370_REG_DAC2_FRAMEADR, (uint32_t)(uintptr_t)dma_buffer);
  outl(io_base + ES1370_REG_DAC2_FRAMECNT, (DMA_BUFFER_SIZE / 4) - 1);

  // 4. Set sample rate (44.1kHz) and enabled DAC2
  uint32_t ctrl = inl(io_base + ES1370_REG_CONTROL);
  ctrl &= ~0x3000; // Clear WTSRSEL
  ctrl |= CTRL_WTSRSEL_44KHZ;
  ctrl |= CTRL_DAC2_EN;
  outl(io_base + ES1370_REG_CONTROL, ctrl);

  // 5. Configure Serial Control (DAC2 format: 8-bit Mono, Loop)
  uint32_t sctrl = inl(io_base + ES1370_REG_SERIAL_CTRL);
  sctrl &= ~SCTRL_P2_PAUSE;   // Ensure not paused
  sctrl |= SCTRL_P2_LOOP_SEL; // Loop playback
  sctrl |= SCTRL_P2_S_EB;     // Enable DAC2
  sctrl &= ~SCTRL_P2_FORMAT_16BIT;
  sctrl &= ~SCTRL_P2_FORMAT_STEREO;
  // Explicitly disable interrupts to prevent storm since we're polling
  sctrl &= ~((1 << 21) | (1 << 19) | (1 << 17)); // P2_INT_EN, R1_INT_EN, P1_INT_EN
  outl(io_base + ES1370_REG_SERIAL_CTRL, sctrl);

  // 6. Register interrupt handler
  register_interrupt_handler(32 + irq, (isr_t)es1370_handler);

  print_serial("ES1370: Initialized at 0x");
  char buf[16];
  k_itoa_hex(audio_dev.io_base, buf);
  print_serial(buf);
  print_serial(" IRQ ");
  k_itoa(32 + audio_dev.irq, buf);
  print_serial(buf);
  print_serial("\n");
}

void es1370_play_test_beep() {
  if (!dma_buffer)
    return;
  print_serial("ES1370: Filling buffer with test wave...\n");
  for (uint32_t i = 0; i < DMA_BUFFER_SIZE; i++) {
    // Square wave: 440Hz roughly (44100/440 = ~100 samples per period)
    dma_buffer[i] = (i / 50) % 2 ? 0x90 : 0x70;
  }
}

void es1370_play_pcm(const uint8_t *data, uint32_t size, uint32_t sample_rate,
                     int bits, int channels) {
  if (!dma_buffer || !data || size == 0)
    return;

  print_serial("ES1370: Playing PCM audio...\n");

  // Copy data to DMA buffer (truncate if too large)
  uint32_t copy_size = (size > DMA_BUFFER_SIZE) ? DMA_BUFFER_SIZE : size;
  memcpy(dma_buffer, data, copy_size);

  // Zero any remaining buffer
  if (copy_size < DMA_BUFFER_SIZE)
    memset(dma_buffer + copy_size, 0, DMA_BUFFER_SIZE - copy_size);

  // Determine sample rate selector
  uint32_t srsel;
  if (sample_rate <= 7000)
    srsel = CTRL_WTSRSEL_5KHZ;
  else if (sample_rate <= 16000)
    srsel = CTRL_WTSRSEL_11KHZ;
  else if (sample_rate <= 33000)
    srsel = CTRL_WTSRSEL_22KHZ;
  else
    srsel = CTRL_WTSRSEL_44KHZ;

  // Update control register with new sample rate
  uint32_t ctrl = inl(audio_dev.io_base + ES1370_REG_CONTROL);
  ctrl &= ~0x3000; // Clear WTSRSEL
  ctrl |= srsel;
  ctrl |= CTRL_DAC2_EN;
  outl(audio_dev.io_base + ES1370_REG_CONTROL, ctrl);

  // Configure format (16-bit, stereo, etc.)
  uint32_t sctrl = inl(audio_dev.io_base + ES1370_REG_SERIAL_CTRL);
  sctrl &= ~SCTRL_P2_PAUSE;
  sctrl |= SCTRL_P2_LOOP_SEL;
  sctrl |= SCTRL_P2_S_EB;

  // Set format bits
  if (bits == 16)
    sctrl |= SCTRL_P2_FORMAT_16BIT;
  else
    sctrl &= ~SCTRL_P2_FORMAT_16BIT;

  if (channels == 2)
    sctrl |= SCTRL_P2_FORMAT_STEREO;
  else
    sctrl &= ~SCTRL_P2_FORMAT_STEREO;

  outl(audio_dev.io_base + ES1370_REG_SERIAL_CTRL, sctrl);

  // Set up DMA (reload frame address and count)
  outl(audio_dev.io_base + ES1370_REG_MEM_PAGE, 1); // Page 1 = DAC2
  outl(audio_dev.io_base + ES1370_REG_DAC2_FRAMEADR,
       (uint32_t)(uintptr_t)dma_buffer);

  // Frame count depends on format:
  // For 16-bit stereo: each frame = 4 bytes (2 channels * 2 bytes)
  // For 8-bit mono: each frame = 1 byte
  uint32_t frame_size = ((bits == 16) ? 2 : 1) * channels;
  uint32_t frame_count = copy_size / frame_size;
  outl(audio_dev.io_base + ES1370_REG_DAC2_FRAMECNT, frame_count - 1);
}

void es1370_set_volume(uint8_t left, uint8_t right) {
  // ES1370 uses the AK4531 codec. Volume registers are accessed via
  // the CODEC register (offset 0x10). Format: [addr(8):data(8):reserved(16)]
  // Master volume: register 0x00 (left), 0x01 (right)
  // Value 0x00 = max volume, 0x1F = muted

  // Invert: our API uses 0=silent, 255=max
  uint8_t l_val = (255 - left) >> 3; // Map 0-255 to 31-0
  uint8_t r_val = (255 - right) >> 3;

  // Write left master
  outw(audio_dev.io_base + ES1370_REG_CODEC, (0x00 << 8) | l_val);
  for (volatile int i = 0; i < 10000; i++)
    ; // Small delay

  // Write right master
  outw(audio_dev.io_base + ES1370_REG_CODEC, (0x01 << 8) | r_val);
  for (volatile int i = 0; i < 10000; i++)
    ;

  print_serial("ES1370: Volume set\n");
}

void es1370_stop(void) {
  if (!audio_dev.io_base)
    return;

  // Pause DAC2
  uint32_t sctrl = inl(audio_dev.io_base + ES1370_REG_SERIAL_CTRL);
  sctrl |= SCTRL_P2_PAUSE;
  outl(audio_dev.io_base + ES1370_REG_SERIAL_CTRL, sctrl);

  // Disable DAC2
  uint32_t ctrl = inl(audio_dev.io_base + ES1370_REG_CONTROL);
  ctrl &= ~CTRL_DAC2_EN;
  outl(audio_dev.io_base + ES1370_REG_CONTROL, ctrl);

  print_serial("ES1370: Playback stopped\n");
}

void es1370_start_capture(uint32_t sample_rate, int bits, int channels) {
  if (!adc_dma_buffer) return;

  capture_bits = bits;
  capture_channels = channels;
  last_adc_pos = 0;

  print_serial("ES1370: Starting audio capture (");
  char b[16];
  k_itoa(sample_rate, b); print_serial(b); print_serial("Hz, ");
  k_itoa(bits, b); print_serial(b); print_serial("bit, ");
  k_itoa(channels, b); print_serial(b); print_serial("ch)...\n");

  uint32_t srsel;
  if (sample_rate <= 7000) srsel = CTRL_WTSRSEL_5KHZ;
  else if (sample_rate <= 16000) srsel = CTRL_WTSRSEL_11KHZ;
  else if (sample_rate <= 33000) srsel = CTRL_WTSRSEL_22KHZ;
  else srsel = CTRL_WTSRSEL_44KHZ;

  uint32_t ctrl = inl(audio_dev.io_base + ES1370_REG_CONTROL);
  ctrl &= ~0x3000;
  ctrl |= srsel;
  ctrl |= CTRL_ADC_EN;
  outl(audio_dev.io_base + ES1370_REG_CONTROL, ctrl);

  uint32_t sctrl = inl(audio_dev.io_base + ES1370_REG_SERIAL_CTRL);
  sctrl &= ~SCTRL_R1_PAUSE;
  sctrl |= SCTRL_R1_LOOP_SEL;
  sctrl |= SCTRL_R1_S_EB;

  if (bits == 16) sctrl |= SCTRL_R1_FORMAT_16BIT;
  else sctrl &= ~SCTRL_R1_FORMAT_16BIT;

  if (channels == 2) sctrl |= SCTRL_R1_FORMAT_STEREO;
  else sctrl &= ~SCTRL_R1_FORMAT_STEREO;

  outl(audio_dev.io_base + ES1370_REG_SERIAL_CTRL, sctrl);

  outl(audio_dev.io_base + ES1370_REG_ADC_FRAMEADR, (uint32_t)(uintptr_t)adc_dma_buffer);
  
  uint32_t frame_size = ((bits == 16) ? 2 : 1) * channels;
  uint32_t frame_count = (DMA_BUFFER_SIZE / frame_size);
  outl(audio_dev.io_base + ES1370_REG_ADC_FRAMECNT, frame_count - 1);
}

void es1370_stop_capture(void) {
  if (!audio_dev.io_base) return;

  uint32_t sctrl = inl(audio_dev.io_base + ES1370_REG_SERIAL_CTRL);
  sctrl |= SCTRL_R1_PAUSE;
  outl(audio_dev.io_base + ES1370_REG_SERIAL_CTRL, sctrl);

  uint32_t ctrl = inl(audio_dev.io_base + ES1370_REG_CONTROL);
  ctrl &= ~CTRL_ADC_EN;
  outl(audio_dev.io_base + ES1370_REG_CONTROL, ctrl);

  print_serial("ES1370: Capture stopped\n");
}

int es1370_read_capture(uint8_t *buffer, uint32_t max_size) {
  if (!adc_dma_buffer) return 0;
  
  uint32_t frame_size = ((capture_bits == 16) ? 2 : 1) * capture_channels;
  uint32_t frames_total = DMA_BUFFER_SIZE / frame_size;
  uint32_t scount = inl(audio_dev.io_base + ES1370_REG_ADC_SCOUNT) & 0xFFFF;
  
  uint32_t current_frame = frames_total - scount - 1; 
  if (current_frame >= frames_total) current_frame = 0;
  
  uint32_t current_pos = current_frame * frame_size;
  
  
  if (current_pos == last_adc_pos) return 0;
  
  // print_serial("D"); 
  
  uint32_t available = 0;
  if (current_pos > last_adc_pos) {
      available = current_pos - last_adc_pos;
  } else {
      available = DMA_BUFFER_SIZE - last_adc_pos + current_pos;
  }
  
  if (available > max_size) available = max_size;
  
  if (current_pos > last_adc_pos) {
      memcpy(buffer, adc_dma_buffer + last_adc_pos, available);
  } else {
      uint32_t part1 = DMA_BUFFER_SIZE - last_adc_pos;
      if (part1 > available) part1 = available;
      memcpy(buffer, adc_dma_buffer + last_adc_pos, part1);
      if (part1 < available) {
          memcpy(buffer + part1, adc_dma_buffer, available - part1);
      }
  }
  
  last_adc_pos = (last_adc_pos + available) % DMA_BUFFER_SIZE;
  return available;
}
