#include "../drivers/pcnet.h"
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *);
extern uint32_t get_timer_ticks(void);
extern void kernel_poll_events(void);
extern void k_itoa(int, char *);

#define MAX_TCP_CONNS 4
static tcp_conn_t *active_conns[MAX_TCP_CONNS] = {0};
static uint16_t local_port_counter = 49152; // Ephemeral port range

// Helper to flush any pending packets in the NIC
static void tcp_flush_rx() {
  static uint8_t dummy[1600];
  uint16_t plen;
  while (pcnet_poll(dummy, &plen) > 0) {
    // Just drain them
  }
}

// Helper to get next local port
static uint16_t get_next_port() {
  uint16_t port = local_port_counter++;
  if (local_port_counter < 49152)
    local_port_counter = 49152;
  return port;
}

// TCP pseudo-header for checksum
typedef struct __attribute__((packed)) {
  uint32_t src_ip;
  uint32_t dst_ip;
  uint8_t zero;
  uint8_t proto;
  uint16_t tcp_len;
} tcp_pseudo_t;

// Calculate TCP checksum (with pseudo-header)
static uint16_t tcp_checksum(uint32_t src_ip, uint32_t dst_ip,
                             const void *tcp_data, uint16_t tcp_len) {
  // Allocate buffer for pseudo_header + tcp data
  uint8_t buf[1600]; // Local buffer for reentrancy
  int total = sizeof(tcp_pseudo_t) + tcp_len;

  tcp_pseudo_t *pseudo = (tcp_pseudo_t *)buf;
  pseudo->src_ip = src_ip;
  pseudo->dst_ip = dst_ip;
  pseudo->zero = 0;
  pseudo->proto = IP_PROTO_TCP;
  pseudo->tcp_len = htons(tcp_len);

  memcpy(buf + sizeof(tcp_pseudo_t), tcp_data, tcp_len);

  return ip_checksum(buf, total);
}

// Send a TCP segment
static void tcp_send_segment(tcp_conn_t *conn, uint8_t flags, const void *data,
                             uint16_t data_len) {
  uint8_t packet[sizeof(tcp_header_t) + 2048];
  tcp_header_t *tcp = (tcp_header_t *)packet;

  tcp->src_port = htons(conn->local_port);
  tcp->dest_port = htons(conn->remote_port);
  tcp->seq_num = htonl(conn->seq_num);
  tcp->ack_num = htonl(conn->ack_num);
  tcp->data_offset = (5 << 4); // 5 * 4 = 20 bytes header, no options
  tcp->flags = flags;
  tcp->window = htons(65535);
  tcp->checksum = 0;
  tcp->urgent = 0;

  int header_len = sizeof(tcp_header_t);

  // Add MSS option to SYN segments (Kind=2, Len=4, Value=1460)
  if (flags & TCP_FLAG_SYN) {
    tcp->data_offset = (6 << 4); // 6 * 4 = 24 bytes
    header_len = 24;
    packet[20] = 2; // MSS Kind
    packet[21] = 4; // MSS Length
    packet[22] = (1460 >> 8) & 0xFF;
    packet[23] = 1460 & 0xFF;
  }

  if (data && data_len > 0) {
    memcpy(packet + header_len, data, data_len);
  }

  uint16_t total = header_len + data_len;

  // Calculate checksum
  tcp->checksum = tcp_checksum(net_cfg.ip, conn->remote_ip, packet, total);

  print_serial("TCP: Sending segment: flags=0x");
  char fstr[16];
  k_itoa_hex(flags, fstr);
  print_serial(fstr);
  print_serial(", seq=");
  char sstr[16];
  k_itoa(conn->seq_num, sstr);
  print_serial(sstr);
  print_serial(", ack=");
  char astr[16];
  k_itoa(conn->ack_num, astr);
  print_serial(astr);
  print_serial(", len=");
  char lstr[16];
  k_itoa(data_len, lstr);
  print_serial(lstr);
  print_serial("\n");

  ipv4_send(conn->remote_ip, IP_PROTO_TCP, packet, total);
}

