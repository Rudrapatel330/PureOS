#include "clipboard.h"
#include "heap.h"
#include "string.h"


#define CLIPBOARD_LIMIT 1024

static char *clipboard_buffer = 0;
static int clipboard_operation = CLIPBOARD_OP_NONE;

void clipboard_init() {
  clipboard_buffer = (char *)kmalloc(CLIPBOARD_LIMIT);
  memset(clipboard_buffer, 0, CLIPBOARD_LIMIT);
}

void clipboard_copy(const char *text) {
  if (!text || !clipboard_buffer)
    return;

  // Copy with limit
  strncpy(clipboard_buffer, text, CLIPBOARD_LIMIT - 1);
  clipboard_buffer[CLIPBOARD_LIMIT - 1] = 0;
}

const char *clipboard_paste() { return (const char *)clipboard_buffer; }

void clipboard_set_operation(int op) { clipboard_operation = op; }

int clipboard_get_operation() { return clipboard_operation; }
