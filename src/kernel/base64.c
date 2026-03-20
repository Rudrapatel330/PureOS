#include "base64.h"

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "0123456789+/";

void base64_encode(const unsigned char *src, size_t len, char *out) {
  size_t i = 0, j = 0;
  unsigned char arr_3[3];
  unsigned char arr_4[4];

  while (len--) {
    arr_3[i++] = *(src++);
    if (i == 3) {
      arr_4[0] = (arr_3[0] & 0xfc) >> 2;
      arr_4[1] = ((arr_3[0] & 0x03) << 4) + ((arr_3[1] & 0xf0) >> 4);
      arr_4[2] = ((arr_3[1] & 0x0f) << 2) + ((arr_3[2] & 0xc0) >> 6);
      arr_4[3] = arr_3[2] & 0x3f;

      for (i = 0; i < 4; i++)
        out[j++] = base64_chars[arr_4[i]];
      i = 0;
    }
  }

  if (i > 0) {
    for (size_t k = i; k < 3; k++)
      arr_3[k] = '\0';

    arr_4[0] = (arr_3[0] & 0xfc) >> 2;
    arr_4[1] = ((arr_3[0] & 0x03) << 4) + ((arr_3[1] & 0xf0) >> 4);
    arr_4[2] = ((arr_3[1] & 0x0f) << 2) + ((arr_3[2] & 0xc0) >> 6);

    for (size_t k = 0; k < i + 1; k++)
      out[j++] = base64_chars[arr_4[k]];

    while (i++ < 3)
      out[j++] = '=';
  }

  out[j] = '\0';
}
