#ifndef GFX_DEVICE_H
#define GFX_DEVICE_H

#include <stdint.h>

typedef struct {
    // Info
    int width;
    int height;
    int bpp;
    int pitch;

    // Hardware pointers
    uint32_t *lfb_base;
    
    // Capabilities
    int has_triple_buffer;
    
    // Ops
    void (*flip)(void);
    uint32_t *(*get_render_buffer)(void);
    void (*set_mode)(int w, int h, int bpp);
    
    // HW Accel Ops
    void (*fill_rect)(int x, int y, int w, int h, uint32_t color);
    void (*copy_rect)(int src_x, int src_y, int dest_x, int dest_y, int w, int h);
} graphics_device_t;

extern graphics_device_t *current_gfx_device;

void gfx_device_register(graphics_device_t *dev);
void gfx_device_flip(void);
uint32_t *gfx_device_get_render_buffer(void);
void gfx_device_set_mode(int w, int h, int bpp);

// HW Accel Wrappers
void gfx_device_fill_rect(int x, int y, int w, int h, uint32_t color);
void gfx_device_copy_rect(int src_x, int src_y, int dest_x, int dest_y, int w, int h);

#endif
