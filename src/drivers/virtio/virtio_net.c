#include "virtio_net.h"
#include "../pci.h"
#include "../ports.h"
#include "../../kernel/string.h"

extern void print_serial(const char *str);
extern void k_itoa_hex(uint32_t value, char *str);
extern uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys);

int virtio_net_initialized = 0;
uint8_t virtio_net_mac[6] = {0};
static uint32_t io_base = 0;

// Legacy Virtio PCI Offsets
#define VIRTIO_PCI_HOST_FEATURES 0x00
#define VIRTIO_PCI_GUEST_FEATURES 0x04
#define VIRTIO_PCI_QUEUE_PFN     0x08
#define VIRTIO_PCI_QUEUE_SIZE    0x0C
#define VIRTIO_PCI_QUEUE_SEL     0x0E
#define VIRTIO_PCI_QUEUE_NOTIFY  0x10
#define VIRTIO_PCI_STATUS        0x12
#define VIRTIO_PCI_ISR           0x13
#define VIRTIO_PCI_CONFIG        0x14 // MAC address starts here for net

#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER      2
#define VIRTIO_STATUS_DRIVER_OK   4
#define VIRTIO_STATUS_FEATURES_OK 8
#define VIRTIO_STATUS_FAILED      128

void virtio_net_init(uint8_t bus, uint8_t slot, uint8_t func) {
    print_serial("VIRTIO-NET: Initializing device...\n");

    // Enable Bus Mastering and IO Space
    uint16_t cmd = pci_config_read_word(bus, slot, func, 0x04);
    pci_config_write_word(bus, slot, func, 0x04, cmd | 0x05); // IO Space (0x1) + Bus Master (0x4)

    uint32_t bar0 = pci_config_read_dword(bus, slot, func, 0x10);
    if (!(bar0 & 1)) {
        print_serial("VIRTIO-NET: BAR0 is not IO space.\n");
        return;
    }

    io_base = bar0 & 0xFFFFFFFC;

    // 1. Reset device
    outb(io_base + VIRTIO_PCI_STATUS, 0);

    // 2. Set ACKNOWLEDGE status bit
    outb(io_base + VIRTIO_PCI_STATUS, VIRTIO_STATUS_ACKNOWLEDGE);

    // 3. Set DRIVER status bit
    outb(io_base + VIRTIO_PCI_STATUS, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);

    // 4. Negotiate features
    uint32_t features = inl(io_base + VIRTIO_PCI_HOST_FEATURES);
    // We can just accept whatever we want, or disable some. For now, just write them back.
    outl(io_base + VIRTIO_PCI_GUEST_FEATURES, features & ~(1 << 5)); // Mask out VIRTIO_NET_F_MAC just in case we want to write it, but usually we just read.

    // 5. Set FEATURES_OK (Optional in legacy, required in modern)
    outb(io_base + VIRTIO_PCI_STATUS, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK);

    // 6. Read MAC address
    char buf[10];
    print_serial("VIRTIO-NET: MAC: ");
    for (int i = 0; i < 6; i++) {
        virtio_net_mac[i] = inb(io_base + VIRTIO_PCI_CONFIG + i);
        k_itoa_hex(virtio_net_mac[i], buf);
        print_serial(buf);
        if (i < 5) print_serial(":");
    }
    print_serial("\n");

    // TODO: Setup virtqueues (RX=0, TX=1)
    
    // 8. Set DRIVER_OK
    outb(io_base + VIRTIO_PCI_STATUS, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK);
    
    virtio_net_initialized = 1;
    print_serial("VIRTIO-NET: Initialization complete (queues not fully hooked yet).\n");
}

void virtio_net_send(const uint8_t *data, uint16_t len) {
    (void)data;
    (void)len;
    // TODO: implement virtqueue TX
}

int virtio_net_poll(uint8_t *buf, uint16_t *len_out) {
    (void)buf;
    (void)len_out;
    // TODO: implement virtqueue RX
    return 0;
}
