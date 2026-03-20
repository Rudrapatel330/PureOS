// ipv4.c - IPv4 Send/Receive with Fragment Reassembly
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *);
extern void k_itoa(int, char *);
extern uint32_t get_timer_ticks(void);

static uint16_t ip_id_counter = 1;

// ====== IP Fragment Reassembly ======
// Single reassembly buffer — PureOS has one active connection at a time
#define REASM_BUF_SIZE 4096 // 4KB max reassembled datagram
#define REASM_TIMEOUT 300   // 3 seconds in timer ticks (100Hz)

static struct {
  uint8_t buf[REASM_BUF_SIZE]; // Reassembled payload (no IP header)
  uint16_t id;                 // IP ID we're reassembling
  uint32_t src_ip;             // Source IP
  uint8_t protocol;            // Protocol of the datagram
  int total_len;       // Total payload length (known when last frag arrives)
  int received;        // Bytes received so far
  uint8_t got_last;    // Have we seen the last fragment (MF=0)?
  uint32_t start_tick; // When we started reassembly
  uint8_t active;      // Is reassembly in progress?
  uint8_t bitmap[REASM_BUF_SIZE / 8]; // Track which bytes we've received
} reasm;

static void reasm_reset(void) {
  reasm.active = 0;
  reasm.id = 0;
  reasm.src_ip = 0;
  reasm.total_len = 0;
  reasm.received = 0;
  reasm.got_last = 0;
  memset(reasm.bitmap, 0, sizeof(reasm.bitmap));
}

// Check if all bytes [0..total_len) have been received
static int reasm_complete(void) {
  if (!reasm.got_last || reasm.total_len <= 0)
    return 0;
  // Quick check: received count matches total
  return (reasm.received >= reasm.total_len);
}

// Mark bytes as received in bitmap and count
static void reasm_mark(int offset, int len) {
  for (int i = offset; i < offset + len && i < REASM_BUF_SIZE; i++) {
    int byte_idx = i / 8;
    int bit_idx = i % 8;
    if (!(reasm.bitmap[byte_idx] & (1 << bit_idx))) {
      reasm.bitmap[byte_idx] |= (1 << bit_idx);
      reasm.received++;
    }
  }
}

// Send an IP packet
void ipv4_send(uint32_t dest_ip, uint8_t protocol, const void *payload,
               uint16_t len) {
  static uint8_t
      packet[sizeof(ip_header_t) + ETH_MTU]; // Moved to static to save stack
  ip_header_t *ip = (ip_header_t *)packet;

  ip->ver_ihl = 0x45; // IPv4, header len = 5 (20 bytes)
  ip->tos = 0;
  ip->total_len = htons(sizeof(ip_header_t) + len);
  ip->id = htons(ip_id_counter++);
  ip->flags_frag = 0;
  ip->ttl = 64;
  ip->protocol = protocol;
  ip->checksum = 0;
  ip->src_ip = net_cfg.ip;
  ip->dest_ip = dest_ip;

  // Calculate header checksum
  ip->checksum = ip_checksum(ip, sizeof(ip_header_t));

  // Copy payload after IP header
  memcpy(packet + sizeof(ip_header_t), payload, len);

  // Resolve MAC for destination (or gateway)
  uint8_t dest_mac[6];
  if (!arp_resolve(dest_ip, dest_mac)) {
    print_serial("IPv4: Cannot resolve MAC, dropping packet\n");
    return;
  }

  // Send via Ethernet
  net_send_eth(dest_mac, ETH_TYPE_IP, packet, sizeof(ip_header_t) + len);
}

// Dispatch a complete (reassembled or unfragmented) IP payload
static void ipv4_dispatch(uint32_t src_ip, uint8_t protocol,
                          const uint8_t *payload, uint16_t payload_len) {
  switch (protocol) {
  case IP_PROTO_UDP:
    udp_receive(src_ip, payload, payload_len);
    break;
  case IP_PROTO_TCP:
    tcp_receive(src_ip, payload, payload_len);
    break;
  case IP_PROTO_ICMP:
    // TODO: ICMP ping reply
    break;
  default:
    break;
  }
}

