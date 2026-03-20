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
} uhci_controller_t;

static uhci_controller_t controllers[4];
static int num_controllers = 0;

#define TD_POOL_SIZE 32
#define TD_STATUS_ACTIVE (1 << 23)
#define TD_STATUS_STALL (1 << 22)
#define TD_STATUS_NAK (1 << 19)
#define TD_STATUS_TIMEOUT (1 << 18)

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
static void uhci_poll_fn(usb_hcd_t *hcd);

static usb_hcd_t uhci_hcd = {
    .name = "UHCI",
    .data = 0,
    .control_transfer = uhci_control_transfer,
    .start_interrupt_in = 0, // Not implemented yet
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

  // Wait for reset to complete (HCRESET bit clears)
  int timeout = 10000;
  while (timeout-- > 0) {
    if (!(inw(base_io + UHCI_USBCMD) & USBCMD_HCRESET))
      break;
    for (volatile int j = 0; j < 1000; j++)
      ; // Short delay
  }

  if (timeout <= 0) {
    print_serial("USB: UHCI Reset FAILED (Timeout).\n");
    return;
  }

  // Check status
  uint16_t status = inw(base_io + UHCI_USBSTS);
  if (status & 0x20) {
    print_serial("USB: UHCI HCHalted.\n");
  }

  // Disable all USB interrupts
  outw(base_io + UHCI_USBINTR, 0x0000);

  // Clear all pending status bits
  outw(base_io + UHCI_USBSTS, 0x003F);

  // Setup Frame List (1024 entries, 4KB aligned)
  uint32_t phys;
  hc->frame_list = (uint32_t *)kmalloc_ap(4096, &phys);
  hc->frame_list_phys = phys;

  for (int i = 0; i < 1024; i++) {
    hc->frame_list[i] = 1; // Terminate bit
  }

  // Allocate TD pool
  uint32_t td_phys;
  hc->td_pool =
      (uhci_td_t *)kmalloc_ap(sizeof(uhci_td_t) * TD_POOL_SIZE, &td_phys);
  hc->td_pool_phys = td_phys;
  memset(hc->td_pool, 0, sizeof(uhci_td_t) * TD_POOL_SIZE);

  // Set Frame List Base Address
  outl(base_io + UHCI_FRBASEADD, hc->frame_list_phys);

  // Set Frame Number to 0
  outw(base_io + UHCI_FRNUM, 0);

  // Start the controller
  outw(base_io + UHCI_USBCMD, USBCMD_RS);

  num_controllers++;

  // Register as HCD
  uhci_hcd.data = hc;
  usb_register_hcd(&uhci_hcd);

  print_serial("USB: UHCI Controller Initialized.\n");
}

static int uhci_control_transfer(struct usb_device *dev,
                                 usb_setup_packet_t *setup, void *buffer,
                                 uint16_t length) {
  uhci_controller_t *hc = (uhci_controller_t *)uhci_hcd.data;
  if (!hc)
    return -1;

  // We need at minimum 2 TDs (SETUP + STATUS), up to 3+ with DATA
  // Use TDs from the pool
  uhci_td_t *td_setup = &hc->td_pool[0];
  uhci_td_t *td_data = &hc->td_pool[1];
  uhci_td_t *td_status = &hc->td_pool[2];

  memset(td_setup, 0, sizeof(uhci_td_t));
  memset(td_data, 0, sizeof(uhci_td_t));
  memset(td_status, 0, sizeof(uhci_td_t));

  // --- SETUP TD ---
  // Copy setup packet to a known physical location
  static usb_setup_packet_t setup_buf __attribute__((aligned(16)));
  memcpy(&setup_buf, setup, 8);

  td_setup->control_status = (3 << 27) | TD_STATUS_ACTIVE; // 3 retries, active
  td_setup->token = UHCI_TOKEN(UHCI_PID_SETUP, dev->address, 0, 0, 8);
  td_setup->buffer_ptr = (uint32_t)(uintptr_t)&setup_buf;

  if (length > 0 && buffer) {
    // --- DATA TD (IN) ---
    td_setup->link_ptr = (uint32_t)(uintptr_t)td_data | 0x4; // Depth-first
    td_data->control_status = (3 << 27) | TD_STATUS_ACTIVE;
    td_data->token = UHCI_TOKEN(UHCI_PID_IN, dev->address, 0, 1, length);
    td_data->buffer_ptr = (uint32_t)(uintptr_t)buffer;

    // --- STATUS TD (OUT, zero-length) ---
    td_data->link_ptr = (uint32_t)(uintptr_t)td_status | 0x4;
    td_status->control_status = (3 << 27) | TD_STATUS_ACTIVE;
    td_status->token = UHCI_TOKEN(UHCI_PID_OUT, dev->address, 0, 1, 0) |
                       (0x7FF << 21); // Zero-length token
    td_status->link_ptr = 1;          // Terminate
    td_status->buffer_ptr = 0;
  } else {
    // No data stage — STATUS is IN
    td_setup->link_ptr = (uint32_t)(uintptr_t)td_status | 0x4;
    td_status->control_status = (3 << 27) | TD_STATUS_ACTIVE;
    td_status->token =
        UHCI_TOKEN(UHCI_PID_IN, dev->address, 0, 1, 0) | (0x7FF << 21);
    td_status->link_ptr = 1; // Terminate
    td_status->buffer_ptr = 0;
  }

  // Insert into frame list at current frame
  uint16_t frame = inw(hc->base_io + UHCI_FRNUM) & 0x3FF;
  uint16_t target_frame = (frame + 2) & 0x3FF; // 2 frames ahead
  hc->frame_list[target_frame] = (uint32_t)(uintptr_t)td_setup;

  // Wait for completion (poll)
  int timeout = 100000;
  while (timeout-- > 0) {
    // Check if last TD completed (active bit cleared)
    uhci_td_t *last_td = (length > 0 && buffer) ? td_status : td_status;
    if (!(last_td->control_status & TD_STATUS_ACTIVE))
      break;
    for (volatile int j = 0; j < 100; j++)
      ;
  }

  // Remove from frame list
  hc->frame_list[target_frame] = 1; // Terminate

  if (timeout <= 0) {
    print_serial("USB: Control transfer timeout\n");
    return -1;
  }

  // Check for errors
  if (td_setup->control_status & (TD_STATUS_STALL | TD_STATUS_TIMEOUT)) {
    print_serial("USB: Control transfer error (SETUP)\n");
    return -1;
  }

  return 0;
}

static void uhci_poll_fn(usb_hcd_t *hcd) {
  (void)hcd;
  // Placeholder — could check for port status changes here
}
