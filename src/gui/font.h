#ifndef FONT_H
#define FONT_H

#include "../kernel/types.h"

extern const uint8_t font8x8_basic[256][8];
extern const uint8_t font16x16_aa[256][16][16];

// Anti-aliased font rendering (grayscale coverage)
void font_init_aa(void);
uint8_t font_get_aa_pixel(unsigned char c, int x, int y);
uint8_t font_get_aa_pixel_16(unsigned char c, int x, int y);
int font_get_width_16(unsigned char c);

#endif

