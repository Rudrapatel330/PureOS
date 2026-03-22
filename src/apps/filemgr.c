// filemgr.c - Windows Explorer-Style File Manager
#include "filemgr.h"
#include "../fs/fat.h"
#include "../fs/fs.h"
#include "../kernel/clipboard.h"
#include "../kernel/compositor.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/window.h"

// ======================== STATE ========================

#define FM_MAX_FILES 64
#define FM_MAX_PATH 128

// File listing
static FileInfo file_list[FM_MAX_FILES];
static int file_count = 0;
static int selected_index = -1;
static int scroll_offset = 0;

// Navigation
static char current_path[FM_MAX_PATH] = "/"; // FAT path
static int at_root = 1; // 1 = showing "My Computer" (C: drive icon)

// UI modes
#define FM_MODE_BROWSE 0
#define FM_MODE_NEW_FOLDER 1
#define FM_MODE_NEW_FILE 2
#define FM_MODE_RENAME 3
#define FM_MODE_MSG 4
static int ui_mode = FM_MODE_BROWSE;
static char input_buffer[64];
static int input_pos = 0;
static char popup_msg[64] = {0};

static void fm_show_popup(const char *msg) {
  strcpy(popup_msg, msg);
  ui_mode = FM_MODE_MSG;
  extern int ui_dirty;
  ui_dirty = 1;
}

// Context Menu
static int ctx_visible = 0;
static int ctx_x = 0, ctx_y = 0;

// Toolbar button hover
static int hover_btn_id = 0;

extern void print_serial(const char *);
extern int screen_width, screen_height;
extern int ui_dirty;

// ======================== HELPERS ========================

static int fm_str_len(const char *s) {
  int len = 0;
  while (s[len])
    len++;
  return len;
}

static void fm_path_up() {
  // Go up one directory
  if (strcmp(current_path, "/") == 0) {
    at_root = 1;
    return;
  }
  // Find last slash
  int last = -1;
  for (int i = 0; current_path[i]; i++)
    if (current_path[i] == '/')
      last = i;

  if (last <= 0) {
    strcpy(current_path, "/");
  } else {
    current_path[last] = 0;
  }
}

static void fm_navigate_to(const char *name) {
  if (at_root) {
    // Entering C: drive
    at_root = 0;
    strcpy(current_path, "/");
    return;
  }

  // Append to path
  if (strcmp(current_path, "/") == 0) {
    // Root + name
    char new_path[FM_MAX_PATH];
    new_path[0] = '/';
    strcpy(new_path + 1, name);
    strcpy(current_path, new_path);
  } else {
    int len = fm_str_len(current_path);
    current_path[len] = '/';
    strcpy(current_path + len + 1, name);
  }
}

// ======================== REFRESH ========================

static void fm_refresh() {
  file_count = 0;
  selected_index = -1;
  scroll_offset = 0;

  if (at_root)
    return; // Root shows C: drive icon only

  // Resolve path to FAT cluster
  uint32_t cluster = fat_resolve_path(current_path, fat_get_root_cluster());
  if (cluster == 0xFFFFFFFF) {
    // Path not found - go to root
    strcpy(current_path, "/");
    cluster = fat_get_root_cluster();
  }

  file_count = fat_list_files_gui_dir(cluster, file_list, FM_MAX_FILES);
}

// ======================== DRAWING ========================

// Draw a small 16x16 folder icon
static void fm_draw_folder_icon(window_t *win, int x, int y) {
  // Folder tab
  winmgr_fill_rect(win, x, y + 2, 8, 3, 0xFE00); // Orange tab
  // Folder body
  winmgr_fill_rect(win, x, y + 4, 16, 12, 0xFE60); // Yellow body
  // Shadow
  winmgr_fill_rect(win, x, y + 15, 16, 1, 0xC560); // Dark edge
  winmgr_fill_rect(win, x + 15, y + 4, 1, 12, 0xC560);
  // Highlight
  winmgr_fill_rect(win, x, y + 4, 1, 11, 0xFFE0); // Light left
  winmgr_fill_rect(win, x, y + 4, 16, 1, 0xFFE0); // Light top
}

