// mouse.h - COMPLETE REWRITE
#ifndef MOUSE_H
#define MOUSE_H

#include "../kernel/hal/isr.h" // For registers_t
#include <stdint.h>

// Simple mouse state
typedef struct {
  int x;
  int y;
  int last_x;
  int last_y;
  uint32_t saved_bg[16 * 16];
  int visible;
} mouse_cursor_t;

// Global mouse cursor
extern mouse_cursor_t g_mouse;
extern int mouse_initialized;

// Functions
void init_mouse(void);
void mouse_handler_main(int dx, int dy); // Renamed to avoid conflict
void draw_cursor(void);
void remove_cursor(void);

// Init
void init_ps2_mouse(void);
uint64_t mouse_callback(registers_t *regs); // ISR callback

// IO helpers
void mouse_wait(uint8_t type);
uint8_t mouse_read(void);
void mouse_write(uint8_t data);

#endif
