// dhcp.c - DHCP Client (RFC 2131)
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *);
extern void k_itoa(int, char *);
extern uint32_t get_timer_ticks(void);
extern int pcnet_poll(uint8_t *, uint16_t *);
extern void kernel_poll_events(void);

// DHCP ports
#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

// DHCP message types
#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_ACK 5

// DHCP magic cookie
#define DHCP_MAGIC 0x63825363

// DHCP packet structure (simplified)
typedef struct __attribute__((packed)) {
  uint8_t op;           // 1 = BOOTREQUEST
  uint8_t htype;        // 1 = Ethernet
  uint8_t hlen;         // 6
  uint8_t hops;         // 0
  uint32_t xid;         // Transaction ID
  uint16_t secs;        // Seconds
  uint16_t flags;       // Flags (0x8000 = broadcast)
  uint32_t ciaddr;      // Client IP
  uint32_t yiaddr;      // Your (client) IP
  uint32_t siaddr;      // Server IP
  uint32_t giaddr;      // Gateway IP
  uint8_t chaddr[16];   // Client MAC (padded)
  uint8_t sname[64];    // Server name
  uint8_t file[128];    // Boot filename
  uint32_t magic;       // Magic cookie (0x63825363)
  uint8_t options[312]; // DHCP options
} dhcp_packet_t;

// Forward declarations
extern void udp_listen(uint16_t port);
extern int udp_check_rx(uint8_t **data, int *len);

static uint32_t dhcp_xid = 0x12345678;

// Build DHCP options
static int dhcp_add_option(uint8_t *opts, int pos, uint8_t type, uint8_t len,
                           const void *data) {
  opts[pos++] = type;
  opts[pos++] = len;
  if (data && len > 0)
    memcpy(&opts[pos], data, len);
  pos += len;
  return pos;
}

// Build a DHCP Discover or Request packet
static int dhcp_build_packet(dhcp_packet_t *pkt, int msg_type,
                             uint32_t requested_ip, uint32_t server_ip) {
  memset(pkt, 0, sizeof(dhcp_packet_t));
  pkt->op = 1;    // BOOTREQUEST
  pkt->htype = 1; // Ethernet
  pkt->hlen = 6;
  pkt->hops = 0;
  pkt->xid = htonl(dhcp_xid);
  pkt->secs = 0;
  pkt->flags = htons(0x8000); // Broadcast

  // Copy our MAC
  for (int i = 0; i < 6; i++)
    pkt->chaddr[i] = net_cfg.mac[i];

  // Magic cookie
  pkt->magic = htonl(DHCP_MAGIC);

  // Build options
  int pos = 0;
  uint8_t mt = (uint8_t)msg_type;
  pos = dhcp_add_option(pkt->options, pos, 53, 1, &mt); // Message type

  // Parameter request list
  uint8_t params[] = {1, 3, 6, 28, 51}; // Mask, Router, DNS, Broadcast, Lease
  pos = dhcp_add_option(pkt->options, pos, 55, sizeof(params), params);

  if (msg_type == DHCP_REQUEST) {
    if (requested_ip) {
      uint32_t rip = requested_ip;
      pos = dhcp_add_option(pkt->options, pos, 50, 4, &rip); // Requested IP
    }
    if (server_ip) {
      uint32_t sip = server_ip;
      pos = dhcp_add_option(pkt->options, pos, 54, 4, &sip); // Server ID
    }
  }

  pkt->options[pos++] = 255; // End
  return (int)((uint8_t *)&pkt->options[pos] - (uint8_t *)pkt);
}

// Parse DHCP options from a response
static int dhcp_parse_options(const uint8_t *opts, int opts_len,
                              uint8_t *msg_type_out, uint32_t *mask_out,
                              uint32_t *router_out, uint32_t *dns_out,
                              uint32_t *server_out) {
  int pos = 0;
  while (pos < opts_len && opts[pos] != 255) {
    uint8_t type = opts[pos++];
    if (type == 0)
      continue; // Padding
    if (pos >= opts_len)
      break;
    uint8_t len = opts[pos++];
    if (pos + len > opts_len)
      break;

    switch (type) {
    case 53: // Message type
      if (len >= 1 && msg_type_out)
        *msg_type_out = opts[pos];
      break;
    case 1: // Subnet mask
      if (len >= 4 && mask_out)
        memcpy(mask_out, &opts[pos], 4);
      break;
    case 3: // Router
      if (len >= 4 && router_out)
        memcpy(router_out, &opts[pos], 4);
      break;
    case 6: // DNS server
      if (len >= 4 && dns_out)
        memcpy(dns_out, &opts[pos], 4);
      break;
    case 54: // Server identifier
      if (len >= 4 && server_out)
        memcpy(server_out, &opts[pos], 4);
      break;
    }
    pos += len;
  }
  return 0;
}

