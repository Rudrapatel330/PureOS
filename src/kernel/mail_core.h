#ifndef MAIL_CORE_H
#define MAIL_CORE_H

#include "../apps/mail/mail_types.h"
#include "../net/net.h"
#include <stdint.h>


#define MAIL_TYPE_POP3 1
#define MAIL_TYPE_SMTP 2

typedef struct {
  char hostname[128];
  uint16_t port;
  char username[128];
  char password[128];
  int use_tls;
  int type; // MAIL_TYPE_POP3, etc.
} mail_account_t;

typedef struct {
  mail_account_t account;
  tls_conn_t tls;
  tcp_conn_t tcp;
  int connected;
  char last_response[1024];
} mail_session_t;

// Session Management
int mail_open_session(mail_session_t *session, mail_account_t *account);
void mail_close_session(mail_session_t *session);

// POP3 Functions
int pop3_login(mail_session_t *session);
int pop3_stat(mail_session_t *session, int *count, int *size);
int pop3_list(mail_session_t *session, int index, int *size);
int pop3_retr(mail_session_t *session, int index, char *buf, int max_len);
int pop3_quit(mail_session_t *session);

// High Level API
int pop3_retrieve_message(mail_session_t *session, int index,
                          mail_message_t *msg_out);
void mail_free_message(mail_message_t *msg);

// SMTP Functions
int smtp_login(mail_session_t *session);
int smtp_send(mail_session_t *session, const char *to, const char *subject,
              const char *body);
int smtp_quit(mail_session_t *session);

#endif
