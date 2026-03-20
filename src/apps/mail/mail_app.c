#include "mail_app.h"
#include "../../fs/fs.h"
#include "../../kernel/heap.h"
#include "../../kernel/mail_core.h"
#include "../../kernel/string.h"
#include "../../kernel/window.h"
#include "../../net/smtp.h"
#include "mail.h"
extern void print_serial(const char *str);
static void debug_print_int(int val) {
  char buf[12];
  k_itoa(val, buf);
  print_serial(buf);
}

#define MAIL_SIDEBAR_WIDTH 120
#define MAIL_LIST_WIDTH 200
#define MAIL_ROW_HEIGHT 24

static window_t *mail_win = 0;
static char current_account[128] = "";
static mail_header_t msg_headers[32];
static int msg_count = 0;
static int selected_msg_idx = -1;
static mail_message_t current_msg;
static int has_msg_loaded = 0;

// Accounts list
static char accounts[16][128];
static int account_count = 0;
static int selected_account_idx = -1;

// Status
static char status_text[64] = "Ready";
static int compose_mode = 0;
static int compose_field = 0; // 0=To, 1=Subject, 2=Body
static char compose_to[MAX_EMAIL_ADDR] = "";
static char compose_subject[MAX_SUBJECT] = "";
static char compose_body[2048] = "";

void mail_app_refresh_accounts() {
  account_count = 0;
  FileInfo files[16];
  int count = fs_list_files("/mail", files, 16);
  if (count < 0)
    return;

  for (int i = 0; i < count && account_count < 16; i++) {
    if (files[i].is_dir) {
      // Filter out system directories
      if (strcmp(files[i].name, ".") == 0 || strcmp(files[i].name, "..") == 0)
        continue;
      strcpy(accounts[account_count++], files[i].name);
    }
  }
}

void mail_app_refresh_messages() {
  msg_count = 0;
  selected_msg_idx = -1;
  has_msg_loaded = 0;
  if (strlen(current_account) == 0)
    return;

  msg_count = mail_list_messages(current_account, msg_headers, 32);
}

void mail_app_load_message(int idx) {
  if (idx < 0 || idx >= msg_count)
    return;

  if (has_msg_loaded) {
    mail_free_message(&current_msg);
    has_msg_loaded = 0;
  }

  if (mail_load_message(current_account, msg_headers[idx].uid, &current_msg) ==
      0) {
    selected_msg_idx = idx;
    has_msg_loaded = 1;
  }
}

int mail_app_sync() {
  if (strlen(current_account) == 0)
    return -1;

  strcpy(status_text, "Syncing...");
  if (mail_win)
    mail_win->needs_redraw = 1;

  // For Phase 3/4 demo, we'll try to load a config file if it exists
  char config_path[256];
  strcpy(config_path, "/mail/");
  strcat(config_path, current_account);
  strcat(config_path, "/.config");

  mail_account_t acc;
  strcpy(acc.hostname, "pop.example.com");
  acc.port = 110;
  strcpy(acc.username, current_account);
  strcpy(acc.password, "password");
  acc.use_tls = 0;
  acc.type = MAIL_TYPE_POP3;

  int sync_success = 0;
  mail_session_t *s = (mail_session_t *)kmalloc(sizeof(mail_session_t));
  if (s) {
    if (mail_open_session(s, &acc) == 0) {
      if (pop3_login(s) == 0) {
        int count, size;
        if (pop3_stat(s, &count, &size) == 0) {
          for (int i = 1; i <= count && i <= 5; i++) {
            mail_message_t *msg =
                (mail_message_t *)kmalloc(sizeof(mail_message_t));
            if (pop3_retrieve_message(s, i, msg) == 0) {
              mail_save_message(current_account, "inbox", msg);
              sync_success = 1;
            }
            mail_free_message(msg);
            kfree(msg);
          }
        }
        pop3_quit(s);
      } else {
        strcpy(status_text, "Login Failed");
        mail_close_session(s);
      }
    } else {
      strcpy(status_text, "DNS/Conn Fail");
    }
    kfree(s);
  }

  // Demo Mode: If sync fails on "default" account, provide mock content
  if (!sync_success && strcasestr(current_account, "default") != NULL) {
    strcpy(status_text, "Demo Mode: Mock Sync");
    mail_message_t mock;
    strcpy(mock.header.from, "pureos-team@pureos.org");
    strcpy(mock.header.subject, "Welcome to PureOS Mail!");
    strcpy(mock.header.uid, "welcome");
    mock.body = "Hello! This is a mock email for demonstration because the "
                "sync failed.\n\nPureOS Mail is now ready for use.";
    mock.body_len = strlen(mock.body);
    mail_save_message("default", "inbox", &mock);

    strcpy(mock.header.from, "security@pureos.org");
    strcpy(mock.header.subject, "Security Update Available");
    strcpy(mock.header.uid, "security");
    mock.body = "A new security update is available for your system. Please "
                "run 'update' from the shell.";
    mock.body_len = strlen(mock.body);
    mail_save_message("default", "inbox", &mock);
    sync_success = 1;
  }

  if (sync_success && strcmp(status_text, "Syncing...") == 0) {
    strcpy(status_text, "Sync complete");
  }

  mail_app_refresh_messages();
  if (mail_win)
    mail_win->needs_redraw = 1;
  return 0;
}

