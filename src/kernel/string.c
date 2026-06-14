#include "string.h"

void *memcpy(void *dest, const void *src, size_t n) {
  void *ret = dest;
  size_t qwords = n / 8;
  size_t bytes = n % 8;
  __asm__ volatile (
      "rep movsq\n\t"
      "mov %3, %%rcx\n\t"
      "rep movsb"
      : "+D"(dest), "+S"(src), "+c"(qwords)
      : "r"(bytes)
      : "memory"
  );
  return ret;
}

void *memmove(void *dest, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dest;
  const unsigned char *s = (const unsigned char *)src;
  if (d < s) {
    while (n--) *d++ = *s++;
  } else {
    d += n;
    s += n;
    while (n--) *--d = *--s;
  }
  return dest;
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = (unsigned char *)s;
  while (n--) *p++ = (unsigned char)c;
  return s;
}

size_t strlen(const char *str) {
  const char *s = str;
  while (*s) s++;
  return (size_t)(s - str);
}

int abs(int n) {
  return n < 0 ? -n : n;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
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

double strtod(const char *nptr, char **endptr) {
    const char *s = nptr;
    double res = 0.0;
    double sign = 1.0;
    int parsed_digits = 0;
    
    // Skip whitespace
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    
    if (*s == '-') {
        sign = -1.0;
        s++;
    } else if (*s == '+') {
        s++;
    }
    
    // Integer part
    while (*s >= '0' && *s <= '9') {
        res = res * 10.0 + (*s - '0');
        parsed_digits = 1;
        s++;
    }
    
    // Fractional part
    if (*s == '.') {
        s++;
        double fraction = 0.1;
        while (*s >= '0' && *s <= '9') {
            res += (*s - '0') * fraction;
            fraction /= 10.0;
            parsed_digits = 1;
            s++;
        }
    }
    
    // Exponent part (basic support)
    if (parsed_digits && (*s == 'e' || *s == 'E')) {
        const char *exp_start = s;
        s++;
        int exp_sign = 1;
        if (*s == '-') {
            exp_sign = -1;
            s++;
        } else if (*s == '+') {
            s++;
        }
        
        int exponent = 0;
        int exp_parsed = 0;
        while (*s >= '0' && *s <= '9') {
            exponent = exponent * 10 + (*s - '0');
            exp_parsed = 1;
            s++;
        }
        
        if (exp_parsed) {
            if (exp_sign > 0) {
                for (int i = 0; i < exponent; i++) res *= 10.0;
            } else {
                for (int i = 0; i < exponent; i++) res /= 10.0;
            }
        } else {
            // rollback exponent parsing if no digits found
            s = exp_start;
        }
    }
    
    if (endptr) {
        if (parsed_digits) *endptr = (char *)s;
        else *endptr = (char *)nptr;
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

int strcasecmp(const char *s1, const char *s2) {
  while (*s1 && (to_upper(*s1) == to_upper(*s2))) {
    s1++;
    s2++;
  }
  return to_upper(*s1) - to_upper(*s2);
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
  while (n && *s1 && (to_upper(*s1) == to_upper(*s2))) {
    s1++;
    s2++;
    n--;
  }
  if (n == 0)
    return 0;
  return to_upper(*s1) - to_upper(*s2);
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
