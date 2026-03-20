#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *);

// Parse URL into host and path
// format: "http://host/path?query" or "host/path?query"
static int parse_url(const char *url, char *host, int host_max, char *path,
                     int path_max) {
  const char *p = url;

  // Skip "http://" or "https://" if present
  if (strncmp(p, "http://", 7) == 0) {
    p += 7;
  } else if (strncmp(p, "https://", 8) == 0) {
    p += 8;
  }

  // Extract host (up to '/', '?', '#' or end)
  int hi = 0;
  while (*p && *p != '/' && *p != '?' && *p != '#' && hi < host_max - 1) {
    host[hi++] = *p++;
  }
  host[hi] = 0;

  // Extract path (rest of URL, starting with '/' if it was a host-only URL)
  if (*p == '/' || *p == '?' || *p == '#' || *p == '\0') {
    int pi = 0;

    // If we're at '?' or '#' or end, we need to prepend '/' if not already
    // there
    if (*p != '/' && pi < path_max - 1) {
      path[pi++] = '/';
    }

    while (*p && pi < path_max - 1) {
      path[pi++] = *p++;
    }
    path[pi] = 0;
  } else {
    path[0] = '/';
    path[1] = 0;
  }

  return 1;
}

// Decode Hex string to integer
static uint32_t hex_to_int(const char *s) {
  uint32_t val = 0;
  while (*s) {
    uint32_t byte = *s;
    if (byte >= '0' && byte <= '9')
      byte = byte - '0';
    else if (byte >= 'a' && byte <= 'f')
      byte = byte - 'a' + 10;
    else if (byte >= 'A' && byte <= 'F')
      byte = byte - 'A' + 10;
    else
      break;
    val = (val << 4) | (byte & 0xF);
    s++;
  }
  return val;
}

// Simple chunked decoder (decodes in-place)
static int http_decode_chunked(char *data, int len) {
  char *src = data;
  char *dst = data;
  int remaining = len;

  while (remaining > 0) {
    // Read chunk size (hex)
    uint32_t chunk_size = hex_to_int(src);
    // Find end of length line (CRLF)
    char *crlf = strstr(src, "\r\n");
    if (!crlf)
      break;
    src = crlf + 2;

    if (chunk_size == 0)
      break; // Last chunk

    // Move chunk data
    int to_copy = (int)chunk_size;
    if (to_copy > (data + len - src))
      to_copy = data + len - src;

    memmove(dst, src, to_copy);
    dst += to_copy;
    src += to_copy;

    // Skip trailing CRLF
    if (src[0] == '\r' && src[1] == '\n')
      src += 2;
    else if (src[0] == '\n')
      src += 1;

    remaining = len - (src - data);
  }

  *dst = 0;
  return (int)(dst - data);
}

// HTTP Proxy configuration - set these for proxy support
#define HTTP_PROXY_ENABLED 0
#define HTTP_PROXY_HOST "10.0.2.2" // Host machine in Bochs (SLiRP)
#define HTTP_PROXY_PORT 8080

// Convert IP string to uint32 (e.g., "10.0.2.2" -> 0x0202000A)
static uint32_t ip_string_to_uint32(const char *ip_str) {
  uint8_t octets[4];
  int count = 0;
  const char *p = ip_str;

  while (*p && count < 4) {
    int val = 0;
    while (*p >= '0' && *p <= '9') {
      val = val * 10 + (*p - '0');
      p++;
    }
    octets[count++] = val;
    if (*p == '.')
      p++;
    else
      break;
  }

  if (count == 4) {
    return (octets[0] | (octets[1] << 8) | (octets[2] << 16) |
            (octets[3] << 24));
  }
  return 0;
}

