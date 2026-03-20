#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>
#include <stdint.h>


// Encodes `len` bytes from `src` into a null-terminated base64 string `out`.
// The `out` buffer must be at least 4 * ((len + 2) / 3) + 1 bytes long.
void base64_encode(const unsigned char *src, size_t len, char *out);

#endif
