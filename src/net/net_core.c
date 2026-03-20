// net_core.c - Network Core: Ethernet framing, IP checksum, packet dispatch
#include "../drivers/pcnet.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *);

// Global network config
net_config_t net_cfg;

void net_init(void) {
  print_serial("NET: Initializing stack...\n");

  // Copy MAC from NE2000 driver first (needed for DHCP)
  for (int i = 0; i < 6; i++) {
    net_cfg.mac[i] = pcnet_dev.mac[i];
  }
  // Set static defaults first (fallback if DHCP fails)
  net_cfg.ip = make_ip(10, 0, 2, 15);
  net_cfg.gateway = make_ip(10, 0, 2, 2);
  net_cfg.mask = make_ip(255, 255, 255, 0);
  net_cfg.dns = make_ip(10, 0, 2, 3);

  arp_init();
  print_serial("NET: ARP init done\n");

  // Try DHCP if hardware exists
  if (pcnet_dev.initialized) {
    if (dhcp_request() == 0) {
      print_serial("NET: IP obtained via DHCP\n");
    } else {
      print_serial("NET: DHCP failed, using static IP 10.0.2.15\n");
    }
  } else {
    print_serial("NET: No hardware, using static defaults\n");
  }

  print_serial("NET: Stack initialized (IP: 10.0.2.15, GW: 10.0.2.2, MAC: ");
  print_serial("NET: net_cfg address: 0x");
  char addr_str[24];
  uintptr_t addr_val = (uintptr_t)&net_cfg;
  for (int i = 15; i >= 0; i--) {
    addr_str[i] = "0123456789ABCDEF"[addr_val & 0xF];
    addr_val >>= 4;
  }
  addr_str[16] = 0;
  print_serial(addr_str);
  print_serial("\n");
}

// IP header checksum (RFC 1071)
uint16_t ip_checksum(void *data, int len) {
  uint32_t sum = 0;
  uint16_t *ptr = (uint16_t *)data;

  while (len > 1) {
    sum += *ptr++;
    len -= 2;
  }
  if (len == 1) {
    // Pad with zero as the least significant byte (network order: high byte)
    // On x86 (little-endian), this is just the byte itself added to the 32-bit
    // sum.
    sum += *(uint8_t *)ptr;
  }

  while (sum >> 16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  return (uint16_t)(~sum);
}

// Send an Ethernet frame
void net_send_eth(const uint8_t *dest_mac, uint16_t type, const void *payload,
                  uint16_t payload_len) {
  uint8_t frame[ETH_HEADER_LEN + ETH_MTU];

  if (payload_len > ETH_MTU)
    payload_len = ETH_MTU;

  // Build Ethernet header
  memcpy(frame, dest_mac, 6);        // Destination MAC
  memcpy(frame + 6, net_cfg.mac, 6); // Source MAC
  frame[12] = (type >> 8) & 0xFF;    // EtherType (big endian)
  frame[13] = type & 0xFF;

  // Copy payload
  memcpy(frame + ETH_HEADER_LEN, payload, payload_len);

  // Pad to minimum 60 bytes
  int total = ETH_HEADER_LEN + payload_len;
  if (total < 60) {
    memset(frame + total, 0, 60 - total);
    total = 60;
  }

  pcnet_send(frame, total);
}

// Dispatch incoming packet by EtherType
void net_receive(const uint8_t *packet, uint16_t len) {
  if (len < ETH_HEADER_LEN)
    return;

  uint16_t type = ((uint16_t)packet[12] << 8) | packet[13];
  const uint8_t *payload = packet + ETH_HEADER_LEN;
  uint16_t payload_len = len - ETH_HEADER_LEN;

  switch (type) {
  case ETH_TYPE_ARP:
    arp_receive(payload, payload_len);
    break;
  case ETH_TYPE_IP:
    ipv4_receive(payload, payload_len);
    break;
  default:
    break;
  }
}
