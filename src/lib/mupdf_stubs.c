/**
 * MuPDF Stub Functions
 *
 * Provides minimal implementations of C standard library functions
 * that MuPDF and its dependencies reference but PureOS doesn't have.
 * These are safe no-ops or minimal implementations sufficient for
 * PDF rendering in a bare-metal environment.
 */

#include "mupdf/fitz.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern void print_serial(const char *s);

/* ======================== STRING FUNCTIONS ======================== */

void *memchr(const void *s, int c, size_t n) {
  const unsigned char *p = (const unsigned char *)s;
  for (size_t i = 0; i < n; i++)
    if (p[i] == (unsigned char)c)
      return (void *)(p + i);
  return 0;
}

char *strpbrk(const char *s, const char *accept) {
  for (; *s; s++)
    for (const char *a = accept; *a; a++)
      if (*s == *a)
        return (char *)s;
  return 0;
}

char *strrchr(const char *s, int c) {
  const char *last = 0;
  for (; *s; s++)
    if (*s == (char)c)
      last = s;
  if ((char)c == '\0')
    return (char *)s;
  return (char *)last;
}

char *strerror(int errnum) {
  (void)errnum;
  return "error";
}

char *realpath(const char *path, char *resolved) {
  if (!path)
    return 0;
  if (!resolved) {
    size_t len = 0;
    while (path[len])
      len++;
    resolved = (char *)kmalloc(len + 1);
    if (!resolved)
      return 0;
  }
  char *dst = resolved;
  while (*path)
    *dst++ = *path++;
  *dst = 0;
  return resolved;
}

long strtol(const char *nptr, char **endptr, int base) {
  long result = 0;
  int neg = 0;
  const char *s = nptr;

  while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
    s++;
  if (*s == '-') {
    neg = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }

  if (base == 0) {
    if (*s == '0' && (s[1] == 'x' || s[1] == 'X')) {
      base = 16;
      s += 2;
    } else if (*s == '0') {
      base = 8;
      s++;
    } else
      base = 10;
  } else if (base == 16 && *s == '0' && (s[1] == 'x' || s[1] == 'X')) {
    s += 2;
  }

  while (*s) {
    int digit;
    if (*s >= '0' && *s <= '9')
      digit = *s - '0';
    else if (*s >= 'a' && *s <= 'f')
      digit = *s - 'a' + 10;
    else if (*s >= 'A' && *s <= 'F')
      digit = *s - 'A' + 10;
    else
      break;
    if (digit >= base)
      break;
    result = result * base + digit;
    s++;
  }

  if (endptr)
    *endptr = (char *)s;
  return neg ? -result : result;
}

long long atoll(const char *nptr) {
  long long result = 0;
  int neg = 0;
  while (*nptr == ' ')
    nptr++;
  if (*nptr == '-') {
    neg = 1;
    nptr++;
  } else if (*nptr == '+') {
    nptr++;
  }
  while (*nptr >= '0' && *nptr <= '9') {
    result = result * 10 + (*nptr - '0');
    nptr++;
  }
  return neg ? -result : result;
}

/* ======================== QSORT / BSEARCH ======================== */

static void swap_bytes(char *a, char *b, size_t size) {
  while (size--) {
    char t = *a;
    *a++ = *b;
    *b++ = t;
  }
}

void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *)) {
  /* Simple insertion sort - adequate for MuPDF's small arrays */
  char *arr = (char *)base;
  for (size_t i = 1; i < nmemb; i++) {
    size_t j = i;
    while (j > 0 && compar(arr + j * size, arr + (j - 1) * size) < 0) {
      swap_bytes(arr + j * size, arr + (j - 1) * size, size);
      j--;
    }
  }
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *)) {
  const char *arr = (const char *)base;
  size_t lo = 0, hi = nmemb;
  while (lo < hi) {
    size_t mid = lo + (hi - lo) / 2;
    int cmp = compar(key, arr + mid * size);
    if (cmp == 0)
      return (void *)(arr + mid * size);
    else if (cmp < 0)
      hi = mid;
    else
      lo = mid + 1;
  }
  return 0;
}

