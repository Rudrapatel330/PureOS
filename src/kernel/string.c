#include "string.h"
#include <stdarg.h>

int abs(int n) { return (n < 0) ? -n : n; }

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

size_t strlen(const char *str) {
  size_t i = 0;
  while (str[i] != (char)0)
    i++;
  return i;
}

void *memset(void *dest, int c, size_t n) {
  uint64_t val = (uint8_t)c;
  val |= (val << 8);
  val |= (val << 16);
  val |= (val << 24);
  val |= (val << 32);
  val |= (val << 40);
  val |= (val << 48);
  val |= (val << 56);

  uint64_t *d8 = (uint64_t *)dest;
  size_t n8 = n / 8;
  size_t rem = n % 8;

  __asm__ volatile("rep stosq" : "+D"(d8), "+c"(n8) : "a"(val) : "memory");

  uint8_t *d1 = (uint8_t *)d8;
  while (rem--) {
    *d1++ = (uint8_t)c;
  }
  return dest;
}

void memset16(void *dest, uint16_t val, size_t count) {
  uint16_t *d = (uint16_t *)dest;
  while (count--) {
    *d++ = val;
  }
}

void *memcpy(void *dest, const void *src, size_t n) {
  uint64_t *d8 = (uint64_t *)dest;
  const uint64_t *s8 = (const uint64_t *)src;
  size_t n8 = n / 8;
  size_t rem = n % 8;

  __asm__ volatile("rep movsq" : "+D"(d8), "+S"(s8), "+c"(n8) : : "memory");

  uint8_t *d1 = (uint8_t *)d8;
  const uint8_t *s1 = (const uint8_t *)s8;
  while (rem--) {
    *d1++ = *s1++;
  }
  return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
  uint8_t *d = (uint8_t *)dest;
  const uint8_t *s = (const uint8_t *)src;

  if (d < s) {
    for (size_t i = 0; i < n; i++)
      d[i] = s[i];
  } else {
    for (size_t i = n; i > 0; i--)
      d[i - 1] = s[i - 1];
  }
  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  while (n--) {
    if (*p1 != *p2)
      return *p1 - *p2;
    p1++;
    p2++;
  }
  return 0;
}

char *strcpy(char *dest, const char *src) {
  char *saved = dest;
  while (*src) {
    *dest++ = *src++;
  }
  *dest = 0;
  return saved;
}

char *strncpy(char *dest, const char *src, size_t n) {
  char *saved = dest;
  while (n && *src) {
    *dest++ = *src++;
    n--;
  }
  while (n) {
    *dest++ = 0;
    n--;
  }
  return saved;
}

