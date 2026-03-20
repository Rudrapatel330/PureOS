#include "vbe.h"

// Pointers to the info stored by bootloader
u32* fb_phy_addr = (u32*)0x7E04;
u16* scr_width = (u16*)0x7E08;
u16* scr_height = (u16*)0x7E0A;
u8* scr_bpp = (u8*)0x7E0C;
u16* scr_pitch = (u16*)0x7E0E;

u32* framebuffer;
int vbe_screen_width;
int vbe_screen_height;
int screen_pitch;
int screen_bpp;

void init_vbe_driver() {
    screen_width = *scr_width;
    screen_height = *scr_height;
    screen_bpp = *scr_bpp;
    screen_pitch = *scr_pitch;
    framebuffer = (u32*)(*fb_phy_addr);
}

u32* vbe_get_framebuffer() {
    return framebuffer;
}

int vbe_get_width() { return screen_width; }
int vbe_get_height() { return screen_height; }
int vbe_get_bpp() { return screen_bpp; }
