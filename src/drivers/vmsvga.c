#include "vmsvga.h"
#include "pci.h"
#include "ports.h"
#include "../kernel/hal/gfx_device.h"

extern void print_serial(const char *str);
extern void k_itoa_hex(uint32_t value, char *str);

#define SVGA_PCI_VENDOR 0x15AD
#define SVGA_PCI_DEVICE 0x0405

// SVGA Registers
#define SVGA_REG_ID 0
#define SVGA_REG_ENABLE 1
#define SVGA_REG_WIDTH 2
#define SVGA_REG_HEIGHT 3
#define SVGA_REG_MAX_WIDTH 4
#define SVGA_REG_MAX_HEIGHT 5
#define SVGA_REG_DEPTH 6
#define SVGA_REG_BPP 7
#define SVGA_REG_PSEUDOCOLOR 8
#define SVGA_REG_RED_MASK 9
#define SVGA_REG_GREEN_MASK 10
#define SVGA_REG_BLUE_MASK 11
#define SVGA_REG_BPL 12
#define SVGA_REG_FB_START 13
#define SVGA_REG_FB_OFFSET 14
#define SVGA_REG_VRAM_SIZE 15
#define SVGA_REG_FB_SIZE 16

#define SVGA_REG_CAPABILITIES 17
#define SVGA_REG_MEM_START 18
#define SVGA_REG_MEM_SIZE 19
#define SVGA_REG_CONFIG_DONE 20
#define SVGA_REG_SYNC 21
#define SVGA_REG_BUSY 22
#define SVGA_REG_GUEST_ID 23
#define SVGA_REG_CURSOR_ID 24
#define SVGA_REG_CURSOR_X 25
#define SVGA_REG_CURSOR_Y 26
#define SVGA_REG_CURSOR_ON 27
#define SVGA_REG_HOST_BITS_PER_PIXEL 28
#define SVGA_REG_SCRATCH_SIZE 29
#define SVGA_REG_MEM_REGS 30
#define SVGA_REG_NUM_DISPLAYS 31
#define SVGA_REG_PITCHLOCK 32

// SVGA IDs
#define SVGA_MAGIC 0x900000
#define SVGA_MAKE_ID(ver) ((SVGA_MAGIC << 8) | (ver))
#define SVGA_VERSION_2 2
#define SVGA_ID_2 SVGA_MAKE_ID(SVGA_VERSION_2)

// FIFO 
#define SVGA_FIFO_MIN 0
#define SVGA_FIFO_MAX 1
#define SVGA_FIFO_NEXT_CMD 2
#define SVGA_FIFO_STOP 3

#define SVGA_CMD_UPDATE 1
#define SVGA_CMD_RECT_FILL 2
#define SVGA_CMD_RECT_COPY 3

static uint16_t vmsvga_io_base = 0;
static uint32_t vmsvga_fb_base = 0;
static uint32_t vmsvga_fifo_base = 0;
static volatile uint32_t *vmsvga_fifo;

static graphics_device_t vmsvga_dev;

static void vmsvga_write_reg(uint32_t index, uint32_t value) {
    outl(vmsvga_io_base + 0, index);
    outl(vmsvga_io_base + 1, value);
}

static uint32_t vmsvga_read_reg(uint32_t index) {
    outl(vmsvga_io_base + 0, index);
    return inl(vmsvga_io_base + 1);
}

static void vmsvga_sync(void) {
    vmsvga_write_reg(SVGA_REG_SYNC, 1);
    while (vmsvga_read_reg(SVGA_REG_BUSY));
}

static void vmsvga_write_fifo(uint32_t value) {
    if (!vmsvga_fifo) return;
    
    uint32_t next = vmsvga_fifo[SVGA_FIFO_NEXT_CMD];
    uint32_t max = vmsvga_fifo[SVGA_FIFO_MAX];
    uint32_t min = vmsvga_fifo[SVGA_FIFO_MIN];
    uint32_t stop = vmsvga_fifo[SVGA_FIFO_STOP];

    // Wait if FIFO is full
    uint32_t next_next = next + sizeof(uint32_t);
    if (next_next >= max) next_next = min;
    while (next_next == stop) {
        stop = vmsvga_fifo[SVGA_FIFO_STOP];
    }
    
    vmsvga_fifo[next / 4] = value;
    vmsvga_fifo[SVGA_FIFO_NEXT_CMD] = next_next;
}

static void vmsvga_flip(void) {
    if (vmsvga_dev.has_triple_buffer) {
        extern void bga_wait_vsync(void);
        bga_wait_vsync();

        // Issue update command
        vmsvga_write_fifo(SVGA_CMD_UPDATE);
        vmsvga_write_fifo(0);
        vmsvga_write_fifo(0);
        vmsvga_write_fifo(vmsvga_dev.width);
        vmsvga_write_fifo(vmsvga_dev.height);
        vmsvga_sync();
    }
}

