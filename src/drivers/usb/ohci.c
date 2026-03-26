#include "ohci.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"
#include "../ports.h"
#include "usb.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);
extern void k_itoa_hex(uint32_t n, char *s);

typedef struct {
    uintptr_t base_addr;
    ohci_hcca_t *hcca;
    uint32_t hcca_phys;
    ohci_ed_t *ctrl_ed; // Default control ED
    uint32_t ctrl_ed_phys;
    ohci_td_t *td_pool;
    uint32_t td_pool_phys;
    
    // Isochronous support
    ohci_ed_t *iso_ed;
    uint32_t iso_ed_phys;
    ohci_iso_td_t *iso_tds;
    uint32_t iso_tds_phys;
    uint8_t *iso_dma_buf;
    uint32_t iso_dma_phys;
    uint8_t *user_iso_buf;
    uint32_t user_iso_buf_offset;
    void (*iso_cb)(uint8_t *, int);
    int iso_running;
    int iso_next_td;
} ohci_controller_t;

static ohci_controller_t controllers[4];
static int num_controllers = 0;

static int ohci_control_transfer(struct usb_device *dev, usb_setup_packet_t *setup, void *buffer, uint16_t length);
static int ohci_start_isochronous_in(usb_device_t *dev, uint8_t endpoint, void *buffer, uint16_t length, void (*callback)(uint8_t *, int));
static void ohci_poll_fn(usb_hcd_t *hcd);

static usb_hcd_t ohci_hcd = {
    .name = "OHCI",
    .data = 0,
    .control_transfer = ohci_control_transfer,
    .start_interrupt_in = 0,
    .start_isochronous_in = ohci_start_isochronous_in,
    .poll = ohci_poll_fn,
    .reset_port = 0,
};

static inline void oh_write(uintptr_t base, uint32_t reg, uint32_t val) {
    *(volatile uint32_t *)(base + reg) = val;
}

static inline uint32_t oh_read(uintptr_t base, uint32_t reg) {
    return *(volatile uint32_t *)(base + reg);
}

