#ifndef VIRTIO_QUEUE_H
#define VIRTIO_QUEUE_H

#include <stdint.h>

#define VIRTQ_DESC_F_NEXT       1
#define VIRTQ_DESC_F_WRITE      2
#define VIRTQ_DESC_F_INDIRECT   4

struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[]; // actual size depends on queue size
} __attribute__((packed));

struct virtq_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    struct virtq_used_elem ring[];
} __attribute__((packed));

typedef struct {
    uint16_t queue_size;
    uint16_t queue_index;
    
    // VirtIO 1.0 specific pointers
    uint32_t notify_offset; // the offset into the notify bar
    
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;

    uint16_t last_used_idx;
    uint16_t free_desc_head;
    uint16_t num_free_descs;
} virtqueue_t;

void virtq_init(virtqueue_t *vq, uint16_t q_index, uint16_t q_size, uint32_t notify_off, void *mem_ptr);
int virtq_alloc_desc(virtqueue_t *vq);
void virtq_free_desc(virtqueue_t *vq, int desc_idx);

#endif
