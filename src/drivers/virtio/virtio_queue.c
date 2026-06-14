#include "virtio_queue.h"

void virtq_init(virtqueue_t *vq, uint16_t q_index, uint16_t q_size, uint32_t notify_off, void *mem_ptr) {
    vq->queue_index = q_index;
    vq->queue_size = q_size;
    vq->notify_offset = notify_off;
    
    vq->desc = (struct virtq_desc *)mem_ptr;
    
    uint32_t avail_offset = q_size * sizeof(struct virtq_desc);
    vq->avail = (struct virtq_avail *)((uint8_t *)mem_ptr + avail_offset);
    
    // Alignment required for used ring (page size 4096 generally)
    uint32_t used_offset = avail_offset + sizeof(uint16_t) * (3 + q_size);
    used_offset = (used_offset + 4095) & ~4095;
    
    vq->used = (struct virtq_used *)((uint8_t *)mem_ptr + used_offset);
    
    vq->last_used_idx = 0;
    vq->free_desc_head = 0;
    vq->num_free_descs = q_size;
    
    for (int i = 0; i < q_size - 1; i++) {
        vq->desc[i].next = i + 1;
    }
    vq->desc[q_size - 1].next = 0xFFFF; // End of list
}

int virtq_alloc_desc(virtqueue_t *vq) {
    if (vq->num_free_descs == 0) return -1;
    
    int head = vq->free_desc_head;
    vq->free_desc_head = vq->desc[head].next;
    vq->num_free_descs--;
    
    return head;
}

void virtq_free_desc(virtqueue_t *vq, int desc_idx) {
    vq->desc[desc_idx].next = vq->free_desc_head;
    vq->free_desc_head = desc_idx;
    vq->num_free_descs++;
}