void ohci_init(uintptr_t base_addr) {
    if (num_controllers >= 4) return;
    print_serial("USB: Initializing OHCI Controller at 0x");
    char hbuf[16];
    k_itoa_hex((uint32_t)base_addr, hbuf);
    print_serial(hbuf);
    print_serial("\n");

    ohci_controller_t *hc = &controllers[num_controllers];
    hc->base_addr = base_addr;

    // 1. Reset Controller
    uint32_t rev = oh_read(base_addr, OH_REVISION);
    uint32_t desc_a = oh_read(base_addr, OH_RH_DESCRIPTOR_A);
    print_serial("OHCI: Revision: 0x");
    k_itoa_hex(rev, hbuf);
    print_serial(hbuf);
    print_serial(" DescA: 0x");
    k_itoa_hex(desc_a, hbuf);
    print_serial(hbuf);
    print_serial("\n");

    oh_write(base_addr, OH_COMMAND_STATUS, (1 << 0)); // HostControllerReset
    int reset_timeout = 1000;
    while ((oh_read(base_addr, OH_COMMAND_STATUS) & 1) && reset_timeout-- > 0) {
        for (volatile int i = 0; i < 1000; i++);
    }
    if (reset_timeout <= 0) {
        print_serial("OHCI: Reset TIMEOUT!\n");
    } else {
        print_serial("OHCI: Reset Completed.\n");
    }

    // 2. Allocate HCCA (256-byte aligned)
    uint32_t h_phys;
    hc->hcca = (ohci_hcca_t *)kmalloc_ap(256, &h_phys);
    hc->hcca_phys = h_phys;
    memset(hc->hcca, 0, 256);
    oh_write(base_addr, OH_HCCA, h_phys);

    // 3. Setup Default Control ED
    uint32_t ed_phys;
    hc->ctrl_ed = (ohci_ed_t *)kmalloc_ap(sizeof(ohci_ed_t), &ed_phys);
    hc->ctrl_ed_phys = ed_phys;
    memset(hc->ctrl_ed, 0, sizeof(ohci_ed_t));
    hc->ctrl_ed->flags = (1 << 13); // Skip bit? No, we'll set it when active
    oh_write(base_addr, OH_CONTROL_HEAD_ED, ed_phys);

    // 4. Register HCD
    ohci_hcd.data = hc;
    usb_register_hcd(&ohci_hcd);

    // 5. Root Hub: Power Up Ports
    uint32_t num_ports = oh_read(base_addr, OH_RH_DESCRIPTOR_A) & 0xFF;
    print_serial("OHCI: Number of Ports: ");
    char pbuf[16];
    k_itoa(num_ports, pbuf);
    print_serial(pbuf);
    print_serial("\n");

    for (uint32_t i = 0; i < num_ports; i++) {
        uint32_t status = oh_read(base_addr, OH_RH_PORT_STATUS + (i * 4));
        print_serial("  Port ");
        k_itoa(i, pbuf);
        print_serial(pbuf);
        print_serial(" Status: 0x");
        k_itoa_hex(status, hbuf);
        print_serial(hbuf);
        print_serial("\n");

        oh_write(base_addr, OH_RH_PORT_STATUS + (i * 4), (1 << 8)); // SetPortPower
        for (volatile int j = 0; j < 50000; j++); // Wait approx 10-20ms per port
    }

    uint32_t rh_status = oh_read(base_addr, OH_RH_STATUS);
    print_serial("OHCI: Root Hub Status: 0x");
    k_itoa_hex(rh_status, hbuf);
    print_serial(hbuf);
    print_serial("\n");

    // 6. Start Controller
    uint32_t ctrl = oh_read(base_addr, OH_CONTROL);
    ctrl &= ~0xC0; // Clear HCFS
    ctrl |= 0x80;  // USBOPERATIONAL
    ctrl |= (1 << 3); // ControlListEnable
    ctrl |= (1 << 2); // PeriodicListEnable
    oh_write(base_addr, OH_CONTROL, ctrl);

    num_controllers++;
    print_serial("USB: OHCI Operational and Powering Root Hub.\n");
}

static int ohci_start_isochronous_in(usb_device_t *dev, uint8_t endpoint, void *buffer, uint16_t length, void (*callback)(uint8_t *, int)) {
    ohci_controller_t *hc = (ohci_controller_t *)ohci_hcd.data;
    if (!hc) return -1;
    (void)callback;
    
    print_serial("OHCI: Starting Isochronous IN...\n");
    
    hc->iso_cb = callback;
    
    hc->iso_ed = (ohci_ed_t *)kmalloc_ap(sizeof(ohci_ed_t), &hc->iso_ed_phys);
    memset(hc->iso_ed, 0, sizeof(ohci_ed_t));
    
    uint32_t ep_num = endpoint & 0x0F;
    uint32_t is_low_speed = (dev->speed == 1) ? 1 : 0;
    uint32_t mps = length; 
    
    hc->iso_ed->flags = (dev->address & 0x7F) | 
                        ((ep_num & 0x0F) << 7) | 
                        (2 << 11) | // IN direction
                        (is_low_speed << 13) | 
                        (0 << 14) | // Skip = 0
                        (1 << 15) | // Format = Isochronous
                        ((mps & 0x7FF) << 16);
                        
    hc->iso_tds = (ohci_iso_td_t *)kmalloc_ap(sizeof(ohci_iso_td_t) * 4, &hc->iso_tds_phys);
    memset(hc->iso_tds, 0, sizeof(ohci_iso_td_t) * 4);
    
    hc->iso_dma_buf = kmalloc_ap(4 * 8192, &hc->iso_dma_phys);
    
    uint16_t start_frame = hc->hcca->frame_number + 10;
    
    for (int i = 0; i < 4; i++) {
        uint32_t td_phys = hc->iso_tds_phys + i * sizeof(ohci_iso_td_t);
        uint32_t next_td_phys = hc->iso_tds_phys + ((i + 1) % 4) * sizeof(ohci_iso_td_t);
        
        hc->iso_tds[i].flags = ((start_frame + i * 8) & 0xFFFF) | 
                               (0 << 21) | // DI = 0
                               (7 << 24);  // FC = 7 (8 frames)
        
        uint32_t buf_start = hc->iso_dma_phys + i * 8192;
        hc->iso_tds[i].bp0 = buf_start & 0xFFFFF000;
        hc->iso_tds[i].bep = (buf_start + 8192 - 1);
        hc->iso_tds[i].next_td = next_td_phys;
        
        for(int j = 0; j < 8; j++) {
            hc->iso_tds[i].psw[j] = (0xE << 12) | ((buf_start & 0xFFF) + j * 1024);
        }
    }
    
    hc->iso_ed->head_td = hc->iso_tds_phys;
    hc->iso_ed->tail_td = 0; 
    
    for (int i = 0; i < 32; i++) {
        hc->hcca->interrupt_table[i] = hc->iso_ed_phys;
    }
    
    uint32_t ctrl = oh_read(hc->base_addr, OH_CONTROL);
    ctrl |= (1 << 2); // PeriodicListEnable
    oh_write(hc->base_addr, OH_CONTROL, ctrl);
    
    hc->iso_running = 1;
    hc->iso_next_td = 0;
    
    print_serial("OHCI: Isochronous IN started successfully.\n");
    return 0;
}

