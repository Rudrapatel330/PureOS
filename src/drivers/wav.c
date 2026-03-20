#include "wav.h"
#include "../kernel/string.h"
#include "es1370.h"


extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);

int wav_play(const uint8_t *data, uint32_t size) {
  if (!data || size < sizeof(wav_header_t) + 8) {
    print_serial("WAV: Data too small\n");
    return -1;
  }

  wav_header_t *hdr = (wav_header_t *)data;

  // Validate RIFF header
  if (memcmp(hdr->riff_id, "RIFF", 4) != 0 ||
      memcmp(hdr->wave_id, "WAVE", 4) != 0) {
    print_serial("WAV: Not a valid RIFF/WAVE file\n");
    return -1;
  }

  // Validate fmt chunk
  if (memcmp(hdr->fmt_id, "fmt ", 4) != 0 || hdr->audio_format != 1) {
    print_serial("WAV: Not PCM format\n");
    return -1;
  }

  // Find "data" chunk (may not be right after fmt)
  const uint8_t *ptr = data + sizeof(wav_header_t);
  // Skip any extra fmt bytes
  if (hdr->fmt_size > 16)
    ptr += (hdr->fmt_size - 16);

  const uint8_t *end = data + size;
  const uint8_t *pcm_data = 0;
  uint32_t pcm_size = 0;

  while (ptr + 8 <= end) {
    uint32_t chunk_size = *(uint32_t *)(ptr + 4);
    if (memcmp(ptr, "data", 4) == 0) {
      pcm_data = ptr + 8;
      pcm_size = chunk_size;
      if (pcm_data + pcm_size > end)
        pcm_size = (uint32_t)(end - pcm_data);
      break;
    }
    ptr += 8 + chunk_size;
    // Align to 2-byte boundary (WAV spec)
    if (chunk_size & 1)
      ptr++;
  }

  if (!pcm_data || pcm_size == 0) {
    print_serial("WAV: No data chunk found\n");
    return -1;
  }

  char buf[16];
  print_serial("WAV: ");
  k_itoa(hdr->sample_rate, buf);
  print_serial(buf);
  print_serial("Hz ");
  k_itoa(hdr->bits_per_sample, buf);
  print_serial(buf);
  print_serial("bit ");
  k_itoa(hdr->num_channels, buf);
  print_serial(buf);
  print_serial("ch ");
  k_itoa(pcm_size, buf);
  print_serial(buf);
  print_serial(" bytes\n");

  // Play via ES1370
  es1370_play_pcm(pcm_data, pcm_size, hdr->sample_rate, hdr->bits_per_sample,
                  hdr->num_channels);
  return 0;
}