/* ======================== SPRINTF ======================== */

/* vsnprintf/snprintf already exist in string.c */
extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
extern int snprintf(char *buf, size_t size, const char *fmt, ...);

int sprintf(char *buf, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int r = vsnprintf(buf, 65536, fmt, ap);
  va_end(ap);
  return r;
}

int sscanf(const char *str, const char *fmt, ...) {
  (void)str;
  (void)fmt;
  return 0;
}

int printf(const char *fmt, ...) {
  char buf[256];
  va_list ap;
  va_start(ap, fmt);
  int r = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  print_serial(buf);
  return r;
}

int fputc(int c, void *stream) {
  (void)stream;
  char buf[2] = {(char)c, 0};
  print_serial(buf);
  return c;
}

int putc(int c, void *stream) { return fputc(c, stream); }

/* ======================== TIME FUNCTIONS ======================== */

typedef long long time_t;

struct tm {
  int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday,
      tm_isdst;
};

static struct tm stub_tm = {0, 0, 0, 1, 0, 125, 4, 0, 0}; /* 2025-01-01 */

time_t time(time_t *tloc) {
  time_t t = 1735689600LL; /* 2025-01-01 approx */
  if (tloc)
    *tloc = t;
  return t;
}

struct tm *localtime(const time_t *timep) {
  (void)timep;
  return &stub_tm;
}

struct tm *gmtime(const time_t *timep) {
  (void)timep;
  return &stub_tm;
}

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm) {
  (void)format;
  (void)tm;
  if (max > 0) {
    const char *def = "2025-01-01";
    size_t i = 0;
    while (def[i] && i < max - 1) {
      s[i] = def[i];
      i++;
    }
    s[i] = 0;
    return i;
  }
  return 0;
}

time_t timegm(struct tm *tm) {
  (void)tm;
  return 1735689600LL;
}

/* ======================== FILE I/O STUBS ======================== */

int fflush(void *stream) {
  (void)stream;
  return 0;
}
long long ftello(void *stream) {
  (void)stream;
  return 0;
}
int fseeko(void *stream, long long offset, int whence) {
  (void)stream;
  (void)offset;
  (void)whence;
  return -1;
}
int fileno(void *stream) {
  (void)stream;
  return -1;
}
int ftruncate(int fd, long long length) {
  (void)fd;
  (void)length;
  return -1;
}
int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
  (void)stream;
  (void)buf;
  (void)mode;
  (void)size;
  return 0;
}
int remove(const char *pathname) {
  (void)pathname;
  return -1;
}

/* ======================== GETENV ======================== */

char *getenv(const char *name) {
  (void)name;
  return 0;
}

/* ======================== MATH FUNCTIONS ======================== */

/* Software float approximations - only used for rare edge cases in MuPDF */

float logf(float x) {
  /* ln(x) using a simple series for x near 1, or decomposition */
  if (x <= 0.0f)
    return -1e30f;
  if (x == 1.0f)
    return 0.0f;
  /* Use decomposition: x = m * 2^e, ln(x) = ln(m) + e*ln(2) */
  int e = 0;
  float m = x;
  while (m >= 2.0f) {
    m *= 0.5f;
    e++;
  }
  while (m < 1.0f) {
    m *= 2.0f;
    e--;
  }
  /* Now 1 <= m < 2, use series: ln(m) = (m-1) - (m-1)^2/2 + (m-1)^3/3 ... */
  float t = m - 1.0f;
  float sum = 0.0f, term = t;
  for (int i = 1; i <= 12; i++) {
    sum += term / (float)i;
    term *= -t;
  }
  return sum + (float)e * 0.693147180559945f;
}

