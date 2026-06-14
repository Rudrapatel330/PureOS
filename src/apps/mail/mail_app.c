#include "mail_app.h"
#include "../../drivers/rtc.h"
#include "../../fs/fs.h"
#include "../../kernel/heap.h"
#include "../../kernel/mail_core.h"
#include "../../kernel/string.h"
#include "../../kernel/theme.h"
#include "../../kernel/window.h"
#include "../../net/smtp.h"
#include "mail.h"

extern void print_serial(const char *str);
static void debug_print_int(int val) {
  char buf[12];
  k_itoa(val, buf);
  print_serial(buf);
}

#define MAIL_SIDEBAR_WIDTH 260
#define MAIL_LIST_WIDTH 340
#define MAIL_ROW_HEIGHT 32
#define MAIL_TOOLBAR_H 36
#define MAIL_SEARCH_H 30

// Folder selection: 0=Inbox, 1=Sent, 2=Trash
static int selected_folder = 0;
static int mail_mouse_x = 0, mail_mouse_y = 0;

// Color helpers
#define MAIL_C_DARK_BG 0xFF0D0F18
#define MAIL_C_SIDEBAR_BG 0xFF111322
#define MAIL_C_CARD_BG 0xFF1A1D2E
#define MAIL_C_CARD_HOVER 0xFF232640
#define MAIL_C_ACCENT 0xFF4A6CF7
#define MAIL_C_BORDER 0xFF2A2D42
#define MAIL_C_INPUT_BG 0xFF161929
#define MAIL_C_TEXT 0xFFE0E0E0
#define MAIL_C_TEXT_DIM 0xFF8890A8
#define MAIL_C_TEXT_VDIM 0xFF5A6080

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
static char status_text[64] = "READY";
static int compose_mode = 0;
static int compose_field = 0; // 0=To, 1=Subject, 2=Body
static char compose_to[MAX_EMAIL_ADDR] = "";
static char compose_subject[MAX_SUBJECT] = "";
static char compose_body[2048] = "";