// TCP Connect (3-way handshake)
int tcp_connect(tcp_conn_t *conn, uint32_t ip, uint16_t port) {
  tcp_flush_rx(); // Clear any stale packets before connecting
  memset(conn, 0, sizeof(tcp_conn_t));
  conn->remote_ip = ip;
  conn->remote_port = port;
  conn->local_port = get_next_port();
  conn->seq_num = get_timer_ticks() * 12345 + 67890; // Simple PRNG for ISN
  conn->ack_num = 0;
  conn->state = TCP_STATE_SYN_SENT;
  conn->connected = 0;
  conn->error = 0;
  conn->rx_len = 0;
  conn->rx_ready = 0;

  // Find a free slot
  int slot = -1;
  for (int i = 0; i < MAX_TCP_CONNS; i++) {
    if (!active_conns[i]) {
      slot = i;
      break;
    }
  }
  if (slot < 0) return -1; // Max connections reached
  
  active_conns[slot] = conn;

  uint32_t start = get_timer_ticks();
  uint32_t last_syn = 0;

  while (get_timer_ticks() - start < 2000) { // 20 seconds timeout
    // Send SYN every 1000ms
    if (get_timer_ticks() - last_syn > 100) {
      print_serial("TCP: (Re)sending SYN...\n");
      tcp_send_segment(conn, TCP_FLAG_SYN, 0, 0);
      last_syn = get_timer_ticks();
    }

    // Poll for input events
    kernel_poll_events();

    // Poll for packets
    static uint8_t buf_conn[1600];
    uint16_t plen;
    int ret = pcnet_poll(buf_conn, &plen);
    if (ret > 0) {
      net_receive(buf_conn, plen);
    }

    if (conn->state == TCP_STATE_ESTABLISHED) {
      print_serial("TCP: Connected!\n");
      conn->connected = 1;
      return 0;
    }

    if (conn->error) {
      print_serial("TCP: Connection error\n");
      for (int i = 0; i < MAX_TCP_CONNS; i++) {
        if (active_conns[i] == conn) active_conns[i] = 0;
      }
      return -1;
    }
  }

  print_serial("TCP: Connection timeout\n");
  conn->state = TCP_STATE_CLOSED;
  for (int i = 0; i < MAX_TCP_CONNS; i++) {
    if (active_conns[i] == conn) active_conns[i] = 0;
  }
  return -1;
}

// TCP Send Data
int tcp_send(tcp_conn_t *conn, const void *data, int len) {
  if (conn->state != TCP_STATE_ESTABLISHED)
    return -1;

  const uint8_t *ptr = (const uint8_t *)data;
  int sent = 0;

  while (sent < len) {
    int chunk = len - sent;
    if (chunk > 1400)
      chunk = 1400; // MSS (safe below MTU)

    tcp_send_segment(conn, TCP_FLAG_ACK | TCP_FLAG_PSH, ptr + sent, chunk);
    conn->seq_num += chunk;
    sent += chunk;

    // Poll for incoming packets between segments to process ACKs
    // and avoid overwhelming the NIC/gateway
    if (sent < len) {
      kernel_poll_events();
      static uint8_t poll_buf[1600];
      uint16_t plen;
      int ret = pcnet_poll(poll_buf, &plen);
      if (ret > 0) {
        net_receive(poll_buf, plen);
      }
    }
  }

  return sent;
}