// Draw a small 16x16 file icon
static void fm_draw_file_icon(window_t *win, int x, int y) {
  // Page body
  winmgr_fill_rect(win, x + 2, y, 12, 16, 0xFFFF); // White
  // Border
  winmgr_draw_rect(win, x + 2, y, 12, 16, 0x8410); // Gray border
  // Folded corner
  winmgr_fill_rect(win, x + 10, y, 4, 4, 0xDEFB); // Light gray
  winmgr_draw_line(win, x + 10, y, x + 14, y + 4, 0x8410);
  // Text lines
  winmgr_fill_rect(win, x + 4, y + 6, 8, 1, 0xBDF7);
  winmgr_fill_rect(win, x + 4, y + 9, 6, 1, 0xBDF7);
  winmgr_fill_rect(win, x + 4, y + 12, 7, 1, 0xBDF7);
}

// Draw C: drive icon (larger, 32x32-ish)
static void fm_draw_drive_icon(window_t *win, int x, int y) {
  // Drive body
  winmgr_fill_rect(win, x, y + 4, 32, 24, 0xC618); // Gray body
  // 3D edges
  winmgr_fill_rect(win, x, y + 4, 32, 1, 0xDEFB);      // Top highlight
  winmgr_fill_rect(win, x, y + 4, 1, 24, 0xDEFB);      // Left highlight
  winmgr_fill_rect(win, x, y + 27, 32, 1, 0x4208);     // Bottom shadow
  winmgr_fill_rect(win, x + 31, y + 4, 1, 24, 0x4208); // Right shadow
  // Light/activity indicator
  winmgr_fill_rect(win, x + 3, y + 22, 4, 3, 0x07E0); // Green LED
  // Drive slot
  winmgr_fill_rect(win, x + 4, y + 8, 24, 3, 0x8410); // Slot
  winmgr_fill_rect(win, x + 4, y + 8, 24, 1, 0x4208); // Slot shadow
}

static void fm_draw_toolbar(window_t *win) {
  int bw = win->width;

  // Toolbar background (gradient)
  for (int r = 0; r < 26; r++) {
    uint16_t c = 0xE71C - (r * 0x0020); // Subtle gradient
    if (c < 0xC618)
      c = 0xC618;
    winmgr_fill_rect(win, 2, 24 + r, bw - 4, 1, c);
  }
  // Bottom border
  winmgr_fill_rect(win, 2, 50, bw - 4, 1, 0x8410);

  // Back button
  uint16_t back_col = (hover_btn_id == 1) ? 0xBDF7 : 0xDEFB;
  winmgr_fill_rect(win, 6, 28, 50, 18, back_col);
  winmgr_draw_rect(win, 6, 28, 50, 18, 0x8410);
  winmgr_draw_text(win, 12, 32, "<Back", 0x0000);

  // Up button
  uint16_t up_col = (hover_btn_id == 2) ? 0xBDF7 : 0xDEFB;
  winmgr_fill_rect(win, 60, 28, 32, 18, up_col);
  winmgr_draw_rect(win, 60, 28, 32, 18, 0x8410);
  winmgr_draw_text(win, 66, 32, "Up", 0x0000);

  // New Folder button
  uint16_t nf_col = (hover_btn_id == 3) ? 0xBDF7 : 0xDEFB;
  winmgr_fill_rect(win, 96, 28, 64, 18, nf_col);
  winmgr_draw_rect(win, 96, 28, 64, 18, 0x8410);
  winmgr_draw_text(win, 100, 32, "NewDir", 0x0000);

  // New File button
  uint16_t nfi_col = (hover_btn_id == 4) ? 0xBDF7 : 0xDEFB;
  winmgr_fill_rect(win, 164, 28, 64, 18, nfi_col);
  winmgr_draw_rect(win, 164, 28, 64, 18, 0x8410);
  winmgr_draw_text(win, 168, 32, "NewFile", 0x0000);

  // Delete button
  uint16_t del_col = (hover_btn_id == 5) ? 0xF800 : 0xDEFB;
  winmgr_fill_rect(win, 232, 28, 50, 18, del_col);
  winmgr_draw_rect(win, 232, 28, 50, 18, 0x8410);
  winmgr_draw_text(win, 238, 32, "Del", (hover_btn_id == 5) ? 0xFFFF : 0x0000);
}

