#include "uhci.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"
#include "../ports.h"
#include "usb.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);
extern void k_itoa_hex(uint32_t n, char *s);

typedef struct {
  uint32_t base_io;
  uint32_t frame_list_phys;
  uint32_t *frame_list;
  uhci_td_t *td_pool; // Pre-allocated pool of TDs
  uint32_t td_pool_phys;
  
  // Isochronous support
  uhci_td_t *iso_tds; 
  uint32_t iso_tds_phys;
  void (*iso_cb)(uint8_t *, int);
  uint8_t iso_ep;
  uint8_t iso_dev_addr;
  uint16_t iso_pkt_size;
  int iso_running;
} uhci_controller_t;

static uhci_controller_t controllers[4];
static int num_controllers = 0;

#define TD_POOL_SIZE 32
#define ISO_TD_COUNT 128 // Support up to 128ms of buffered isochronous data
#define TD_STATUS_ACTIVE   (1 << 23)
#define TD_STATUS_STALL    (1 << 22)
#define TD_STATUS_ISOCH    (1 << 25)
#define TD_STATUS_IOC      (1 << 24)
#define TD_STATUS_NAK      (1 << 19)
#define TD_STATUS_TIMEOUT  (1 << 18)

// PID tokens
#define UHCI_PID_SETUP 0x2D
#define UHCI_PID_IN 0x69
#define UHCI_PID_OUT 0xE1

// Token field helpers
#define UHCI_TOKEN(pid, addr, endp, toggle, maxlen)                            \
  ((((uint32_t)(maxlen) - 1) << 21) | ((uint32_t)(toggle) << 19) |             \
   ((uint32_t)(endp) << 15) | ((uint32_t)(addr) << 8) | (uint32_t)(pid))

static int uhci_control_transfer(struct usb_device *dev,
                                 usb_setup_packet_t *setup, void *buffer,
                                 uint16_t length);
static int uhci_start_isochronous_in(struct usb_device *dev, uint8_t endpoint,
                                    void *buffer, uint16_t length,
                                    void (*callback)(uint8_t *data, int len));
static void uhci_poll_fn(usb_hcd_t *hcd);

static usb_hcd_t uhci_hcd = {
    .name = "UHCI",
    .data = 0,
    .control_transfer = uhci_control_transfer,
    .start_interrupt_in = 0, 
    .start_isochronous_in = uhci_start_isochronous_in,
    .poll = uhci_poll_fn,
    .reset_port = 0,
};

void uhci_init(uint32_t base_io) {
  if (num_controllers >= 4)
    return;

  print_serial("USB: Initializing UHCI Controller...\n");

  uhci_controller_t *hc = &controllers[num_controllers];
  hc->base_io = base_io;

  // Reset Host Controller
  outw(base_io + UHCI_USBCMD, USBCMD_HCRESET);

  // Wait for reset
  int timeout = 10000;
  while (timeout-- > 0) {
    if (!(inw(base_io + UHCI_USBCMD) & USBCMD_HCRESET))
      break;
    for (volatile int j = 0; j < 1000; j++);
  }

  // Setup Frame List
  uint32_t phys;
  hc->frame_list = (uint32_t *)kmalloc_ap(4096, &phys);
  hc->frame_list_phys = phys;
  for (int i = 0; i < 1024; i++) hc->frame_list[i] = 1;

  // Allocate Standard TD pool
  uint32_t td_phys;
  hc->td_pool = (uhci_td_t *)kmalloc_ap(sizeof(uhci_td_t) * TD_POOL_SIZE, &td_phys);
  hc->td_pool_phys = td_phys;
  memset(hc->td_pool, 0, sizeof(uhci_td_t) * TD_POOL_SIZE);

  // Allocate Isochronous TD pool
  uint32_t iso_phys;
  hc->iso_tds = (uhci_td_t *)kmalloc_ap(sizeof(uhci_td_t) * ISO_TD_COUNT, &iso_phys);
  hc->iso_tds_phys = iso_phys;
  memset(hc->iso_tds, 0, sizeof(uhci_td_t) * ISO_TD_COUNT);

  // Set FRBASEADD and Run
  outl(base_io + UHCI_FRBASEADD, hc->frame_list_phys);
  outw(base_io + UHCI_FRNUM, 0);
  outw(base_io + UHCI_USBCMD, USBCMD_RS);

  uhci_hcd.data = hc;
  usb_register_hcd(&uhci_hcd);
  num_controllers++;
}

