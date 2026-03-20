#ifndef WAV_H
#define WAV_H

#include <stdint.h>

// WAV file header structure
typedef struct {
  char riff_id[4];       // "RIFF"
  uint32_t file_size;    // File size - 8
  char wave_id[4];       // "WAVE"
  char fmt_id[4];        // "fmt "
  uint32_t fmt_size;     // Format chunk size (16 for PCM)
  uint16_t audio_format; // 1 = PCM
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
} __attribute__((packed)) wav_header_t;

// Parse and play a WAV file from memory
// Returns 0 on success, -1 on error
int wav_play(const uint8_t *data, uint32_t size);

#endif
