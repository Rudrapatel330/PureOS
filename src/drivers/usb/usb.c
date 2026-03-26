#include "usb.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"

extern void print_serial(const char *str);
extern void mouse_handler(int dx, int dy, int buttons, int scroll);
extern void uvc_init_device(usb_device_t *dev);

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

  // 1. GET_DESCRIPTOR (Device) - First 8 bytes to get max packet size
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

  // 3. GET_DESCRIPTOR (Device, Full)
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
  print_serial(" Class: 0x");
  k_itoa_hex(dev->descriptor.device_class, hbuf);
  print_serial(hbuf);
  print_serial(" Vendor: 0x");
  k_itoa_hex(dev->descriptor.vendor_id, hbuf);
  print_serial(hbuf);
  print_serial(" Product: 0x");
  k_itoa_hex(dev->descriptor.product_id, hbuf);
  print_serial(hbuf);
  print_serial("\n");

  // 4. GET_DESCRIPTOR (Config, Header only)
  setup.request_type = 0x80;
  setup.request = USB_REQ_GET_DESCRIPTOR;
  setup.value = (USB_DESC_CONFIG << 8);
  setup.index = 0;
  setup.length = 9;

  usb_config_desc_t cfg_header;
  if (hcd->control_transfer(dev, &setup, &cfg_header, 9) < 0) {
    print_serial("USB: GET_DESCRIPTOR (Config Header) failed!\n");
    return;
  }

  // 5. GET_DESCRIPTOR (Config, Full)
  uint32_t full_len = cfg_header.total_length;
  uint8_t *full_cfg = (uint8_t *)kmalloc(full_len);
  setup.length = (uint16_t)full_len;
  if (hcd->control_transfer(dev, &setup, full_cfg, (uint16_t)full_len) < 0) {
    print_serial("USB: GET_DESCRIPTOR (Config Full) failed!\n");
    kfree(full_cfg);
    return;
  }

  print_serial("USB: Parsing configuration descriptors...\n");
  uint8_t *ptr = full_cfg;
  uint8_t found_uvc = 0;
  uint8_t iso_ep_addr = 0;
  uint16_t iso_max_pkt = 0;
  while (ptr < full_cfg + full_len) {
    uint8_t len = ptr[0];
    uint8_t type = ptr[1];

    if (type == USB_DESC_INTERFACE) {
      usb_interface_desc_t *iface = (usb_interface_desc_t *)ptr;
      print_serial("  Interface: ");
      k_itoa(iface->interface_number, hbuf);
      print_serial(hbuf);
      print_serial(" Class: 0x");
      k_itoa_hex(iface->interface_class, hbuf);
      print_serial(hbuf);
      print_serial(" Subclass: 0x");
      k_itoa_hex(iface->interface_subclass, hbuf);
      print_serial(hbuf);
      print_serial("\n");

      // Check for UVC (Video Control Interface: Class 0x0E, Subclass 0x01)
      if (iface->interface_class == 0x0E && iface->interface_subclass == 0x01) {
        print_serial("USB: UVC Video Control Interface detected!\n");
        found_uvc = 1;
      }
      
      // Check for HID Mouse if device class was 0
      if (dev->descriptor.device_class == 0 && iface->interface_class == 0x03 && iface->interface_subclass == 0x01 && iface->interface_protocol == 0x02) {
          print_serial("USB: HID Mouse Interface detected!\n");
          // Re-enable HID polling if we are setting config
      }
    } else if (type == USB_DESC_ENDPOINT) {
        usb_endpoint_desc_t *ep = (usb_endpoint_desc_t *)ptr;
        // Check for ISO IN endpoint (attributes bits 0-1 = 01 for ISO, bit 7 of address = IN)
        if ((ep->attributes & 0x03) == 0x01 && (ep->endpoint_address & 0x80)) {
            iso_ep_addr = ep->endpoint_address;
            iso_max_pkt = ep->max_packet_size;
        }
    }
    ptr += len;
    if (len == 0) break; // Safety
  }

  // 6. SET_CONFIGURATION 1
  // Note: This might still break PS/2 legacy in some BIOSes, but we need it for UVC.
  setup.request_type = 0x00;
  setup.request = USB_REQ_SET_CONFIGURATION;
  setup.value = 1;
  setup.index = 0;
  setup.length = 0;

  if (hcd->control_transfer(dev, &setup, NULL, 0) < 0) {
    print_serial("USB: SET_CONFIGURATION failed!\n");
  } else {
    print_serial("USB: Configuration SET (1).\n");
    
    // Check if it's a known UVC device or contains UVC interfaces
    // For now, we'll try to initialize UVC if we saw the interface
    if (found_uvc && iso_ep_addr) {
      dev->driver_data = (void*)(uintptr_t)((iso_ep_addr << 16) | iso_max_pkt);
      uvc_init_device(dev);
    }
  }

  kfree(full_cfg);
}
