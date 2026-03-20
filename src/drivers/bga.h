#ifndef BGA_H
#define BGA_H

#include <stdint.h>

// BGA Ports
#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

// BGA Registers
#define VBE_DISPI_INDEX_ID 0
#define VBE_DISPI_INDEX_XRES 1
#define VBE_DISPI_INDEX_YRES 2
#define VBE_DISPI_INDEX_BPP 3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_INDEX_BANK 5
#define VBE_DISPI_INDEX_VIRT_WIDTH 6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET 8
#define VBE_DISPI_INDEX_Y_OFFSET 9

// BGA Values
#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x80

// Versions
#define VBE_DISPI_ID0 0xB0C0
#define VBE_DISPI_ID1 0xB0C1
#define VBE_DISPI_ID2 0xB0C2
#define VBE_DISPI_ID3 0xB0C3
#define VBE_DISPI_ID4 0xB0C4
#define VBE_DISPI_ID5 0xB0C5

// Functions
int bga_is_available();
void bga_set_video_mode(uint16_t width, uint16_t height, uint16_t bpp,
                        int use_linear_framebuffer, int clear_video_memory);
void bga_write_register(uint16_t index, uint16_t value);
uint16_t bga_read_register(uint16_t index);

int bga_init();

// Hardware Double Buffering
void bga_flip();
uint32_t *bga_get_render_buffer();

extern uint32_t *bga_lfb;
extern uint32_t bga_width;
extern uint32_t bga_height;
extern uint32_t bga_bpp;
extern int bga_current_page;

void bga_wait_vsync(void);

#endif
