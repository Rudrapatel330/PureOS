// dns.c - DNS Client (A record resolution over UDP)
#include "../drivers/pcnet.h"
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *);
extern uint32_t get_timer_ticks(void);
extern int pcnet_poll(uint8_t *, uint16_t *);

// DNS port and our source port
#define DNS_PORT 53
#define DNS_LOCAL_PORT 12345

// Simple DNS cache with TTL
#define DNS_CACHE_SIZE 8
typedef struct {
  char hostname[64];
  uint32_t ip;
  uint32_t ttl;       // TTL in ticks
  uint32_t cached_at; // Timer tick when cached
  int valid;
} dns_cache_entry_t;

static dns_cache_entry_t dns_cache[DNS_CACHE_SIZE];
static uint16_t dns_id_counter = 0x1234;

// Forward declarations for UDP listen
extern void udp_listen(uint16_t port);
extern int udp_check_rx(uint8_t **data, int *len);

// Encode a hostname into DNS wire format (e.g., "example.com" ->
// "\7example\3com\0")
static int dns_encode_name(const char *name, uint8_t *buf) {
  int pos = 0;
  const char *p = name;

  while (*p) {
    // Find next dot or end
    const char *dot = p;
    int label_len = 0;
    while (*dot && *dot != '.') {
      dot++;
      label_len++;
    }

    buf[pos++] = label_len;
    for (int i = 0; i < label_len; i++) {
      buf[pos++] = p[i];
    }

    p = dot;
    if (*p == '.')
      p++;
  }

  buf[pos++] = 0; // Root label
  return pos;
}

// Flush all DNS cache entries
void dns_cache_flush(void) {
  for (int i = 0; i < DNS_CACHE_SIZE; i++)
    dns_cache[i].valid = 0;
  print_serial("DNS: Cache flushed\n");
}