char *strchr(const char *s, int c) {
  while (*s) {
    if (*s == (char)c)
      return (char *)s;
    s++;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n && *s1 && (*s1 == *s2)) {
    s1++;
    s2++;
    n--;
  }
  if (n == 0)
    return 0;
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *strcat(char *dest, const char *src) {
  char *ptr = dest + strlen(dest);
  while (*src != '\0') {
    *ptr++ = *src++;
  }
  *ptr = '\0';
  return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
  char *ptr = dest + strlen(dest);
  while (n && *src != '\0') {
    *ptr++ = *src++;
    n--;
  }
  *ptr = '\0';
  return dest;
}

void k_itoa(int n, char *s) {
  int i = 0;
  unsigned int num;
  int is_negative = 0;

  if (n == 0) {
    s[i++] = '0';
    s[i] = '\0';
    return;
  }

  if (n < 0) {
    is_negative = 1;
    num = (unsigned int)-n;
  } else {
    num = (unsigned int)n;
  }

  while (num > 0) {
    s[i++] = (num % 10) + '0';
    num = num / 10;
  }

  if (is_negative) {
    s[i++] = '-';
  }
  s[i] = '\0';

  // Reverse the string
  for (int j = 0, k = i - 1; j < k; j++, k--) {
    char temp = s[j];
    s[j] = s[k];
    s[k] = temp;
  }
}

int atoi(const char *s) {
  int res = 0;
  int sign = 1;
  if (*s == '-') {
    sign = -1;
    s++;
  }
  while (*s >= '0' && *s <= '9') {
    res = res * 10 + (*s - '0');
    s++;
  }
  return res * sign;
}

void k_itoa_hex(uint32_t n, char *s) {
  int i = 0;
  if (n == 0) {
    s[i++] = '0';
    s[i] = '\0';
    return;
  }
  while (n > 0) {
    int rem = n % 16;
    if (rem < 10)
      s[i++] = rem + '0';
    else
      s[i++] = rem - 10 + 'a';
    n = n / 16;
  }
  s[i] = '\0';
  // Reverse
  for (int j = 0, k = i - 1; j < k; j++, k--) {
    char temp = s[j];
    s[j] = s[k];
    s[k] = temp;
  }
}
char *strstr(const char *haystack, const char *needle) {
  if (!*needle)
    return (char *)haystack;
  while (*haystack) {
    if (*haystack == *needle) {
      const char *h = haystack;
      const char *n = needle;
      while (*h && *n && *h == *n) {
        h++;
        n++;
      }
      if (!*n)
        return (char *)haystack;
    }
    haystack++;
  }
  return 0;
}

static char to_upper(char c) {
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 'A';
  return c;
}

char *strcasestr(const char *haystack, const char *needle) {
  if (!*needle)
    return (char *)haystack;
  while (*haystack) {
    if (to_upper(*haystack) == to_upper(*needle)) {
      const char *h = haystack;
      const char *n = needle;
      while (*h && *n && to_upper(*h) == to_upper(*n)) {
        h++;
        n++;
      }
      if (!*n)
        return (char *)haystack;
    }
    haystack++;
  }
  return 0;
}

static char *strtok_saved_ptr = 0;
char *strtok(char *str, const char *delim) {
  if (str == 0)
    str = strtok_saved_ptr;
  if (str == 0)
    return 0;

  // Skip leading delimiters
  while (*str && strchr(delim, *str))
    str++;
  if (*str == 0) {
    strtok_saved_ptr = 0;
    return 0;
  }

  char *start = str;
  while (*str && !strchr(delim, *str))
    str++;

  if (*str) {
    *str = 0;
    strtok_saved_ptr = str + 1;
  } else {
    strtok_saved_ptr = 0;
  }
  return start;
}

// ====== snprintf / vsnprintf ======

// Helper: write a single char to buffer with bounds check
static int snp_putc(char *buf, size_t size, size_t pos, char c) {
  if (pos < size - 1)
    buf[pos] = c;
  return 1;
}

// Helper: write a string
static int snp_puts(char *buf, size_t size, size_t pos, const char *s) {
  int written = 0;
  while (*s) {
    if (pos + written < size - 1)
      buf[pos + written] = *s;
    written++;
    s++;
  }
  return written;
}

// Helper: write unsigned int as decimal
static int snp_putd(char *buf, size_t size, size_t pos, int val) {
  char tmp[12];
  int i = 0;
  unsigned int u;
  int neg = 0;

  if (val < 0) {
    neg = 1;
    u = (unsigned int)(-val);
  } else {
    u = (unsigned int)val;
  }

  if (u == 0) {
    tmp[i++] = '0';
  } else {
    while (u > 0) {
      tmp[i++] = '0' + (u % 10);
      u /= 10;
    }
  }
  if (neg)
    tmp[i++] = '-';

  // Reverse and write
  int written = 0;
  for (int j = i - 1; j >= 0; j--) {
    if (pos + written < size - 1)
      buf[pos + written] = tmp[j];
    written++;
  }
  return written;
}

// Helper: write unsigned int
static int snp_putu(char *buf, size_t size, size_t pos, unsigned int val) {
  char tmp[12];
  int i = 0;

  if (val == 0) {
    tmp[i++] = '0';
  } else {
    while (val > 0) {
      tmp[i++] = '0' + (val % 10);
      val /= 10;
    }
  }

  int written = 0;
  for (int j = i - 1; j >= 0; j--) {
    if (pos + written < size - 1)
      buf[pos + written] = tmp[j];
    written++;
  }
  return written;
}

// Helper: write hex
static int snp_putx(char *buf, size_t size, size_t pos, unsigned int val) {
  char tmp[9];
  int i = 0;

  if (val == 0) {
    tmp[i++] = '0';
  } else {
    while (val > 0) {
      int d = val & 0xF;
      tmp[i++] = (d < 10) ? ('0' + d) : ('a' + d - 10);
      val >>= 4;
    }
  }

  int written = 0;
  for (int j = i - 1; j >= 0; j--) {
    if (pos + written < size - 1)
      buf[pos + written] = tmp[j];
    written++;
  }
  return written;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
  if (size == 0)
    return 0;

  size_t pos = 0;

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
      case 's': {
        const char *s = va_arg(args, const char *);
        if (!s)
          s = "(null)";
        pos += snp_puts(buf, size, pos, s);
        break;
      }
      case 'd': {
        int v = va_arg(args, int);
        pos += snp_putd(buf, size, pos, v);
        break;
      }
      case 'u': {
        unsigned int v = va_arg(args, unsigned int);
        pos += snp_putu(buf, size, pos, v);
        break;
      }
      case 'x': {
        unsigned int v = va_arg(args, unsigned int);
        pos += snp_putx(buf, size, pos, v);
        break;
      }
      case 'c': {
        char c = (char)va_arg(args, int);
        pos += snp_putc(buf, size, pos, c);
        break;
      }
      case '%':
        pos += snp_putc(buf, size, pos, '%');
        break;
      case '\0':
        goto done;
      default:
        // Unknown format, just print as-is
        pos += snp_putc(buf, size, pos, '%');
        pos += snp_putc(buf, size, pos, *fmt);
        break;
      }
    } else {
      pos += snp_putc(buf, size, pos, *fmt);
    }
    fmt++;
  }

done:
  // Null terminate
  if (pos < size)
    buf[pos] = '\0';
  else
    buf[size - 1] = '\0';

  return (int)pos;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(buf, size, fmt, args);
  va_end(args);
  return ret;
}
