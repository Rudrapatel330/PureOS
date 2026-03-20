// udp.c - User Datagram Protocol (stateless, used for DNS)
#include "net.h"
#include "../kernel/string.h"

extern void print_serial(const char*);

// Simple UDP callback system for DNS
static uint16_t udp_listen_port = 0;
static uint8_t  udp_rx_buf[2048];
static int      udp_rx_len = 0;
static int      udp_rx_ready = 0;

void udp_listen(uint16_t port) {
    udp_listen_port = port;
    udp_rx_ready = 0;
    udp_rx_len = 0;
}

int udp_check_rx(uint8_t** data, int* len) {
    if (!udp_rx_ready) return 0;
    *data = udp_rx_buf;
    *len = udp_rx_len;
    udp_rx_ready = 0;
    return 1;
}

void udp_send(uint32_t dest_ip, uint16_t src_port, uint16_t dest_port, const void* data, uint16_t len) {
    uint8_t packet[sizeof(udp_header_t) + 2048];
    udp_header_t* udp = (udp_header_t*)packet;
    
    udp->src_port = htons(src_port);
    udp->dest_port = htons(dest_port);
    udp->length = htons(sizeof(udp_header_t) + len);
    udp->checksum = 0; // UDP checksum is optional in IPv4
    
    memcpy(packet + sizeof(udp_header_t), data, len);
    
    ipv4_send(dest_ip, IP_PROTO_UDP, packet, sizeof(udp_header_t) + len);
}

void udp_receive(uint32_t src_ip, const uint8_t* data, uint16_t len) {
    if (len < sizeof(udp_header_t)) return;
    
    const udp_header_t* udp = (const udp_header_t*)data;
    uint16_t dst_port = ntohs(udp->dest_port);
    uint16_t payload_len = ntohs(udp->length) - sizeof(udp_header_t);
    const uint8_t* payload = data + sizeof(udp_header_t);
    
    // Check if someone is listening on this port
    if (dst_port == udp_listen_port && !udp_rx_ready) {
        if (payload_len > sizeof(udp_rx_buf)) payload_len = sizeof(udp_rx_buf);
        memcpy(udp_rx_buf, payload, payload_len);
        udp_rx_len = payload_len;
        udp_rx_ready = 1;
        print_serial("UDP: Received data on listened port\n");
    }
}
