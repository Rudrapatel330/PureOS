#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>


extern int close(int fd);
extern ssize_t read(int fd, void *buf, size_t count);
extern ssize_t write(int fd, const void *buf, size_t count);
extern off_t lseek(int fd, off_t offset, int whence);

#endif
