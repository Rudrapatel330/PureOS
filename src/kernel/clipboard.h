#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stdint.h>

#define CLIPBOARD_OP_NONE 0
#define CLIPBOARD_OP_COPY 1
#define CLIPBOARD_OP_CUT 2

void clipboard_init();
void clipboard_copy(const char *text);
const char *clipboard_paste();
void clipboard_set_operation(int op);
int clipboard_get_operation();

#endif
