#ifndef ASSERT_H
#define ASSERT_H

#include "types.h"

void kpanic(const char *message, const char *file, int line);

#define KASSERT(cond)                                                          \
  do {                                                                         \
    if (!(cond)) {                                                             \
      kpanic("Assertion Failed: " #cond, __FILE__, __LINE__);                  \
    }                                                                          \
  } while (0)

#define PANIC(msg) kpanic(msg, __FILE__, __LINE__)

#endif
