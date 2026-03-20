// arp.c - Address Resolution Protocol
#include "../drivers/pcnet.h"
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *);

// ARP Cache
#define ARP_CACHE_SIZE 16
typedef struct {
  uint32_t ip;
  uint8_t mac[6];
  int valid;
} arp_entry_t;

static arp_entry_t arp_cache[ARP_CACHE_SIZE];
static const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void arp_init(void) { memset(arp_cache, 0, sizeof(arp_cache)); }

// Add/update an ARP cache entry
static void arp_cache_add(uint32_t ip, const uint8_t *mac) {
  // Check if already exists
  for (int i = 0; i < ARP_CACHE_SIZE; i++) {
    if (arp_cache[i].valid && arp_cache[i].ip == ip) {
      memcpy(arp_cache[i].mac, mac, 6);
      return;
    }
  }
  // Find empty slot
  for (int i = 0; i < ARP_CACHE_SIZE; i++) {
    if (!arp_cache[i].valid) {
      arp_cache[i].ip = ip;
      memcpy(arp_cache[i].mac, mac, 6);
      arp_cache[i].valid = 1;
      return;
    }
  }
  // Cache full, overwrite first entry
  arp_cache[0].ip = ip;
  memcpy(arp_cache[0].mac, mac, 6);
  arp_cache[0].valid = 1;
}

// Lookup MAC for an IP in cache
static int arp_cache_lookup(uint32_t ip, uint8_t *mac_out) {
  for (int i = 0; i < ARP_CACHE_SIZE; i++) {
    if (arp_cache[i].valid && arp_cache[i].ip == ip) {
      memcpy(mac_out, arp_cache[i].mac, 6);
      return 1;
    }
  }
  return 0;
}

// Send an ARP request
static void arp_send_request(uint32_t target_ip) {
  arp_packet_t arp;
  arp.hw_type = htons(ARP_HW_ETHER);
  arp.proto_type = htons(ETH_TYPE_IP);
  arp.hw_len = 6;
  arp.proto_len = 4;
  arp.opcode = htons(ARP_OP_REQUEST);
  memcpy(arp.sender_mac, net_cfg.mac, 6);
  arp.sender_ip = net_cfg.ip;
  memset(arp.target_mac, 0, 6);
  arp.target_ip = target_ip;

  net_send_eth(broadcast_mac, ETH_TYPE_ARP, &arp, sizeof(arp));
  print_serial("ARP: Sent request for IP: ");
  char ipstr[32];
  k_itoa(target_ip & 0xFF, ipstr);
  print_serial(ipstr);
  print_serial(".");
  k_itoa((target_ip >> 8) & 0xFF, ipstr);
  print_serial(ipstr);
  print_serial(".");
  k_itoa((target_ip >> 16) & 0xFF, ipstr);
  print_serial(ipstr);
  print_serial(".");
  k_itoa((target_ip >> 24) & 0xFF, ipstr);
  print_serial(ipstr);
  print_serial("\n");
}

// Send ARP reply
static void arp_send_reply(const uint8_t *dest_mac, uint32_t dest_ip) {
  arp_packet_t arp;
  arp.hw_type = htons(ARP_HW_ETHER);
  arp.proto_type = htons(ETH_TYPE_IP);
  arp.hw_len = 6;
  arp.proto_len = 4;
  arp.opcode = htons(ARP_OP_REPLY);
  memcpy(arp.sender_mac, net_cfg.mac, 6);
  arp.sender_ip = net_cfg.ip;
  memcpy(arp.target_mac, dest_mac, 6);
  arp.target_ip = dest_ip;

  net_send_eth(dest_mac, ETH_TYPE_ARP, &arp, sizeof(arp));
}