int dhcp_request(void) {
  print_serial("DHCP: Starting discovery...\n");

  dhcp_xid++;

  // Listen for DHCP responses
  udp_listen(DHCP_CLIENT_PORT);

  // --- Phase 1: DISCOVER ---
  dhcp_packet_t discover;
  int discover_len = dhcp_build_packet(&discover, DHCP_DISCOVER, 0, 0);

  // Send to broadcast (255.255.255.255)
  udp_send(make_ip(255, 255, 255, 255), DHCP_CLIENT_PORT, DHCP_SERVER_PORT,
           &discover, discover_len);
  print_serial("DHCP: DISCOVER sent\n");

  // Wait for OFFER (5 second timeout)
  uint32_t offered_ip = 0;
  uint32_t offered_mask = 0;
  uint32_t offered_router = 0;
  uint32_t offered_dns = 0;
  uint32_t server_id = 0;

  uint32_t start = get_timer_ticks();
  while (get_timer_ticks() - start < 500) { // 5 seconds
    kernel_poll_events();
    static uint8_t poll_buf[1600];
    uint16_t plen;
    if (pcnet_poll(poll_buf, &plen) > 0) {
      net_receive(poll_buf, plen);
    }

    uint8_t *rx_data;
    int rx_len;
    if (udp_check_rx(&rx_data, &rx_len)) {
      if (rx_len >= (int)sizeof(dhcp_packet_t) - 312) {
        dhcp_packet_t *resp = (dhcp_packet_t *)rx_data;
        if (ntohl(resp->xid) == dhcp_xid && resp->op == 2) {
          uint8_t msg_type = 0;
          int opts_off = (int)((uint8_t *)resp->options - (uint8_t *)resp);
          int opts_len = rx_len - opts_off;
          dhcp_parse_options(resp->options, opts_len, &msg_type, &offered_mask,
                             &offered_router, &offered_dns, &server_id);

          if (msg_type == DHCP_OFFER) {
            offered_ip = resp->yiaddr;
            print_serial("DHCP: OFFER received\n");
            break;
          }
        }
      }
    }
  }

  if (!offered_ip) {
    print_serial("DHCP: No offer received, using static config\n");
    return -1;
  }

  // --- Phase 2: REQUEST ---
  dhcp_packet_t request;
  int request_len =
      dhcp_build_packet(&request, DHCP_REQUEST, offered_ip, server_id);
  udp_send(make_ip(255, 255, 255, 255), DHCP_CLIENT_PORT, DHCP_SERVER_PORT,
           &request, request_len);
  print_serial("DHCP: REQUEST sent\n");

  // Wait for ACK
  start = get_timer_ticks();
  while (get_timer_ticks() - start < 500) {
    kernel_poll_events();
    static uint8_t poll_buf2[1600];
    uint16_t plen;
    if (pcnet_poll(poll_buf2, &plen) > 0) {
      net_receive(poll_buf2, plen);
    }

    uint8_t *rx_data;
    int rx_len;
    if (udp_check_rx(&rx_data, &rx_len)) {
      if (rx_len >= (int)sizeof(dhcp_packet_t) - 312) {
        dhcp_packet_t *resp = (dhcp_packet_t *)rx_data;
        if (ntohl(resp->xid) == dhcp_xid && resp->op == 2) {
          uint8_t msg_type = 0;
          int opts_off = (int)((uint8_t *)resp->options - (uint8_t *)resp);
          int opts_len = rx_len - opts_off;
          dhcp_parse_options(resp->options, opts_len, &msg_type, &offered_mask,
                             &offered_router, &offered_dns, NULL);

          if (msg_type == DHCP_ACK) {
            // Apply configuration
            net_cfg.ip = offered_ip;
            if (offered_mask)
              net_cfg.mask = offered_mask;
            if (offered_router)
              net_cfg.gateway = offered_router;
            if (offered_dns)
              net_cfg.dns = offered_dns;

            // Print the obtained IP
            char buf[16];
            print_serial("DHCP: Obtained IP: ");
            k_itoa(net_cfg.ip & 0xFF, buf);
            print_serial(buf);
            print_serial(".");
            k_itoa((net_cfg.ip >> 8) & 0xFF, buf);
            print_serial(buf);
            print_serial(".");
            k_itoa((net_cfg.ip >> 16) & 0xFF, buf);
            print_serial(buf);
            print_serial(".");
            k_itoa((net_cfg.ip >> 24) & 0xFF, buf);
            print_serial(buf);
            print_serial("\n");

            return 0;
          }
        }
      }
    }
  }

  print_serial("DHCP: No ACK received, using offered IP anyway\n");
  net_cfg.ip = offered_ip;
  if (offered_mask)
    net_cfg.mask = offered_mask;
  if (offered_router)
    net_cfg.gateway = offered_router;
  if (offered_dns)
    net_cfg.dns = offered_dns;
  return 0;
}
