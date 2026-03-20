#ifndef MAIL_H
#define MAIL_H

#include "mail_types.h"
#include <stdint.h>

// Storage API
int mail_storage_init(const char *account_name);
int mail_save_message(const char *account_name, const char *folder,
                      mail_message_t *msg);
int mail_load_message(const char *account_name, const char *uid,
                      mail_message_t *msg_out);
int mail_list_messages(const char *account_name, mail_header_t *headers_out,
                       int max_count);

void mail_free_message(mail_message_t *msg);

#endif
