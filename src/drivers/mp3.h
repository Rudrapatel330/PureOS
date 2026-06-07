#ifndef MP3_H
#define MP3_H

#include <stdint.h>

// Parse and play an MP3 file from memory
// Returns 0 on success, -1 on error
int mp3_play(const uint8_t *data, uint32_t size);
void mp3_stop(void);
void mp3_get_progress(uint32_t *pos_ms, uint32_t *dur_ms);

#endif
