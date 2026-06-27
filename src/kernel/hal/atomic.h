#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdint.h>

// Use GCC built-ins for locked increments/decrements
#define atomic_inc(ptr)   __atomic_add_fetch((ptr), 1, __ATOMIC_RELAXED)
#define atomic_dec(ptr)   __atomic_sub_fetch((ptr), 1, __ATOMIC_RELAXED)

// Fetch-and-add (returns old value, useful for post-increment replacements)
#define atomic_fetch_inc(ptr) __atomic_fetch_add((ptr), 1, __ATOMIC_RELAXED)

// Raw reads/writes
#define atomic_read(ptr)  __atomic_load_n((ptr), __ATOMIC_RELAXED)
#define atomic_write(ptr, val) __atomic_store_n((ptr), (val), __ATOMIC_RELAXED)

#endif
