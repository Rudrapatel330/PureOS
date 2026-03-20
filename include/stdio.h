#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define EOF (-1)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct {
  int fd;
  int err;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list ap);

FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
int fgetc(FILE *stream);
int ferror(FILE *stream);

typedef struct {
  char name[128];
  uint32_t size;
  uint8_t flags;
} dirent_t;

int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
dirent_t *readdir(int fd, int index);
void exit(int status);
int remove(const char *pathname);

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

#endif
