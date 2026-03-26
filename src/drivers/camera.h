#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t *frame_buffer;
    int is_active;
} camera_ctx_t;

void camera_init(void);
void camera_update(void);
camera_ctx_t *camera_get_ctx(void);

#endif
