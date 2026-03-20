#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>
#include <stdint.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

extern void exit(int status) __attribute__((noreturn));
extern long strtol(const char *nptr, char **endptr, int base);
extern long long atoll(const char *nptr);

extern void *kmalloc(size_t size);
extern void kfree(void *ptr);

extern void *krealloc(void *ptr, size_t size);

extern void *malloc(size_t size);
extern void free(void *ptr);
extern void *realloc(void *ptr, size_t size);
extern void *calloc(size_t nmemb, size_t size);

#endif
