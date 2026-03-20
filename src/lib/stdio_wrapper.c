#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


// From vfs.h
extern int vfs_open(const char *path, int flags);
extern int vfs_read(int fd, uint8_t *buffer, uint32_t size);
extern int vfs_write(int fd, const uint8_t *buffer, uint32_t size);
extern void vfs_close(int fd);
extern uint32_t vfs_lseek(int fd, uint32_t offset, int whence);

// From stdlib.h
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);

FILE *fopen(const char *pathname, const char *mode) {
  int fd = vfs_open(pathname, 0);
  if (fd < 0)
    return NULL;
  FILE *f = (FILE *)kmalloc(sizeof(FILE));
  if (!f) {
    vfs_close(fd);
    return NULL;
  }
  f->fd = fd;
  f->err = 0;
  return f;
}

int fclose(FILE *stream) {
  if (!stream)
    return -1;
  // Don't close standard streams
  if (stream->fd <= 2)
    return 0;

  vfs_close(stream->fd);
  kfree(stream);
  return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (!stream || !ptr || size == 0 || nmemb == 0)
    return 0;
  int bytes = vfs_read(stream->fd, (uint8_t *)ptr, size * nmemb);
  if (bytes <= 0) {
    if (bytes < 0)
      stream->err = 1;
    return 0;
  }
  return (size_t)(bytes / size);
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (!stream || !ptr || size == 0 || nmemb == 0)
    return 0;
  int bytes = vfs_write(stream->fd, (const uint8_t *)ptr, size * nmemb);
  if (bytes <= 0) {
    if (bytes < 0)
      stream->err = 1;
    return 0;
  }
  return (size_t)(bytes / size);
}

int fseek(FILE *stream, long offset, int whence) {
  if (!stream)
    return -1;
  vfs_lseek(stream->fd, offset, whence);
  return 0;
}

long ftell(FILE *stream) {
  if (!stream)
    return -1;
  return (long)vfs_lseek(stream->fd, 0, SEEK_CUR);
}

int fgetc(FILE *stream) {
  unsigned char c;
  if (fread(&c, 1, 1, stream) != 1)
    return EOF;
  return c;
}

int ferror(FILE *stream) {
  if (!stream)
    return 1;
  return stream->err;
}

int fprintf(FILE *stream, const char *format, ...) { return 0; }

int vfprintf(FILE *stream, const char *format, va_list ap) { return 0; }

static FILE _stdin = {0, 0};
static FILE _stdout = {1, 0};
static FILE _stderr = {2, 0};

FILE *stdin = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;
