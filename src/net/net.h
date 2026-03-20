// net.h - Core Networking Structures and Utilities
#ifndef NET_H
#define NET_H

#include <stdint.h>

// ====== Byte Order Helpers ======
static inline uint16_t htons(uint16_t x) { return (x >> 8) | (x << 8); }
static inline uint16_t ntohs(uint16_t x) { return (x >> 8) | (x << 8); }
static inline uint32_t htonl(uint32_t x) {
  return ((x >> 24) & 0xFF) | ((x >> 8) & 0xFF00) | ((x << 8) & 0xFF0000) |
         ((x << 24) & 0xFF000000);
}
static inline uint32_t ntohl(uint32_t x) { return htonl(x); }

// ====== MAC Address ======
#define MAC_ADDR_LEN 6

// ====== Ethernet Frame ======
#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_IP 0x0800
#define ETH_MTU 1500
#define ETH_HEADER_LEN 14

typedef struct __attribute__((packed)) {
  uint8_t dest[6];
  uint8_t src[6];
  uint16_t type; // Network byte order
} eth_header_t;

// ====== ARP ======
#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2
#define ARP_HW_ETHER 1

typedef struct __attribute__((packed)) {
  uint16_t hw_type;    // 0x0001 = Ethernet
  uint16_t proto_type; // 0x0800 = IPv4
  uint8_t hw_len;      // 6
  uint8_t proto_len;   // 4
  uint16_t opcode;     // 1=request, 2=reply
  uint8_t sender_mac[6];
  uint32_t sender_ip;
  uint8_t target_mac[6];
  uint32_t target_ip;
} arp_packet_t;

// ====== IPv4 ======
#define IP_PROTO_ICMP 1
#define IP_PROTO_TCP 6
#define IP_PROTO_UDP 17

typedef struct __attribute__((packed)) {
  uint8_t ver_ihl; // Version (4) + IHL (5)
  uint8_t tos;
  uint16_t total_len;
  uint16_t id;
  uint16_t flags_frag;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t checksum;
  uint32_t src_ip;
  uint32_t dest_ip;
} ip_header_t;

// ====== UDP ======
typedef struct __attribute__((packed)) {
  uint16_t src_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;
} udp_header_t;

// ====== TCP ======
#define TCP_FLAG_FIN 0x01
#define TCP_FLAG_SYN 0x02
#define TCP_FLAG_RST 0x04
#define TCP_FLAG_PSH 0x08
#define TCP_FLAG_ACK 0x10

typedef struct __attribute__((packed)) {
  uint16_t src_port;
  uint16_t dest_port;
  uint32_t seq_num;
  uint32_t ack_num;
  uint8_t data_offset; // Upper 4 bits = offset in 32-bit words
  uint8_t flags;
  uint16_t window;
  uint16_t checksum;
  uint16_t urgent;
} tcp_header_t;

// ====== DNS ======
typedef struct __attribute__((packed)) {
  uint16_t id;
  uint16_t flags;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
} dns_header_t;

// ====== Network Configuration ======
typedef struct {
  uint32_t ip;      // Our IP (10.0.2.15)
  uint32_t gateway; // Gateway (10.0.2.2)
  uint32_t mask;    // Subnet mask (255.255.255.0)
  uint32_t dns;     // DNS server (10.0.2.3)
  uint8_t mac[6];   // Our MAC
} net_config_t;

extern net_config_t net_cfg;

// ====== Helper to make IP from octets ======
static inline uint32_t make_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  return ((uint32_t)a) | ((uint32_t)b << 8) | ((uint32_t)c << 16) |
         ((uint32_t)d << 24);
}

// ====== IP Checksum ======
uint16_t ip_checksum(void *data, int len);

// ====== Network Init ======
void net_init(void);

// ====== Send raw Ethernet frame ======
void net_send_eth(const uint8_t *dest_mac, uint16_t type, const void *payload,
                  uint16_t payload_len);

// ====== Receive dispatch ======
void net_receive(const uint8_t *packet, uint16_t len);

// ====== ARP ======
void arp_init(void);
int arp_resolve(uint32_t ip, uint8_t *mac_out);
void arp_receive(const uint8_t *packet, uint16_t len);

// ====== IPv4 ======
void ipv4_send(uint32_t dest_ip, uint8_t protocol, const void *payload,
               uint16_t len);
void ipv4_receive(const uint8_t *packet, uint16_t len);

// ====== UDP ======
void udp_send(uint32_t dest_ip, uint16_t src_port, uint16_t dest_port,
              const void *data, uint16_t len);
void udp_receive(uint32_t src_ip, const uint8_t *data, uint16_t len);

// ====== TCP ======
// TCP connection states
#define TCP_STATE_CLOSED 0
#define TCP_STATE_SYN_SENT 1
#define TCP_STATE_ESTABLISHED 2
#define TCP_STATE_FIN_WAIT 3
#define TCP_STATE_CLOSE_WAIT 4
#define TCP_STATE_LAST_ACK 5

typedef struct {
  int state;
  uint32_t remote_ip;
  uint16_t local_port;
  uint16_t remote_port;
  uint32_t seq_num;
  uint32_t ack_num;
  uint16_t remote_window;
  // Receive buffer
  uint8_t rx_buf[65536];
  int rx_len;
  int rx_ready;
  // Status
  int connected;
  int error;
} tcp_conn_t;

int tcp_connect(tcp_conn_t *conn, uint32_t ip, uint16_t port);
int tcp_send(tcp_conn_t *conn, const void *data, int len);
int tcp_recv(tcp_conn_t *conn, void *buf, int max_len);
void tcp_close(tcp_conn_t *conn);
void tcp_receive(uint32_t src_ip, const uint8_t *data, uint16_t len);
void tcp_tick(void);

// ====== DNS ======
uint32_t dns_resolve(const char *hostname);
void dns_cache_flush(void);

// ====== TLS (HTTPS) ======
typedef struct {
  tcp_conn_t tcp;
  void *ssl_ctx;   // Pointer to BearSSL context
  void *x509_ctx;  // Pointer to X509 context
  uint8_t *io_buf; // TLS I/O buffer
  int io_buf_len;
  int connected;
} tls_conn_t;

int tls_connect(tls_conn_t *conn, uint32_t ip, uint16_t port,
                const char *hostname);
int tls_send(tls_conn_t *conn, const void *data, int len);
int tls_recv(tls_conn_t *conn, void *buf, int max_len);
void tls_close(tls_conn_t *conn);

// ====== HTTP ======
int http_get(const char *url, char *response, int max_len);
int https_get(const char *url, char *response, int max_len);
int http_post(const char *url, const char *post_data, int post_len,
              const char *content_type, char *response, int max_len);

// ====== DHCP ======
int dhcp_request(void);

#endif
