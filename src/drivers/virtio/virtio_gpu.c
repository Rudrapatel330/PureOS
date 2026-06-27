#include "virtio_gpu.h"
#include "../../kernel/hal/gfx_device.h"

extern void print_serial(const char *str);
extern void k_itoa_hex(uint32_t value, char *str);

static graphics_device_t vgpu_dev;
static virtqueue_t controlq;
static virtqueue_t cursorq;

static uint32_t *vgpu_fb = 0;
static int vgpu_width = 1600;
static int vgpu_height = 900;

static void vgpu_flip(void) {
    // Send TRANSFER_TO_HOST_2D
    // Send RESOURCE_FLUSH
}

static uint32_t *vgpu_get_render_buffer(void) {
    return vgpu_fb;
}

void virtio_gpu_init_device(void) {
    print_serial("VIRTIO-GPU: Initializing device...\n");
    
    // 1. Reset device
    // 2. Acknowledge device
    // 3. Negotiate features
    // 4. Setup virtqueues (controlq and cursorq)
    // 5. DRIVER_OK
    
    // Allocate framebuffer in RAM
    // Send RESOURCE_CREATE_2D
    // Send RESOURCE_ATTACH_BACKING
    // Send SET_SCANOUT
    
    // Register the graphics device abstraction
    // vgpu_dev.width = vgpu_width;
    // vgpu_dev.height = vgpu_height;
    // vgpu_dev.bpp = 32;
    // vgpu_dev.pitch = vgpu_width;
    // vgpu_dev.lfb_base = vgpu_fb;
    // vgpu_dev.has_triple_buffer = 0;
    // vgpu_dev.flip = vgpu_flip;
    // vgpu_dev.get_render_buffer = vgpu_get_render_buffer;
    // vgpu_dev.set_mode = 0;
    // gfx_device_register(&vgpu_dev);
}
