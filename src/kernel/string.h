#ifndef STRING_H
#define STRING_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

int abs(int n);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
char *strchr(const char *s, int c);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strstr(const char *haystack, const char *needle);
char *strcasestr(const char *haystack, const char *needle);
void k_itoa(int n, char *s);
int atoi(const char *s);
void k_itoa_hex(uint32_t n, char *s);
char *strtok(char *str, const char *delim);
void memset16(void *dest, uint16_t val, size_t count);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);
char *strerror(int errnum);
void *memchr(const void *s, int c, size_t n);
char *strpbrk(const char *s, const char *accept);
char *strrchr(const char *s, int c);
char *realpath(const char *path, char *resolved_path);

#endif
