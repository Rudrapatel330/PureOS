#include "bga.h"
#include "ports.h"
#include "timer.h"
#include "gfx_device.h"
#include "../kernel/screen.h"

uint32_t bga_missed_vsyncs = 0;
static uint64_t bga_last_flip_ms = 0;

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
  bga_write_register(VBE_DISPI_INDEX_VIRT_WIDTH, width);
  bga_write_register(VBE_DISPI_INDEX_VIRT_HEIGHT, height * 3);
  uint16_t actual_virt_height = bga_read_register(VBE_DISPI_INDEX_VIRT_HEIGHT);
  
  extern int bga_triple_buffering_enabled;
  if (actual_virt_height >= height * 3) {
      bga_triple_buffering_enabled = 2; // 2 means Triple Buffering (3 pages)
  } else {
      // Try Double Buffering (2 pages)
      bga_write_register(VBE_DISPI_INDEX_VIRT_HEIGHT, height * 2);
      actual_virt_height = bga_read_register(VBE_DISPI_INDEX_VIRT_HEIGHT);
      if (actual_virt_height >= height * 2) {
          bga_triple_buffering_enabled = 1; // 1 means Double Buffering (2 pages)
      } else {
          bga_triple_buffering_enabled = 0; // Single Buffering
      }
  }

  bga_write_register(VBE_DISPI_INDEX_X_OFFSET, 0);
  bga_write_register(VBE_DISPI_INDEX_Y_OFFSET, 0);
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
int bga_third_page = 2;   // Force Single Buffering.
// VirtualBox's Mouse Integration triggers asynchronous host repaints whenever the mouse moves.
// These repaints completely ignore the 60Hz guest VSYNC (port 0x3DA) and read VRAM immediately.
// If hardware double buffering (Y_OFFSET paging) is used, VirtualBox frequently reads the VRAM
// exactly while Y_OFFSET is being flipped, causing massive horizontal tears where the top half 
// is Frame N and the bottom half is Frame N+1.
// By forcing Single Buffering, we composite in system RAM and use dirty-rects to copy only 
// the changed pixels (a fast ~2ms memcpy) to LFB+0. This prevents the Mouse Integration tear.
int bga_triple_buffering_enabled = 0;

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

    // Enable bus mastering for the GPU device
    uint16_t cmd = pci_config_read_word(bus, slot, func, 0x04);
    cmd |= 0x07; // I/O + Memory + Bus Master
    pci_config_write_word(bus, slot, func, 0x04, cmd);
  } else {
    print_serial(
        "BGA: WARNING - PCI Graphics Device NOT found!\n");
    print_serial(
        "BGA: Falling back to 0xE0000000 - this may not work on all VMs!\n");
    bga_lfb = (uint32_t *)0xE0000000;
  }

  // Clear all 3 potential pages to Black
  if (bga_lfb) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 3; i++) {
      bga_lfb[i] = 0; // Black
    }
  }

  // Force single buffering — see comment at declaration.
  // VirtualBox mouse integration reads LFB+0 directly (ignoring Y_OFFSET paging).
  // Triple/double buffering with Y_OFFSET causes VirtualBox to see stale pages
  // (missing the latest rendering, including the sysmenu overlay), producing
  // flickering/ghosting. Single buffering writes everything to LFB+0, so
  // VirtualBox always sees the correct composited frame.
  bga_triple_buffering_enabled = 0;
  // Ensure display shows LFB+0 (page 0) — reset any Y_OFFSET from previous
  // multi-buffer initialization.
  bga_current_page = 0;
  bga_back_page = 0;
  bga_third_page = 0;
  bga_write_register(VBE_DISPI_INDEX_Y_OFFSET, 0);

  char buf[16];
  print_serial("BGA: Linear Framebuffer at 0x");
  k_itoa_hex((uintptr_t)bga_lfb, buf);
  print_serial(buf);
  print_serial("\n");

  static graphics_device_t bga_dev;
  bga_dev.width = bga_width;
  bga_dev.height = bga_height;
  bga_dev.bpp = bga_bpp;
  bga_dev.pitch = bga_width;
  bga_dev.lfb_base = bga_lfb;
  bga_dev.has_triple_buffer = bga_triple_buffering_enabled; // Always 0 (single buffer)
  bga_dev.flip = bga_flip;
  bga_dev.get_render_buffer = bga_get_render_buffer;
  bga_dev.set_mode = 0; // Not fully mapped yet
  gfx_device_register(&bga_dev);

  return 1;
}