// Handle incoming ARP packet
void arp_receive(const uint8_t *packet, uint16_t len) {
  if (len < sizeof(arp_packet_t))
    return;

  const arp_packet_t *arp = (const arp_packet_t *)packet;
  uint16_t op = ntohs(arp->opcode);

  // Always cache sender if it's an Ethernet/IPv4 ARP
  if (ntohs(arp->hw_type) == ARP_HW_ETHER &&
      ntohs(arp->proto_type) == ETH_TYPE_IP) {
    arp_cache_add(arp->sender_ip, arp->sender_mac);
  }

  if (op == ARP_OP_REQUEST) {
    // Someone is asking for our MAC
    if (arp->target_ip == net_cfg.ip) {
      print_serial("ARP: Replying to request for our IP\n");
      arp_send_reply(arp->sender_mac, arp->sender_ip);
    }
  } else if (op == ARP_OP_REPLY) {
    print_serial("ARP: Received reply\n");
  }
}

// Blocking ARP resolve: send request and wait for reply
int arp_resolve(uint32_t target_ip, uint8_t *mac_out) {
  if (!pcnet_dev.initialized)
    return 0;

  uint32_t resolve_ip = target_ip;

  // Check if target is on same subnet
  // If not, we must resolve the Gateway's MAC instead
  if ((target_ip & net_cfg.mask) != (net_cfg.ip & net_cfg.mask)) {
    resolve_ip = net_cfg.gateway;
  }

  // Check cache first
  if (arp_cache_lookup(resolve_ip, mac_out))
    return 1;

  print_serial("ARP: Resolving IP: ");
  char ripstr[32];
  k_itoa(resolve_ip & 0xFF, ripstr);
  print_serial(ripstr);
  print_serial(".");
  k_itoa((resolve_ip >> 8) & 0xFF, ripstr);
  print_serial(ripstr);
  print_serial(".");
  k_itoa((resolve_ip >> 16) & 0xFF, ripstr);
  print_serial(ripstr);
  print_serial(".");
  k_itoa((resolve_ip >> 24) & 0xFF, ripstr);
  print_serial(ripstr);
  print_serial("\n");

  print_serial("ARP: net_cfg addr=");
  char a_str[16];
  k_itoa((int)&net_cfg, a_str);
  print_serial(a_str);
  print_serial(" GW=");
  k_itoa(net_cfg.gateway & 0xFF, ripstr);
  print_serial(ripstr);
  print_serial(".");
  k_itoa((net_cfg.gateway >> 8) & 0xFF, ripstr);
  print_serial(ripstr);
  print_serial(".");
  k_itoa((net_cfg.gateway >> 16) & 0xFF, ripstr);
  print_serial(ripstr);
  print_serial(".");
  k_itoa((net_cfg.gateway >> 24) & 0xFF, ripstr);
  print_serial(ripstr);
  print_serial("\n");

  // Send ARP request for the next hop
  arp_send_request(resolve_ip);

  // Wait up to ~2 seconds for reply (polling)
  extern uint32_t get_timer_ticks(void);
  extern void kernel_poll_events(void); // Keeping UI alive

  uint32_t start = get_timer_ticks();
  uint32_t last_request = 0;

  while (get_timer_ticks() - start < 300) { // 3 seconds timeout
    // Send ARP request every 500ms
    if (get_timer_ticks() - last_request > 50) { // 50 ticks = 500ms
      arp_send_request(resolve_ip);
      last_request = get_timer_ticks();
    }

    // Poll Input
    kernel_poll_events();

    // Poll NE2000 for packets
    static uint8_t buf_arp[1600];
    uint16_t plen;
    int ret = pcnet_poll(buf_arp, &plen);
    if (ret > 0) {
      net_receive(buf_arp, plen);
    }

    // Check cache again for the RESOLVED IP (Gateway or Target)
    if (arp_cache_lookup(resolve_ip, mac_out)) {
      print_serial("ARP: Resolved!\n");
      return 1;
    }
  }

  print_serial("ARP: Resolve timeout\n");
  return 0;
}
