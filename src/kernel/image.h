#ifndef IMAGE_H
#define IMAGE_H

#include "hal/math.h"
#include "heap.h"
#include <stddef.h>
#include <stdint.h>


// stb_image configuration
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_NO_THREAD_LOCALS
#define STBI_ASSERT(x)
#define STBI_MALLOC(sz) kmalloc(sz)
#define STBI_FREE(p) kfree(p)
#define STBI_REALLOC(p, newsz) krealloc(p, newsz)

// stb_image_write configuration
#define STBI_WRITE_NO_STDIO
#define STBIW_ASSERT(x)
#define STBIW_MALLOC(sz) kmalloc(sz)
#define STBIW_FREE(p) kfree(p)
#define STBIW_REALLOC(p, newsz) krealloc(p, newsz)

#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"

// Wrapper functions if needed, or just include the headers where used.
// For now, including the headers in image.h is sufficient for the API.

#endif // IMAGE_H
