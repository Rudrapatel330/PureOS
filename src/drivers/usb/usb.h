#ifndef USB_H
#define USB_H

#include "../../kernel/types.h"

// USB Standard Request Codes
#define USB_REQ_GET_STATUS 0x00
#define USB_REQ_CLEAR_FEATURE 0x01
#define USB_REQ_SET_FEATURE 0x03
#define USB_REQ_SET_ADDRESS 0x05
#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_DESCRIPTOR 0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09

// Descriptor Types
#define USB_DESC_DEVICE 0x01
#define USB_DESC_CONFIG 0x02
#define USB_DESC_STRING 0x03
#define USB_DESC_INTERFACE 0x04
#define USB_DESC_ENDPOINT 0x05

// Device Descriptor
typedef struct {
  uint8_t length;
  uint8_t type;
  uint16_t usb_version;
  uint8_t device_class;
  uint8_t device_subclass;
  uint8_t device_protocol;
  uint8_t max_packet_size;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t device_version;
  uint8_t manufacturer_str_idx;
  uint8_t product_str_idx;
  uint8_t serial_str_idx;
  uint8_t num_configurations;
} __attribute__((packed)) usb_device_desc_t;

// Setup Packet (8 bytes)
typedef struct {
  uint8_t request_type;
  uint8_t request;
  uint16_t value;
  uint16_t index;
  uint16_t length;
} __attribute__((packed)) usb_setup_packet_t;

// Forward declaration
struct usb_device;

// Host Controller Driver Interface
typedef struct usb_hcd {
  const char *name;
  void *data;
  int (*control_transfer)(struct usb_device *dev, usb_setup_packet_t *setup,
                          void *buffer, uint16_t length);
  int (*start_interrupt_in)(struct usb_device *dev, uint8_t endpoint,
                            void *buffer, uint16_t length,
                            void (*callback)(uint8_t *data, int len));
  void (*poll)(struct usb_hcd *hcd);
  void (*reset_port)(struct usb_hcd *hcd, uint8_t port);
} usb_hcd_t;

// USB Device representation
typedef struct usb_device {
  uint8_t address;
  uint8_t port;
  uint32_t speed;
  usb_hcd_t *hcd;
  usb_device_desc_t descriptor;
} usb_device_t;

void usb_init();
void usb_register_hcd(usb_hcd_t *hcd);
void usb_new_device(usb_hcd_t *hcd, uint8_t port, uint32_t speed);
void usb_poll();

#endif // USB_H