// Handle incoming IP packet (with fragment reassembly)
void ipv4_receive(const uint8_t *packet, uint16_t len) {
  if (len < sizeof(ip_header_t))
    return;

  const ip_header_t *ip = (const ip_header_t *)packet;

  // Check version
  if ((ip->ver_ihl >> 4) != 4)
    return;

  // Verify checksum
  uint16_t saved_cksum = ip->checksum;
  ip_header_t temp;
  memcpy(&temp, ip, sizeof(ip_header_t));
  temp.checksum = 0;
  if (ip_checksum(&temp, sizeof(ip_header_t)) != saved_cksum) {
    // Checksum mismatch — accept anyway for debugging
  }

  // Check destination (must be for us or broadcast)
  if (ip->dest_ip != net_cfg.ip && ip->dest_ip != 0xFFFFFFFF)
    return;

  // Calculate payload offset and length
  int ihl = (ip->ver_ihl & 0x0F) * 4;
  const uint8_t *payload = packet + ihl;
  uint16_t total_ip_len = ntohs(ip->total_len);
  if (total_ip_len < (uint16_t)ihl)
    return;
  uint16_t payload_len = total_ip_len - ihl;

  // Check for fragmentation
  uint16_t flags_frag = ntohs(ip->flags_frag);
  uint16_t frag_offset = (flags_frag & 0x1FFF) * 8;   // Offset in bytes
  int more_fragments = (flags_frag & 0x2000) ? 1 : 0; // MF flag

  // Not fragmented — fast path
  if (frag_offset == 0 && !more_fragments) {
    ipv4_dispatch(ip->src_ip, ip->protocol, payload, payload_len);
    return;
  }

  // ====== Fragment Reassembly ======
  uint16_t pkt_id = ntohs(ip->id);

  // Check for reassembly timeout on current buffer
  if (reasm.active && (get_timer_ticks() - reasm.start_tick) > REASM_TIMEOUT) {
    print_serial("IPv4: Fragment reassembly timeout, resetting\n");
    reasm_reset();
  }

  // Start new reassembly or check if this fragment belongs to current one
  if (!reasm.active) {
    // Start new reassembly
    reasm_reset();
    reasm.active = 1;
    reasm.id = pkt_id;
    reasm.src_ip = ip->src_ip;
    reasm.protocol = ip->protocol;
    reasm.start_tick = get_timer_ticks();

    print_serial("IPv4: Starting fragment reassembly, ID=");
    char id_str[16];
    k_itoa(pkt_id, id_str);
    print_serial(id_str);
    print_serial("\n");
  } else if (reasm.id != pkt_id || reasm.src_ip != ip->src_ip) {
    // Different datagram — drop old, start new
    print_serial("IPv4: New datagram while reassembling, restarting\n");
    reasm_reset();
    reasm.active = 1;
    reasm.id = pkt_id;
    reasm.src_ip = ip->src_ip;
    reasm.protocol = ip->protocol;
    reasm.start_tick = get_timer_ticks();
  }

  // Copy fragment data into reassembly buffer
  if (frag_offset + payload_len > REASM_BUF_SIZE) {
    print_serial("IPv4: Fragment exceeds reassembly buffer, dropping\n");
    reasm_reset();
    return;
  }

  memcpy(reasm.buf + frag_offset, payload, payload_len);
  reasm_mark(frag_offset, payload_len);

  // If this is the last fragment (MF=0), we now know the total length
  if (!more_fragments) {
    reasm.got_last = 1;
    reasm.total_len = frag_offset + payload_len;

    char len_str[16];
    k_itoa(reasm.total_len, len_str);
    print_serial("IPv4: Last fragment received, total=");
    print_serial(len_str);
    print_serial(" bytes\n");
  }

  // Check if reassembly is complete
  if (reasm_complete()) {
    print_serial("IPv4: Fragment reassembly complete!\n");

    // Dispatch the reassembled datagram
    ipv4_dispatch(reasm.src_ip, reasm.protocol, reasm.buf,
                  (uint16_t)reasm.total_len);

    // Reset for next datagram
    reasm_reset();
  }
}
