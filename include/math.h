#ifndef _MATH_H
#define _MATH_H

#define M_PI 3.14159265358979323846
#define M_SQRT2 1.41421356237309504880

#ifndef NAN
#define NAN (0.0 / 0.0)
#endif

#define isnan(x) ((x) != (x))
#define isinf(x) (!isnan(x) && isnan(x - x))
#define signbit(x) ((x) < 0.0)

double sin(double x);
float sinf(float x);
double cos(double x);
float cosf(float x);
double tan(double x);
float tanf(float x);
double atan2(double y, double x);
float atan2f(float y, float x);
double sqrt(double x);
float sqrtf(float x);
double floor(double x);
float floorf(float x);
double ceil(double x);
float ceilf(float x);
double pow(double x, double y);
float powf(float x, float y);
double fabs(double x);
float fabsf(float x);
double fmod(double x, double y);
float fmodf(float x, float y);
double asin(double x);
float asinf(float x);
double acos(double x);
float acosf(float x);

double log(double x);
double exp(double x);
double frexp(double x, int *exp);
double ldexp(double x, int exp);

long int lrintf(float x);

#endif
