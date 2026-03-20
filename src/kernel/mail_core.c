#include "mail_core.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"

extern void print_serial(const char *);

static int send_command(mail_session_t *s, const char *cmd) {
  if (!s->connected)
    return -1;

  print_serial("MAIL: Sending: ");
  print_serial(cmd);
  print_serial("\n");

  int len = strlen(cmd);
  if (s->account.use_tls) {
    return tls_send(&s->tls, cmd, len);
  } else {
    return tcp_send(&s->tcp, cmd, len);
  }
}

static int receive_response(mail_session_t *s) {
  memset(s->last_response, 0, sizeof(s->last_response));
  int received = 0;

  if (s->account.use_tls) {
    received =
        tls_recv(&s->tls, s->last_response, sizeof(s->last_response) - 1);
  } else {
    received =
        tcp_recv(&s->tcp, s->last_response, sizeof(s->last_response) - 1);
  }

  if (received > 0) {
    print_serial("MAIL: Received: ");
    print_serial(s->last_response);
    print_serial("\n");
  }
  return received;
}

int mail_open_session(mail_session_t *session, mail_account_t *account) {
  memset(session, 0, sizeof(mail_session_t));
  memcpy(&session->account, account, sizeof(mail_account_t));

  uint32_t ip = dns_resolve(account->hostname);
  if (ip == 0) {
    print_serial("MAIL: DNS fail\n");
    return -1;
  }

  if (account->use_tls) {
    if (tls_connect(&session->tls, ip, account->port, account->hostname) != 0) {
      print_serial("MAIL: TLS connect fail\n");
      return -1;
    }
  } else {
    if (tcp_connect(&session->tcp, ip, account->port) != 0) {
      print_serial("MAIL: TCP connect fail\n");
      return -1;
    }
  }

  session->connected = 1;

  // Initial greeting
  receive_response(session);

  return 0;
}

void mail_close_session(mail_session_t *session) {
  if (!session->connected)
    return;

  if (session->account.use_tls) {
    tls_close(&session->tls);
  } else {
    tcp_close(&session->tcp);
  }
  session->connected = 0;
}

// POP3 Implementation
int pop3_login(mail_session_t *s) {
  char cmd[256];

  // USER
  strcpy(cmd, "USER ");
  strcat(cmd, s->account.username);
  strcat(cmd, "\r\n");
  send_command(s, cmd);
  if (receive_response(s) <= 0 || s->last_response[0] != '+')
    return -1;

  // PASS
  strcpy(cmd, "PASS ");
  strcat(cmd, s->account.password);
  strcat(cmd, "\r\n");
  send_command(s, cmd);
  if (receive_response(s) <= 0 || s->last_response[0] != '+')
    return -1;

  return 0;
}

int pop3_stat(mail_session_t *s, int *count, int *size) {
  send_command(s, "STAT\r\n");
  if (receive_response(s) <= 0 || s->last_response[0] != '+')
    return -1;

  // Parse "+OK 10 12345"
  char *p = s->last_response + 4; // Skip "+OK "
  *count = atoi(p);
  while (*p && *p != ' ')
    p++;
  if (*p == ' ')
    p++;
  *size = atoi(p);

  return 0;
}

int pop3_list(mail_session_t *s, int index, int *size) {
  char cmd[32];
  k_itoa(index, cmd);
  char final_cmd[64];
  strcpy(final_cmd, "LIST ");
  strcat(final_cmd, cmd);
  strcat(final_cmd, "\r\n");
  send_command(s, final_cmd);
  if (receive_response(s) <= 0 || s->last_response[0] != '+')
    return -1;
  // Parse "+OK 1 1234"
  char *p = s->last_response + 4;
  while (*p && *p != ' ')
    p++;
  if (*p == ' ')
    p++;
  *size = atoi(p);
  return 0;
}