static int ohci_control_transfer(struct usb_device *dev, usb_setup_packet_t *setup, void *buffer, uint16_t length) {
    ohci_controller_t *hc = (ohci_controller_t *)ohci_hcd.data;
    if (!hc) return -1;
    
    print_serial("OHCI: Control Transfer Starting...\n");

    // Allocate 3 TDs (Setup, Data, Status)
    uint32_t tdp;
    ohci_td_t *tds = (ohci_td_t *)kmalloc_ap(sizeof(ohci_td_t) * 3, &tdp);
    memset(tds, 0, sizeof(ohci_td_t) * 3);

    // SETUP TD
    tds[0].flags = (0 << 19) | (2 << 21) | (1 << 24); // SETUP PID, 0xFF max err, Data0
    tds[0].cbp = (uint32_t)(uintptr_t)setup;
    tds[0].bep = tds[0].cbp + 7;
    tds[0].next_td = tdp + sizeof(ohci_td_t);

    // DATA TD
    if (length > 0) {
        tds[1].flags = (2 << 19) | (1 << 24); // IN PID, Data1
        tds[1].cbp = (uint32_t)(uintptr_t)buffer;
        tds[1].bep = tds[1].cbp + length - 1;
        tds[1].next_td = tdp + 2 * sizeof(ohci_td_t);
    } else {
        tds[0].next_td = tdp + 2 * sizeof(ohci_td_t);
    }

    // STATUS TD
    tds[2].flags = (1 << 19) | (1 << 24); // OUT PID, Data1
    tds[2].cbp = 0;
    tds[2].bep = 0;
    tds[2].next_td = 0;

    // Setup ED
    hc->ctrl_ed->flags = (dev->address) | (length > 64 ? 64 : 8 << 16); // MaxPacketSize=8 or 64
    hc->ctrl_ed->head_td = tdp;
    hc->ctrl_ed->tail_td = 0; // Terminal

    // Wait for completion (poll done_head or TDs)
    int timeout = 100000;
    while (timeout-- > 0) {
        if (hc->ctrl_ed->head_td == 0) break;
        for (volatile int i = 0; i < 100; i++);
    }

    kfree(tds);
    if (timeout <= 0) {
        print_serial("OHCI: Control Transfer TIMEOUT!\n");
        return -1;
    }
    return 0;
}

