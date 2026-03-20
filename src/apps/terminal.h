#ifndef TERMINAL_H
#define TERMINAL_H

void terminal_init();
void terminal_process_command(const char* cmd);
void terminal_handle_key(char c);
void terminal_clear();

#include "../kernel/window.h"
void terminal_print(window_t* win, const char* str);

#endif
