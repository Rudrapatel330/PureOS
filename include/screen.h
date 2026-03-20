#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

// Direct definitions for the kernel (Single source of truth)
// These should match the VESA mode set in stage2.asm (0x117)
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define SCREEN_BPP    16

// External variables for dynamic access (if needed)
extern int screen_width;
extern int screen_height;

#endif