void bga_flip() {
  if (!bga_lfb)
    return;
  if (!bga_triple_buffering_enabled)
    return;

  if (bga_triple_buffering_enabled == 2) {
      // Triple buffer swap
      int next_front = bga_back_page;
      int next_back = bga_third_page;
      int next_third = bga_current_page;
    
      bga_current_page = next_front;
      bga_back_page = next_back;
      bga_third_page = next_third;
  } else if (bga_triple_buffering_enabled == 1) {
      // Double buffer swap
      int next_front = bga_back_page;
      bga_back_page = bga_current_page;
      bga_current_page = next_front;
  }

  // === VSync ===
  // Phase 1: Try VGA retrace polling (port 0x3DA bit 3).
  // This works on real VGA and some VMs (VirtualBox), but often fails on Bochs/QEMU BGA.
  // Use a generous timeout: ~1M iterations (~10ms on a slow emulated CPU, ~0.1ms native).
  // If it succeeds, the flip lands inside the vertical blanking interval.  
  int vsync_ok = 0;
  {
    int timeout = 1000000;
    // Wait for end of current retrace (if we're already in retrace)
    while ((inb(0x3DA) & 0x08) && --timeout);
    if (timeout > 0) {
      timeout = 1000000;
      // Wait for start of NEXT retrace
      while (!(inb(0x3DA) & 0x08) && --timeout);
      if (timeout > 0)
        vsync_ok = 1;
    }
  }

  // Phase 2: Software frame-pacing fallback.
  // Align flips to a consistent ~60 Hz cadence using the high-res timer.
  // This prevents tearing when VGA retrace polling is unavailable.
  uint64_t current_ms = get_timer_ms_hires();
  
  if (bga_last_flip_ms != 0) {
    uint64_t elapsed = current_ms - bga_last_flip_ms;

    if (!vsync_ok) {
      // Hardware VSync unavailable — use software pacing.
      // Target 15 ms (slightly under 16.67 ms) to build in a small amount
      // of slack so we don't perpetually drift past the real VSync.
      if (elapsed < 15) {
        uint64_t target = bga_last_flip_ms + 15;
        while (get_timer_ms_hires() < target) {
          __asm__ volatile("pause");
        }
      } else if (elapsed > 20) {
        bga_missed_vsyncs++;
      }
    } else {
      // VGA retrace polling worked — still check for missed frames
      // (frame took longer than one refresh cycle).
      if (elapsed > 20) {
        bga_missed_vsyncs++;
      }
    }
  }

  // Record flip timestamp AFTER any waiting
  bga_last_flip_ms = get_timer_ms_hires();

  // Memory barrier ensures all framebuffer writes are visible before flip
  __asm__ volatile("sfence" ::: "memory");
  bga_write_register(VBE_DISPI_INDEX_Y_OFFSET, bga_current_page * bga_height);
  __asm__ volatile("sfence" ::: "memory");
}

uint32_t *bga_get_render_buffer() {
  if (!bga_lfb)
    return 0;

  if (!bga_triple_buffering_enabled)
    return bga_lfb;

  // Safety: validate page index is within bounds
  if (bga_triple_buffering_enabled == 2) {
      if (bga_back_page < 0 || bga_back_page > 2)
          bga_back_page = 0;
  } else {
      if (bga_back_page < 0 || bga_back_page > 1)
          bga_back_page = 0;
  }

  // Return the BACK buffer
  return bga_lfb + (bga_back_page * (uintptr_t)bga_width * bga_height);
}

void bga_wait_vsync(void) {
  int timeout = 100000;
  while ((inb(0x3DA) & 0x08) && --timeout);
  timeout = 100000;
  while (!(inb(0x3DA) & 0x08) && --timeout);
}
