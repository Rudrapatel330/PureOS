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
  if (y == 0.0) return NAN;
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
  if (x == 0.0) return (y == 0.0) ? 1.0 : 0.0;
  if (x < 0.0) {
    long long iy = (long long)y;
    if ((double)iy == y) {
      double r = pow(-x, y);
      return (iy % 2 == 0) ? r : -r;
    }
    return NAN;
  }
  double res;
  __asm__("fyl2x\n\t"
          "fld %%st(0)\n\t"
          "frndint\n\t"
          "fxch\n\t"
          "fsub %%st(1)\n\t"
          "f2xm1\n\t"
          "fld1\n\t"
          "faddp\n\t"
          "fscale\n\t"
          "fstp %%st(1)\n\t"
          : "=t"(res)
          : "0"(x), "u"(y)
          : "st(1)");
  return res;
}
float powf(float x, float y) { return (float)pow(x, y); }

double asin(double x) {
  return atan2(x, sqrt(1.0 - x * x));
}
float asinf(float x) { return (float)asin(x); }

double acos(double x) {
  return atan2(sqrt(1.0 - x * x), x);
}
float acosf(float x) { return (float)acos(x); }

double log2(double x) {
    if (x <= 0.0) return NAN;
    double res;
    __asm__("fld1\n\t"
            "fxch\n\t"
            "fyl2x" : "=t"(res) : "0"(x) : "st(1)");
    return res;
}

double log(double x) {
    if (x <= 0.0) return NAN;
    double res;
    __asm__("fldln2\n\t"
            "fxch\n\t"
            "fyl2x" : "=t"(res) : "0"(x) : "st(1)");
    return res;
}

double exp(double x) {
    return pow(2.718281828459045, x);
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

double trunc(double x) {
    if (x >= 0.0) return floor(x);
    return ceil(x);
}

double log10(double x) {
    // log10(x) = log2(x) / log2(10)
    return log2(x) / 3.321928094887362;
}

double cbrt(double x) {
    if (x == 0.0) return 0.0;
    if (x > 0.0) return pow(x, 1.0/3.0);
    return -pow(-x, 1.0/3.0);
}

double atan(double x) {
    return atan2(x, 1.0);
}

double sinh(double x) {
    return (exp(x) - exp(-x)) / 2.0;
}

double cosh(double x) {
    return (exp(x) + exp(-x)) / 2.0;
}

double tanh(double x) {
    double e2x = exp(2.0 * x);
    return (e2x - 1.0) / (e2x + 1.0);
}
