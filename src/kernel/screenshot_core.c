#define STBI_WRITE_NO_STDIO
#include "../drivers/rtc.h"
#include "../fs/fs.h"
#include "../lib/stb_image_write.h"
#include "heap.h"
#include "screen.h"
#include "string.h"

extern uint32_t *backbuffer;
extern int screen_width;
extern int screen_height;
extern void print_serial(const char *);
uint8_t screenshot_busy = 0;

// Callback to write data to a dynamically allocated buffer
typedef struct {
  uint8_t *data;
  size_t size;
  size_t capacity;
} screenshot_buffer_t;

static void screenshot_writer(void *context, void *data, int size) {
  screenshot_buffer_t *buf = (screenshot_buffer_t *)context;
  if (buf->size + size > buf->capacity) {
    size_t new_cap = (buf->capacity == 0) ? 1024 : buf->capacity * 2;
    while (new_cap < buf->size + size)
      new_cap *= 2;
    uint8_t *new_data = (uint8_t *)realloc(buf->data, new_cap);
    if (!new_data) {
      print_serial("SCREENSHOT: ERROR - Writer realloc failed!\n");
      return;
    }
    buf->data = new_data;
    buf->capacity = new_cap;
  }
  memcpy(buf->data + buf->size, data, size);
  buf->size += size;
}

void capture_screenshot() {
  print_serial("SCREENSHOT: Capturing...\n");
  if (screenshot_busy) {
    print_serial("SCREENSHOT: Already in progress, ignoring.\n");
    return;
  }
  screenshot_busy = 1;

  if (!backbuffer) {
    print_serial("SCREENSHOT: ERROR - No backbuffer!\n");
    screenshot_busy = 0;
    return;
  }

  int w = screen_width;
  int h = screen_height;

  // Allocate buffer for RGBA conversion (stb expects RGBA, we have BGRA)
  uint8_t *rgba_data = (uint8_t *)malloc(w * h * 4);
  if (!rgba_data) {
    print_serial("SCREENSHOT: ERROR - Out of memory for capture!\n");
    screenshot_busy = 0;
    return;
  }

  // Convert BGRA to RGBA
  for (int i = 0; i < w * h; i++) {
    uint32_t pixel = backbuffer[i];
    rgba_data[i * 4 + 0] = (pixel >> 16) & 0xFF; // R
    rgba_data[i * 4 + 1] = (pixel >> 8) & 0xFF;  // G
    rgba_data[i * 4 + 2] = pixel & 0xFF;         // B
    rgba_data[i * 4 + 3] = 255;                  // A (Opaque)
  }

  screenshot_buffer_t buf = {0};
  buf.capacity = 2 * 1024 * 1024; // Pre-allocate 2MB to reduce reallocs
  buf.data = (uint8_t *)malloc(buf.capacity);
  if (!buf.data) {
    print_serial("SCREENSHOT: ERROR - Out of memory for buffer!\n");
    free(rgba_data);
    screenshot_busy = 0;
    return;
  }

  // Encode to PNG in memory
  if (stbi_write_png_to_func(screenshot_writer, &buf, w, h, 4, rgba_data,
                             w * 4)) {
    // Create filename with timestamp if possible
    char filename[64] = "shot.png";
    rtc_time_t now;
    rtc_read(&now);
    // "shot_HHMMSS.png"
    strcpy(filename, "shot_");
    char h_str[4], m_str[4], s_str[4];
    k_itoa(now.hour, h_str);
    k_itoa(now.minute, m_str);
    k_itoa(now.second, s_str);
    if (now.hour < 10)
      strcat(filename, "0");
    strcat(filename, h_str);
    if (now.minute < 10)
      strcat(filename, "0");
    strcat(filename, m_str);
    if (now.second < 10)
      strcat(filename, "0");
    strcat(filename, s_str);
    strcat(filename, ".png");

    print_serial("SCREENSHOT: Saving to ");
    print_serial(filename);
    print_serial("\n");

    // Write to filesystem (Root directory for now)
    extern int fs_write(const char *filename, const uint8_t *buffer,
                        uint32_t size);
    if (fs_write(filename, buf.data, (uint32_t)buf.size) == 0) {
      print_serial("SCREENSHOT: SUCCESS!\n");
    } else {
      print_serial("SCREENSHOT: FAILED to write file!\n");
    }
  } else {
    print_serial("SCREENSHOT: FAILED to encode PNG!\n");
  }

  if (buf.data)
    free(buf.data);
  free(rgba_data);
  screenshot_busy = 0;
}
