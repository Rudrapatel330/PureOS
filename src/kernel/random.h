#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

// Seed the RNG or get high-quality entropy for TLS
void get_entropy(void *buf, int len);

// Get a random 32-bit integer
uint32_t k_rand(void);

#endif