void mail_app_draw(window_t *win) {
  if (!win)
    return;

  // Background
  winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, 0xFFFFFFFF);

  // 1. TOOLBAR
  winmgr_fill_rect(win, 0, 24, win->width, 30, 0xFFF0F0F0);
  winmgr_fill_rect(win, 0, 53, win->width, 1, 0xFFD0D0D0);

  // Buttons
  winmgr_fill_rect(win, 10, 28, 60, 20, 0xFFE0E0E0);
  winmgr_draw_text(win, 20, 32, "Sync", 0xFF000000);

  winmgr_fill_rect(win, 80, 28, 80, 20, compose_mode ? 0xFF0078D7 : 0xFFE0E0E0);
  winmgr_draw_text(win, 90, 32, "Compose",
                   compose_mode ? 0xFFFFFFFF : 0xFF000000);

  winmgr_draw_text(win, win->width - 150, 32, status_text, 0xFF555555);

  // 2. SIDEBAR (Accounts)
  winmgr_fill_rect(win, 0, 54, MAIL_SIDEBAR_WIDTH, win->height - 54 - 26,
                   0xFFF8F8F8);
  winmgr_fill_rect(win, MAIL_SIDEBAR_WIDTH, 54, 1, win->height - 54 - 26,
                   0xFFD0D0D0);

  winmgr_draw_text(win, 5, 60, "ACCOUNTS", 0xFF888888);
  for (int i = 0; i < account_count; i++) {
    int y = 80 + i * MAIL_ROW_HEIGHT;
    if (i == selected_account_idx) {
      winmgr_fill_rect(win, 2, y, MAIL_SIDEBAR_WIDTH - 4, MAIL_ROW_HEIGHT,
                       0xFF0078D7);
      winmgr_draw_text(win, 5, y + 5, accounts[i], 0xFFFFFFFF);
    } else {
      winmgr_draw_text(win, 5, y + 5, accounts[i], 0xFF000000);
    }
  }

  // 3. MESSAGE LIST (OR COMPOSE FORM)
  int list_x = MAIL_SIDEBAR_WIDTH + 1;
  if (compose_mode) {
    winmgr_fill_rect(win, list_x, 54, win->width - list_x,
                     win->height - 54 - 26, 0xFFFFFFFF);

    // "To:" field
    uint32_t to_border = (compose_field == 0) ? 0xFF0078D7 : 0xFFCCCCCC;
    winmgr_draw_text(win, list_x + 10, 82, "To:", 0xFF888888);
    winmgr_fill_rect(win, list_x + 70, 73, win->width - list_x - 90, 26,
                     0xFFF5F5F5);
    winmgr_fill_rect(win, list_x + 70, 99, win->width - list_x - 90, 1,
                     to_border);
    winmgr_draw_text(win, list_x + 75, 82, compose_to, 0xFF000000);

    // "Subject:" field
    uint32_t subj_border = (compose_field == 1) ? 0xFF0078D7 : 0xFFCCCCCC;
    winmgr_draw_text(win, list_x + 10, 118, "Subject:", 0xFF888888);
    winmgr_fill_rect(win, list_x + 70, 108, win->width - list_x - 90, 26,
                     0xFFF5F5F5);
    winmgr_fill_rect(win, list_x + 70, 134, win->width - list_x - 90, 1,
                     subj_border);
    winmgr_draw_text(win, list_x + 75, 118, compose_subject, 0xFF000000);

    // Divider
    winmgr_fill_rect(win, list_x, 140, win->width - list_x, 1, 0xFFE0E0E0);

    // Body field
    uint32_t body_border = (compose_field == 2) ? 0xFF0078D7 : 0xFFCCCCCC;
    winmgr_fill_rect(win, list_x + 10, 148, win->width - list_x - 20,
                     win->height - 220, 0xFFF8F8F8);
    winmgr_fill_rect(win, list_x + 10, win->height - 72,
                     win->width - list_x - 20, 1, body_border);
    winmgr_draw_text(win, list_x + 15, 155, compose_body, 0xFF000000);

    // Field hint
    winmgr_draw_text(win, list_x + 10, win->height - 60,
                     "Tab=Next Field  Backspace=Delete", 0xFF999999);

    // Send Button
    winmgr_fill_rect(win, list_x + 10, win->height - 48, 70, 26, 0xFF0078D7);
    winmgr_draw_text(win, list_x + 22, win->height - 41, "SEND", 0xFFFFFFFF);
    return;
  }

  winmgr_fill_rect(win, list_x, 54, MAIL_LIST_WIDTH, win->height - 54 - 26,
                   0xFFFFFFFF);
  winmgr_fill_rect(win, list_x + MAIL_LIST_WIDTH, 54, 1, win->height - 54 - 26,
                   0xFFD0D0D0);

  for (int i = 0; i < msg_count; i++) {
    int y = 54 + i * (MAIL_ROW_HEIGHT + 14);
    if (i == selected_msg_idx) {
      winmgr_fill_rect(win, list_x + 2, y + 2, MAIL_LIST_WIDTH - 4,
                       MAIL_ROW_HEIGHT + 10, 0xFFCCE8FF);
    }

    char sender[26];
    strncpy(sender, msg_headers[i].from, 22);
    sender[22] = 0; // Force null termination
    winmgr_draw_text(win, list_x + 8, y + 6, sender, 0xFF000000);

    char subj[26];
    strncpy(subj, msg_headers[i].subject, 22);
    subj[22] = 0; // Force null termination
    winmgr_draw_text(win, list_x + 8, y + 20, subj, 0xFF555555);

    winmgr_fill_rect(win, list_x, y + MAIL_ROW_HEIGHT + 12, MAIL_LIST_WIDTH, 1,
                     0xFFEEEEEE);
  }

  // 4. MESSAGE VIEW
  int view_x = list_x + MAIL_LIST_WIDTH + 1;
  if (has_msg_loaded) {
    winmgr_draw_text(win, view_x + 10, 60, "From: ", 0xFF888888);
    winmgr_draw_text(win, view_x + 60, 60, current_msg.header.from, 0xFF000000);

    winmgr_draw_text(win, view_x + 10, 80, "Subject: ", 0xFF888888);
    winmgr_draw_text(win, view_x + 80, 80, current_msg.header.subject,
                     0xFF000000);

    winmgr_fill_rect(win, view_x + 10, 100, win->width - view_x - 20, 1,
                     0xFFD0D0D0);

    // Body (Simple)
    winmgr_draw_text(win, view_x + 10, 110, current_msg.body, 0xFF000000);
  } else {
    winmgr_draw_text(win, view_x + 50, win->height / 2,
                     "Select a message to read.", 0xFF888888);
  }

  // 5. STATUS BAR
  int sb_y = win->height - 26;
  winmgr_fill_rect(win, 0, sb_y, win->width, 26, 0xFFF0F0F0);
  winmgr_fill_rect(win, 0, sb_y, win->width, 1, 0xFFD0D0D0);
  winmgr_draw_text(win, 10, sb_y + 5, status_text, 0xFF333333);
}