// Perform an HTTP GET request (with proxy support)
// Returns: number of body bytes placed in response, or -1 on error
int http_get(const char *url, char *response, int max_len) {
  static char host[128];
  static char path[256];
  int use_proxy = HTTP_PROXY_ENABLED;
  uint32_t proxy_ip = 0;

#if HTTP_PROXY_ENABLED
  // Resolve proxy IP at startup
  static uint32_t cached_proxy_ip = 0;
  if (cached_proxy_ip == 0) {
    // First try to parse as IP string directly
    cached_proxy_ip = ip_string_to_uint32(HTTP_PROXY_HOST);
    if (cached_proxy_ip == 0) {
      // If that fails, try DNS
      cached_proxy_ip = dns_resolve(HTTP_PROXY_HOST);
    }
    if (cached_proxy_ip == 0) {
      print_serial("HTTP: Proxy DNS failed, trying direct\n");
      use_proxy = 0;
    }
  }
  proxy_ip = cached_proxy_ip;
#endif

  if (!parse_url(url, host, sizeof(host), path, sizeof(path))) {
    print_serial("HTTP: Invalid URL\n");
    return -1;
  }

  print_serial("HTTP: Connecting to ");
  print_serial(host);
  print_serial(path);
  print_serial("\n");

  // Determine target IP and port
  uint32_t target_ip;
  uint16_t target_port;

  if (use_proxy && proxy_ip != 0) {
    // Use proxy
    target_ip = proxy_ip;
    target_port = HTTP_PROXY_PORT;
    print_serial("HTTP: Using proxy\n");
  } else {
    // Direct connection
    target_ip = dns_resolve(host);
    target_port = 80;
    if (target_ip == 0) {
      print_serial("HTTP: DNS failed for ");
      print_serial(host);
      print_serial("\n");
      return -2;
    }
  }

  char ip_str[32];
  k_itoa(target_ip & 0xFF, ip_str);
  print_serial("HTTP: Target IP ");
  print_serial(ip_str);
  print_serial(".");
  k_itoa((target_ip >> 8) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial(".");
  k_itoa((target_ip >> 16) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial(".");
  k_itoa((target_ip >> 24) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial("\n");

  // TCP Connect
  tcp_conn_t *conn = kmalloc(sizeof(tcp_conn_t));
  if (!conn) {
    print_serial("HTTP: Out of memory for connection\n");
    return -1;
  }
  int conn_res = tcp_connect(conn, target_ip, target_port);
  if (conn_res != 0) {
    print_serial("HTTP: TCP connect failed\n");
    kfree(conn);
    return -3;
  }

  // Build HTTP request
  static char request[1024];
  int rlen = 0;

  if (use_proxy) {
    // Proxy request - send full URL
    strcpy(request, "GET ");
    strcpy(request, url); // Full URL for proxy
    strcat(request, " HTTP/1.1\r\n");
    strcat(request, "Host: ");
    strcat(request, host);
    strcat(request, "\r\n");
    strcat(request,
           "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
           "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36\r\n");
    strcat(request, "Accept: */*\r\n");
    strcat(request, "Connection: close\r\n");
    strcat(request, "\r\n");
  } else {
    // Direct request
    strcpy(request, "GET ");
    strcat(request, path);
    strcat(request, " HTTP/1.1\r\n");
    strcat(request, "Host: ");
    strcat(request, host);
    strcat(request, "\r\n");
    strcat(request,
           "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
           "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36\r\n");
    strcat(request, "Accept: */*\r\n");
    strcat(request, "Connection: close\r\n");
    strcat(request, "\r\n");
  }
  rlen = strlen(request);

  // 4. Send request
  if (tcp_send(conn, request, rlen) < 0) {
    print_serial("HTTP: Send failed\n");
    tcp_close(conn);
    kfree(conn);
    return -1;
  }

  print_serial("HTTP: Request sent, waiting for response...\n");

  // 5. Receive response (accumulate all data)
  int total = 0;
  while (total < max_len - 1) {
    int n = tcp_recv(conn, response + total, max_len - 1 - total);
    if (n < 0) {
      print_serial("HTTP: Receive error\n");
      break;
    }
    if (n == 0)
      break;
    total += n;
  }
  response[total] = 0;

  tcp_close(conn);
  kfree(conn);

  if (total == 0) {
    print_serial(
        "HTTP: Received 0 bytes (Server closed connection immediately or "
        "timed out)\n");
    return 0;
  }

  // 6. Find body (after "\r\n\r\n" or "\n\n")
  char *body = 0;
  for (int i = 0; i < total - 1; i++) {
    if (i < total - 3 && response[i] == '\r' && response[i + 1] == '\n' &&
        response[i + 2] == '\r' && response[i + 3] == '\n') {
      body = response + i + 4;
      break;
    }
    if (response[i] == '\n' && response[i + 1] == '\n') {
      body = response + i + 2;
      break;
    }
  }

  if (!body) {
    print_serial("HTTP: No delimiter found in response\n");
    return -4; // Format error
  }

  // Parse status code from first line
  int status = 0;
  const char *sp = response;
  // Skip "HTTP/1.x "
  while (sp < body && *sp && *sp != ' ')
    sp++;
  if (sp < body && *sp == ' ') {
    sp++;
    // Status code is exactly 3 digits
    if (sp[0] >= '0' && sp[0] <= '9' && sp[1] >= '0' && sp[1] <= '9' &&
        sp[2] >= '0' && sp[2] <= '9') {
      status = (sp[0] - '0') * 100 + (sp[1] - '0') * 10 + (sp[2] - '0');
    }
  }

  // Debug: Print first line of response
  print_serial("HTTP: Response Line: ");
  const char *lp = response;
  while (lp < body && *lp && *lp != '\r' && *lp != '\n') {
    char s[2] = {*lp++, 0};
    print_serial(s);
  }
  print_serial("\n");

  char stat_str[16];
  k_itoa(status, stat_str);
  print_serial("HTTP: Status ");
  print_serial(stat_str);
  print_serial("\n");

  // Handle redirects (301, 302, 307, 308)
  if (status == 301 || status == 302 || status == 303 || status == 307 ||
      status == 308) {
    // Find "Location:" or "location:" header
    const char *loc = response;
    while (loc < body) {
      if ((loc[0] == 'L' || loc[0] == 'l') &&
          strncmp(loc + 1, "ocation:", 8) == 0) {
        loc += 9;
        while (*loc == ' ')
          loc++;
        // Copy location URL to response buffer with REDIRECT: prefix
        strcpy(response, "REDIRECT:");
        int ri = 9;
        while (*loc && *loc != '\r' && *loc != '\n' && ri < max_len - 1) {
          response[ri++] = *loc++;
        }
        response[ri] = 0;
        print_serial("HTTP: Redirect to ");
        print_serial(response + 9);
        print_serial("\n");
        return -301; // Special return code for redirect
      }
      loc++;
    }
  }

  // Handle errors (4xx, 5xx)
  if (status >= 400) {
    print_serial("HTTP: Error status detected: ");
    char s[16];
    k_itoa(status, s);
    print_serial(s);
    print_serial("\n");

    int body_offset = body - response;
    int body_len = total - body_offset;
    if (body_len > 0) {
      memmove(response, body, body_len);
      response[body_len] = 0;
    } else {
      response[0] = 0;
      body_len = 0;
    }
    // Always return negative status for errors, even if body is empty
    return -status;
  }

  int body_len = total - (body - response);
  if (body_len <= 0) {
    print_serial("HTTP: Body 0 bytes\n");
    return 0; // Truly empty
  }

  // Check for chunked encoding
  int is_chunked = 0;
  const char *te = response;
  while (te < body) {
    if ((te[0] == 'T' || te[0] == 't') &&
        strncmp(te + 1, "ransfer-Encoding:", 17) == 0) {
      if (strstr(te, "chunked")) {
        is_chunked = 1;
        break;
      }
    }
    te++;
  }

  memmove(response, body, body_len);
  response[body_len] = 0;

  if (is_chunked) {
    print_serial("HTTP: Decoding chunked body...\n");
    body_len = http_decode_chunked(response, body_len);
  }

  print_serial("HTTP: Received body ");
  char len_str[16];
  k_itoa(body_len, len_str);
  print_serial(len_str);
  print_serial(" bytes\n");

  return body_len;
}

// Perform an HTTPS GET request using TLS
int https_get(const char *url, char *response, int max_len) {
  static char host[128];
  static char path[256];

  if (!parse_url(url, host, sizeof(host), path, sizeof(path))) {
    print_serial("HTTPS: Invalid URL\n");
    return -1;
  }

  print_serial("HTTPS: Connecting to ");
  print_serial(host);
  print_serial(path);
  print_serial("\n");

  uint32_t ip = dns_resolve(host);
  if (ip == 0) {
    print_serial("HTTPS: DNS failed for ");
    print_serial(host);
    print_serial("\n");
    return -2;
  }

  char ip_str[16];
  k_itoa(ip & 0xFF, ip_str);
  print_serial("HTTPS: Target IP ");
  print_serial(ip_str);
  print_serial(".");
  k_itoa((ip >> 8) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial(".");
  k_itoa((ip >> 16) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial(".");
  k_itoa((ip >> 24) & 0xFF, ip_str);
  print_serial(ip_str);
  print_serial("\n");

  tls_conn_t *conn = kmalloc(sizeof(tls_conn_t));
  if (!conn) {
    print_serial("HTTPS: Out of memory for connection\n");
    return -1;
  }
  int conn_res = tls_connect(conn, ip, 443, host);
  if (conn_res != 0) {
    print_serial("HTTPS: TLS connect failed\n");
    tls_close(conn);
    kfree(conn);
    return conn_res;
  }

  static char request[1024];
  int rlen = 0;

  strcpy(request, "GET ");
  strcat(request, path);
  strcat(request, " HTTP/1.1\r\n");
  strcat(request, "Host: ");
  strcat(request, host);
  strcat(request, "\r\n");
  strcat(request,
         "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
         "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36\r\n");
  strcat(request, "Accept: text/html, */*\r\n");
  strcat(request, "Connection: close\r\n");
  strcat(request, "\r\n");
  rlen = strlen(request);

  if (tls_send(conn, request, rlen) < 0) {
    print_serial("HTTPS: Send failed\n");
    tls_close(conn);
    kfree(conn);
    return -1;
  }

  print_serial("HTTPS: Request sent, waiting for response...\n");

  int total = 0;
  while (total < max_len - 1) {
    int n = tls_recv(conn, response + total, max_len - 1 - total);
    if (n < 0) {
      print_serial("HTTPS: Receive error\n");
      break;
    }
    if (n == 0)
      break;
    total += n;
  }
  response[total] = 0;

  tls_close(conn);
  kfree(conn);

  if (total == 0) {
    print_serial(
        "HTTPS: Received 0 bytes (Server closed connection immediately)\n");
    return 0;
  }

  char *body = 0;
  for (int i = 0; i < total - 1; i++) {
    if (i < total - 3 && response[i] == '\r' && response[i + 1] == '\n' &&
        response[i + 2] == '\r' && response[i + 3] == '\n') {
      body = response + i + 4;
      break;
    }
    if (response[i] == '\n' && response[i + 1] == '\n') {
      body = response + i + 2;
      break;
    }
  }

  if (!body) {
    print_serial("HTTPS: No delimiter found in response\n");
    return -4;
  }

  int status = 0;
  const char *sp = response;
  while (sp < body && *sp && *sp != ' ')
    sp++;
  if (sp < body && *sp == ' ') {
    sp++;
    if (sp[0] >= '0' && sp[0] <= '9' && sp[1] >= '0' && sp[1] <= '9' &&
        sp[2] >= '0' && sp[2] <= '9') {
      status = (sp[0] - '0') * 100 + (sp[1] - '0') * 10 + (sp[2] - '0');
    }
  }

  print_serial("HTTPS: Response Line: ");
  const char *lp = response;
  while (lp < body && *lp && *lp != '\r' && *lp != '\n') {
    char s[2] = {*lp++, 0};
    print_serial(s);
  }
  print_serial("\n");

  char stat_str[16];
  k_itoa(status, stat_str);
  print_serial("HTTPS: Status ");
  print_serial(stat_str);
  print_serial("\n");

  if (status == 301 || status == 302 || status == 303 || status == 307 ||
      status == 308) {
    const char *loc = response;
    while (loc < body) {
      if ((loc[0] == 'L' || loc[0] == 'l') &&
          strncmp(loc + 1, "ocation:", 8) == 0) {
        loc += 9;
        while (*loc == ' ')
          loc++;
        strcpy(response, "REDIRECT:");
        int ri = 9;
        while (*loc && *loc != '\r' && *loc != '\n' && ri < max_len - 1) {
          response[ri++] = *loc++;
        }
        response[ri] = 0;
        print_serial("HTTPS: Redirect to ");
        print_serial(response + 9);
        print_serial("\n");
        return -301;
      }
      loc++;
    }
  }

  if (status >= 400) {
    print_serial("HTTPS: Error status detected: ");
    print_serial(stat_str);
    print_serial("\n");
    int body_offset = body - response;
    int body_len = total - body_offset;
    if (body_len > 0) {
      memmove(response, body, body_len);
      response[body_len] = 0;
    } else {
      response[0] = 0;
      body_len = 0;
    }
    return -status;
  }

  int body_len = total - (body - response);
  if (body_len <= 0) {
    print_serial("HTTPS: Body 0 bytes\n");
    return 0;
  }

  // Check for chunked encoding
  int is_chunked = 0;
  const char *te = response;
  while (te < body) {
    if ((te[0] == 'T' || te[0] == 't') &&
        strncmp(te + 1, "ransfer-Encoding:", 17) == 0) {
      if (strstr(te, "chunked")) {
        is_chunked = 1;
        break;
      }
    }
    te++;
  }

  memmove(response, body, body_len);
  response[body_len] = 0;

  if (is_chunked) {
    print_serial("HTTPS: Decoding chunked body...\n");
    body_len = http_decode_chunked(response, body_len);
  }

  print_serial("HTTPS: Received body ");
  char len_str[16];
  k_itoa(body_len, len_str);
  print_serial(len_str);
  print_serial(" bytes\n");

  return body_len;
}

// Perform an HTTP POST request
// Returns: number of body bytes placed in response, or -1 on error
int http_post(const char *url, const char *post_data, int post_len,
              const char *content_type, char *response, int max_len) {
  static char host[128];
  static char path[256];

  if (!parse_url(url, host, sizeof(host), path, sizeof(path))) {
    print_serial("HTTP POST: Invalid URL\n");
    return -1;
  }

  // Determine if HTTPS
  int is_https = (strncmp(url, "https://", 8) == 0);

  print_serial("HTTP POST: ");
  print_serial(host);
  print_serial(path);
  print_serial("\n");

  uint32_t ip = dns_resolve(host);
  if (ip == 0) {
    print_serial("HTTP POST: DNS failed\n");
    return -2;
  }

  // Build POST request
  static char request[2048];
  strcpy(request, "POST ");
  strcat(request, path);
  strcat(request, " HTTP/1.1\r\n");
  strcat(request, "Host: ");
  strcat(request, host);
  strcat(request, "\r\n");
  strcat(request,
         "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
         "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36\r\n");
  strcat(request, "Content-Type: ");
  strcat(request,
         content_type ? content_type : "application/x-www-form-urlencoded");
  strcat(request, "\r\n");
  strcat(request, "Content-Length: ");
  char cl_str[16];
  k_itoa(post_len, cl_str);
  strcat(request, cl_str);
  strcat(request, "\r\n");
  strcat(request, "Connection: close\r\n");
  strcat(request, "\r\n");

  int hdr_len = strlen(request);

  if (is_https) {
    tls_conn_t *conn = kmalloc(sizeof(tls_conn_t));
    if (!conn)
      return -1;
    if (tls_connect(conn, ip, 443, host) != 0) {
      tls_close(conn);
      kfree(conn);
      return -3;
    }
    tls_send(conn, request, hdr_len);
    if (post_data && post_len > 0)
      tls_send(conn, post_data, post_len);

    int total = 0;
    while (total < max_len - 1) {
      int n = tls_recv(conn, response + total, max_len - 1 - total);
      if (n <= 0)
        break;
      total += n;
    }
    response[total] = 0;
    tls_close(conn);
    kfree(conn);
  } else {
    tcp_conn_t *conn = kmalloc(sizeof(tcp_conn_t));
    if (!conn)
      return -1;
    if (tcp_connect(conn, ip, 80) != 0) {
      tcp_close(conn);
      kfree(conn);
      return -3;
    }
    tcp_send(conn, request, hdr_len);
    if (post_data && post_len > 0)
      tcp_send(conn, post_data, post_len);

    int total = 0;
    while (total < max_len - 1) {
      int n = tcp_recv(conn, response + total, max_len - 1 - total);
      if (n <= 0)
        break;
      total += n;
    }
    response[total] = 0;
    tcp_close(conn);
    kfree(conn);
  }

  // Find body after headers
  char *body = strstr(response, "\r\n\r\n");
  if (body)
    body += 4;
  else {
    body = strstr(response, "\n\n");
    if (body)
      body += 2;
  }

  if (!body)
    return -4;

  int body_len = strlen(body);
  memmove(response, body, body_len + 1);

  print_serial("HTTP POST: Response body ");
  char lb[16];
  k_itoa(body_len, lb);
  print_serial(lb);
  print_serial(" bytes\n");

  return body_len;
}
