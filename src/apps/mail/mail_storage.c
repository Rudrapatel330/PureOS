#include "../../fs/fs.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"
#include "mail.h"

extern void print_serial(const char *);

int mail_storage_init(const char *account_name) {
  // Ensure root /mail exists
  fs_mkdir("/mail");

  // Ensure account directory exists
  char path[256];
  strcpy(path, "/mail/");
  strcat(path, account_name);
  fs_mkdir(path);

  // Ensure inbox and sent exist
  char inbox_path[256];
  strcpy(inbox_path, path);
  strcat(inbox_path, "/inbox");
  fs_mkdir(inbox_path);

  char sent_path[256];
  strcpy(sent_path, path);
  strcat(sent_path, "/sent");
  fs_mkdir(sent_path);

  return 0;
}

int mail_save_message(const char *account_name, const char *folder,
                      mail_message_t *msg) {
  char path[256];
  strcpy(path, "/mail/");
  strcat(path, account_name);
  strcat(path, "/");
  strcat(path, folder);
  strcat(path, "/");
  strcat(path, msg->header.uid);
  strcat(path, ".msg");

  // Format:
  // From: <from>\n
  // Subject: <subject>\n
  // <body>

  int header_len = strlen(msg->header.from) + strlen(msg->header.subject) + 32;
  char *buf = (char *)kmalloc(msg->body_len + header_len);
  if (!buf)
    return -1;

  strcpy(buf, "From: ");
  strcat(buf, msg->header.from);
  strcat(buf, "\nSubject: ");
  strcat(buf, msg->header.subject);
  strcat(buf, "\n");

  int head_size = strlen(buf);
  memcpy(buf + head_size, msg->body, msg->body_len);

  if (fs_write(path, (const uint8_t *)buf, head_size + msg->body_len)) {
    print_serial("MAIL: Saved message ");
    print_serial(msg->header.uid);
    print_serial("\n");
    kfree(buf);
    return 0;
  }

  kfree(buf);
  return -1;
}

int mail_load_message(const char *account_name, const char *uid,
                      mail_message_t *msg_out) {
  char path[256];
  strcpy(path, "/mail/");
  strcat(path, account_name);
  strcat(path, "/inbox/");
  strcat(path, uid);
  strcat(path, ".msg");

  file_entry_t *entry = fs_find(path);
  if (!entry)
    return -1;

  uint8_t *fbuf = (uint8_t *)kmalloc(entry->size + 1);
  if (!fbuf)
    return -1;

  if (fs_read(path, fbuf)) {
    fbuf[entry->size] = 0;

    // Parse headers (simplified)
    char *ptr = (char *)fbuf;
    if (strncmp(ptr, "From: ", 6) == 0) {
      char *nl = strchr(ptr, '\n');
      if (nl) {
        int len = nl - (ptr + 6);
        if (len >= MAX_EMAIL_ADDR)
          len = MAX_EMAIL_ADDR - 1;
        strncpy(msg_out->header.from, ptr + 6, len);
        msg_out->header.from[len] = 0;
        ptr = nl + 1;
      }
    }

    if (strncmp(ptr, "Subject: ", 9) == 0) {
      char *nl = strchr(ptr, '\n');
      if (nl) {
        int len = nl - (ptr + 9);
        if (len >= MAX_SUBJECT)
          len = MAX_SUBJECT - 1;
        strncpy(msg_out->header.subject, ptr + 9, len);
        msg_out->header.subject[len] = 0;
        ptr = nl + 1;
      }
    }

    // The rest is the body
    int body_len = entry->size - (ptr - (char *)fbuf);
    msg_out->body = (char *)kmalloc(body_len + 1);
    if (msg_out->body) {
      memcpy(msg_out->body, ptr, body_len);
      msg_out->body[body_len] = 0;
      msg_out->body_len = body_len;
      strcpy(msg_out->header.uid, uid);
      kfree(fbuf);
      return 0;
    }
  }

  kfree(fbuf);
  return -1;
}

int mail_list_messages(const char *account_name, mail_header_t *headers_out,
                       int max_count) {
  char path[256];
  strcpy(path, "/mail/");
  strcat(path, account_name);
  strcat(path, "/inbox");

  FileInfo files[32];
  int count = fs_list_files(path, files, 32);
  if (count < 0)
    return 0;

  int processed = 0;
  for (int i = 0; i < count && processed < max_count; i++) {
    if (files[i].is_dir)
      continue;

    // Open file to read headers
    char fpath[256];
    strcpy(fpath, path);
    strcat(fpath, "/");
    strcat(fpath, files[i].name);

    uint8_t head_buf[256];
    memset(head_buf, 0, 256);
    fs_read(fpath, head_buf); // Just read first 256 bytes

    // Default values
    strcpy(headers_out[processed].from, "Unknown");
    strcpy(headers_out[processed].subject, "(No Subject)");

    char *ptr = (char *)head_buf;
    if (strncmp(ptr, "From: ", 6) == 0) {
      char *nl = strchr(ptr, '\n');
      if (nl) {
        int len = nl - (ptr + 6);
        if (len >= MAX_EMAIL_ADDR)
          len = MAX_EMAIL_ADDR - 1;
        strncpy(headers_out[processed].from, ptr + 6, len);
        headers_out[processed].from[len] = 0;
        ptr = nl + 1;
      }
    }

    if (strncmp(ptr, "Subject: ", 9) == 0) {
      char *nl = strchr(ptr, '\n');
      if (nl) {
        int len = nl - (ptr + 9);
        if (len >= MAX_SUBJECT)
          len = MAX_SUBJECT - 1;
        strncpy(headers_out[processed].subject, ptr + 9, len);
        headers_out[processed].subject[len] = 0;
      }
    }

    // Strip .msg extension for UID
    char name[128];
    strcpy(name, files[i].name);
    char *dot = strchr(name, '.');
    if (dot)
      *dot = 0;

    strcpy(headers_out[processed].uid, name);
    headers_out[processed].size = files[i].size;
    processed++;
  }

  return processed;
}