// Resolve a hostname to an IP address using DNS
uint32_t dns_resolve(const char *hostname) {
  if (!pcnet_dev.initialized) {
    return 0; // Return 0 (invalid IP) if network device is not initialized
  }

  // Static host file to bypass SLIRP emulator DNS timeouts
  if (strcmp(hostname, "frogfind.com") == 0) {
    return make_ip(64, 227, 13, 248);
  }
  if (strcmp(hostname, "wiby.me") == 0) {
    return make_ip(172, 93, 49, 252);
  }
  if (strcmp(hostname, "google.com") == 0 ||
      strcmp(hostname, "www.google.com") == 0) {
    return make_ip(142, 251, 43, 100);
  }
  if (strcmp(hostname, "httpforever.com") == 0) {
    return make_ip(146, 190, 62, 39);
  }
  if (strcmp(hostname, "theoldnet.com") == 0) {
    return make_ip(159, 203, 14, 9);
  }

  // Check if it's already an IP address
  int is_ip = 1;
  int dots = 0;
  for (int i = 0; hostname[i]; i++) {
    if (hostname[i] == '.')
      dots++;
    else if (hostname[i] < '0' || hostname[i] > '9')
      is_ip = 0;
  }
  if (is_ip && dots == 3) {
    uint32_t a = 0, b = 0, c = 0, d = 0;
    const char *p = hostname;
    while (*p >= '0' && *p <= '9') {
      a = a * 10 + (*p - '0');
      p++;
    }
    if (*p == '.')
      p++;
    while (*p >= '0' && *p <= '9') {
      b = b * 10 + (*p - '0');
      p++;
    }
    if (*p == '.')
      p++;
    while (*p >= '0' && *p <= '9') {
      c = c * 10 + (*p - '0');
      p++;
    }
    if (*p == '.')
      p++;
    while (*p >= '0' && *p <= '9') {
      d = d * 10 + (*p - '0');
      p++;
    }
    return make_ip(a, b, c, d);
  }

  // Check cache first (with TTL expiry)
  uint32_t now = get_timer_ticks();
  for (int i = 0; i < DNS_CACHE_SIZE; i++) {
    if (dns_cache[i].valid && strcmp(dns_cache[i].hostname, hostname) == 0) {
      // Check TTL expiry (ttl is in seconds, ticks are ~100/sec)
      uint32_t elapsed = (now - dns_cache[i].cached_at) / 100;
      if (elapsed < dns_cache[i].ttl) {
        print_serial("DNS: Cache hit for ");
        print_serial(hostname);
        print_serial("\n");
        return dns_cache[i].ip;
      } else {
        // Expired — invalidate
        dns_cache[i].valid = 0;
      }
    }
  }

  // Build DNS query packet
  static uint8_t query[512];
  int pos = 0;

  // DNS Header
  dns_header_t *hdr = (dns_header_t *)query;
  hdr->id = htons(dns_id_counter++);
  hdr->flags = htons(0x0100); // Standard query, recursion desired
  hdr->qdcount = htons(1);    // 1 question
  hdr->ancount = 0;
  hdr->nscount = 0;
  hdr->arcount = 0;
  pos += sizeof(dns_header_t);

  // Question: hostname
  pos += dns_encode_name(hostname, query + pos);

  // Type A (1) = IPv4 address
  query[pos++] = 0x00;
  query[pos++] = 0x01;
  // Class IN (1) = Internet
  query[pos++] = 0x00;
  query[pos++] = 0x01;

  // Set up UDP listener for response
  udp_listen(DNS_LOCAL_PORT);

  // Send DNS query via UDP to configured DNS server
  udp_send(net_cfg.dns, DNS_LOCAL_PORT, DNS_PORT, query, pos);
  print_serial("DNS: Query sent for ");
  print_serial(hostname);
  print_serial("\n");

  // Wait for response (up to 15 seconds)
  extern void kernel_poll_events(void);
  uint32_t start = get_timer_ticks();
  while (get_timer_ticks() - start < 1500) {
    // Poll for input events to keep UI alive
    kernel_poll_events();

    // Poll for packets
    static uint8_t buf_dns[1600];
    uint16_t plen;
    int ret = pcnet_poll(buf_dns, &plen);
    if (ret > 0) {
      net_receive(buf_dns, plen);
      // print_serial("DNS: Packet received during resolution\n");
    }

    // Check if UDP received our DNS response
    uint8_t *rx_data;
    int rx_len;
    if (udp_check_rx(&rx_data, &rx_len)) {
      // Parse DNS response
      if (rx_len < (int)sizeof(dns_header_t))
        continue;

      dns_header_t *resp = (dns_header_t *)rx_data;
      int ancount = ntohs(resp->ancount);

      if (ancount == 0) {
        print_serial("DNS: No answers\n");
        return 0;
      }

      // Skip header + question section to get to answers
      int rpos = sizeof(dns_header_t);

      // Skip question (encoded name + type + class)
      while (rpos < rx_len) {
        if (rx_data[rpos] == 0) {
          rpos++;
          break;
        }
        if ((rx_data[rpos] & 0xC0) == 0xC0) {
          rpos += 2;
          break; // Compressed name consumes 2 bytes
        }
        // Normal label
        rpos += rx_data[rpos] + 1;
      }
      rpos += 4; // Skip QTYPE + QCLASS

      // Parse answer records
      for (int i = 0; i < ancount && rpos < rx_len - 10; i++) {
        // Skip name (may be compressed)
        if ((rx_data[rpos] & 0xC0) == 0xC0) {
          rpos += 2; // Pointer
        } else {
          while (rpos < rx_len && rx_data[rpos] != 0)
            rpos += rx_data[rpos] + 1;
          rpos++;
        }

        uint16_t rtype = (rx_data[rpos] << 8) | rx_data[rpos + 1];
        rpos += 2; // Type
        rpos += 2; // Class
        rpos += 4; // TTL
        uint16_t rdlen = (rx_data[rpos] << 8) | rx_data[rpos + 1];
        rpos += 2; // RDLENGTH

        if (rtype == 1 && rdlen == 4) { // A record = IPv4
          uint32_t ip = *(uint32_t *)(rx_data + rpos);

          // Parse TTL from the record (4 bytes before RDLENGTH)
          uint32_t record_ttl = ((uint32_t)rx_data[rpos - 6] << 24) |
                                ((uint32_t)rx_data[rpos - 5] << 16) |
                                ((uint32_t)rx_data[rpos - 4] << 8) |
                                (uint32_t)rx_data[rpos - 3];
          if (record_ttl == 0)
            record_ttl = 300; // Default 5 min

          // Cache it — LRU eviction (replace oldest entry)
          int slot = -1;
          uint32_t oldest_tick = 0xFFFFFFFF;
          for (int c = 0; c < DNS_CACHE_SIZE; c++) {
            if (!dns_cache[c].valid) {
              slot = c;
              break;
            }
            if (dns_cache[c].cached_at < oldest_tick) {
              oldest_tick = dns_cache[c].cached_at;
              slot = c;
            }
          }
          if (slot >= 0) {
            strncpy(dns_cache[slot].hostname, hostname, 63);
            dns_cache[slot].hostname[63] = 0;
            dns_cache[slot].ip = ip;
            dns_cache[slot].ttl = record_ttl;
            dns_cache[slot].cached_at = get_timer_ticks();
            dns_cache[slot].valid = 1;
          }

          print_serial("DNS: Resolved!\n");
          return ip;
        }

        rpos += rdlen;
      }
    }
  }

  print_serial("DNS: Resolution timeout\n");
  return 0;
}