int pop3_retr(mail_session_t *s, int index, char *buf, int max_len) {
  char cmd[32];
  k_itoa(index, cmd);

  char final_cmd[64];
  strcpy(final_cmd, "RETR ");
  strcat(final_cmd, cmd);
  strcat(final_cmd, "\r\n");

  send_command(s, final_cmd);
  if (receive_response(s) <= 0 || s->last_response[0] != '+')
    return -1;

  // Read multi-line response until "."
  int total_read = 0;
  while (total_read < max_len - 1) {
    int r = 0;
    if (s->account.use_tls) {
      r = tls_recv(&s->tls, buf + total_read, max_len - total_read - 1);
    } else {
      r = tcp_recv(&s->tcp, buf + total_read, max_len - total_read - 1);
    }

    if (r <= 0)
      break;
    total_read += r;
    buf[total_read] = 0;

    // Check for ending dot on its own line
    // Standard POP3 end is "\r\n.\r\n"
    if (strstr(buf, "\r\n.\r\n"))
      break;
  }

  return total_read;
}

int pop3_retrieve_message(mail_session_t *s, int index,
                          mail_message_t *msg_out) {
  int size = 0;
  if (pop3_list(s, index, &size) != 0)
    return -1;

  // Allocate buffer for message content
  char *buf = (char *)kmalloc(size + 1024);
  if (!buf)
    return -1;

  int read = pop3_retr(s, index, buf, size + 1024);
  if (read <= 0) {
    kfree(buf);
    return -1;
  }

  msg_out->body = buf;
  msg_out->body_len = read;

  // Simple header parsing
  memset(&msg_out->header, 0, sizeof(mail_header_t));

  char *from_ptr = strstr(buf, "From: ");
  if (from_ptr) {
    char *end = strstr(from_ptr, "\r\n");
    if (end) {
      int len = end - (from_ptr + 6);
      if (len > 127)
        len = 127;
      memcpy(msg_out->header.from, from_ptr + 6, len);
      msg_out->header.from[len] = 0;
    }
  }

  char *subj_ptr = strstr(buf, "Subject: ");
  if (subj_ptr) {
    char *end = strstr(subj_ptr, "\r\n");
    if (end) {
      int len = end - (subj_ptr + 9);
      if (len > 255)
        len = 255;
      memcpy(msg_out->header.subject, subj_ptr + 9, len);
      msg_out->header.subject[len] = 0;
    }
  }

  k_itoa(index, msg_out->header.uid);
  msg_out->header.size = read;

  return 0;
}

void mail_free_message(mail_message_t *msg) {
  if (msg && msg->body) {
    kfree(msg->body);
    msg->body = 0;
    msg->body_len = 0;
  }
}

int pop3_quit(mail_session_t *s) {
  send_command(s, "QUIT\r\n");
  receive_response(s);
  mail_close_session(s);
  return 0;
}

// SMTP Implementation (Stub)
int smtp_login(mail_session_t *s) {
  send_command(s, "EHLO pureos\r\n");
  if (receive_response(s) <= 0)
    return -1;
  return 0;
}

int smtp_send(mail_session_t *s, const char *to, const char *subject,
              const char *body) {
  char cmd[512];

  // MAIL FROM
  strcpy(cmd, "MAIL FROM:<");
  strcat(cmd, s->account.username);
  strcat(cmd, ">\r\n");
  send_command(s, cmd);
  if (receive_response(s) <= 0)
    return -1;

  // RCPT TO
  strcpy(cmd, "RCPT TO:<");
  strcat(cmd, to);
  strcat(cmd, ">\r\n");
  send_command(s, cmd);
  if (receive_response(s) <= 0)
    return -1;

  // DATA
  send_command(s, "DATA\r\n");
  if (receive_response(s) <= 0)
    return -1;

  // Send Subject and Body
  strcpy(cmd, "Subject: ");
  strcat(cmd, subject);
  strcat(cmd, "\r\n\r\n");
  send_command(s, cmd);
  send_command(s, body);
  send_command(s, "\r\n.\r\n");

  if (receive_response(s) <= 0)
    return -1;

  return 0;
}

int smtp_quit(mail_session_t *s) {
  send_command(s, "QUIT\r\n");
  receive_response(s);
  mail_close_session(s);
  return 0;
}
