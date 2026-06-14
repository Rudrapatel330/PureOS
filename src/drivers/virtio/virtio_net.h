#ifndef VIRTIO_NET_H
#define VIRTIO_NET_H

#include <stdint.h>

#define VIRTIO_VENDOR_ID 0x1AF4
#define VIRTIO_NET_DEVICE_ID 0x1000

void virtio_net_init(uint8_t bus, uint8_t slot, uint8_t func);
void virtio_net_send(const uint8_t *data, uint16_t len);
int virtio_net_poll(uint8_t *buf, uint16_t *len_out);

extern int virtio_net_initialized;
extern uint8_t virtio_net_mac[6];

#endif
