#include "usb.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"

extern void print_serial(const char *str);
extern void mouse_handler(int dx, int dy, int buttons, int scroll);

// Raw HID report buffer
static uint8_t mouse_report[4];

static void hid_mouse_callback(uint8_t *data, int len) {
  if (len >= 3) {
    int buttons = data[0] & 0x07;
    int dx = (int8_t)data[1];
    int dy = (int8_t)data[2];
    int scroll = (len >= 4) ? (int8_t)data[3] : 0;

    // Some USB mice send dy as inverted compared to PS/2
    if (dx != 0 || dy != 0 || buttons != 0 || scroll != 0) {
      mouse_handler(dx, dy, buttons, scroll);
    }
  }
}

static usb_hcd_t *hcds[4];
static int num_hcds = 0;

static usb_device_t *devices[127];
static int num_devices = 0;

void usb_init() {
  print_serial("USB: Core Stack Initializing...\n");
  memset(hcds, 0, sizeof(hcds));
  memset(devices, 0, sizeof(devices));
}

void usb_register_hcd(usb_hcd_t *hcd) {
  if (num_hcds >= 4)
    return;
  hcds[num_hcds++] = hcd;
  print_serial("USB: Registered HCD: ");
  print_serial(hcd->name);
  print_serial("\n");
}

void usb_poll() {
  for (int i = 0; i < num_hcds; i++) {
    if (hcds[i]->poll) {
      hcds[i]->poll(hcds[i]);
    }
  }
}

void usb_new_device(usb_hcd_t *hcd, uint8_t port, uint32_t speed) {
  print_serial("USB: New device detected. Resetting port...\n");

  if (hcd->reset_port) {
    hcd->reset_port(hcd, port);
  }

  usb_device_t *dev = (usb_device_t *)kmalloc(sizeof(usb_device_t));
  memset(dev, 0, sizeof(usb_device_t));
  dev->hcd = hcd;
  dev->port = port;
  dev->speed = speed;
  dev->address = 0;

  // 1. GET_DESCRIPTOR (Device) - First 8 bytes
  usb_setup_packet_t setup;
  setup.request_type = 0x80;
  setup.request = USB_REQ_GET_DESCRIPTOR;
  setup.value = (USB_DESC_DEVICE << 8);
  setup.index = 0;
  setup.length = 8;

  uint8_t desc_buf[8];
  if (hcd->control_transfer(dev, &setup, desc_buf, 8) < 0) {
    print_serial("USB: GET_DESCRIPTOR (Initial) failed!\n");
    return;
  }

  dev->descriptor.max_packet_size = desc_buf[7];

  // 2. SET_ADDRESS
  uint8_t new_addr = ++num_devices;
  setup.request_type = 0x00;
  setup.request = USB_REQ_SET_ADDRESS;
  setup.value = new_addr;
  setup.index = 0;
  setup.length = 0;

  if (hcd->control_transfer(dev, &setup, NULL, 0) < 0) {
    print_serial("USB: SET_ADDRESS failed!\n");
    return;
  }

  dev->address = new_addr;
  devices[new_addr] = dev;

  // 3. GET_DESCRIPTOR (Full)
  setup.request_type = 0x80;
  setup.request = USB_REQ_GET_DESCRIPTOR;
  setup.value = (USB_DESC_DEVICE << 8);
  setup.index = 0;
  setup.length = 18;

  if (hcd->control_transfer(dev, &setup, &dev->descriptor, 18) < 0) {
    print_serial("USB: GET_DESCRIPTOR (Full) failed!\n");
    return;
  }

  char hbuf[16];
  print_serial("USB: Device enumerated. Address: ");
  k_itoa(new_addr, hbuf);
  print_serial(hbuf);
  print_serial(" Vendor: 0x");
  k_itoa_hex(dev->descriptor.vendor_id, hbuf);
  print_serial(hbuf);
  print_serial(" Product: 0x");
  k_itoa_hex(dev->descriptor.product_id, hbuf);
  print_serial(hbuf);
  print_serial("\n");

  print_serial("USB: Enumeration Complete.\n");

  // 4. SET_CONFIGURATION 1 (assuming config 1 is standard)
  // We are skipping SET_CONFIGURATION for now to prevent the BIOS
  // from disabling PS/2 USB Legacy Support (SMI) emulation.
  // If we send SET_CONFIGURATION, the mouse stops sending IRQ12.
  /*
  setup.request_type = 0x00;
  setup.request = USB_REQ_SET_CONFIGURATION;
  setup.value = 1;
  setup.index = 0;
  setup.length = 0;

  if (hcd->control_transfer(dev, &setup, NULL, 0) < 0) {
    print_serial("USB: SET_CONFIGURATION failed!\n");
    return;
  }
  print_serial("USB: Configuration SET (1).\n");

  // 5. Start Polling Interrupt Endpoint 1 IN (assuming standard HID mouse
  // profile)
  if (dev->descriptor.device_class == 0 || dev->descriptor.device_class == 9) {
    // If it's the Bochs mouse (or class defined at interface level), assume EP1
    // IN for now.
    if (hcd->start_interrupt_in) {
      hcd->start_interrupt_in(dev, 1, mouse_report, 4, hid_mouse_callback);
      print_serial("USB: Started HID Interrupt Polling (Endpoint 1 IN).\n");
    }
  }
  */
}
