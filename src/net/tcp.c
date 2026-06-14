#include "../kernel/string.h"
#include "net.h"
#include "lwip/sockets.h"

#undef tcp_connect
#undef tcp_recv
#undef tcp_close
#undef udp_send

extern void print_serial(const char *);

int tcp_connect(tcp_conn_t *conn, uint32_t ip, uint16_t port) {
  memset(conn, 0, sizeof(tcp_conn_t));
  conn->remote_ip = ip;
  conn->remote_port = port;
  conn->lwip_fd = -1;

  int fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    print_serial("LWIP: socket failed\n");
    return -1;
  }

  // Set timeout for recv (20 seconds)
  struct timeval tv;
  tv.tv_sec = 20;
  tv.tv_usec = 0;
  lwip_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  
  // Also set send timeout
  lwip_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip;

  if (lwip_connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    print_serial("LWIP: connect failed\n");
    lwip_close(fd);
    return -1;
  }

  conn->lwip_fd = fd;
  conn->state = TCP_STATE_ESTABLISHED;
  conn->connected = 1;
  print_serial("LWIP: TCP Connected!\n");
  return 0;
}

int tcp_send(tcp_conn_t *conn, const void *data, int len) {
  if (conn->lwip_fd < 0) return -1;
  
  int sent = 0;
  const uint8_t *ptr = (const uint8_t *)data;
  while (sent < len) {
    int n = lwip_write(conn->lwip_fd, ptr + sent, len - sent);
    if (n < 0) {
      print_serial("LWIP: write failed\n");
      return -1;
    }
    sent += n;
  }
  return sent;
}

int tcp_recv(tcp_conn_t *conn, void *buf, int max_len) {
  if (conn->lwip_fd < 0) return -1;
  
  // If we have previously buffered data, use it first
  if (conn->rx_len > 0) {
    int copy = conn->rx_len;
    if (copy > max_len) copy = max_len;
    memcpy(buf, conn->rx_buf, copy);
    if (copy < conn->rx_len) {
      memmove(conn->rx_buf, conn->rx_buf + copy, conn->rx_len - copy);
      conn->rx_len -= copy;
    } else {
      conn->rx_len = 0;
    }
    return copy;
  }

  int n = lwip_read(conn->lwip_fd, buf, max_len);
  if (n < 0) {
    // Timeout or error
    return 0; 
  }
  return n;
}

void tcp_close(tcp_conn_t *conn) {
  if (conn->lwip_fd >= 0) {
    lwip_close(conn->lwip_fd);
    conn->lwip_fd = -1;
  }
  conn->state = TCP_STATE_CLOSED;
  conn->connected = 0;
  print_serial("LWIP: TCP Connection closed\n");
}

void tcp_receive(uint32_t src_ip, const uint8_t *data, uint16_t len) {
  // Unused now, as lwIP handles packet receiving natively.
  (void)src_ip;
  (void)data;
  (void)len;
}

void tcp_tick(void) {
  // Unused. lwIP handles timers natively.
}
