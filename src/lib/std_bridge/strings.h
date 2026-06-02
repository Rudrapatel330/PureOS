#ifndef STD_BRIDGE_STRINGS_H
#define STD_BRIDGE_STRINGS_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

#ifdef __cplusplus
}
#endif

#endif