static void ohci_poll_fn(usb_hcd_t *hcd) {
    ohci_controller_t *hc = (ohci_controller_t *)hcd->data;
    if (!hc) return;

    uint32_t num_ports = oh_read(hc->base_addr, OH_RH_DESCRIPTOR_A) & 0xFF;
    for (uint32_t i = 0; i < num_ports; i++) {
        uint32_t status = oh_read(hc->base_addr, OH_RH_PORT_STATUS + (i * 4));
        
        static int poll_count = 0;
        static uint32_t last_status[16] = {0};
        
        if (status != last_status[i] || (poll_count % 5000) == 0) {
            if (status & 0xFFFF0000) { // Any change bits set
                print_serial("OHCI Port ");
                char buf[16];
                k_itoa(i, buf);
                print_serial(buf);
                print_serial(" CHANGE: 0x");
                k_itoa_hex(status, buf);
                print_serial(buf);
                print_serial("\n");
            } else if ((poll_count % 5000) == 0) {
                print_serial("OHCI Port ");
                char buf[16];
                k_itoa(i, buf);
                print_serial(buf);
                print_serial(" Status: 0x");
                k_itoa_hex(status, buf);
                print_serial(buf);
                print_serial("\n");
            }
            last_status[i] = status;
        }
        if (i == num_ports - 1) poll_count++;

        // Check for ConnectStatusChange (bit 16)
        if (status & (1 << 16)) {
            print_serial("OHCI: Port Connection Change Detected!\n");
            
            // Clear the change bit
            oh_write(hc->base_addr, OH_RH_PORT_STATUS + (i * 4), (1 << 16));
            
            // If bit 0 is set, a device is connected
            if (status & (1 << 0)) {
                print_serial("OHCI: Device Connected! Resetting port...\n");
                
                // 1. Reset Root Hub Port (SetPortReset bit 4)
                oh_write(hc->base_addr, OH_RH_PORT_STATUS + (i * 4), (1 << 4));
                for (volatile int j = 0; j < 50000; j++); // Wait for reset
                
                // 2. Clear PortResetStatusChange (bit 20)
                oh_write(hc->base_addr, OH_RH_PORT_STATUS + (i * 4), (1 << 20));
                
                // 3. Trigger USB Core Enumeration
                // We'll assume Full Speed (12Mb/s) for now as it's common for UVC on OHCI
                usb_new_device(hcd, i, 12); 
            }
        }
    }
    
    // Process Isochronous TDs
    if (hc->iso_running) {
        int curr = hc->iso_next_td;
        // Check if the 8th frame of the TD is completed (ConditionCode != NotAccessed(0xE))
        if ((hc->iso_tds[curr].psw[7] >> 12) != 0xE) {
            // TD is completed. Copy valid frames
            uint32_t base_offset = hc->iso_tds[curr].psw[0] & 0xFFF;
            uint8_t *dma_ptr = hc->iso_dma_buf + curr * 8192;
            
            for (int j = 0; j < 8; j++) {
                uint16_t psw = hc->iso_tds[curr].psw[j];
                uint16_t cc = psw >> 12;
                uint16_t size = psw & 0x7FF; // For IN, PSW contains size
                
                if (cc == 0 && size > 2) { 
                    if (hc->iso_cb) {
                        hc->iso_cb(dma_ptr + j * 1024, size);
                    }
                }
            }
            
            // Reset TD for next use
            uint16_t next_start = (hc->iso_tds[(curr + 3) % 4].flags & 0xFFFF) + 8;
            hc->iso_tds[curr].flags = (next_start & 0xFFFF) | (0 << 21) | (7 << 24);
            
            uint32_t buf_start = hc->iso_dma_phys + curr * 8192;
            for(int j = 0; j < 8; j++) {
                hc->iso_tds[curr].psw[j] = (0xE << 12) | ((buf_start & 0xFFF) + j * 1024);
            }
            
            hc->iso_next_td = (curr + 1) % 4;
            
            // Re-link if head_td advanced past it
            hc->iso_ed->head_td = hc->iso_tds_phys + hc->iso_next_td * sizeof(ohci_iso_td_t);
        }
    }
}