static animation_t msg_hover_anims[32];
static animation_t row_swipe_x[32];
static int mail_drag_start_mx = 0;
static int mail_drag_idx = -1;

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

  // Demo Mode: If sync fails, provide mock content
  if (!sync_success) {
    strcpy(status_text, "READY");
    mail_message_t mock;

    strcpy(mock.header.from, "github@github.com");
    strcpy(mock.header.subject, "New login detected on PureOS");
    strcpy(mock.header.uid, "github_login");
    mock.body = "Hi rudra, A new login was detected on your "
                "account from a browser on PureOS.\n\n"
                "If this was you, no action is needed.";
    mock.body_len = strlen(mock.body);
    mail_save_message(current_account, "inbox", &mock);

    strcpy(mock.header.from, "pureos-team@pureos.org");
    strcpy(mock.header.subject, "Welcome to PureOS Mail!");
    strcpy(mock.header.uid, "welcome");
    mock.body = "Hello! This is a mock email for demonstration because the "
                "POP3 sync could not reach the server.\n\n"
                "PureOS Mail is now ready for use.";
    mock.body_len = strlen(mock.body);
    mail_save_message(current_account, "inbox", &mock);

    strcpy(mock.header.from, "security@pureos.org");
    strcpy(mock.header.subject, "Security Update Available");
    strcpy(mock.header.uid, "security");
    mock.body = "A new security update is available for your system. Please "
                "run 'update' from the shell.";
    mock.body_len = strlen(mock.body);
    mail_save_message(current_account, "inbox", &mock);
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

  (void)theme_get(); // Keep include valid
  int tb_y = 24;     // Below titlebar

  // === FULL BACKGROUND ===
  winmgr_fill_rect(win, 0, tb_y, win->width, win->height - tb_y,
                   MAIL_C_DARK_BG);

  // === 1. TOOLBAR ===
  winmgr_fill_rect(win, 0, tb_y, win->width, MAIL_TOOLBAR_H, 0xFF0F1120);
  winmgr_fill_rect(win, 0, tb_y + MAIL_TOOLBAR_H, win->width, 1, MAIL_C_BORDER);

  // Sync button
  int sync_hov = (mail_mouse_x >= 12 && mail_mouse_x <= 82 &&
                  mail_mouse_y >= tb_y + 6 && mail_mouse_y <= tb_y + 30);
  winmgr_fill_rect(win, 12, tb_y + 6, 70, 24,
                   sync_hov ? MAIL_C_CARD_HOVER : 0x00000000);
  winmgr_draw_text(win, 22, tb_y + 11, "Sync", MAIL_C_TEXT);

  // Compose button
  int comp_hov = (mail_mouse_x >= 90 && mail_mouse_x <= 180 &&
                  mail_mouse_y >= tb_y + 6 && mail_mouse_y <= tb_y + 30);
  winmgr_fill_rect(win, 90, tb_y + 6, 90, 24,
                   compose_mode ? MAIL_C_ACCENT
                                : (comp_hov ? MAIL_C_CARD_HOVER : 0x00000000));
  winmgr_draw_text(win, 100, tb_y + 11, "Compose",
                   compose_mode ? 0xFFFFFFFF : MAIL_C_TEXT);

  // Status text (right side)
  winmgr_draw_text(win, win->width - 120, tb_y + 11, status_text,
                   MAIL_C_TEXT_DIM);

  int content_y = tb_y + MAIL_TOOLBAR_H + 1;
  int content_h = win->height - content_y;

  // === 2. SIDEBAR ===
  winmgr_fill_rect(win, 0, content_y, MAIL_SIDEBAR_WIDTH, content_h,
                   MAIL_C_SIDEBAR_BG);
  winmgr_fill_rect(win, MAIL_SIDEBAR_WIDTH, content_y, 1, content_h,
                   MAIL_C_BORDER);

  // Accounts section
  winmgr_draw_text(win, 10, content_y + 10, "ACCOUNTS", MAIL_C_TEXT_VDIM);

  // Account dropdown
  int acc_y = content_y + 28;
  int acc_dropdown_w = MAIL_SIDEBAR_WIDTH - 46;
  winmgr_fill_rect(win, 8, acc_y, acc_dropdown_w, 24, MAIL_C_INPUT_BG);
  winmgr_fill_rect(win, 8, acc_y, acc_dropdown_w, 24, MAIL_C_BORDER);
  if (selected_account_idx >= 0 && selected_account_idx < account_count) {
    // Truncate to fit dropdown width (~20 chars at 8px font)
    int max_ch = (acc_dropdown_w - 20) / 8;
    if (max_ch > 60)
      max_ch = 60;
    char acct_disp[64];
    strncpy(acct_disp, accounts[selected_account_idx], max_ch);
    acct_disp[max_ch] = 0;
    winmgr_draw_text(win, 14, acc_y + 6, acct_disp, MAIL_C_TEXT);
  }
  // Dropdown arrow
  winmgr_draw_text(win, 8 + acc_dropdown_w - 16, acc_y + 6, "v",
                   MAIL_C_TEXT_DIM);
  // + button
  winmgr_fill_rect(win, MAIL_SIDEBAR_WIDTH - 34, acc_y, 26, 24, MAIL_C_CARD_BG);
  winmgr_draw_text(win, MAIL_SIDEBAR_WIDTH - 26, acc_y + 6, "+", MAIL_C_TEXT);

  // Folders section
  int folder_y = acc_y + 34;
  winmgr_draw_text(win, 10, folder_y, "FOLDERS", MAIL_C_TEXT_VDIM);
  folder_y += 18;

  const char *folders[] = {"Inbox", "Sent", "Trash"};
  for (int i = 0; i < 3; i++) {
    int fy = folder_y + i * (MAIL_ROW_HEIGHT + 4);
    int fhov = (mail_mouse_x >= 4 && mail_mouse_x < MAIL_SIDEBAR_WIDTH - 4 &&
                mail_mouse_y >= fy && mail_mouse_y < fy + MAIL_ROW_HEIGHT);
    if (i == selected_folder) {
      winmgr_fill_rect(win, 4, fy, MAIL_SIDEBAR_WIDTH - 8, MAIL_ROW_HEIGHT,
                       MAIL_C_ACCENT);
      winmgr_draw_text(win, 28, fy + 7, folders[i], 0xFFFFFFFF);
    } else {
      if (fhov)
        winmgr_fill_rect(win, 4, fy, MAIL_SIDEBAR_WIDTH - 8, MAIL_ROW_HEIGHT,
                         MAIL_C_CARD_HOVER);
      winmgr_draw_text(win, 28, fy + 7, folders[i], MAIL_C_TEXT_DIM);
    }
  }

  // === 3. MESSAGE LIST (OR COMPOSE FORM) ===
  int list_x = MAIL_SIDEBAR_WIDTH + 1;

  if (compose_mode) {
    winmgr_fill_rect(win, list_x, content_y, win->width - list_x, content_h,
                     MAIL_C_DARK_BG);

    int cf_y = content_y + 15;
    // To field
    uint32_t to_bc = (compose_field == 0) ? MAIL_C_ACCENT : MAIL_C_BORDER;
    winmgr_draw_text(win, list_x + 15, cf_y + 7, "To:", MAIL_C_TEXT_DIM);
    winmgr_fill_rect(win, list_x + 70, cf_y, win->width - list_x - 90, 26,
                     MAIL_C_INPUT_BG);
    winmgr_fill_rect(win, list_x + 70, cf_y + 25, win->width - list_x - 90, 1,
                     to_bc);
    winmgr_draw_text(win, list_x + 75, cf_y + 7, compose_to, MAIL_C_TEXT);

    cf_y += 38;
    // Subject field
    uint32_t su_bc = (compose_field == 1) ? MAIL_C_ACCENT : MAIL_C_BORDER;
    winmgr_draw_text(win, list_x + 15, cf_y + 7, "Subject:", MAIL_C_TEXT_DIM);
    winmgr_fill_rect(win, list_x + 70, cf_y, win->width - list_x - 90, 26,
                     MAIL_C_INPUT_BG);
    winmgr_fill_rect(win, list_x + 70, cf_y + 25, win->width - list_x - 90, 1,
                     su_bc);
    winmgr_draw_text(win, list_x + 75, cf_y + 7, compose_subject, MAIL_C_TEXT);

    cf_y += 38;
    winmgr_fill_rect(win, list_x, cf_y, win->width - list_x, 1, MAIL_C_BORDER);
    cf_y += 5;

    // Body
    uint32_t bo_bc = (compose_field == 2) ? MAIL_C_ACCENT : MAIL_C_BORDER;
    winmgr_fill_rect(win, list_x + 15, cf_y, win->width - list_x - 30,
                     win->height - cf_y - 60, MAIL_C_INPUT_BG);
    winmgr_fill_rect(win, list_x + 15, win->height - 62,
                     win->width - list_x - 30, 1, bo_bc);
    winmgr_draw_text(win, list_x + 20, cf_y + 8, compose_body, MAIL_C_TEXT);

    winmgr_draw_text(win, list_x + 15, win->height - 50,
                     "Tab=Next  Backspace=Del", MAIL_C_TEXT_VDIM);

    // Send button
    winmgr_fill_rect(win, list_x + 15, win->height - 48, 70, 26, MAIL_C_ACCENT);
    winmgr_draw_text(win, list_x + 30, win->height - 41, "SEND", 0xFFFFFFFF);
    return;
  }

  // --- Message list panel ---
  winmgr_fill_rect(win, list_x, content_y, MAIL_LIST_WIDTH, content_h,
                   MAIL_C_DARK_BG);
  winmgr_fill_rect(win, list_x + MAIL_LIST_WIDTH, content_y, 1, content_h,
                   MAIL_C_BORDER);

  // Search bar
  int search_y = content_y + 8;
  winmgr_fill_rect(win, list_x + 10, search_y, MAIL_LIST_WIDTH - 20,
                   MAIL_SEARCH_H, MAIL_C_INPUT_BG);
  winmgr_fill_rect(win, list_x + 10, search_y, MAIL_LIST_WIDTH - 20,
                   MAIL_SEARCH_H, MAIL_C_BORDER);
  winmgr_draw_text(win, list_x + 18, search_y + 8, "Search mail...",
                   MAIL_C_TEXT_VDIM);

  // Message cards
  int msg_y = search_y + MAIL_SEARCH_H + 10;
  int card_w = MAIL_LIST_WIDTH - 16;
  int max_sender_ch = (card_w - 90) / 8; // Leave room for date
  if (max_sender_ch > 40)
    max_sender_ch = 40;
  int max_subj_ch = (card_w - 16) / 8;
  if (max_subj_ch > 50)
    max_subj_ch = 50;
  int max_prev_ch = (card_w - 16) / 8;
  if (max_prev_ch > 50)
    max_prev_ch = 50;

  for (int i = 0; i < msg_count; i++) {
    int card_h = 70;
    int cy = msg_y + i * (card_h + 8);
    int mhov = (mail_mouse_x >= list_x + 8 &&
                mail_mouse_x < list_x + MAIL_LIST_WIDTH - 8 &&
                mail_mouse_y >= cy && mail_mouse_y < cy + card_h);

    // Tick the animation for this card
    if (!msg_hover_anims[i].active && msg_hover_anims[i].current_val != (mhov ? 1.0f : 0.0f)) {
        anim_start(&msg_hover_anims[i], msg_hover_anims[i].current_val, mhov ? 1.0f : 0.0f, 0.2f, EASE_LINEAR);
    }
    anim_tick(&msg_hover_anims[i], 0.04f);
    if (msg_hover_anims[i].active) {
        win->needs_redraw = 1;
        extern int ui_dirty;
        ui_dirty = 1;
    }
    
    float p = msg_hover_anims[i].current_val;
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    
    uint32_t card_bg;
    if (i == selected_msg_idx) {
        card_bg = MAIL_C_ACCENT;
    } else {
        uint32_t c1 = MAIL_C_CARD_BG;
        uint32_t c2 = MAIL_C_CARD_HOVER;
        int r = ((c1 >> 16) & 0xFF) + (int)((((c2 >> 16) & 0xFF) - ((c1 >> 16) & 0xFF)) * p);
        int g = ((c1 >> 8) & 0xFF) + (int)((((c2 >> 8) & 0xFF) - ((c1 >> 8) & 0xFF)) * p);
        int b = (c1 & 0xFF) + (int)(((c2 & 0xFF) - (c1 & 0xFF)) * p);
        card_bg = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
    
    anim_tick(&row_swipe_x[i], 0.04f);
    if (row_swipe_x[i].active) {
        win->needs_redraw = 1;
        extern int ui_dirty;
        ui_dirty = 1;
    }
    
    int swipe_offset = (int)row_swipe_x[i].current_val;
    
    // Draw red delete background underneath
    if (swipe_offset < 0) {
        winmgr_fill_rect(win, list_x + 8 + card_w + swipe_offset, cy, -swipe_offset, card_h, 0xFFE53935);
        winmgr_draw_text(win, list_x + 8 + card_w - 60, cy + card_h/2 - 8, "Delete", 0xFFFFFFFF);
    }
    
    winmgr_fill_rect(win, list_x + 8 + swipe_offset, cy, card_w, card_h, card_bg);

    uint32_t tcol = (i == selected_msg_idx) ? 0xFFFFFFFF : MAIL_C_TEXT;
    uint32_t scol = (i == selected_msg_idx) ? 0xFFDDDDFF : MAIL_C_TEXT_DIM;

    // Sender (truncated to fit)
    char sender[48];
    strncpy(sender, msg_headers[i].from, max_sender_ch);
    sender[max_sender_ch] = 0;
    winmgr_draw_text(win, list_x + 16 + swipe_offset, cy + 8, sender, tcol);

    // Date (dynamic, right-aligned)
    char date_str[16];
    if (strlen(msg_headers[i].date) > 0) {
      strcpy(date_str, msg_headers[i].date);
    } else {
      rtc_time_t now;
      rtc_read(&now);
      char numbuf[8];
      strcpy(date_str, "20");
      k_itoa(now.year, numbuf);
      if (now.year < 10)
        strcat(date_str, "0");
      strcat(date_str, numbuf);
      strcat(date_str, "-");

      k_itoa(now.month, numbuf);
      if (now.month < 10)
        strcat(date_str, "0");
      strcat(date_str, numbuf);
      strcat(date_str, "-");

      k_itoa(now.day, numbuf);
      if (now.day < 10)
        strcat(date_str, "0");
      strcat(date_str, numbuf);
    }

    // Draw further left to avoid clipping
    winmgr_draw_text(win, list_x + MAIL_LIST_WIDTH - 100 + swipe_offset, cy + 8, date_str,
                     scol);

    // Subject (bold line)
    char subj[54];
    strncpy(subj, msg_headers[i].subject, max_subj_ch);
    subj[max_subj_ch] = 0;
    winmgr_draw_text(win, list_x + 16 + swipe_offset, cy + 26, subj, tcol);

    // Preview text
    char preview[54];
    const char *mock_preview = msg_headers[i].subject; // Fallback to subject
    if (strstr(msg_headers[i].subject, "New login")) {
      mock_preview =
          "Hi rudra, A new login was detected on your account from a b...";
    } else if (strstr(msg_headers[i].subject, "Welcome")) {
      mock_preview =
          "Hello! This is a mock email for demonstration because the PO...";
    } else if (strstr(msg_headers[i].subject, "Security")) {
      mock_preview =
          "A new security update is available for your system. Please r...";
    }

    strncpy(preview, mock_preview, max_prev_ch);
    preview[max_prev_ch] = 0;
    // Add ellipsis if truncated
    int plen = strlen(preview);
    if (plen > max_prev_ch - 4 &&
        strlen(mock_preview) > (unsigned)max_prev_ch) {
      preview[plen - 3] = '.';
      preview[plen - 2] = '.';
      preview[plen - 1] = '.';
    }
    winmgr_draw_text(win, list_x + 16 + swipe_offset, cy + 44, preview, scol);
  }

  // === 4. MESSAGE VIEW ===
  int view_x = list_x + MAIL_LIST_WIDTH + 1;
  int view_w = win->width - view_x;

  if (has_msg_loaded) {
    // From header
    winmgr_draw_text(win, view_x + 20, content_y + 16,
                     "From:", MAIL_C_TEXT_DIM);
    winmgr_draw_text(win, view_x + 70, content_y + 16, current_msg.header.from,
                     MAIL_C_TEXT);

    // Subject header
    winmgr_draw_text(win, view_x + 20, content_y + 36,
                     "Subject:", MAIL_C_TEXT_DIM);
    winmgr_draw_text(win, view_x + 85, content_y + 36,
                     current_msg.header.subject, MAIL_C_TEXT);

    // Divider
    winmgr_fill_rect(win, view_x + 20, content_y + 56, view_w - 40, 1,
                     MAIL_C_BORDER);

    // Body
    winmgr_draw_text(win, view_x + 20, content_y + 68, current_msg.body,
                     MAIL_C_TEXT);
  } else {
    // Empty state - centered envelope icon area
    int cx = view_x + view_w / 2;
    int cy = content_y + content_h / 2 - 30;

    // Envelope icon (simple rectangle)
    winmgr_fill_rect(win, cx - 25, cy - 20, 50, 35, MAIL_C_CARD_BG);
    winmgr_fill_rect(win, cx - 25, cy - 20, 50, 1, MAIL_C_BORDER);
    winmgr_fill_rect(win, cx - 25, cy + 14, 50, 1, MAIL_C_BORDER);
    winmgr_fill_rect(win, cx - 25, cy - 20, 1, 35, MAIL_C_BORDER);
    winmgr_fill_rect(win, cx + 24, cy - 20, 1, 35, MAIL_C_BORDER);

    // Title
    winmgr_draw_text(win, cx - 80, cy + 30, "SELECT AN EMAIL TO READ",
                     MAIL_C_TEXT_DIM);
    // Subtitle
    winmgr_draw_text(win, cx - 95, cy + 48, "Or compose a new message using",
                     MAIL_C_TEXT_VDIM);
    winmgr_draw_text(win, cx - 60, cy + 62, "the toolbar button.",
                     MAIL_C_TEXT_VDIM);
  }
}

void mail_app_on_mouse(window_t *win, int mx, int my, int buttons) {
  // Track mouse for hover effects
  if (mail_mouse_x != mx || mail_mouse_y != my) {
    mail_mouse_x = mx;
    mail_mouse_y = my;
    win->needs_redraw = 1;
  }

  if (!(buttons & 1)) {
    if (mail_drag_idx != -1) {
        float current = row_swipe_x[mail_drag_idx].current_val;
        if (current < -40.0f) {
            anim_start_spring(&row_swipe_x[mail_drag_idx], current, -80.0f, 400.0f, 30.0f);
        } else {
            anim_start_spring(&row_swipe_x[mail_drag_idx], current, 0.0f, 400.0f, 30.0f);
            // If barely moved, count as click
            if (current > -5.0f && current < 5.0f) {
                mail_app_load_message(mail_drag_idx);
            }
        }
        mail_drag_idx = -1;
        win->needs_redraw = 1;
    }
    return;
  }
  
  if (mail_drag_idx != -1) {
      int delta = mx - mail_drag_start_mx;
      if (delta > 0) delta = 0;
      if (delta < -120) delta = -120;
      row_swipe_x[mail_drag_idx].current_val = (float)delta;
      row_swipe_x[mail_drag_idx].active = 0;
      win->needs_redraw = 1;
      return;
  }

  print_serial("MAIL: Mouse down at ");
  debug_print_int(mx);
  print_serial(",");
  debug_print_int(my);
  print_serial("\n");

  int tb_y = 24;
  int content_y = tb_y + MAIL_TOOLBAR_H + 1;

  // Toolbar Sync (new position)
  if (mx >= 12 && mx <= 82 && my >= tb_y + 6 && my <= tb_y + 30) {
    print_serial("MAIL: Sync button clicked\n");
    mail_app_sync();
    return;
  }

  // Toolbar Compose (new position)
  if (mx >= 90 && mx <= 180 && my >= tb_y + 6 && my <= tb_y + 30) {
    print_serial("MAIL: Compose button clicked\n");
    compose_mode = !compose_mode;
    win->needs_redraw = 1;
    return;
  }

  // Compose View Actions
  if (compose_mode) {
    int list_x = MAIL_SIDEBAR_WIDTH + 1;
    int cf_y0 = content_y + 15;

    // Field Selection (To, Subject, Body)
    if (mx >= list_x + 70 && mx <= win->width - 20) {
      if (my >= cf_y0 && my <= cf_y0 + 26) {
        compose_field = 0; // To
        win->needs_redraw = 1;
      } else if (my >= cf_y0 + 38 && my <= cf_y0 + 64) {
        compose_field = 1; // Subject
        win->needs_redraw = 1;
      }
    }
    if (mx >= list_x + 15 && mx <= win->width - 15 && my >= cf_y0 + 81 &&
        my <= win->height - 60) {
      compose_field = 2; // Body
      win->needs_redraw = 1;
    }

    // Send Button
    if (mx >= list_x + 15 && mx <= list_x + 85 && my >= win->height - 48 &&
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
        const char *pass = "google app password here";

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

  // Sidebar: Folder selection
  int acc_y = content_y + 28;
  int folder_y = acc_y + 34 + 18;
  if (mx >= 4 && mx < MAIL_SIDEBAR_WIDTH - 4 && my >= folder_y &&
      my < folder_y + 3 * (MAIL_ROW_HEIGHT + 4)) {
    int idx = (my - folder_y) / (MAIL_ROW_HEIGHT + 4);
    if (idx >= 0 && idx < 3) {
      selected_folder = idx;
      win->needs_redraw = 1;
    }
    return;
  }

  // Sidebar: Account dropdown area (click cycles accounts)
  if (mx >= 8 && mx < MAIL_SIDEBAR_WIDTH - 34 && my >= acc_y &&
      my <= acc_y + 24) {
    if (account_count > 0) {
      selected_account_idx = (selected_account_idx + 1) % account_count;
      strcpy(current_account, accounts[selected_account_idx]);
      mail_app_refresh_messages();
      win->needs_redraw = 1;
    }
    return;
  }

  // Message List
  int list_x = MAIL_SIDEBAR_WIDTH + 1;
  int search_y = content_y + 8;
  int msg_start_y = search_y + MAIL_SEARCH_H + 10;
  int card_h = 70;
  if (mx >= list_x && mx <= list_x + MAIL_LIST_WIDTH && my >= msg_start_y &&
      my < win->height) {
    int idx = (my - msg_start_y) / (card_h + 8);
    if (idx >= 0 && idx < msg_count) {
      // Check if delete button clicked
      if (row_swipe_x[idx].current_val <= -70.0f && mx > list_x + MAIL_LIST_WIDTH - 80) {
          // Delete clicked! (Placeholder for actual delete logic)
          print_serial("MAIL: Delete clicked\n");
          anim_start_spring(&row_swipe_x[idx], row_swipe_x[idx].current_val, 0.0f, 400.0f, 30.0f);
          win->needs_redraw = 1;
          return;
      }
      
      if (mail_drag_idx == -1) {
          mail_drag_idx = idx;
          mail_drag_start_mx = mx;
      }
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
  mail_win = winmgr_create_window(-1, -1, 1000, 750, "MAIL");
  if (!mail_win)
    return;
    
  for (int i = 0; i < 32; i++) {
      anim_init(&msg_hover_anims[i]);
      anim_init_val(&row_swipe_x[i], 0.0f);
  }

  mail_win->draw = (void (*)(void *))mail_app_draw;
  mail_win->on_mouse = (void (*)(void *, int, int, int))mail_app_on_mouse;
  mail_win->on_key = (void (*)(void *, int, char))mail_app_on_key;
  mail_win->app_type = 13;
  mail_win->owner_pid = 1; // FORCE KERNEL OWNER to ensure callbacks are used

  strcpy(compose_to, "recipient@example.com");
  strcpy(compose_subject, "Draft from PureOS");
  strcpy(compose_body, "This is a test message composed on PureOS.");

  mail_storage_init(
      "rudraptl2611@gmail.com"); // Ensure at least one account exists
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
