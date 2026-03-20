#ifndef MAIL_TYPES_H
#define MAIL_TYPES_H

#include <stdint.h>

// Max sizes for email fields
#define MAX_EMAIL_ADDR 128
#define MAX_SUBJECT 256
#define MAX_DATE 64
#define MAX_UID 64

// Email Header Structure
typedef struct {
  char from[MAX_EMAIL_ADDR];
  char to[MAX_EMAIL_ADDR];
  char subject[MAX_SUBJECT];
  char date[MAX_DATE];
  char uid[MAX_UID];
  uint32_t size;
  int read;
} mail_header_t;

// Email Message Structure (Full)
typedef struct {
  mail_header_t header;
  char *body; // Dynamically allocated body
  uint32_t body_len;
} mail_message_t;

#endif
