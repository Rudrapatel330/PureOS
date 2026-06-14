#include "gfx_device.h"

graphics_device_t *current_gfx_device = 0;

void gfx_device_register(graphics_device_t *dev) {
    current_gfx_device = dev;
}

void gfx_device_flip(void) {
    if (current_gfx_device && current_gfx_device->flip) {
        current_gfx_device->flip();
    }
}

uint32_t *gfx_device_get_render_buffer(void) {
    if (current_gfx_device && current_gfx_device->get_render_buffer) {
        return current_gfx_device->get_render_buffer();
    }
    if (current_gfx_device) {
        return current_gfx_device->lfb_base;
    }
    return 0;
}

void gfx_device_set_mode(int w, int h, int bpp) {
    if (current_gfx_device && current_gfx_device->set_mode) {
        current_gfx_device->set_mode(w, h, bpp);
    }
}

void gfx_device_fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (current_gfx_device && current_gfx_device->fill_rect) {
        current_gfx_device->fill_rect(x, y, w, h, color);
    }
}

void gfx_device_copy_rect(int src_x, int src_y, int dest_x, int dest_y, int w, int h) {
    if (current_gfx_device && current_gfx_device->copy_rect) {
        current_gfx_device->copy_rect(src_x, src_y, dest_x, dest_y, w, h);
    }
}


