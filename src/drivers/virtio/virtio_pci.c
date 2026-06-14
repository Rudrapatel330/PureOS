#include "virtio_pci.h"
#include "../pci.h"

extern void print_serial(const char *str);
extern void k_itoa_hex(uint32_t value, char *str);

#define PCI_CAP_ID_VNDR 0x09

// VirtIO Configuration Types
#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
#define VIRTIO_PCI_CAP_ISR_CFG    3
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTIO_PCI_CAP_PCI_CFG    5
#define VIRTIO_PCI_CAP_SHARED_MEMORY_CFG 8

void virtio_gpu_init(void) {
    print_serial("VIRTIO: Scanning for VirtIO-GPU (0x1AF4:0x1050)...\n");
    uint8_t bus, slot, func;
    
    if (!pci_find_device(VIRTIO_VENDOR_ID, VIRTIO_GPU_DEVICE_ID, &bus, &slot, &func)) {
        print_serial("VIRTIO: VirtIO-GPU device NOT found.\n");
        return;
    }

    print_serial("VIRTIO: Found VirtIO-GPU device at Bus ");
    char b[10]; k_itoa_hex(bus, b); print_serial(b);
    print_serial("\n");

    // Enable bus mastering & Memory Space
    uint16_t cmd = pci_config_read_word(bus, slot, func, 0x04);
    pci_config_write_word(bus, slot, func, 0x04, cmd | 0x06); // Mem Space (0x2) + Bus Master (0x4)

    // Check if capabilities list is available
    uint16_t status = pci_config_read_word(bus, slot, func, 0x06);
    if (!(status & 0x0010)) {
        print_serial("VIRTIO: PCI capabilities list not available!\n");
        return;
    }

    // Read capability pointer (offset 0x34)
    uint8_t cap_ptr = pci_config_read_byte(bus, slot, func, 0x34);
    cap_ptr &= ~3;
    
    print_serial("VIRTIO: Parsing PCI Capabilities...\n");
    while (cap_ptr != 0) {
        uint8_t cap_id = pci_config_read_byte(bus, slot, func, cap_ptr);
        uint8_t next_ptr = pci_config_read_byte(bus, slot, func, cap_ptr + 1);
        
        if (cap_id == PCI_CAP_ID_VNDR) {
            uint8_t cfg_type = pci_config_read_byte(bus, slot, func, cap_ptr + 3);
            uint8_t bar = pci_config_read_byte(bus, slot, func, cap_ptr + 4);
            
            print_serial("VIRTIO: Found Vendor Cap - Type: 0x");
            char ct[10]; k_itoa_hex(cfg_type, ct); print_serial(ct);
            print_serial(" BAR: 0x");
            char br[10]; k_itoa_hex(bar, br); print_serial(br);
            print_serial("\n");
        }
        
        cap_ptr = next_ptr & ~3;
    }
}