static void fm_draw_address_bar(window_t *win) {
  int bw = win->width;

  // Address bar background
  winmgr_fill_rect(win, 2, 52, bw - 4, 20, 0xFFFF);
  winmgr_draw_rect(win, 2, 52, bw - 4, 20, 0x8410);

  // Path label
  winmgr_draw_text(win, 6, 56, "Addr:", 0x8410);

  // Path text
  if (at_root) {
    winmgr_draw_text(win, 50, 56, "My Computer", 0x0000);
  } else {
    // Show C:\path format
    char display[FM_MAX_PATH + 3];
    display[0] = 'C';
    display[1] = ':';
    display[2] = '\\';
    int di = 3;
    for (int i = 1; current_path[i] && di < FM_MAX_PATH; i++) {
      if (current_path[i] == '/')
        display[di++] = '\\';
      else
        display[di++] = current_path[i];
    }
    display[di] = 0;
    if (di == 3) { // Root
      display[2] = '\\';
      display[3] = 0;
    }
    winmgr_draw_text(win, 50, 56, display, 0x0000);
  }
}

static void fm_draw_column_headers(window_t *win) {
  int bw = win->width;

  // Header row
  winmgr_fill_rect(win, 2, 74, bw - 4, 18, 0xDEFB);
  winmgr_draw_rect(win, 2, 74, bw - 4, 18, 0x8410);

  // Column: Name
  winmgr_draw_text(win, 30, 78, "Name", 0x0000);
  // Separator
  winmgr_fill_rect(win, bw - 104, 74, 1, 18, 0x8410);
  // Column: Size
  winmgr_draw_text(win, bw - 100, 78, "Size", 0x0000);
  // Separator
  winmgr_fill_rect(win, bw - 52, 74, 1, 18, 0x8410);
  // Column: Type
  winmgr_draw_text(win, bw - 48, 78, "Type", 0x0000);
}

static void fm_draw_file_list(window_t *win) {
  int bw = win->width;
  int list_y = 93;
  int row_h = 20;
  int max_visible = (win->height - list_y - 30) / row_h;

  // List area background
  winmgr_fill_rect(win, 2, list_y, bw - 4, win->height - list_y - 28, 0xFFFF);

  if (at_root) {
    // Show "My Computer" with C: drive
    winmgr_fill_rect(win, 20, list_y + 10, bw - 44, 40,
                     (selected_index == 0) ? 0x001F : 0xFFFF);
    fm_draw_drive_icon(win, 24, list_y + 14);
    winmgr_draw_text(win, 62, list_y + 22, "Local Disk (C:)",
                     (selected_index == 0) ? 0xFFFF : 0x0000);
    winmgr_draw_text(win, 62, list_y + 34, "10 MB  FAT12",
                     (selected_index == 0) ? 0xBDF7 : 0x8410);
    return;
  }

  if (file_count == 0) {
    winmgr_draw_text(win, 20, list_y + 20, "(Empty Folder)", 0x8410);
    return;
  }

  for (int i = scroll_offset;
       i < file_count && (i - scroll_offset) < max_visible; i++) {
    int y = list_y + (i - scroll_offset) * row_h;
    int is_sel = (i == selected_index);

    // Skip . and .. display entries (but handle .. for navigation)
    if (strcmp(file_list[i].name, ".") == 0)
      continue;

    // Selection highlight
    if (is_sel) {
      winmgr_fill_rect(win, 3, y, bw - 6, row_h - 1, 0x001F); // Blue selection
    }

    // Icon
    if (file_list[i].is_dir) {
      if (strcmp(file_list[i].name, "..") == 0) {
        // Up arrow style for ..
        winmgr_draw_text(win, 8, y + 2, "..", is_sel ? 0xFFFF : 0xFE00);
      } else {
        fm_draw_folder_icon(win, 6, y + 1);
      }
    } else {
      fm_draw_file_icon(win, 6, y + 1);
    }

    // Name
    uint16_t text_col = is_sel ? 0xFFFF : 0x0000;
    winmgr_draw_text(win, 26, y + 4, file_list[i].name, text_col);

    // Size (right-aligned area)
    if (!file_list[i].is_dir) {
      char size_str[16];
      int sz = file_list[i].size;
      if (sz < 1024) {
        // Show bytes
        k_itoa(sz, size_str);
        int sl = fm_str_len(size_str);
        size_str[sl] = 'B';
        size_str[sl + 1] = 0;
      } else {
        k_itoa(sz / 1024, size_str);
        int sl = fm_str_len(size_str);
        size_str[sl] = 'K';
        size_str[sl + 1] = 'B';
        size_str[sl + 2] = 0;
      }
      winmgr_draw_text(win, bw - 100, y + 4, size_str,
                       is_sel ? 0xBDF7 : 0x8410);
    } else {
      winmgr_draw_text(win, bw - 100, y + 4, "-", is_sel ? 0xBDF7 : 0x8410);
    }

    // Type
    const char *type = file_list[i].is_dir ? "Dir" : "File";
    winmgr_draw_text(win, bw - 48, y + 4, type, is_sel ? 0xBDF7 : 0x8410);
  }
}

