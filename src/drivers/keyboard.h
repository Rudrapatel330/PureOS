#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

char *keyboard_get_buffer();
char keyboard_getc();
void init_keyboard();
void keyboard_reset_buffer();
void keyboard_process_queue();
void print_char(char c);
void print_str(const char *str);
void set_terminal_color(uint8_t color);
void reset_cursor();

#endif