// TCP Receive Data (blocking, with timeout)
int tcp_recv(tcp_conn_t *conn, void *buf, int max_len) {
  if (conn->state != TCP_STATE_ESTABLISHED &&
      conn->state != TCP_STATE_CLOSE_WAIT)
    return -1;

  // If we already have data buffered, return it immediately
  // This is critical for BearSSL which makes many small reads
  if (conn->rx_len > 0) {
    int copy = conn->rx_len;
    if (copy > max_len)
      copy = max_len;

    memcpy(buf, conn->rx_buf, copy);

    if (copy < conn->rx_len) {
      int remaining = conn->rx_len - copy;
      memmove(conn->rx_buf, conn->rx_buf + copy, remaining);
      conn->rx_len = remaining;
      conn->rx_ready = 1;
    } else {
      conn->rx_len = 0;
      conn->rx_ready = 0;
    }

    return copy;
  }

  // Wait for data (up to 20 seconds)
  uint32_t start = get_timer_ticks();
  while (get_timer_ticks() - start < 2000) {
    // Poll UI
    kernel_poll_events();

    // Poll for packets
    static uint8_t pbuf[1600];
    uint16_t plen;
    int ret = pcnet_poll(pbuf, &plen);
    if (ret > 0) {
      net_receive(pbuf, plen);
    }

    if (conn->rx_len > 0) {
      int copy = conn->rx_len;
      if (copy > max_len)
        copy = max_len;

      memcpy(buf, conn->rx_buf, copy);

      // Shift remaining data if any
      if (copy < conn->rx_len) {
        int remaining = conn->rx_len - copy;
        memmove(conn->rx_buf, conn->rx_buf + copy, remaining);
        conn->rx_len = remaining;
        conn->rx_ready = 1;
      } else {
        conn->rx_len = 0;
        conn->rx_ready = 0;
      }

      return copy;
    }

    // Check for connection close
    if (conn->state == TCP_STATE_CLOSE_WAIT ||
        conn->state == TCP_STATE_CLOSED) {
      if (conn->rx_len > 0) {
        int copy = conn->rx_len;
        if (copy > max_len)
          copy = max_len;
        memcpy(buf, conn->rx_buf, copy);

        if (copy < conn->rx_len) {
          int remaining = conn->rx_len - copy;
          memmove(conn->rx_buf, conn->rx_buf + copy, remaining);
          conn->rx_len = remaining;
        } else {
          conn->rx_len = 0;
          conn->rx_ready = 0;
        }
        return copy;
      }
      return 0; // Connection closed, no more data
    }
  }

  return 0; // Timeout
}

// TCP Close
void tcp_close(tcp_conn_t *conn) {
  if (conn->state == TCP_STATE_ESTABLISHED) {
    tcp_send_segment(conn, TCP_FLAG_FIN | TCP_FLAG_ACK, 0, 0);
    conn->seq_num++;
    conn->state = TCP_STATE_FIN_WAIT;

    // Wait briefly for FIN-ACK
    uint32_t start = get_timer_ticks();
    while (get_timer_ticks() - start < 200) {
      kernel_poll_events();
      static uint8_t buf_close[1600];
      uint16_t plen;
      int ret = pcnet_poll(buf_close, &plen);
      if (ret > 0)
        net_receive(buf_close, plen);
      if (conn->state == TCP_STATE_CLOSED)
        break;
    }
  }

  conn->state = TCP_STATE_CLOSED;
  conn->connected = 0;
  for (int i = 0; i < MAX_TCP_CONNS; i++) {
    if (active_conns[i] == conn) active_conns[i] = 0;
  }
  print_serial("TCP: Connection closed\n");
}

