#include "uvc.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"
#include "../camera.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);

static usb_device_t *uvc_dev = 0;
static uint8_t *uvc_front_buffer = 0;
static uint8_t *uvc_back_buffer = 0;
static uint32_t uvc_back_offset = 0;
static volatile int uvc_frame_ready = 0;
int uvc_is_streaming = 0;

static void uvc_payload_cb(uint8_t *payload, int size) {
    if (size < 2) return;
    
    uint8_t hlen = payload[0];
    if (size <= hlen) return; // Empty or invalid payload
    
    uint8_t bitfield = payload[1];
    int data_len = size - hlen;
    
    // Append data to back buffer
    if (uvc_back_offset + data_len <= 160 * 120 * 2) {
        memcpy(uvc_back_buffer + uvc_back_offset, payload + hlen, data_len);
        uvc_back_offset += data_len;
    }
    
    // Check EOF bit (bit 1)
    if (bitfield & (1 << 1)) {
        // Frame complete
        if (uvc_back_offset == 160 * 120 * 2) {
            memcpy(uvc_front_buffer, uvc_back_buffer, 160 * 120 * 2);
            uvc_frame_ready = 1;
        }
        uvc_back_offset = 0;
    }
}

void uvc_init_device(usb_device_t *dev) {
  if (uvc_dev) return; // Only support one camera for now

  uint8_t ep_addr = (uint32_t)(uintptr_t)dev->driver_data >> 16;
  uint16_t max_pkt = (uint32_t)(uintptr_t)dev->driver_data & 0xFFFF;

  print_serial("UVC: Initializing UVC Device...\n");
  
  if (!uvc_front_buffer) {
      uvc_front_buffer = (uint8_t *)kmalloc(160 * 120 * 2);
      uvc_back_buffer = (uint8_t *)kmalloc(160 * 120 * 2);
  }
  
  // 1. Negotiation (Probe)
  uvc_probe_commit_t probe;
  memset(&probe, 0, sizeof(probe));
  probe.bmHint = 1;        // Hint: dwFrameInterval
  probe.bFormatIndex = 1;  // Use first format (usually YUY2)
  probe.bFrameIndex = 1;   // Use first frame (usually 160x120 or similar)
  probe.dwFrameInterval = 666666; // 15 fps (10^7 / interval)

  usb_setup_packet_t setup;
  setup.request_type = 0x21; // Class, Interface, OUT
  setup.request = UVC_SET_CUR;
  setup.value = (UVC_VS_PROBE_CONTROL << 8);
  setup.index = 1; // VS interface index (usually 1)
  setup.length = 26;

  if (dev->hcd->control_transfer(dev, &setup, &probe, 26) < 0) {
    print_serial("UVC: Probe SET_CUR failed!\n");
    return;
  }

  // 2. Negotiation (Commit)
  setup.request = UVC_SET_CUR;
  setup.value = (UVC_VS_COMMIT_CONTROL << 8);
  if (dev->hcd->control_transfer(dev, &setup, &probe, 26) < 0) {
    print_serial("UVC: Commit SET_CUR failed!\n");
    return;
  }

  print_serial("UVC: Negotiation Complete.\n");

  // Select Alternate Setting 1 on VS Interface (Interface 1)
  setup.request_type = 0x01; // Standard, Interface, OUT
  setup.request = 0x0B;      // SET_INTERFACE (11)
  setup.value = 1;           // Alt Setting 1
  setup.index = 1;           // Interface 1 (VS)
  setup.length = 0;
  if (dev->hcd->control_transfer(dev, &setup, NULL, 0) < 0) {
      print_serial("UVC: SET_INTERFACE failed! Continuing anyway...\n");
  }

  if (dev->hcd->start_isochronous_in) {
    print_serial("UVC: Calling start_isochronous_in...\n");
    dev->hcd->start_isochronous_in(dev, ep_addr, NULL, max_pkt, uvc_payload_cb);
    uvc_dev = dev;
    uvc_is_streaming = 1;
    print_serial("UVC: Isochronous streaming started.\n");
  } else {
    print_serial("UVC: HCD does NOT support isochronous transfers!\n");
  }
}

// Simple YUY2 to RGB conversion
static void yuy2_to_rgb(uint8_t *yuy2, uint32_t *rgb, int w, int h) {
  for (int i = 0; i < w * h / 2; i++) {
    int y1 = yuy2[i * 4];
    int u = yuy2[i * 4 + 1] - 128;
    int y2 = yuy2[i * 4 + 2];
    int v = yuy2[i * 4 + 3] - 128;

    // Convert pixel 1
    int r1 = y1 + ((1370705 * v) >> 20);
    int g1 = y1 - ((337233 * u + 698200 * v) >> 20);
    int b1 = y1 + ((1732480 * u) >> 20);

    // Convert pixel 2
    int r2 = y2 + ((1370705 * v) >> 20);
    int g2 = y2 - ((337233 * u + 698200 * v) >> 20);
    int b2 = y2 + ((1732480 * u) >> 20);
    
    // Clamp and pack ARGB
    #define CLAMP(x) ((x) < 0 ? 0 : ((x) > 255 ? 255 : (x)))
    rgb[i * 2] = 0xFF000000 | (CLAMP(r1) << 16) | (CLAMP(g1) << 8) | CLAMP(b1);
    rgb[i * 2 + 1] = 0xFF000000 | (CLAMP(r2) << 16) | (CLAMP(g2) << 8) | CLAMP(b2);
  }
}

void uvc_update(void) {
  if (!uvc_dev || !uvc_is_streaming || !uvc_frame_ready) return;

  camera_ctx_t *cam = camera_get_ctx();
  if (!cam) return;

  yuy2_to_rgb(uvc_front_buffer, cam->frame_buffer, 160, 120);
  uvc_frame_ready = 0;
}