static void fm_draw_status_bar(window_t *win) {
  int bw = win->width;
  int by = win->height - 26;

  // Status bar background (3D sunken look)
  winmgr_fill_rect(win, 2, by, bw - 4, 22, 0xDEFB);
  winmgr_fill_rect(win, 2, by, bw - 4, 1, 0x8410);     // Top shadow
  winmgr_fill_rect(win, 2, by + 1, bw - 4, 1, 0xFFFF); // Inner highlight

  // File count
  char status[64];
  char num[12];
  if (at_root) {
    strcpy(status, "1 Drive");
  } else {
    k_itoa(file_count, num);
    strcpy(status, num);
    strcat(status, " items");
  }
  winmgr_draw_text(win, 8, by + 6, status, 0x0000);
}

static void fm_draw_input_dialog(window_t *win) {
  if (ui_mode == FM_MODE_BROWSE)
    return;

  int bw = win->width;
  int dx = bw / 2 - 100;
  int dy = win->height / 2 - 40;

  // Dialog box background
  winmgr_fill_rect(win, dx, dy, 200, 80, 0xDEFB);
  // 3D border
  winmgr_draw_rect(win, dx, dy, 200, 80, 0x4208);
  winmgr_fill_rect(win, dx + 1, dy + 1, 198, 1, 0xFFFF);
  winmgr_fill_rect(win, dx + 1, dy + 1, 1, 78, 0xFFFF);

  // Title
  const char *title =
      (ui_mode == FM_MODE_NEW_FOLDER) ? "New Folder" : "New File";
  winmgr_fill_rect(win, dx + 2, dy + 2, 196, 16, 0x001F); // Blue title
  winmgr_draw_text(win, dx + 6, dy + 5, title, 0xFFFF);

  // Label
  winmgr_draw_text(win, dx + 10, dy + 24, "Name:", 0x0000);

  // Input field
  winmgr_fill_rect(win, dx + 10, dy + 38, 180, 16, 0xFFFF);
  winmgr_draw_rect(win, dx + 10, dy + 38, 180, 16, 0x8410);
  winmgr_draw_text(win, dx + 14, dy + 42, input_buffer, 0x0000);

  // Cursor blink
  int cx = dx + 14 + input_pos * 8;
  winmgr_fill_rect(win, cx, dy + 39, 1, 14, 0x0000);

  // OK/Cancel buttons
  winmgr_fill_rect(win, dx + 60, dy + 58, 35, 16, 0xDEFB);
  winmgr_draw_rect(win, dx + 60, dy + 58, 35, 16, 0x8410);
  winmgr_draw_text(win, dx + 66, dy + 61, "OK", 0x0000);

  winmgr_fill_rect(win, dx + 105, dy + 58, 55, 16, 0xDEFB);
  winmgr_draw_rect(win, dx + 105, dy + 58, 55, 16, 0x8410);
  winmgr_draw_text(win, dx + 109, dy + 61, "Cancel", 0x0000);
}

