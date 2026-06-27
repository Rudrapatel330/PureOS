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

  // Set timeout for recv and send (1 millisecond to prevent blocking)
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000;
  lwip_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  lwip_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip;

  // Set non-blocking mode
  int flags = lwip_fcntl(fd, F_GETFL, 0);
  lwip_fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  if (lwip_connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    if (errno == EINPROGRESS) {
        conn->lwip_fd = fd;
        conn->state = TCP_STATE_SYN_SENT;
        conn->connected = 0;
        conn->rx_ready = 1;
        return 1; // 1 means IN PROGRESS
    }
    print_serial("LWIP: connect failed\n");
    lwip_close(fd);
    return -1;
  }

  conn->lwip_fd = fd;
  conn->state = TCP_STATE_ESTABLISHED;
  conn->connected = 1;
  conn->rx_ready = 1; // <--- This enables receiving in phone/chat apps
  print_serial("LWIP: TCP Connected instantly!\n");
  return 0;
}

int tcp_check_connect(tcp_conn_t *conn) {
    if (conn->lwip_fd < 0) return -1;
    
    fd_set writeset, errset;
    FD_ZERO(&writeset);
    FD_ZERO(&errset);
    FD_SET(conn->lwip_fd, &writeset);
    FD_SET(conn->lwip_fd, &errset);
    
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    int ret = lwip_select(conn->lwip_fd + 1, NULL, &writeset, &errset, &tv);
    if (ret > 0) {
        if (FD_ISSET(conn->lwip_fd, &errset)) {
            print_serial("LWIP: connect failed (delayed)\n");
            return -1;
        }
        if (FD_ISSET(conn->lwip_fd, &writeset)) {
            conn->state = TCP_STATE_ESTABLISHED;
            conn->connected = 1;
            conn->rx_ready = 1;
            print_serial("LWIP: TCP Connected (delayed)!\n");
            return 0; 
        }
    }
    return 1; // Still connecting
}

int tcp_send(tcp_conn_t *conn, const void *data, int len) {
  if (conn->lwip_fd < 0) return -1;
  
  if (conn->tx_len + len > (int)sizeof(conn->tx_buf)) {
      // Buffer full. We must drop the ENTIRE packet to avoid corrupting the stream
      print_serial("LWIP: tx buffer full, dropping packet\n");
      return -1;
  }
  
  memcpy(conn->tx_buf + conn->tx_len, data, len);
  conn->tx_len += len;
  
  // Try to flush immediately
  tcp_flush(conn);
  
  return len;
}

void tcp_flush(tcp_conn_t *conn) {
    if (conn->lwip_fd < 0 || conn->tx_len <= 0) return;
    
    int n = lwip_write(conn->lwip_fd, conn->tx_buf, conn->tx_len);
    if (n > 0) {
        conn->tx_len -= n;
        if (conn->tx_len > 0) {
            memmove(conn->tx_buf, conn->tx_buf + n, conn->tx_len);
        }
    } else if (n < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            print_serial("LWIP: write failed in flush\n");
            // If it's a fatal error, we could close, but for now just clear buffer to avoid infinite loop
            // Actually, keep it. The connection will be closed eventually.
        }
    }
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
    if (errno == EWOULDBLOCK || errno == EAGAIN) return 0;
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
  conn->rx_ready = 0;
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
