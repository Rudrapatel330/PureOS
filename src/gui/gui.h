#ifndef GUI_H
#define GUI_H

#include "../kernel/types.h"

#include "../kernel/types.h"
#include "../drivers/vbe.h" // Include VBE for kernel access

void gui_init();
void put_pixel(int x, int y, u32 color);
void draw_rect(int x, int y, int w, int h, u32 color);
void fill_screen(u32 color);
void draw_char(int x, int y, char c, u32 color);
void draw_string(int x, int y, char* str, u32 color);

#endif