static void fm_draw_msg_dialog(window_t *win) {
  if (ui_mode != FM_MODE_MSG)
    return;

  int bw = win->width;
  int dx = bw / 2 - 100;
  int dy = win->height / 2 - 40;

  // Dialog box background
  winmgr_fill_rect(win, dx, dy, 200, 80, 0xDEFB);
  winmgr_draw_rect(win, dx, dy, 200, 80, 0x4208);
  winmgr_fill_rect(win, dx + 1, dy + 1, 198, 1, 0xFFFF);
  winmgr_fill_rect(win, dx + 1, dy + 1, 1, 78, 0xFFFF);

  winmgr_fill_rect(win, dx + 2, dy + 2, 196, 16, 0x001F);
  winmgr_draw_text(win, dx + 6, dy + 5, "Transfer Status", 0xFFFF);

  winmgr_draw_text(win, dx + 10, dy + 32, popup_msg, 0x0000);

  // OK button
  winmgr_fill_rect(win, dx + 60, dy + 58, 80, 16, 0xDEFB);
  winmgr_draw_rect(win, dx + 60, dy + 58, 80, 16, 0x8410);
  winmgr_draw_text(win, dx + 96, dy + 62, "OK", 0x0000);
}

// ======================== MAIN DRAW ========================

void filemgr_draw(window_t *win) {
  if (!win)
    return;

  // Background
  winmgr_fill_rect(win, 2, 24, win->width - 4, win->height - 26, 0xFFFF);

  fm_draw_toolbar(win);
  fm_draw_address_bar(win);
  fm_draw_column_headers(win);
  fm_draw_file_list(win);
  fm_draw_status_bar(win);
  fm_draw_input_dialog(win);
  fm_draw_msg_dialog(win);
}

// ======================== ACTIONS ========================

static void fm_action_new_folder() {
  if (at_root || input_pos == 0)
    return;

  // Build full path
  char full_path[FM_MAX_PATH];
  if (strcmp(current_path, "/") == 0) {
    full_path[0] = '/';
    strcpy(full_path + 1, input_buffer);
  } else {
    strcpy(full_path, current_path);
    int len = fm_str_len(full_path);
    full_path[len] = '/';
    strcpy(full_path + len + 1, input_buffer);
  }

  fat_mkdir(full_path);
  print_serial("FILEMGR: Created folder: ");
  print_serial(full_path);
  print_serial("\n");

  fm_refresh();
}

static void fm_action_new_file() {
  if (at_root || input_pos == 0)
    return;

  // Build full path
  char full_path[FM_MAX_PATH];
  if (strcmp(current_path, "/") == 0) {
    full_path[0] = '/';
    strcpy(full_path + 1, input_buffer);
  } else {
    strcpy(full_path, current_path);
    int len = fm_str_len(full_path);
    full_path[len] = '/';
    strcpy(full_path + len + 1, input_buffer);
  }

  // Create empty file
  uint8_t empty_data[1] = {0};
  fat_write_file(full_path, empty_data, 0);
  print_serial("FILEMGR: Created file: ");
  print_serial(full_path);
  print_serial("\n");

  fm_refresh();
}

static void fm_action_delete() {
  if (at_root || selected_index < 0 || selected_index >= file_count)
    return;

  // Don't delete . or ..
  if (strcmp(file_list[selected_index].name, ".") == 0)
    return;
  if (strcmp(file_list[selected_index].name, "..") == 0)
    return;

  // Build full path
  char full_path[FM_MAX_PATH];
  if (strcmp(current_path, "/") == 0) {
    full_path[0] = '/';
    strcpy(full_path + 1, file_list[selected_index].name);
  } else {
    strcpy(full_path, current_path);
    int len = fm_str_len(full_path);
    full_path[len] = '/';
    strcpy(full_path + len + 1, file_list[selected_index].name);
  }

  fat_delete_file(full_path);
  print_serial("FILEMGR: Deleted: ");
  print_serial(full_path);
  print_serial("\n");

  fm_refresh();
}

