#include <math.h>

double sin(double x) {
  double res;
  __asm__("fsin" : "=t"(res) : "0"(x));
  return res;
}
float sinf(float x) { return (float)sin(x); }

double cos(double x) {
  double res;
  __asm__("fcos" : "=t"(res) : "0"(x));
  return res;
}
float cosf(float x) { return (float)cos(x); }

double sqrt(double x) {
  double res;
  __asm__("fsqrt" : "=t"(res) : "0"(x));
  return res;
}
float sqrtf(float x) { return (float)sqrt(x); }

double fabs(double x) {
  double res;
  __asm__("fabs" : "=t"(res) : "0"(x));
  return res;
}
float fabsf(float x) { return (float)fabs(x); }

double atan2(double y, double x) {
  double res;
  __asm__("fpatan" : "=t"(res) : "0"(y), "u"(x) : "st(1)");
  return res;
}
float atan2f(float y, float x) { return (float)atan2(y, x); }

double fmod(double x, double y) {
  double res;
  __asm__("1: fprem; fnstsw %%ax; testb $4, %%ah; jnz 1b"
          : "=t"(res)
          : "0"(x), "u"(y)
          : "ax");
  return res;
}
float fmodf(float x, float y) { return (float)fmod(x, y); }

// Fallbacks for less trivial ones (MuPDF might not use all extensively)
double tan(double x) { return sin(x) / cos(x); }
float tanf(float x) { return (float)tan(x); }

double floor(double x) {
  if (x >= 0.0) {
    long long i = (long long)x;
    return (double)i;
  } else {
    long long i = (long long)x;
    double d = (double)i;
    if (d == x)
      return d;
    return d - 1.0;
  }
}
float floorf(float x) { return (float)floor(x); }

long int lrintf(float x) { return (long int)(x + (x >= 0.0f ? 0.5f : -0.5f)); }

double ceil(double x) {
  if (x < 0.0) {
    long long i = (long long)x;
    return (double)i;
  } else {
    long long i = (long long)x;
    double d = (double)i;
    if (d == x)
      return d;
    return d + 1.0;
  }
}
float ceilf(float x) { return (float)ceil(x); }

double pow(double x, double y) {
  // Simple naive pow, usually MuPDF needs basic stuff or integer powers
  // We can do real pow using fyl2x and f2xm1, but that's complex.
  // Let's implement x^y = 2^(y * log2(x))
  double res;
  __asm__("fyl2x\n\t"        // st(0) = y * log2(x)
          "fld %%st(0)\n\t"  // dup
          "frndint\n\t"      // st(0) = int(y * log2(x))
          "fxch\n\t"         // swap st(0), st(1)
          "fsub %%st(1)\n\t" // st(0) = frac(y * log2(x))
          "f2xm1\n\t"        // st(0) = 2^frac - 1
          "fld1\n\t"
          "faddp\n\t"        // st(0) = 2^frac
          "fscale\n\t"       // st(0) = 2^frac * 2^int
          "fstp %%st(1)\n\t" // pop int
          : "=t"(res)
          : "0"(x), "u"(y)
          : "st(1)");
  return res;
}
float powf(float x, float y) { return (float)pow(x, y); }

double asin(double x) {
  // asin(x) = atan2(x, sqrt(1-x*x))
  return atan2(x, sqrt(1.0 - x * x));
}
float asinf(float x) { return (float)asin(x); }

double acos(double x) {
  // acos(x) = atan2(sqrt(1-x*x), x)
  return atan2(sqrt(1.0 - x * x), x);
}
float acosf(float x) { return (float)acos(x); }

double log(double x) {
  (void)x;
  return 0.0;
}

double exp(double x) {
  (void)x;
  return 0.0;
}

double frexp(double x, int *exp) {
  (void)x;
  *exp = 0;
  return 0.0;
}

double ldexp(double x, int exp) {
  (void)x;
  (void)exp;
  return 0.0;
}