// Handle incoming TCP segment
void tcp_receive(uint32_t src_ip, const uint8_t *data, uint16_t len) {
  if (len < sizeof(tcp_header_t))
    return;
  const tcp_header_t *tcp = (const tcp_header_t *)data;
  tcp_conn_t *conn = 0;
  for (int i = 0; i < MAX_TCP_CONNS; i++) {
    if (active_conns[i] && 
        active_conns[i]->remote_ip == src_ip && 
        active_conns[i]->remote_port == ntohs(tcp->src_port) &&
        active_conns[i]->local_port == ntohs(tcp->dest_port)) {
      conn = active_conns[i];
      break;
    }
  }
  
  if (!conn) return;

  int data_offset = (tcp->data_offset >> 4) * 4;
  const uint8_t *payload = data + data_offset;
  int payload_len = len - data_offset;

  uint32_t seq = ntohl(tcp->seq_num);
  uint32_t ack = ntohl(tcp->ack_num);

  print_serial("TCP: Recv segment from ");
  char ip_str[32];
  k_itoa(src_ip & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial(".");
  k_itoa((src_ip >> 8) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial(".");
  k_itoa((src_ip >> 16) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial(".");
  k_itoa((src_ip >> 24) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial(" flags=0x");
  char fstr[16];
  k_itoa_hex(tcp->flags, fstr);
  print_serial(fstr);
  print_serial(" seq=");
  char sstr[16];
  k_itoa(seq, sstr);
  print_serial(sstr);
  print_serial(" ack=");
  char astr[16];
  k_itoa(ack, astr);
  print_serial(astr);
  print_serial("\n");

  switch (conn->state) {
  case TCP_STATE_SYN_SENT:
    if ((tcp->flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) ==
        (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
      conn->ack_num = seq + 1;
      conn->seq_num = ack; // Server acknowledged our SYN

      // Send ACK to complete handshake
      tcp_send_segment(conn, TCP_FLAG_ACK, 0, 0);
      conn->state = TCP_STATE_ESTABLISHED;
      print_serial("TCP: Handshake complete (ESTABLISHED)\n");
    } else if (tcp->flags & TCP_FLAG_RST) {
      print_serial("TCP: Connection reset (RST received in SYN_SENT)\n");
      conn->state = TCP_STATE_CLOSED;
      conn->error = 1;
    } else {
      print_serial("TCP: Unexpected response in SYN_SENT: flags=0x");
      char fstr[16];
      k_itoa(tcp->flags, fstr);
      print_serial(fstr);
      print_serial("\n");
    }
    break;

  case TCP_STATE_ESTABLISHED:
    if (tcp->flags & TCP_FLAG_RST) {
      conn->state = TCP_STATE_CLOSED;
      conn->error = 1;
      return;
    }

    // Handle incoming data
    if (payload_len > 0) {
      // Basic reassembly: only accept if it's the next expected segment
      if (seq == conn->ack_num) {
        // Append to receive buffer
        int space = (int)sizeof(conn->rx_buf) - conn->rx_len;
        if (payload_len <= space) {
          memcpy(conn->rx_buf + conn->rx_len, payload, payload_len);
          conn->rx_len += payload_len;
          conn->rx_ready = 1;
          conn->ack_num += payload_len;
        } else {
          print_serial(
              "TCP: rx_buf FULL, DROPPING segment to avoid corruption\n");
          // Do NOT increment ack_num, the host will retransmit.
        }
      } else if (seq < conn->ack_num) {
        // Old data (retransmission), just ACK it again
        print_serial("TCP: Ignoring retransmission\n");
      } else {
        // Gap in data (out of order), we don't support reordering yet
        print_serial("TCP: Gap detected (OOD packet)\n");
        // We'll just stay at current ack_num and wait for missing part
      }

      tcp_send_segment(conn, TCP_FLAG_ACK, 0, 0);
    }

    // Handle FIN
    if (tcp->flags & TCP_FLAG_FIN) {
      conn->ack_num = seq + 1;
      tcp_send_segment(conn, TCP_FLAG_ACK, 0, 0);
      conn->state = TCP_STATE_CLOSE_WAIT;
      conn->rx_ready = 1; // Signal remaining data is available
      print_serial("TCP: Remote closed (FIN received)\n");
    }
    break;

  case TCP_STATE_FIN_WAIT:
    if (tcp->flags & TCP_FLAG_ACK) {
      if (tcp->flags & TCP_FLAG_FIN) {
        conn->ack_num = seq + 1;
        tcp_send_segment(conn, TCP_FLAG_ACK, 0, 0);
      }
      conn->state = TCP_STATE_CLOSED;
    }
    break;

  case TCP_STATE_LAST_ACK:
    if (tcp->flags & TCP_FLAG_ACK) {
      conn->state = TCP_STATE_CLOSED;
    }
    break;
  }
}

void tcp_tick(void) {
  // Future: retransmission timer
}
