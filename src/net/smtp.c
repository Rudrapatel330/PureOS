#include "smtp.h"
#include "../kernel/base64.h"
#include "../kernel/string.h"
#include "net.h"

extern void print_serial(const char *str);
extern uint32_t get_timer_ticks(void);
extern void kernel_poll_events(void);

// Helper to drain TLS until expected string is found or timeout occurs
static int smtp_wait_for(tls_conn_t *conn, const char *expected,
                         uint32_t timeout_ms) {
  uint32_t start = get_timer_ticks();
  char buf[512];
  int total_read = 0;

  print_serial("SMTP: Waiting for '");
  print_serial(expected);
  print_serial("'\n");

  while (get_timer_ticks() - start < timeout_ms) {
    if (total_read >= sizeof(buf) - 1) {
      buf[sizeof(buf) - 1] = '\0';
      print_serial("SMTP: Buffer full: ");
      print_serial(buf);
      print_serial("\n");
      total_read = 0; // Reset buffer
    }

    int n = tls_recv(conn, buf + total_read, sizeof(buf) - 1 - total_read);
    if (n > 0) {
      total_read += n;
      buf[total_read] = '\0';

      // Check for expected code
      if (strstr(buf, expected)) {
        print_serial("SMTP: Received: ");
        print_serial(buf);
        return 0; // Success
      }

      // Check for common error codes (4xx, 5xx)
      if (total_read >= 3 && (buf[0] == '4' || buf[0] == '5')) {
        print_serial("SMTP: Error received: ");
        print_serial(buf);
        return -1;
      }
    } else if (n < 0) {
      print_serial("SMTP: TLS receive failed\n");
      return -1;
    }
  }

  print_serial("SMTP: Timeout waiting for '");
  print_serial(expected);
  print_serial("'\n");
  if (total_read > 0) {
    print_serial("SMTP: Last buffer contents: ");
    print_serial(buf);
  }
  return -1;
}

static int smtp_send_str(tls_conn_t *conn, const char *str) {
  print_serial("SMTP: Sending ");
  // Don't print passwords
  if (strncmp(str, "AUTH LOGIN", 10) != 0 && strlen(str) > 10) {
    char first_10[11];
    strncpy(first_10, str, 10);
    first_10[10] = '\0';
    print_serial(first_10);
    print_serial("...\n");
  } else {
    print_serial(str);
  }

  int len = strlen(str);
  int sent = tls_send(conn, str, len);
  if (sent != len) {
    print_serial("SMTP: Send failed\n");
    return -1;
  }
  return 0;
}

int smtp_send_email(const char *server, uint16_t port, const char *user,
                    const char *pass, const char *to, const char *subject,
                    const char *body) {
  print_serial("SMTP: Resolving ");
  print_serial(server);
  print_serial("...\n");

  uint32_t ip = dns_resolve(server);
  if (!ip) {
    print_serial("SMTP: DNS resolution failed\n");
    return -1;
  }

  tls_conn_t conn;
  print_serial("SMTP: Connecting to server...\n");
  if (tls_connect(&conn, ip, port, server) < 0) {
    print_serial("SMTP: Connection failed\n");
    return -2;
  }

  // 1. Wait for 220 Greeting
  if (smtp_wait_for(&conn, "220", 5000) < 0)
    goto err;

  // 2. Send EHLO
  if (smtp_send_str(&conn, "EHLO pureos\r\n") < 0)
    goto err;
  if (smtp_wait_for(&conn, "250", 5000) < 0)
    goto err;

  // 3. Authenticate
  if (smtp_send_str(&conn, "AUTH LOGIN\r\n") < 0)
    goto err;
  if (smtp_wait_for(&conn, "334", 5000) < 0)
    goto err;

  char b64_user[128];
  base64_encode((const unsigned char *)user, strlen(user), b64_user);
  strcat(b64_user, "\r\n");
  if (smtp_send_str(&conn, b64_user) < 0)
    goto err;
  if (smtp_wait_for(&conn, "334", 5000) < 0)
    goto err;

  char b64_pass[128];
  base64_encode((const unsigned char *)pass, strlen(pass), b64_pass);
  strcat(b64_pass, "\r\n");
  if (smtp_send_str(&conn, b64_pass) < 0)
    goto err;
  if (smtp_wait_for(&conn, "235", 10000) < 0)
    goto err; // 235 Authentication successful

  // 4. Envelope Sender
  char buf[256];
  strcpy(buf, "MAIL FROM:<");
  strcat(buf, user);
  strcat(buf, ">\r\n");
  if (smtp_send_str(&conn, buf) < 0)
    goto err;
  if (smtp_wait_for(&conn, "250", 5000) < 0)
    goto err;

  // 5. Envelope Recipient
  strcpy(buf, "RCPT TO:<");
  strcat(buf, to);
  strcat(buf, ">\r\n");
  if (smtp_send_str(&conn, buf) < 0)
    goto err;
  if (smtp_wait_for(&conn, "250", 5000) < 0)
    goto err;

  // 6. Begin DATA
  if (smtp_send_str(&conn, "DATA\r\n") < 0)
    goto err;
  if (smtp_wait_for(&conn, "354", 5000) < 0)
    goto err;

  // 7. Send RFC 2822 Headers
  strcpy(buf, "From: PureOS <");
  strcat(buf, user);
  strcat(buf, ">\r\n");
  if (smtp_send_str(&conn, buf) < 0)
    goto err;

  strcpy(buf, "To: <");
  strcat(buf, to);
  strcat(buf, ">\r\n");
  if (smtp_send_str(&conn, buf) < 0)
    goto err;

  strcpy(buf, "Subject: ");
  strcat(buf, subject);
  strcat(buf, "\r\n");
  if (smtp_send_str(&conn, buf) < 0)
    goto err;

  // End headers with double CRLF
  if (smtp_send_str(&conn, "\r\n") < 0)
    goto err;

  // 8. Send Body
  if (smtp_send_str(&conn, body) < 0)
    goto err;

  // 9. End DATA with CRLF . CRLF
  if (smtp_send_str(&conn, "\r\n.\r\n") < 0)
    goto err;
  if (smtp_wait_for(&conn, "250", 10000) < 0)
    goto err;

  // 10. Quit
  smtp_send_str(&conn, "QUIT\r\n");

  print_serial("SMTP: Email sent successfully!\n");
  tls_close(&conn);
  return 0;

err:
  print_serial("SMTP: Protocol failed\n");
  tls_close(&conn);
  return -3;
}
