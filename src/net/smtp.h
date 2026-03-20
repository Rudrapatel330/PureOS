#ifndef SMTP_H
#define SMTP_H

#include <stdint.h>

// Sends an email via SMTP over TLS to the specified provider.
// server: SMTP server hostname (e.g., "smtp.gmail.com")
// port: SMTPS port (e.g., 465)
// user: Email address for AUTH LOGIN
// pass: App password for AUTH LOGIN
// to: Destination email address
// subject: Email subject line
// body: Email body text
//
// Returns 0 on success, negative value on failure.
int smtp_send_email(const char *server, uint16_t port, const char *user,
                    const char *pass, const char *to, const char *subject,
                    const char *body);

#endif