static uint32_t* vmsvga_get_render_buffer(void) {
    return (uint32_t*)(uintptr_t)vmsvga_fb_base;
}



static void vmsvga_fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (vmsvga_dev.has_triple_buffer) {
        // HW acceleration only writes to VRAM, not the backbuffer RAM!
        // But if we are rendering directly to VRAM, we can use this.
        vmsvga_write_fifo(SVGA_CMD_RECT_FILL);
        vmsvga_write_fifo(color);
        vmsvga_write_fifo(x);
        vmsvga_write_fifo(y);
        vmsvga_write_fifo(w);
        vmsvga_write_fifo(h);
        // Note: Do not wait for sync immediately, let it run asynchronously unless needed.
    }
}

static void vmsvga_copy_rect(int src_x, int src_y, int dest_x, int dest_y, int w, int h) {
    if (vmsvga_dev.has_triple_buffer) {
        vmsvga_write_fifo(SVGA_CMD_RECT_COPY);
        vmsvga_write_fifo(src_x);
        vmsvga_write_fifo(src_y);
        vmsvga_write_fifo(dest_x);
        vmsvga_write_fifo(dest_y);
        vmsvga_write_fifo(w);
        vmsvga_write_fifo(h);
    }
}



void vmsvga_init(void) {
    print_serial("VMSVGA: Scanning for VMware SVGA II (0x15AD:0x0405)...\n");
    uint8_t bus, slot, func;
    
    if (!pci_find_device(SVGA_PCI_VENDOR, SVGA_PCI_DEVICE, &bus, &slot, &func)) {
        print_serial("VMSVGA: VMware SVGA II device NOT found.\n");
        return;
    }

    print_serial("VMSVGA: Found VMware SVGA II device at Bus ");
    char b[10]; k_itoa_hex(bus, b); print_serial(b);
    print_serial("\n");

    // Enable bus mastering & Memory Space & I/O Space
    uint16_t cmd = pci_config_read_word(bus, slot, func, 0x04);
    pci_config_write_word(bus, slot, func, 0x04, cmd | 0x07);

    // Read BAR0 (I/O)
    vmsvga_io_base = pci_config_read_dword(bus, slot, func, 0x10) & 0xFFFFFFFC;
    // Read BAR1 (Framebuffer)
    vmsvga_fb_base = pci_config_read_dword(bus, slot, func, 0x14) & 0xFFFFFFF0;
    // Read BAR2 (FIFO)
    vmsvga_fifo_base = pci_config_read_dword(bus, slot, func, 0x18) & 0xFFFFFFF0;

    print_serial("VMSVGA: IO Base: 0x"); k_itoa_hex(vmsvga_io_base, b); print_serial(b);
    print_serial(" FB Base: 0x"); k_itoa_hex(vmsvga_fb_base, b); print_serial(b);
    print_serial(" FIFO Base: 0x"); k_itoa_hex(vmsvga_fifo_base, b); print_serial(b); print_serial("\n");

    // Negotiate version
    vmsvga_write_reg(SVGA_REG_ID, SVGA_ID_2);
    if (vmsvga_read_reg(SVGA_REG_ID) != SVGA_ID_2) {
        print_serial("VMSVGA: Version negotiation failed!\n");
        return;
    }
    
    // Setup FIFO
    vmsvga_fifo = (volatile uint32_t *)(uintptr_t)vmsvga_fifo_base;
    uint32_t fifo_size = vmsvga_read_reg(SVGA_REG_MEM_SIZE);
    
    vmsvga_fifo[SVGA_FIFO_MIN] = 4 * sizeof(uint32_t); 
    vmsvga_fifo[SVGA_FIFO_MAX] = fifo_size;
    vmsvga_fifo[SVGA_FIFO_NEXT_CMD] = vmsvga_fifo[SVGA_FIFO_MIN];
    vmsvga_fifo[SVGA_FIFO_STOP] = vmsvga_fifo[SVGA_FIFO_MIN];
    vmsvga_write_reg(SVGA_REG_CONFIG_DONE, 1);
    
    print_serial("VMSVGA: FIFO Initialized.\n");
    
    extern int screen_width;
    extern int screen_height;
    vmsvga_dev.width = screen_width;
    vmsvga_dev.height = screen_height;
    vmsvga_dev.lfb_base = vmsvga_fb_base;
    vmsvga_dev.has_triple_buffer = 1;
    vmsvga_dev.flip = vmsvga_flip;
    vmsvga_dev.get_render_buffer = vmsvga_get_render_buffer;
    vmsvga_dev.fill_rect = vmsvga_fill_rect;
    vmsvga_dev.copy_rect = vmsvga_copy_rect;
    
    gfx_device_register(&vmsvga_dev);
    
    print_serial("VMSVGA: Registered with Graphics HAL.\n");
}