static void fm_on_copy(void *w) {
  if (at_root || selected_index < 0 || selected_index >= file_count)
    return;
  if (strcmp(file_list[selected_index].name, ".") == 0 ||
      strcmp(file_list[selected_index].name, "..") == 0)
    return;

  char full_path[FM_MAX_PATH];
  if (strcmp(current_path, "/") == 0) {
    full_path[0] = '/';
    strcpy(full_path + 1, file_list[selected_index].name);
  } else {
    strcpy(full_path, current_path);
    int len = fm_str_len(full_path);
    full_path[len] = '/';
    strcpy(full_path + len + 1, file_list[selected_index].name);
  }

  if (file_list[selected_index].is_dir) {
    fm_show_popup("Folders not supported");
    ((window_t *)w)->needs_redraw = 1;
    return;
  }

  clipboard_copy(full_path);
  clipboard_set_operation(CLIPBOARD_OP_COPY);

  char msg[64];
  strcpy(msg, "Copied ");
  strcat(msg, file_list[selected_index].name);
  fm_show_popup(msg);
  ((window_t *)w)->needs_redraw = 1;
}

static void fm_on_cut(void *w) {
  if (at_root || selected_index < 0 || selected_index >= file_count)
    return;
  if (strcmp(file_list[selected_index].name, ".") == 0 ||
      strcmp(file_list[selected_index].name, "..") == 0)
    return;

  char full_path[FM_MAX_PATH];
  if (strcmp(current_path, "/") == 0) {
    full_path[0] = '/';
    strcpy(full_path + 1, file_list[selected_index].name);
  } else {
    strcpy(full_path, current_path);
    int len = fm_str_len(full_path);
    full_path[len] = '/';
    strcpy(full_path + len + 1, file_list[selected_index].name);
  }

  if (file_list[selected_index].is_dir) {
    fm_show_popup("Folders not supported");
    ((window_t *)w)->needs_redraw = 1;
    return;
  }

  clipboard_copy(full_path);
  clipboard_set_operation(CLIPBOARD_OP_CUT);

  char msg[64];
  strcpy(msg, "Cut ");
  strcat(msg, file_list[selected_index].name);
  fm_show_popup(msg);
  ((window_t *)w)->needs_redraw = 1;
}

static void fm_on_paste(void *w, const char *path) {
  if (at_root || !path || path[0] == 0)
    return;

  // Extract filename from source path
  const char *filename = path;
  for (int i = 0; path[i]; i++) {
    if (path[i] == '/')
      filename = &path[i + 1];
  }

  // Build destination path
  char dest_path[FM_MAX_PATH];
  if (strcmp(current_path, "/") == 0) {
    dest_path[0] = '/';
    strcpy(dest_path + 1, filename);
  } else {
    strcpy(dest_path, current_path);
    int len = fm_str_len(dest_path);
    dest_path[len] = '/';
    strcpy(dest_path + len + 1, filename);
  }

  // Check if source and destination are the same
  if (strcmp(path, dest_path) == 0) {
    fm_show_popup("Error: Same location!");
    ((window_t *)w)->needs_redraw = 1;
    return;
  }

  int op = clipboard_get_operation();
  char msg[64] = "Transfer failed!";
  if (op == CLIPBOARD_OP_COPY) {
    if (fat_copy_file(path, dest_path) == 0) {
      strcpy(msg, "Copy completed!");
    }
  } else if (op == CLIPBOARD_OP_CUT) {
    if (fat_move_file(path, dest_path) == 0) {
      strcpy(msg, "Move completed!");
      clipboard_set_operation(CLIPBOARD_OP_NONE);
    }
  } else {
    strcpy(msg, "Nothing to paste.");
  }

  fm_show_popup(msg);
  fm_refresh();
  ((window_t *)w)->needs_redraw = 1;
}

// ======================== INPUT HANDLERS ========================