void mail_app_on_mouse(window_t *win, int mx, int my, int buttons) {
  if (!(buttons & 1))
    return;

  print_serial("MAIL: Mouse down at ");
  debug_print_int(mx);
  print_serial(",");
  debug_print_int(my);
  print_serial("\n");

  // Toolbar Sync
  if (mx >= 10 && mx <= 70 && my >= 28 && my <= 48) {
    print_serial("MAIL: Sync button clicked\n");
    mail_app_sync();
    return;
  }

  // Toolbar Compose
  if (mx >= 80 && mx <= 160 && my >= 28 && my <= 48) {
    print_serial("MAIL: Compose button clicked\n");
    compose_mode = !compose_mode;
    win->needs_redraw = 1;
    return;
  }

  // Compose View Actions
  if (compose_mode) {
    int list_x = MAIL_SIDEBAR_WIDTH + 1;

    // Field Selection (To, Subject, Body)
    if (mx >= list_x + 70 && mx <= win->width - 20) {
      if (my >= 73 && my <= 99) {
        compose_field = 0; // To
        win->needs_redraw = 1;
      } else if (my >= 108 && my <= 134) {
        compose_field = 1; // Subject
        win->needs_redraw = 1;
      }
    }
    if (mx >= list_x + 10 && mx <= win->width - 10 && my >= 148 &&
        my <= win->height - 60) {
      compose_field = 2; // Body
      win->needs_redraw = 1;
    }

    // Send Button
    if (mx >= list_x + 10 && mx <= list_x + 80 && my >= win->height - 48 &&
        my <= win->height - 22) {
      print_serial("MAIL: Send clicked\n");
      mail_message_t msg;
      strcpy(msg.header.from, current_account);
      strcpy(msg.header.subject, compose_subject);
      strcpy(msg.header.uid, "sent_"); // Simple UID
      // Append ticks to UID
      extern unsigned int get_timer_ticks();
      char tbuf[16];
      k_itoa(get_timer_ticks(), tbuf);
      strcat(msg.header.uid, tbuf);

      msg.body = compose_body;
      msg.body_len = strlen(compose_body);

      if (mail_save_message(current_account, "sent", &msg) == 0) {
        strcpy(status_text, "Sending via SMTP...");
        win->needs_redraw = 1;
        // Force synchronous redraw for standard GUI apps since we block
        // during SMTP (ideally would be threaded in a real microkernel)
        extern void compositor_render(void);
        compositor_render();

        print_serial("MAIL: Beginning SMTP Transmission...\n");
        // User MUST change this string to a real 16 character App Password!
        const char *user = "rudraptl2611@gmail.com";
        const char *pass = "jhik kzcx uroy diwz";

        int res = smtp_send_email("smtp.gmail.com", 465, user, pass, compose_to,
                                  compose_subject, compose_body);

        if (res == 0) {
          strcpy(status_text, "Email Sent Successfully!");
          compose_mode = 0;
          // Optionally switch to sent folder? Not yet implemented in listing.
        } else {
          strcpy(status_text, "SMTP Send Failed :-(");
        }
      } else {
        strcpy(status_text, "Error saving sent");
      }
      win->needs_redraw = 1;
    }
    return;
  }

  // Sidebar Accounts
  if (mx < MAIL_SIDEBAR_WIDTH && my > 80 && my < win->height - 26) {
    int idx = (my - 80) / MAIL_ROW_HEIGHT;
    if (idx >= 0 && idx < account_count) {
      print_serial("MAIL: Account selected: ");
      print_serial(accounts[idx]);
      print_serial("\n");
      selected_account_idx = idx;
      strcpy(current_account, accounts[idx]);
      mail_app_refresh_messages();
      win->needs_redraw = 1;
    }
    return;
  }

  // Message List
  int list_x = MAIL_SIDEBAR_WIDTH + 1;
  if (mx >= list_x && mx <= list_x + MAIL_LIST_WIDTH && my > 54 &&
      my < win->height - 26) {
    int idx = (my - 54) / (MAIL_ROW_HEIGHT + 14);
    if (idx >= 0 && idx < msg_count) {
      print_serial("MAIL: Message selected index ");
      debug_print_int(idx);
      print_serial("\n");
      mail_app_load_message(idx);
      win->needs_redraw = 1;
    }
    return;
  }
}

