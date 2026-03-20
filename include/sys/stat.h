#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/types.h>

struct stat {
  int st_mode;
  long st_size;
  long st_mtime;
  long st_atime;
  long st_ctime;
};

#define S_IFMT 0170000
#define S_IFDIR 0040000
#define S_IFREG 0100000

#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)

int stat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);

#endif
