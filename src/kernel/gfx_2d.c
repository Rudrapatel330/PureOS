#include "gfx_2d.h"
#include "simd.h"
#include "hal/gfx_device.h"

void gfx_fill_rect(uint32_t *dest, uint32_t color, int width, int height,
                   int dest_pitch) {
  if (width <= 0 || height <= 0 || !dest)
    return;
    
  uint32_t *fb = gfx_device_get_render_buffer();
  if (current_gfx_device && current_gfx_device->fill_rect && dest >= fb && dest < fb + (current_gfx_device->pitch * current_gfx_device->height)) {
      int offset = dest - fb;
      int y = offset / current_gfx_device->pitch;
      int x = offset % current_gfx_device->pitch;
      gfx_device_fill_rect(x, y, width, height, color);
      return;
  }
  
  for (int row = 0; row < height; row++) {
    simd_fill_32(dest + row * dest_pitch, color, width);
  }
}

void gfx_blit_rect(uint32_t *dest, const uint32_t *src, int width, int height,
                   int dest_pitch, int src_pitch) {
  if (width <= 0 || height <= 0 || !dest || !src)
    return;
  // Safety: clamp width to source pitch
  if (width > src_pitch) width = src_pitch;
  
  uint32_t *fb = gfx_device_get_render_buffer();
  if (current_gfx_device && current_gfx_device->copy_rect) {
      if (dest >= fb && dest < fb + (current_gfx_device->pitch * current_gfx_device->height) &&
          src >= fb && src < fb + (current_gfx_device->pitch * current_gfx_device->height)) {
          
          int dest_offset = dest - fb;
          int dest_y = dest_offset / current_gfx_device->pitch;
          int dest_x = dest_offset % current_gfx_device->pitch;
          
          int src_offset = src - fb;
          int src_y = src_offset / current_gfx_device->pitch;
          int src_x = src_offset % current_gfx_device->pitch;
          
          gfx_device_copy_rect(src_x, src_y, dest_x, dest_y, width, height);
          return;
      }
  }
  
  for (int row = 0; row < height; row++) {
    simd_memcpy_32(dest + row * dest_pitch, src + row * src_pitch, width);
  }
}

void gfx_blend_rect(uint32_t *dest, const uint32_t *src, uint8_t global_alpha,
                    int width, int height, int dest_pitch, int src_pitch) {
  if (width <= 0 || height <= 0 || !dest || !src)
    return;
  // Safety: clamp width to source pitch to prevent OOB reads if metadata is stale
  if (width > src_pitch) width = src_pitch;
  
  for (int row = 0; row < height; row++) {
    simd_blend_32(dest + row * dest_pitch, src + row * src_pitch, global_alpha,
                  width);
  }
}