void filemgr_on_key(window_t *win, int key, char c) {
  if (ui_mode == FM_MODE_MSG) {
    if (c == '\n' || key == 0x1C || key == 0x01) {
      ui_mode = FM_MODE_BROWSE;
      win->needs_redraw = 1;
      extern int ui_dirty;
      ui_dirty = 1;
    }
    return;
  }

  if (ui_mode != FM_MODE_BROWSE) {
    // Dialog has focus — handle text input
    if (c == '\n' || key == 0x1C) { // Enter
      if (ui_mode == FM_MODE_NEW_FOLDER)
        fm_action_new_folder();
      else if (ui_mode == FM_MODE_NEW_FILE)
        fm_action_new_file();
      ui_mode = FM_MODE_BROWSE;
      win->needs_redraw = 1;
      ui_dirty = 1;
    } else if (key == 0x01) { // Escape
      ui_mode = FM_MODE_BROWSE;
      win->needs_redraw = 1;
      ui_dirty = 1;
    } else if (c == '\b') {
      if (input_pos > 0) {
        input_pos--;
        input_buffer[input_pos] = 0;
        win->needs_redraw = 1;
        ui_dirty = 1;
      }
    } else if (c >= 32 && c < 127 && input_pos < 11) {
      // FAT 8.3 limit: max 12 chars including dot
      input_buffer[input_pos++] = c;
      input_buffer[input_pos] = 0;
      win->needs_redraw = 1;
      ui_dirty = 1;
    }
    return;
  }

  // Browse mode keys
  if (key == 0x48) { // Up arrow
    if (selected_index > 0)
      selected_index--;
    win->needs_redraw = 1;
    ui_dirty = 1;
  } else if (key == 0x50) { // Down arrow
    if (selected_index < file_count - 1)
      selected_index++;
    win->needs_redraw = 1;
    ui_dirty = 1;
  } else if (c == '\n' || key == 0x1C) { // Enter
    if (at_root) {
      fm_navigate_to("C:");
      fm_refresh();
    } else if (selected_index >= 0 && selected_index < file_count) {
      if (file_list[selected_index].is_dir) {
        if (strcmp(file_list[selected_index].name, "..") == 0) {
          fm_path_up();
        } else if (strcmp(file_list[selected_index].name, ".") != 0) {
          fm_navigate_to(file_list[selected_index].name);
        }
        fm_refresh();
      }
    }
    win->needs_redraw = 1;
    ui_dirty = 1;
  } else if (c == '\b') { // Backspace = go up
    if (at_root)
      return;
    fm_path_up();
    fm_refresh();
    win->needs_redraw = 1;
    ui_dirty = 1;
  }
}

void filemgr_on_mouse(window_t *win, int mx, int my, int buttons) {
  // mx, my are already window-relative
  int rx = mx;
  int ry = my;

  // Update toolbar hover
  int old_hover = hover_btn_id;
  hover_btn_id = 0;
  if (ry >= 28 && ry < 46) {
    if (rx >= 6 && rx < 56)
      hover_btn_id = 1; // Back
    else if (rx >= 60 && rx < 92)
      hover_btn_id = 2; // Up
    else if (rx >= 96 && rx < 160)
      hover_btn_id = 3; // New Folder
    else if (rx >= 164 && rx < 228)
      hover_btn_id = 4; // New File
    else if (rx >= 232 && rx < 282)
      hover_btn_id = 5; // Delete
  }
  if (hover_btn_id != old_hover) {
    win->needs_redraw = 1;
    ui_dirty = 1;
  }

  if (!(buttons & 1))
    return; // Only handle left click

  if (ui_mode == FM_MODE_MSG) {
    int bw = win->width;
    int dx = bw / 2 - 100;
    int dy = win->height / 2 - 40;
    if (rx >= dx + 60 && rx < dx + 140 && ry >= dy + 58 && ry < dy + 74) {
      ui_mode = FM_MODE_BROWSE;
      win->needs_redraw = 1;
      extern int ui_dirty;
      ui_dirty = 1;
    }
    return;
  }

  // Check dialog buttons first
  if (ui_mode != FM_MODE_BROWSE) {
    int bw = win->width;
    int dx = bw / 2 - 100;
    int dy = win->height / 2 - 40;

    // OK button
    if (rx >= dx + 60 && rx < dx + 95 && ry >= dy + 58 && ry < dy + 74) {
      if (ui_mode == FM_MODE_NEW_FOLDER)
        fm_action_new_folder();
      else if (ui_mode == FM_MODE_NEW_FILE)
        fm_action_new_file();
      ui_mode = FM_MODE_BROWSE;
      win->needs_redraw = 1;
      ui_dirty = 1;
      return;
    }
    // Cancel button
    if (rx >= dx + 105 && rx < dx + 160 && ry >= dy + 58 && ry < dy + 74) {
      ui_mode = FM_MODE_BROWSE;
      win->needs_redraw = 1;
      ui_dirty = 1;
      return;
    }
    return; // Dialog eats all clicks
  }

  // Toolbar clicks
  if (ry >= 28 && ry < 46) {
    if (hover_btn_id == 1) { // Back
      if (!at_root) {
        fm_path_up();
        fm_refresh();
      }
    } else if (hover_btn_id == 2) { // Up
      if (!at_root) {
        fm_path_up();
        fm_refresh();
      } else {
        return; // Already at top
      }
    } else if (hover_btn_id == 3) { // New Folder
      if (!at_root) {
        ui_mode = FM_MODE_NEW_FOLDER;
        memset(input_buffer, 0, sizeof(input_buffer));
        input_pos = 0;
      }
    } else if (hover_btn_id == 4) { // New File
      if (!at_root) {
        ui_mode = FM_MODE_NEW_FILE;
        memset(input_buffer, 0, sizeof(input_buffer));
        input_pos = 0;
      }
    } else if (hover_btn_id == 5) { // Delete
      fm_action_delete();
    }
    win->needs_redraw = 1;
    ui_dirty = 1;
    return;
  }

  // File list clicks
  int list_y = 93;
  int row_h = 20;

  if (ry >= list_y && ry < win->height - 28) {
    int clicked_index = scroll_offset + (ry - list_y) / row_h;

    if (at_root) {
      // C: drive click
      if (clicked_index == 0) {
        if (selected_index == 0) {
          // Double-click: enter C: drive
          fm_navigate_to("C:");
          fm_refresh();
        } else {
          selected_index = 0;
        }
      }
    } else {
      if (clicked_index >= 0 && clicked_index < file_count) {
        if (selected_index == clicked_index) {
          // Double-click: enter folder
          if (file_list[clicked_index].is_dir) {
            if (strcmp(file_list[clicked_index].name, "..") == 0) {
              fm_path_up();
            } else if (strcmp(file_list[clicked_index].name, ".") != 0) {
              fm_navigate_to(file_list[clicked_index].name);
            }
            fm_refresh();
          }
        } else {
          selected_index = clicked_index;
        }
      }
    }
    win->needs_redraw = 1;
    ui_dirty = 1;
  }
}