void mail_app_on_key(window_t *win, int key, char ascii) {
  if (!compose_mode)
    return;

  // Tab cycles focus between fields: 0=To, 1=Subject, 2=Body
  if (key == 15) { // TAB scancode is 15
    compose_field = (compose_field + 1) % 3;
    win->needs_redraw = 1;
    return;
  }

  // Backspace (key 14)
  if (key == 14 || ascii == '\b') {
    char *field = (compose_field == 0)   ? compose_to
                  : (compose_field == 1) ? compose_subject
                                         : compose_body;
    int len = strlen(field);
    if (len > 0) {
      field[len - 1] = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  // Printable character
  if (ascii >= 32 && ascii < 127) {
    if (compose_field == 0) {
      int len = strlen(compose_to);
      if (len < MAX_EMAIL_ADDR - 1) {
        compose_to[len] = ascii;
        compose_to[len + 1] = 0;
      }
    } else if (compose_field == 1) {
      int len = strlen(compose_subject);
      if (len < MAX_SUBJECT - 1) {
        compose_subject[len] = ascii;
        compose_subject[len + 1] = 0;
      }
    } else {
      int len = strlen(compose_body);
      if (len < 2047) {
        compose_body[len] = ascii;
        compose_body[len + 1] = 0;
      }
    }
    win->needs_redraw = 1;
  }
}

void mail_app_init() {
  print_serial("MAIL: Initializing app...\n");
  mail_win = winmgr_create_window(100, 100, 700, 500, "PureOS Mail");
  if (!mail_win)
    return;

  mail_win->draw = (void (*)(void *))mail_app_draw;
  mail_win->on_mouse = (void (*)(void *, int, int, int))mail_app_on_mouse;
  mail_win->on_key = (void (*)(void *, int, char))mail_app_on_key;
  mail_win->app_type = 13;
  mail_win->owner_pid = 1; // FORCE KERNEL OWNER to ensure callbacks are used

  strcpy(compose_to, "recipient@example.com");
  strcpy(compose_subject, "Draft from PureOS");
  strcpy(compose_body, "This is a test message composed on PureOS.");

  mail_storage_init("default"); // Ensure at least one account exists
  mail_app_refresh_accounts();
  if (account_count > 0) {
    selected_account_idx = 0;
    strcpy(current_account, accounts[0]);
    mail_app_refresh_messages();
  }
  print_serial("MAIL: Init complete. Accounts: ");
  debug_print_int(account_count);
  print_serial("\n");
}
