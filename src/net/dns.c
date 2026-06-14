// dns.c - DNS Client (A record resolution over UDP)
#include "../drivers/pcnet.h"
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *);
extern uint32_t get_timer_ticks(void);
extern int pcnet_poll(uint8_t *, uint16_t *);

// Forward declarations for UDP listen (no longer used, but kept for signature)
extern void udp_listen(uint16_t port);
extern int udp_check_rx(uint8_t **data, int *len);

#include "lwip/api.h"

// Flush all DNS cache entries (no-op since lwIP handles cache)
void dns_cache_flush(void) {
  print_serial("DNS: Cache flushed (delegated to lwIP)\n");
}

// Resolve a hostname to an IP address using DNS
uint32_t dns_resolve(const char *hostname) {
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

  print_serial("DNS: Resolving via lwIP: ");
  print_serial(hostname);
  print_serial("...\n");

  ip_addr_t addr;
  err_t err = netconn_gethostbyname(hostname, &addr);
  if (err == ERR_OK) {
    print_serial("DNS: Resolved!\n");
    return addr.addr; // In lwIP IPV4, addr.addr contains the u32 IP in network byte order
  }

  print_serial("DNS: Resolution failed\n");
  return 0;
}