static void filemgr_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  int list_y = 93;
  int row_h = 20;
  int max_visible = (win->height - list_y - 30) / row_h;
  int max_scroll = file_count > max_visible ? file_count - max_visible : 0;

  // Wheel UP (direction > 0) -> scroll viewport UP -> decrease scroll_offset
  scroll_offset -= direction;

  if (scroll_offset < 0)
    scroll_offset = 0;
  if (scroll_offset > max_scroll)
    scroll_offset = max_scroll;
  win->needs_redraw = 1;
  ui_dirty = 1;
}

// ======================== INIT ========================

static void filemgr_on_close(void *w) {
  (void)w;
  // Reset state for next launch
  at_root = 1;
  strcpy(current_path, "/");
  ui_mode = FM_MODE_BROWSE;
}

void filemgr_init() {
  // Singleton check using app_type
  window_t *existing_win = winmgr_get_window_by_app_type(5);
  if (existing_win) {
    existing_win->is_minimized = 0;
    extern window_t *active_window;
    active_window = existing_win;
    winmgr_bring_to_front(existing_win);
    fm_refresh();
    existing_win->needs_redraw = 1;
    extern int ui_dirty;
    ui_dirty = 1;
    return;
  }

  window_t *filemgr_win =
      winmgr_create_window(-1, -1, 720, 540, "File Manager");
  if (!filemgr_win)
    return;

  filemgr_win->draw = (void (*)(void *))filemgr_draw;
  filemgr_win->on_mouse = (void (*)(void *, int, int, int))filemgr_on_mouse;
  filemgr_win->on_key = (void (*)(void *, int, char))filemgr_on_key;
  filemgr_win->on_scroll = (void (*)(void *, int))filemgr_on_scroll;
  filemgr_win->on_copy = fm_on_copy;
  filemgr_win->on_cut = fm_on_cut;
  filemgr_win->on_paste = fm_on_paste;
  filemgr_win->on_close = filemgr_on_close;
  filemgr_win->bg_color = 0xFFFFFF; // White (32-bit)
  filemgr_win->app_type = 5;        // Files
  filemgr_win->blur_enabled = 0;

  // Start at "My Computer"
  at_root = 1;
  strcpy(current_path, "/");

  fm_refresh();
}