float expf(float x) {
  if (x == 0.0f)
    return 1.0f;
  if (x > 88.0f)
    return 1e30f;
  if (x < -88.0f)
    return 0.0f;
  /* e^x = sum(x^n / n!) */
  float sum = 1.0f, term = 1.0f;
  for (int i = 1; i <= 20; i++) {
    term *= x / (float)i;
    sum += term;
    if (term < 1e-8f && term > -1e-8f)
      break;
  }
  return sum;
}

float log10f(float x) { return logf(x) * 0.4342944819f; /* 1/ln(10) */ }

float hypotf(float x, float y) {
  if (x < 0)
    x = -x;
  if (y < 0)
    y = -y;
  if (x == 0.0f)
    return y;
  if (y == 0.0f)
    return x;
  /* sqrt(x^2 + y^2) using the larger value to avoid overflow */
  float hi = x > y ? x : y;
  float lo = x > y ? y : x;
  float r = lo / hi;
  /* Approximate sqrt(1 + r^2) * hi */
  float r2 = r * r;
  float s = 1.0f + r2 * 0.5f; /* first-order approx */
  return s * hi;
}

/* ======================== __popcountdi2 ======================== */
/* GCC builtin for 64-bit popcount */
long long __popcountdi2(long long a) {
  unsigned long long v = (unsigned long long)a;
  int count = 0;
  while (v) {
    count += (v & 1);
    v >>= 1;
  }
  return count;
}

/* ======================== MuPDF XML STUBS ======================== */
/* These are from xml.c which was excluded from the build.
   Only called from XFA form parsing which we don't need. */

void fz_drop_xml(fz_context *ctx, fz_xml_doc *xml) {
  (void)ctx;
  (void)xml;
}
fz_xml *fz_parse_xml(fz_context *ctx, fz_buffer *buf, int preserve_white) {
  (void)ctx;
  (void)buf;
  (void)preserve_white;
  return 0;
}

fz_xml *fz_xml_find_down(fz_xml *item, const char *tag) {
  (void)item;
  (void)tag;
  return 0;
}
fz_xml *fz_xml_find_next(fz_xml *item, const char *tag) {
  (void)item;
  (void)tag;
  return 0;
}
fz_xml *fz_xml_find_down_match(fz_xml *item, const char *tag, const char *att,
                               const char *val) {
  (void)item;
  (void)tag;
  (void)att;
  (void)val;
  return 0;
}
fz_xml *fz_xml_find_next_match(fz_xml *item, const char *tag, const char *att,
                               const char *val) {
  (void)item;
  (void)tag;
  (void)att;
  (void)val;
  return 0;
}
char *fz_xml_att(fz_xml *item, const char *att) {
  (void)item;
  (void)att;
  return 0;
}
fz_xml *fz_xml_up(fz_xml *item) {
  (void)item;
  return 0;
}
fz_xml *fz_xml_down(fz_xml *item) {
  (void)item;
  return 0;
}
char *fz_xml_text(fz_xml *item) {
  (void)item;
  return 0;
}
char *fz_xml_tag(fz_xml *item) {
  (void)item;
  return 0;
}
fz_xml *fz_xml_root(fz_xml_doc *xml) {
  (void)xml;
  return 0;
}

/* ======================== MuPDF FONT STUBS ======================== */
/* These satisfy references in noto.c when font blobs are missing */
/* Removed - Now provided by fonts.asm */

/* ======================== MuPDF HANDLER STUBS ======================== */
fz_document_handler img_document_handler;
fz_document_handler xps_document_handler;
fz_document_handler svg_document_handler;
fz_document_handler cbz_document_handler;
fz_document_handler fb2_document_handler;
fz_document_handler xhtml_document_handler;
fz_document_handler mobi_document_handler;
fz_document_handler epub_document_handler;

/* ======================== PDF READER STUB ======================== */
/* Removed - Now using real implementation */
