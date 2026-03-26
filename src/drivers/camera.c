#include "camera.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "usb/uvc.h"

static camera_ctx_t *camera_ctx = 0;
static uint32_t camera_tick = 0;
extern int uvc_is_streaming; // Defined in uvc.c

void camera_init(void) {
    camera_ctx = (camera_ctx_t *)kmalloc(sizeof(camera_ctx_t));
    if (!camera_ctx) return;

    camera_ctx->width = 160;
    camera_ctx->height = 120;
    camera_ctx->frame_buffer = (uint32_t *)kmalloc(camera_ctx->width * camera_ctx->height * 4);
    if (!camera_ctx->frame_buffer) {
        kfree(camera_ctx);
        camera_ctx = 0;
        return;
    }
    memset(camera_ctx->frame_buffer, 0, camera_ctx->width * camera_ctx->height * 4);
    camera_ctx->is_active = 1;
}

void camera_update(void) {
    if (!camera_ctx || !camera_ctx->is_active) return;

    // If we have a real UVC camera, let it handle the frame buffer
    if (uvc_is_streaming) {
        uvc_update();
        return;
    }

    // Otherwise, show the test pattern (Rainbow)
    camera_tick++;
    uint32_t *fb = camera_ctx->frame_buffer;
    uint32_t w = camera_ctx->width;
    uint32_t h = camera_ctx->height;

    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            uint8_t r = (uint8_t)(x + camera_tick);
            uint8_t g = (uint8_t)(y + camera_tick * 2);
            uint8_t b = (uint8_t)(x + y + camera_tick / 2);
            if ((y + camera_tick / 4) % 20 < 2) {
                r /= 2; g /= 2; b /= 2;
            }
            fb[y * w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }
}

camera_ctx_t *camera_get_ctx(void) {
    return camera_ctx;
}
