#ifndef VIRTIO_PCI_H
#define VIRTIO_PCI_H

#include <stdint.h>

#define VIRTIO_VENDOR_ID 0x1AF4
#define VIRTIO_GPU_DEVICE_ID 0x1050

void virtio_gpu_init(void);

#endif