static int uhci_control_transfer(struct usb_device *dev,
                                 usb_setup_packet_t *setup, void *buffer,
                                 uint16_t length) {
  uhci_controller_t *hc = (uhci_controller_t *)uhci_hcd.data;
  if (!hc) return -1;

  uhci_td_t *td_setup = &hc->td_pool[0];
  uhci_td_t *td_data = &hc->td_pool[1];
  uhci_td_t *td_status = &hc->td_pool[2];

  static usb_setup_packet_t setup_buf __attribute__((aligned(16)));
  memcpy(&setup_buf, setup, 8);

  td_setup->control_status = (3 << 27) | TD_STATUS_ACTIVE;
  td_setup->token = UHCI_TOKEN(UHCI_PID_SETUP, dev->address, 0, 0, 8);
  td_setup->buffer_ptr = (uint32_t)(uintptr_t)&setup_buf;

  if (length > 0 && buffer) {
    td_setup->link_ptr = (uint32_t)(uintptr_t)td_data | 0x4;
    td_data->control_status = (3 << 27) | TD_STATUS_ACTIVE;
    td_data->token = UHCI_TOKEN(UHCI_PID_IN, dev->address, 0, 1, length);
    td_data->buffer_ptr = (uint32_t)(uintptr_t)buffer;
    td_data->link_ptr = (uint32_t)(uintptr_t)td_status | 0x4;
    td_status->control_status = (3 << 27) | TD_STATUS_ACTIVE;
    td_status->token = UHCI_TOKEN(UHCI_PID_OUT, dev->address, 0, 1, 0) | (0x7FF << 21);
    td_status->link_ptr = 1;
  } else {
    td_setup->link_ptr = (uint32_t)(uintptr_t)td_status | 0x4;
    td_status->control_status = (3 << 27) | TD_STATUS_ACTIVE;
    td_status->token = UHCI_TOKEN(UHCI_PID_IN, dev->address, 0, 1, 0) | (0x7FF << 21);
    td_status->link_ptr = 1;
  }

  uint16_t target_frame = (inw(hc->base_io + UHCI_FRNUM) + 2) & 0x3FF;
  hc->frame_list[target_frame] = (uint32_t)(uintptr_t)td_setup;

  int timeout = 100000;
  while (timeout-- > 0) {
    if (!(td_status->control_status & TD_STATUS_ACTIVE)) break;
    for (volatile int j = 0; j < 100; j++);
  }
  hc->frame_list[target_frame] = 1;

  return (timeout <= 0) ? -1 : 0;
}

static int uhci_start_isochronous_in(struct usb_device *dev, uint8_t endpoint,
                                   void *buffer, uint16_t length,
                                   void (*callback)(uint8_t *data, int len)) {
  uhci_controller_t *hc = (uhci_controller_t *)uhci_hcd.data;
  if (!hc) return -1;

  print_serial("USB: Scheduling Isochronous streaming...\n");

  // For simplicity, we fill 1024 frames with isochronous TDs pointing to parts of the buffer
  // This is a VERY crude implementation of a periodic isochronous stream.
  // We assume 'buffer' is large enough to hold multiple packets.
  
  uint32_t num_packets = 1024; // One per ms frame
  uint32_t packet_size = length; // size per frame
  
  // Actually, we'll just use the 128 pre-allocated TDs for now to test.
  if (num_packets > ISO_TD_COUNT) num_packets = ISO_TD_COUNT;

  for (uint32_t i = 0; i < num_packets; i++) {
    uhci_td_t *td = &hc->iso_tds[i];
    memset(td, 0, sizeof(uhci_td_t));
    td->link_ptr = 1; // Terminate (isochronous is at bottom of frame list)
    td->control_status = TD_STATUS_ISOCH | TD_STATUS_ACTIVE | (3 << 27);
    td->token = UHCI_TOKEN(UHCI_PID_IN, dev->address, endpoint, 0, packet_size);
    td->buffer_ptr = (uint32_t)(uintptr_t)buffer + (i * packet_size);
    
    // Insert into frame list
    hc->frame_list[i] = (uint32_t)(uintptr_t)td;
  }

  hc->iso_cb = callback;
  hc->iso_ep = endpoint & 0x0F;
  hc->iso_dev_addr = dev->address;
  hc->iso_pkt_size = length;
  hc->iso_running = 1;
  
  print_serial("UHCI: Isochronous streaming scheduled.\n");
  return 0;
}

static void uhci_poll_fn(usb_hcd_t *hcd) {
  uhci_controller_t *hc = (uhci_controller_t *)hcd->data;
  if (!hc || !hc->iso_running) return;
  
  uint16_t cur_frame = inw(hc->base_io + UHCI_FRNUM) & 0x3FF;
  
  // Check a window of recently completed frames
  for (int i = 0; i < ISO_TD_COUNT; i++) {
    uhci_td_t *td = &hc->iso_tds[i];
    
    // Check if TD completed (no longer active)
    if (!(td->control_status & TD_STATUS_ACTIVE) && (td->buffer_ptr != 0)) {
      uint32_t actual_len = ((td->control_status >> 0) & 0x7FF) + 1;
      uint16_t cc = (td->control_status >> 17) & 0x03; // UHCI error bits
      
      if (actual_len > 0 && actual_len <= hc->iso_pkt_size && hc->iso_cb) {
        hc->iso_cb((uint8_t *)(uintptr_t)td->buffer_ptr, (int)actual_len);
      }
      
      // Re-arm the TD for the next cycle
      td->control_status = TD_STATUS_ISOCH | TD_STATUS_ACTIVE | (3 << 27);
      td->token = UHCI_TOKEN(UHCI_PID_IN, hc->iso_dev_addr, hc->iso_ep, 0, hc->iso_pkt_size);
    }
  }
}
