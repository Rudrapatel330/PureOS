#include "bga.h"
#include "ports.h"
#include "../kernel/screen.h"

void bga_write_register(uint16_t index, uint16_t value) {
  outw(VBE_DISPI_IOPORT_INDEX, index);
  outw(VBE_DISPI_IOPORT_DATA, value);
}

uint16_t bga_read_register(uint16_t index) {
  outw(VBE_DISPI_IOPORT_INDEX, index);
  return inw(VBE_DISPI_IOPORT_DATA);
}

int bga_is_available() {
  uint16_t id = bga_read_register(VBE_DISPI_INDEX_ID);
  // basic check for BGA versions B0C0 to B0C5
  return (id >= VBE_DISPI_ID0 && id <= VBE_DISPI_ID5);
}

void bga_set_video_mode(uint16_t width, uint16_t height, uint16_t bpp,
                        int use_linear_framebuffer, int clear_video_memory) {
  bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
  bga_write_register(VBE_DISPI_INDEX_XRES, width);
  bga_write_register(VBE_DISPI_INDEX_YRES, height);
  bga_write_register(VBE_DISPI_INDEX_BPP, bpp);

  uint16_t enable = VBE_DISPI_ENABLED;
  if (use_linear_framebuffer)
    enable |= VBE_DISPI_LFB_ENABLED;
  if (!clear_video_memory)
    enable |= VBE_DISPI_NOCLEARMEM;

  bga_write_register(VBE_DISPI_INDEX_ENABLE, enable);
}

uint32_t *bga_lfb = 0;
uint32_t bga_width = 0;
uint32_t bga_height = 0;
uint32_t bga_bpp = 0;
int bga_current_page = 0; // The page currently being displayed
int bga_back_page = 1;    // The page currently being rendered to
int bga_third_page = 2;   // The spare page

#include "pci.h"
extern void print_serial(const char *str);
extern void k_itoa_hex(uint32_t value, char *str);

int bga_init() {
  uint16_t bga_id = bga_read_register(VBE_DISPI_INDEX_ID);
  char id_str[16];
  k_itoa_hex(bga_id, id_str);
  print_serial("BGA: Checking availability... ID: 0x");
  print_serial(id_str);
  print_serial("\n");

  if (!bga_is_available()) {
    print_serial("BGA: Bochs Graphics Adapter NOT found via VBE ports.\n");
    return 0;
  }
  print_serial("BGA: Adapter found. Setting resolution...\n");

  // Set 32-bit mode
  bga_set_video_mode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 1, 1);
  print_serial("BGA: Mode set. Initializing VRAM...\n");

  bga_width = SCREEN_WIDTH;
  bga_height = SCREEN_HEIGHT;
  bga_bpp = 32;
  bga_current_page = 0;

  // Try PCI for LFB address (Bochs, QEMU, VirtualBox)
  uint8_t bus, slot, func;
  int found = 0;

  if (pci_find_device(0x1234, 0x1111, &bus, &slot, &func)) {
    print_serial("BGA: Found Bochs/QEMU PCI Device 0x1234:0x1111\n");
    found = 1;
  } else if (pci_find_device(0x80EE, 0xBEEF, &bus, &slot, &func)) {
    print_serial("BGA: Found VirtualBox PCI Device 0x80EE:0xBEEF\n");
    found = 1;
  }

  if (found) {
    uint32_t bar0 = pci_config_read_dword(bus, slot, func, 0x10);
    bga_lfb = (uint32_t *)(uintptr_t)(bar0 & 0xFFFFFFF0);
    print_serial("BGA: PCI LFB address obtained: 0x");
    char l_str[16];
    k_itoa_hex((uintptr_t)bga_lfb, l_str);
    print_serial(l_str);
    print_serial("\n");
  } else {
    print_serial(
        "BGA: PCI Graphics Device NOT found. Falling back to 0xE0000000\n");
    bga_lfb = (uint32_t *)0xE0000000;
  }

  // Clear all 3 potential pages to Black
  if (bga_lfb) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 3; i++) {
      bga_lfb[i] = 0; // Black
    }
  }

  bga_write_register(VBE_DISPI_INDEX_VIRT_HEIGHT, SCREEN_HEIGHT * 3);
  bga_write_register(VBE_DISPI_INDEX_X_OFFSET, 0);
  bga_write_register(VBE_DISPI_INDEX_Y_OFFSET, 0);

  char buf[16];
  print_serial("BGA: Linear Framebuffer at 0x");
  k_itoa_hex((uintptr_t)bga_lfb, buf);
  print_serial(buf);
  print_serial("\n");

  return 1;
}

void bga_flip() {
  if (!bga_lfb)
    return;

  // Triple buffer swap
  int next_front = bga_back_page;
  int next_back = bga_third_page;
  int next_third = bga_current_page;

  bga_current_page = next_front;
  bga_back_page = next_back;
  bga_third_page = next_third;

  // Wait for vertical retrace
  while (inb(0x3DA) & 0x08)
    ;
  while (!(inb(0x3DA) & 0x08))
    ;

  // Set display pointer
  __asm__ volatile("sfence" ::: "memory");
  bga_write_register(VBE_DISPI_INDEX_Y_OFFSET, bga_current_page * bga_height);
  __asm__ volatile("sfence" ::: "memory");
}

uint32_t *bga_get_render_buffer() {
  if (!bga_lfb)
    return 0;

  // Return the BACK buffer
  return bga_lfb + (bga_back_page * (uintptr_t)bga_width * bga_height);
}

void bga_wait_vsync(void) {
  while (inb(0x3DA) & 0x08)
    ;
  while (!(inb(0x3DA) & 0x08))
    ;
}
