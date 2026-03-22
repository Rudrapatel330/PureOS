// explorer.c - Windows "This PC" Style File Explorer
#include "../fs/fat.h"
#include "../fs/fs.h"
#include "../fs/vfs.h"
#include "../kernel/clipboard.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/window.h"

extern void print_serial(const char *);
extern int ui_dirty;

// ======================== STATE ========================
static window_t *explorer_win = NULL;
static int selected_index = -1;
static char explorer_path[128] = "/";
static int scroll_offset = 0;
static int at_this_pc = 1; // 1 = showing "This PC" root (C: drive view)

// Directory Cache for modern snappiness
#define EXPLORER_CACHE_SIZE 8
typedef struct {
  char path[128];
  FileInfo files[64];
  int count;
  uint32_t last_used;
  int valid;
} explorer_cache_t;

static explorer_cache_t dir_cache[EXPLORER_CACHE_SIZE];
static uint32_t explorer_access_counter = 0;
static int sidebar_sel = 0; // 0=This PC, 1=Docs, 2=Apps

#define SIDEBAR_WIDTH 120

// Double-click
static int prev_mouse_buttons = 0;

// Input dialog
static int dialog_active = 0; // 0=none, 1=new folder, 2=new file, 3=message
static char dialog_input[32] = {0};
static char popup_msg[64] = {0};
static int dialog_cursor = 0;

// Search State
typedef struct {
  char name[32];
  char full_path[160];
  int is_dir;
  int size;
} search_result_t;

#define MAX_SEARCH_RESULTS 64
static search_result_t search_results[MAX_SEARCH_RESULTS];
static int search_count = 0;
static char search_query[32] = {0};
static int search_cursor = 0;
static int search_active = 0;
static int search_focus = 0;

static void explorer_show_popup(const char *msg) {
  strcpy(popup_msg, msg);
  dialog_active = 3;
  extern int ui_dirty;
  ui_dirty = 1;
}

// Hover tracking for toolbar
static int hover_btn = -1;

// ======================== COLORS ========================
// iCloud Drive / Modern Light Theme
#define COL_TITLE_BG 0xFFEBEBEB   // Light Grey
#define COL_TOOLBAR_BG 0xFFFFFFFF // White
#define COL_ADDR_BG 0xFFFFFFFF
#define COL_ADDR_EDGE 0xFFD0D0D0
#define COL_ADDR_FIELD 0xFFFFFFFF
#define COL_CONTENT_BG 0xFFFFFFFF // Pure White
#define COL_SIDEBAR_BG 0xFFD3E8E8 // Light Teal
#define COL_STATUS_BG 0xFFF0F0F0
#define COL_SEL_BG 0xFFC2E0FF // Modern Blue Highlight
#define COL_SEL_BORDER 0xFF7FBFFF
#define COL_TEXT_BLACK 0xFF000000
#define COL_TEXT_DARK 0xFF333333
#define COL_TEXT_GRAY 0xFF777777
#define COL_DIVIDER 0xFFD0D0D0
#define COL_ICON_FOLDER 0xFFFFD15C // Manila Yellow
#define COL_ICON_FILE 0xFFF0F5FA   // Soft Blue-White Document
#define COL_CARD_BG 0xFFFFFFFF
#define COL_CARD_BORDER 0xFFD0D0D0
#define COL_TEXT_WHT 0xFFFFFFFF

// ======================== HELPERS ========================

static void explorer_go_up() {
  if (at_this_pc)
    return;
  if (strcmp(explorer_path, "/") == 0) {
    at_this_pc = 1;
    return;
  }
  char *last_slash = 0;
  for (int i = 0; explorer_path[i]; i++)
    if (explorer_path[i] == '/')
      last_slash = explorer_path + i;
  if (last_slash == explorer_path)
    explorer_path[1] = 0;
  else if (last_slash)
    *last_slash = 0;
}

static void build_display_path(char *out, int max) {
  // Convert /DOCS/PROJECTS to "This PC > C: > DOCS > PROJECTS"
  if (at_this_pc) {
    strcpy(out, "This PC");
    return;
  }
  strcpy(out, "This PC > C:");
  if (explorer_path[0] == '/' && explorer_path[1] == 0)
    strcpy(out, "This PC > C:");
  int start = strlen(out);
  char *p = explorer_path;
  if (*p == '/')
    p++;
  while (*p) {
    if (start < max - 4) {
      if (*p == '/') {
        strcat(out, " > ");
        start += 3;
      } else {
        out[start++] = *p;
        out[start] = 0;
      }
    }
    p++;
  }
}

// ======================== SEARCH ========================
static void explorer_search_recursive(const char *path, const char *query) {
  if (search_count >= MAX_SEARCH_RESULTS)
    return;

  int fd = vfs_open(path, 0); // O_RDONLY
  if (fd < 0)
    return;

  int i = 0;
  vfs_node_t *node;
  while ((node = vfs_readdir(fd, i++))) {
    if (strcmp(node->name, ".") == 0 || strcmp(node->name, "..") == 0) {
      kfree(node);
      continue;
    }

    // Match?
    if (strcasestr(node->name, query)) {
      search_result_t *res = &search_results[search_count++];
      strncpy(res->name, node->name, 31);
      res->name[31] = 0;
      res->is_dir = (node->flags & 2) ? 1 : 0;
      res->size = node->length;

      // Build full path
      strcpy(res->full_path, path);
      if (res->full_path[strlen(res->full_path) - 1] != '/')
        strcat(res->full_path, "/");
      strcat(res->full_path, node->name);

      if (search_count >= MAX_SEARCH_RESULTS) {
        kfree(node);
        break;
      }
    }

    // Recurse?
    if (node->flags & 2) {
      char sub[256];
      strcpy(sub, path);
      if (sub[strlen(sub) - 1] != '/')
        strcat(sub, "/");
      strcat(sub, node->name);
      kfree(node);
      explorer_search_recursive(sub, query);
    } else {
      kfree(node);
    }

    if (search_count >= MAX_SEARCH_RESULTS)
      break;
  }

  vfs_close(fd);
}

static void explorer_do_search() {
  if (strlen(search_query) < 1) {
    search_active = 0;
    return;
  }
  search_count = 0;
  search_active = 1;
  explorer_search_recursive("/", search_query);
  selected_index = -1;
  scroll_offset = 0;
  extern int ui_dirty;
  ui_dirty = 1;
}

// ======================== REFRESH ========================

void explorer_refresh() {
  if (at_this_pc) {
    selected_index = -1;
    scroll_offset = 0;
    if (explorer_win) {
      explorer_win->scroll_position = 0;
      explorer_win->needs_redraw = 1;
      ui_dirty = 1;
    }
    return;
  }

  // Check multi-dir cache
  int cache_slot = -1;
  int lru_slot = 0;
  uint32_t oldest = 0xFFFFFFFF;

  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (dir_cache[i].valid && strcmp(dir_cache[i].path, explorer_path) == 0) {
      cache_slot = i;
      break;
    }
    if (!dir_cache[i].valid) {
      lru_slot = i;
    } else if (dir_cache[i].last_used < oldest) {
      oldest = dir_cache[i].last_used;
      lru_slot = i;
    }
  }

  if (cache_slot != -1) {
    // Cache Hit
    dir_cache[cache_slot].last_used = ++explorer_access_counter;
  } else {
    // Cache Miss - read from disk
    cache_slot = lru_slot;
    dir_cache[cache_slot].count =
        fs_list_files(explorer_path, dir_cache[cache_slot].files, 64);
    strncpy(dir_cache[cache_slot].path, explorer_path, 127);
    dir_cache[cache_slot].path[127] = 0;
    dir_cache[cache_slot].valid = 1;
    dir_cache[cache_slot].last_used = ++explorer_access_counter;
  }

  selected_index = -1;
  scroll_offset = 0;
  if (explorer_win) {
    explorer_win->scroll_position = 0;
    explorer_win->needs_redraw = 1;
    ui_dirty = 1;
  }
}

// ======================== ICON DRAWING ========================

// Large folder icon (40x32)
static void draw_folder_icon_large(window_t *win, int x, int y) {
  // Manila folder look
  winmgr_fill_rect(win, x, y + 4, 14, 4, 0xFFE0C040);       // Darker tab
  winmgr_fill_rect(win, x, y + 8, 38, 24, COL_ICON_FOLDER); // Body
  winmgr_fill_rect(win, x + 1, y + 10, 36, 1, 0x40FFFFFF);  // Inner highlight
  winmgr_fill_rect(win, x, y + 8, 38, 1, 0x20000000);       // Top shadow
  winmgr_draw_rect(win, x, y + 8, 38, 24, 0xFFC0A030);      // Outline
}

// Large file icon (36x40)
static void draw_file_icon_large(window_t *win, int x, int y) {
  // Document body
  winmgr_fill_rect(win, x + 4, y, 30, 38, COL_ICON_FILE);
  winmgr_draw_rect(win, x + 4, y, 30, 38, 0xFFD0D0D0); // Outline
  // Folded corner
  winmgr_fill_rect(win, x + 26, y, 8, 8, 0xFFF8FBFF);
  winmgr_draw_rect(win, x + 26, y, 8, 8, 0xFFD0D0D0);
  // Text lines (subtle gray)
  winmgr_fill_rect(win, x + 10, y + 14, 18, 1, 0xFFCCCCCC);
  winmgr_fill_rect(win, x + 10, y + 20, 14, 1, 0xFFCCCCCC);
  winmgr_fill_rect(win, x + 10, y + 26, 16, 1, 0xFFCCCCCC);
}

static void draw_sidebar(window_t *win) {
  int h = win->height;
  // Solid light teal sidebar
  winmgr_fill_rect(win, 0, 24, SIDEBAR_WIDTH, h - 24, COL_SIDEBAR_BG);
  winmgr_fill_rect(win, SIDEBAR_WIDTH - 1, 24, 1, h - 24, COL_DIVIDER);

  const char *items[] = {"This PC", "All Files", "Texts (.txt)", "Images (.png)", "Apps (.app)"};
  for (int i = 0; i < 5; i++) {
    if (sidebar_sel == i) {
      winmgr_fill_rect(win, 5, 40 + i * 28, SIDEBAR_WIDTH - 10, 24,
                       0xFF99CCFF); // Opaque Blue selection
    }
    winmgr_draw_text(win, 10, 48 + i * 28, items[i], COL_TEXT_DARK);
  }
}

// ======================== DRAWING ========================

static void draw_navigation_bar(window_t *win) {
  int w = win->width;

  // Navigation bar background (Dark Frosted Gradient)
  // Modern Flat Light Navigation
  winmgr_fill_rect(win, 0, 24, w, 28, COL_TITLE_BG);
  winmgr_fill_rect(win, 0, 51, w, 1, COL_DIVIDER);

  // Back/Forward buttons mimicking macOS
  winmgr_draw_text(win, 15, 31, "<  >", COL_TEXT_DARK);

  // Address bar title
  char title[128];
  build_display_path(title, 128);
  int title_len = strlen(title);
  winmgr_draw_text(win, (w - title_len * 8) / 2, 31, title, COL_TEXT_BLACK);

  // Search Bar (Right Aligned)
  int sw = 150;
  int sx = w - sw - 10;
  winmgr_fill_rect(win, sx, 28, sw, 20,
                   search_focus ? COL_ADDR_BG : COL_TITLE_BG);
  winmgr_draw_rect(win, sx, 28, sw, 20, COL_ADDR_EDGE);

  if (strlen(search_query) == 0 && !search_focus) {
    winmgr_draw_text(win, sx + 5, 33, "Search files...", COL_TEXT_GRAY);
  } else {
    winmgr_draw_text(win, sx + 5, 33, search_query, COL_TEXT_BLACK);
    if (search_focus) {
      // Cursor
      int cur_x = sx + 5 + strlen(search_query) * 8;
      winmgr_fill_rect(win, cur_x, 33, 1, 12, COL_TEXT_BLACK);
    }
  }

  // Clear search button
  if (search_active) {
    winmgr_fill_rect(win, sx - 25, 28, 20, 20, 0xFFEE6666);
    winmgr_draw_text(win, sx - 19, 33, "X", COL_TEXT_WHT);
  }
}

// C: Drive icon with storage bar (Improved)
static void draw_drive_block(window_t *win, int x, int y, int w, int selected) {
  uint32_t bg = selected ? COL_SEL_BG : COL_CARD_BG;
  uint32_t border = selected ? COL_SEL_BORDER : COL_DIVIDER;

  // Card base with subtle shadow/gradient feel
  winmgr_fill_rect(win, x, y, w, 70, bg);
  winmgr_draw_rect(win, x, y, w, 70, border);

  // Drive label
  winmgr_draw_text(win, x + 15, y + 12, "Local Disk (C:)", COL_TEXT_DARK);

  // Storage bar (Modern Gradient Feel)
  int bar_x = x + 15;
  int bar_y = y + 32;
  int bar_w = w - 30;
  if (bar_w > 200)
    bar_w = 200;

  // Track
  winmgr_fill_rect(win, bar_x, bar_y, bar_w, 8, 0xFF1A1E24);
  // Progress (Gradient-like: Sakura Pink)
  int used = bar_w * 3 / 10;
  winmgr_fill_rect(win, bar_x, bar_y, used, 8, COL_ICON_FOLDER);

  // Size text
  winmgr_draw_text(win, bar_x, bar_y + 14, "7.2 GB free of 10 GB",
                   0xFF555555); // Darker gray for readability

  // "Manage" Button Simulation
  int mx = x + w - 70;
  int my = y + 10;
  winmgr_fill_rect(win, mx, my, 60, 20, 0xFFE0E0E0); // Opaque Light Gray
  winmgr_draw_text(win, mx + 8, my + 5, "Manage", COL_TEXT_DARK);
}

static void draw_this_pc_view(window_t *win) {
  int w = win->width;
  int content_x = SIDEBAR_WIDTH + 10;
  int content_y = 60;

  winmgr_draw_text(win, content_x, content_y, "Devices", COL_TEXT_BLACK);

  int drive_w = (w - content_x - 30);
  if (drive_w > 260)
    drive_w = 260;
  draw_drive_block(win, content_x, content_y + 25, drive_w,
                   (selected_index == 0));
}

static void draw_file_grid(window_t *win) {
  int w = win->width;
  int content_y = 78;
  int content_h = win->height - content_y - 24;

  int content_x = SIDEBAR_WIDTH;
  // Content background
  winmgr_fill_rect(win, content_x, content_y, w - content_x, content_h,
                   COL_CONTENT_BG);

  // Get current cache slot for this path
  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (dir_cache[i].valid && strcmp(dir_cache[i].path, explorer_path) == 0) {
      slot = i;
      break;
    }
  }

  if (slot == -1 || dir_cache[slot].count == 0) {
    win->max_scroll = 0;
    winmgr_draw_text(win, content_x + 20, content_y + 30,
                     "This folder is empty or not in cache", COL_TEXT_GRAY);
    return;
  }

  int f_count = dir_cache[slot].count;
  FileInfo *f_cache = dir_cache[slot].files;

  // Grid layout
  int start_x = content_x + 10;
  int start_y = content_y + 8;
  int cell_w = 110; // Increased from 80 to prevent overlap
  int cell_h = 90;  // Increased from 80
  int cols = (w - content_x - 10) / cell_w;
  if (cols < 1)
    cols = 1;

  int total_rows = (f_count + cols - 1) / cols;
  int max_scroll_pixels = total_rows * cell_h - (content_h - 10);
  if (max_scroll_pixels < 0)
    max_scroll_pixels = 0;
  win->max_scroll = max_scroll_pixels;

  for (int i = 0; i < f_count && i < 64; i++) {
    int col = i % cols;
    int row = i / cols;

    int fx = start_x + col * cell_w;
    int fy = start_y + row * cell_h - win->scroll_position;

    // Bounds check
    if (fy + cell_h < 78 || fy > win->height - 24)
      continue;

    // Skip . entry
    if (strcmp(f_cache[i].name, ".") == 0)
      continue;

    // Selection highlight
    if (i == selected_index) {
      winmgr_fill_rect(win, fx, fy, cell_w - 4, cell_h - 4, COL_SEL_BG);
      winmgr_draw_rect(win, fx, fy, cell_w - 4, cell_h - 4, COL_SEL_BORDER);
    }

    // Icon
    int icon_x = fx + (cell_w - 36) / 2;
    int icon_y = fy + 4;

    if (f_cache[i].is_dir) {
      if (strcmp(f_cache[i].name, "..") == 0) {
        // Up arrow for ..
        winmgr_fill_rect(win, icon_x + 8, icon_y + 8, 20, 20, COL_ICON_FOLDER);
        winmgr_draw_text(win, icon_x + 12, icon_y + 12, "..", COL_TEXT_BLACK);
      } else {
        draw_folder_icon_large(win, icon_x, icon_y);
      }
    } else {
      draw_file_icon_large(win, icon_x, icon_y);
    }

    // Name truncation logic
    char name_buf[16] = {0};
    strncpy(name_buf, f_cache[i].name, 11);
    if (strlen(f_cache[i].name) > 11) {
      name_buf[8] = '.';
      name_buf[9] = '.';
      name_buf[10] = '.';
      name_buf[11] = 0;
    }
    int name_len = strlen(name_buf);
    int tx = fx + (cell_w - 4 - name_len * 8) / 2;
    if (tx < fx)
      tx = fx;

    uint32_t name_col = (i == selected_index) ? COL_TEXT_BLACK : COL_TEXT_DARK;
    winmgr_draw_text(win, tx, fy + cell_h - 18, name_buf, name_col);
  }
}

static void draw_search_results(window_t *win) {
  int w = win->width;
  int content_y = 78;
  int content_h = win->height - content_y - 24;
  int content_x = SIDEBAR_WIDTH;

  winmgr_fill_rect(win, content_x, content_y, w - content_x, content_h,
                   COL_CONTENT_BG);

  if (search_count == 0) {
    winmgr_draw_text(win, content_x + 20, content_y + 30, "No results found.",
                     COL_TEXT_GRAY);
    return;
  }

  // Grid layout for results (similar to file_grid but uses search_results)
  int start_x = content_x + 10;
  int start_y = content_y + 8;
  int cell_w = 110;
  int cell_h = 90;
  int cols = (w - content_x - 10) / cell_w;
  if (cols < 1)
    cols = 1;

  int total_rows = (search_count + cols - 1) / cols;
  win->max_scroll =
      (total_rows * cell_h > content_h) ? total_rows * cell_h - content_h : 0;

  for (int i = 0; i < search_count; i++) {
    int col = i % cols;
    int row = i / cols;
    int fx = start_x + col * cell_w;
    int fy = start_y + row * cell_h - win->scroll_position;

    if (fy + cell_h < 78 || fy > win->height - 24)
      continue;

    if (i == selected_index) {
      winmgr_fill_rect(win, fx, fy, cell_w - 4, cell_h - 4, COL_SEL_BG);
      winmgr_draw_rect(win, fx, fy, cell_w - 4, cell_h - 4, COL_SEL_BORDER);
    }

    int icon_x = fx + (cell_w - 36) / 2;
    int icon_y = fy + 4;

    if (search_results[i].is_dir) {
      draw_folder_icon_large(win, icon_x, icon_y);
    } else {
      draw_file_icon_large(win, icon_x, icon_y);
    }

    char name_buf[16] = {0};
    strncpy(name_buf, search_results[i].name, 11);
    if (strlen(search_results[i].name) > 11) {
      strcpy(name_buf + 8, "...");
    }
    int name_len = strlen(name_buf);
    int tx = fx + (cell_w - 4 - name_len * 8) / 2;
    winmgr_draw_text(win, tx, fy + cell_h - 18, name_buf, COL_TEXT_DARK);
  }
}

static void draw_action_toolbar(window_t *win) {
  int w = win->width;
  int tx = SIDEBAR_WIDTH + 5;

  // Toolbar background
  winmgr_fill_rect(win, tx, 52, w - tx, 24, COL_TOOLBAR_BG);
  // Bottom divider
  winmgr_fill_rect(win, tx, 76, w - tx, 1, COL_DIVIDER);

  if (at_this_pc)
    return;

  // New Folder button
  uint32_t nf = (hover_btn == 10) ? COL_SEL_BG : 0x20FFFFFF;
  winmgr_fill_rect(win, tx + 6, 54, 60, 19, nf);
  winmgr_draw_text(win, tx + 10, 57, "+ Folder", COL_TEXT_DARK);

  // Refresh
  uint32_t ref = (hover_btn == 13) ? COL_SEL_BG : 0x20FFFFFF;
  winmgr_fill_rect(win, tx + 70, 54, 55, 19, ref);
  winmgr_draw_text(win, tx + 74, 57, "Refresh", COL_TEXT_DARK);
}

static void draw_status_bar(window_t *win) {
  int w = win->width;
  int sy = win->height - 24;

  winmgr_fill_rect(win, 0, sy, w, 24, COL_STATUS_BG);
  winmgr_fill_rect(win, 0, sy, w, 1, COL_DIVIDER);

  char status[128];
  if (at_this_pc) {
    strcpy(status, "1 drive connected");
  } else {
    // Get current cache slot for this path
    int slot = -1;
    for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
      if (dir_cache[i].valid && strcmp(dir_cache[i].path, explorer_path) == 0) {
        slot = i;
        break;
      }
    }
    int f_count = (slot != -1) ? dir_cache[slot].count : 0;

    char num[12];
    k_itoa(f_count, num);
    strcpy(status, num);
    strcat(status, " items found");
  }
  winmgr_draw_text(win, 10, sy + 6, status, COL_TEXT_DARK);
}

static void draw_input_dialog(window_t *win) {
  if (!dialog_active)
    return;

  int dw = 280, dh = 110;
  int dx = (win->width - dw) / 2;
  int dy = (win->height - dh) / 2;

  // Dialog background (Opaque)
  winmgr_fill_rect(win, dx, dy, dw, dh, 0xFF2C313A);
  winmgr_draw_rect(win, dx, dy, dw, dh, COL_SEL_BORDER);

  if (dialog_active == 3) {
    winmgr_draw_text(win, dx + 10, dy + 8, "Transfer Status", COL_TEXT_DARK);
    winmgr_draw_text(win, dx + 16, dy + 44, popup_msg, COL_TEXT_DARK);

    winmgr_fill_rect(win, dx + 105, dy + 75, 70, 24, 0xFF98C379); // Green
    winmgr_draw_text(win, dx + 130, dy + 81, "OK", COL_TEXT_DARK);
    // Draw white highlight text for better contrast against dark popup
    // background if needed, but we use dark here over pale? Wait, the popup is
    // 0xFF2C313A which is dark gray! Let's redraw text in white.
    winmgr_draw_text(win, dx + 10, dy + 8, "Transfer Status", 0xFFFFFFFF);
    winmgr_draw_text(win, dx + 16, dy + 44, popup_msg, 0xFFFFFFFF);
    return;
  }

  // Title
  const char *title = (dialog_active == 1) ? "New Folder" : "New File";
  winmgr_draw_text(win, dx + 10, dy + 8, title, 0xFFFFFFFF); // Fixed contrast

  // Input field
  winmgr_fill_rect(win, dx + 12, dy + 40, dw - 24, 22, COL_ADDR_FIELD);
  winmgr_draw_rect(win, dx + 12, dy + 40, dw - 24, 22, COL_ADDR_EDGE);
  winmgr_draw_text(win, dx + 16, dy + 44, dialog_input, COL_TEXT_DARK);

  // Buttons
  winmgr_fill_rect(win, dx + 60, dy + 75, 70, 24, 0xFF98C379); // Green
  winmgr_draw_text(win, dx + 85, dy + 81, "OK", COL_TEXT_DARK);

  winmgr_fill_rect(win, dx + 150, dy + 75, 70, 24, COL_SEL_BG); // Sakura
  winmgr_draw_text(win, dx + 162, dy + 81, "Cancel", COL_TEXT_DARK);
}

// ======================== MAIN DRAW ========================

void explorer_draw(window_t *win) {
  // Clear the entire window area to white (fixes black theme bug during
  // animations)
  winmgr_fill_rect(win, 0, 0, win->width, win->height, 0xFFFFFFFF);

  draw_sidebar(win);
  draw_navigation_bar(win);
  draw_action_toolbar(win);

  if (at_this_pc) {
    draw_this_pc_view(win);
  } else {
    if (search_active)
      draw_search_results(win);
    else
      draw_file_grid(win);
  }

  draw_status_bar(win);
  draw_input_dialog(win);
}

// ======================== FILE ACTIONS ========================

void explorer_open_file(int index) {
  char final_path[160];
  int is_dir = 0;

  if (search_active) {
    if (index < 0 || index >= search_count)
      return;
    strcpy(final_path, search_results[index].full_path);
    is_dir = search_results[index].is_dir;
  } else {
    int slot = -1;
    for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
      if (dir_cache[i].valid && strcmp(dir_cache[i].path, explorer_path) == 0) {
        slot = i;
        break;
      }
    }
    if (slot == -1 || index < 0 || index >= dir_cache[slot].count)
      return;

    char *name = dir_cache[slot].files[index].name;

    // . and ..
    if (strcmp(name, ".") == 0) {
      explorer_refresh();
      return;
    }
    if (strcmp(name, "..") == 0) {
      explorer_go_up();
      explorer_refresh();
      return;
    }

    is_dir = dir_cache[slot].files[index].is_dir;
    strcpy(final_path, (explorer_path[0] == 0) ? "/" : explorer_path);
    int path_len = strlen(final_path);
    if (path_len > 0 && final_path[path_len - 1] != '/')
      strcat(final_path, "/");
    strcat(final_path, name);
  }

  if (is_dir) {
    strcpy(explorer_path, final_path);
    search_active = 0; // Exit search when entering folder
    search_query[0] = 0;
    search_cursor = 0;
    at_this_pc = 0;
    explorer_refresh();
    return;
  }

  // Case-insensitive Extension Check
  char *name_only = final_path;
  for (int i = 0; final_path[i]; i++)
    if (final_path[i] == '/')
      name_only = &final_path[i + 1];

  extern void editor_open(const char *fn);
  int len = strlen(name_only);
  if (len > 4) {
    char *ext = name_only + len - 4;
    char ext_up[5];
    for (int k = 0; k < 4; k++) {
      char c = ext[k];
      ext_up[k] = (c >= 'a' && c <= 'z') ? c - 32 : c;
    }
    ext_up[4] = 0;

    if (strcmp(ext_up, ".RAW") == 0) {
      extern void paint_open(const char *);
      paint_open(final_path);
      return;
    } else if (strcmp(ext_up, ".PDF") == 0) {
      extern void pdfreader_open(const char *);
      pdfreader_open(final_path);
      return;
    } else if (strcmp(ext_up, ".PNG") == 0 || strcmp(ext_up, ".BMP") == 0) {
      extern void photos_open(const char *);
      photos_open(final_path);
      return;
    } else if (strcmp(ext_up, ".APP") == 0) {
      char base[24];
      strcpy(base, name_only);
      base[len - 4] = 0;
      if (strcmp(base, "TERMINAL") == 0) {
        extern void terminal_init();
        terminal_init();
      } else if (strcmp(base, "EDITOR") == 0) {
        extern void editor_init();
        editor_init();
      } else if (strcmp(base, "CALC") == 0) {
        extern void calculator_init();
        calculator_init();
      } else if (strcmp(base, "PAINT") == 0) {
        extern void paint_init();
        paint_init();
      } else if (strcmp(base, "TASKMGR") == 0) {
        extern void taskmgr_init();
        taskmgr_init();
      } else if (strcmp(base, "PDFREADER") == 0) {
        extern void pdfreader_init();
        pdfreader_init();
      }
      return;
    }
  }
  editor_open(final_path);
}

static void dialog_confirm(void) {
  if (dialog_cursor == 0) {
    dialog_active = 0;
    return;
  }

  char full_path[160];
  strcpy(full_path, explorer_path);
  if (full_path[strlen(full_path) - 1] != '/')
    strcat(full_path, "/");
  strcat(full_path, dialog_input);

  if (dialog_active == 1) {
    fs_mkdir(full_path);
  } else if (dialog_active == 2) {
    fs_write(full_path, (const uint8_t *)"", 0);
  }

  dialog_active = 0;
  dialog_input[0] = 0;
  dialog_cursor = 0;
  // Invalidate all caches after mutation
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
    dir_cache[i].valid = 0;
  explorer_refresh();
}

// ======================== SHORTCUTS ========================

static void explorer_on_copy(void *w) {
  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (dir_cache[i].valid && strcmp(dir_cache[i].path, explorer_path) == 0) {
      slot = i;
      break;
    }
  }
  if (at_this_pc || slot == -1 || selected_index < 0 ||
      selected_index >= dir_cache[slot].count)
    return;
  if (strcmp(dir_cache[slot].files[selected_index].name, ".") == 0 ||
      strcmp(dir_cache[slot].files[selected_index].name, "..") == 0)
    return;

  char full_path[160];
  strcpy(full_path, (explorer_path[0] == 0) ? "/" : explorer_path);
  if (full_path[strlen(full_path) - 1] != '/')
    strcat(full_path, "/");
  strcat(full_path, dir_cache[slot].files[selected_index].name);

  if (dir_cache[slot].files[selected_index].is_dir) {
    explorer_show_popup("Folders not supported");
    ((window_t *)w)->needs_redraw = 1;
    return;
  }

  clipboard_copy(full_path);
  clipboard_set_operation(CLIPBOARD_OP_COPY);

  char msg[64];
  strcpy(msg, "Copied ");
  strcat(msg, dir_cache[slot].files[selected_index].name);
  explorer_show_popup(msg);
  ((window_t *)w)->needs_redraw = 1;
}

static void explorer_on_cut(void *w) {
  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (dir_cache[i].valid && strcmp(dir_cache[i].path, explorer_path) == 0) {
      slot = i;
      break;
    }
  }
  if (at_this_pc || slot == -1 || selected_index < 0 ||
      selected_index >= dir_cache[slot].count)
    return;
  if (strcmp(dir_cache[slot].files[selected_index].name, ".") == 0 ||
      strcmp(dir_cache[slot].files[selected_index].name, "..") == 0)
    return;

  char full_path[160];
  strcpy(full_path, (explorer_path[0] == 0) ? "/" : explorer_path);
  if (full_path[strlen(full_path) - 1] != '/')
    strcat(full_path, "/");
  strcat(full_path, dir_cache[slot].files[selected_index].name);

  if (dir_cache[slot].files[selected_index].is_dir) {
    explorer_show_popup("Folders not supported");
    ((window_t *)w)->needs_redraw = 1;
    return;
  }

  clipboard_copy(full_path);
  clipboard_set_operation(CLIPBOARD_OP_CUT);

  char msg[64];
  strcpy(msg, "Cut ");
  strcat(msg, dir_cache[slot].files[selected_index].name);
  explorer_show_popup(msg);
  ((window_t *)w)->needs_redraw = 1;
}

static void explorer_on_paste(void *w, const char *path) {
  if (at_this_pc || !path || path[0] == 0)
    return;

  const char *filename = path;
  for (int i = 0; path[i]; i++) {
    if (path[i] == '/')
      filename = &path[i + 1];
  }

  char dest_path[160];
  strcpy(dest_path, (explorer_path[0] == 0) ? "/" : explorer_path);
  if (dest_path[strlen(dest_path) - 1] != '/')
    strcat(dest_path, "/");
  strcat(dest_path, filename);

  if (strcmp(path, dest_path) == 0) {
    explorer_show_popup("Error: Same location!");
    ((window_t *)w)->needs_redraw = 1;
    return;
  }

  int op = clipboard_get_operation();
  char msg[64] = "Transfer failed!";

  print_serial("EXPLORER PASTE: Src='");
  print_serial(path);
  print_serial("' Dest='");
  print_serial(dest_path);
  print_serial("'\n");

  if (op == CLIPBOARD_OP_COPY) {
    int res = fat_copy_file(path, dest_path);
    print_serial("EXPLORER PASTE: fat_copy_file returned ");
    char nb[12];
    k_itoa(res, nb);
    print_serial(nb);
    print_serial("\n");
    if (res == 0) {
      strcpy(msg, "Copy completed!");
    }
  } else if (op == CLIPBOARD_OP_CUT) {
    int res = fat_move_file(path, dest_path);
    print_serial("EXPLORER PASTE: fat_move_file returned ");
    char nb[12];
    k_itoa(res, nb);
    print_serial(nb);
    print_serial("\n");
    if (res == 0) {
      strcpy(msg, "Move completed!");
      clipboard_set_operation(CLIPBOARD_OP_NONE);
    }
  } else {
    strcpy(msg, "Nothing to paste.");
  }

  explorer_show_popup(msg);
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
    dir_cache[i].valid = 0;
  explorer_refresh();
  ((window_t *)w)->needs_redraw = 1;
}

// ======================== INPUT HANDLERS ========================

void explorer_handle_key(window_t *win, int key, char ascii) {
  if (dialog_active == 3) {
    if (key == 0x1C || key == 0x01 || ascii == '\n') {
      dialog_active = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  if (dialog_active) {
    if (key == 0x1C) {
      dialog_confirm();
      win->needs_redraw = 1;
      return;
    }
    if (key == 0x01) {
      dialog_active = 0;
      dialog_input[0] = 0;
      dialog_cursor = 0;
      win->needs_redraw = 1;
      return;
    }
    if (key == 0x0E && dialog_cursor > 0) {
      dialog_input[--dialog_cursor] = 0;
      win->needs_redraw = 1;
      return;
    }
    if (ascii >= 32 && ascii < 127 && dialog_cursor < 24) {
      dialog_input[dialog_cursor++] =
          (ascii >= 'a' && ascii <= 'z') ? ascii - 32 : ascii;
      dialog_input[dialog_cursor] = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  if (key == 0x1E) { // 'A' key - test for redraw issue
    print_serial("EXPLORER: Manually triggering redraw\n");
    win->needs_redraw = 1;
  }

  if (key == 0x0E || ascii == '\b') {
    if (search_focus) {
      if (search_cursor > 0) {
        search_query[--search_cursor] = 0;
        explorer_do_search();
      }
      return;
    }
    if (!at_this_pc) {
      explorer_go_up();
      explorer_refresh();
    }
    if (search_focus) {
      explorer_do_search();
      search_focus = 0;
      return;
    }
    if (at_this_pc && selected_index == 0) {
      at_this_pc = 0;
      strcpy(explorer_path, "/");
      explorer_refresh();
    } else if (!at_this_pc && selected_index >= 0) {
      explorer_open_file(selected_index);
    }
  } else if (search_focus && ascii >= 32 && ascii < 127 && search_cursor < 31) {
    search_query[search_cursor++] = ascii;
    search_query[search_cursor] = 0;
    explorer_do_search(); // Instant search
    win->needs_redraw = 1;
  }
}

void explorer_on_scroll(window_t *win, int direction) {
  if (at_this_pc)
    return;
  // Wheel UP (direction > 0) -> scroll viewport UP -> decrease scroll_position
  // Move 60 pixels per notch for smooth but snappy feel
  int new_pos = win->scroll_position - direction * 60;
  if (new_pos < 0)
    new_pos = 0;
  if (new_pos > win->max_scroll)
    new_pos = win->max_scroll;

  if (new_pos != win->scroll_position) {
    win->scroll_position = new_pos;
    win->needs_redraw = 1;
    ui_dirty = 1;
  }
}

void explorer_handle_mouse(window_t *win, int mx, int my, int buttons) {
  // Rising edge detection
  if (!(buttons & 1)) {
    prev_mouse_buttons = 0;
    return;
  }
  if (prev_mouse_buttons)
    return;
  prev_mouse_buttons = 1;

  // mx, my are already window-relative (WM subtracts win->x/y)
  int rx = mx;
  int ry = my;

  if (dialog_active == 3) {
    int dw = 280, dh = 110;
    int dx = (win->width - dw) / 2;
    int dy = (win->height - dh) / 2;
    if (rx >= dx + 105 && rx < dx + 175 && ry >= dy + 75 && ry < dy + 99) {
      dialog_active = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  if (dialog_active) {
    int dw = 280, dh = 110;
    int dx = (win->width - dw) / 2;
    int dy = (win->height - dh) / 2;

    // OK button
    if (rx >= dx + 60 && rx < dx + 130 && ry >= dy + 75 && ry < dy + 99) {
      dialog_confirm();
      win->needs_redraw = 1;
    }
    // Cancel button
    else if (rx >= dx + 150 && rx < dx + 220 && ry >= dy + 75 && ry < dy + 99) {
      dialog_active = 0;
      dialog_input[0] = 0;
      dialog_cursor = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  // Sidebar hit test (items are 28px tall, drawn starting at y=40)
  if (rx < SIDEBAR_WIDTH && ry >= 40 && ry < 40 + 5 * 28) {
    sidebar_sel = (ry - 40) / 28;
    win->needs_redraw = 1;

    if (sidebar_sel == 0) {
      at_this_pc = 1;
      search_active = 0;
      explorer_refresh();
    } else if (sidebar_sel == 1) {
      at_this_pc = 0;
      search_active = 0;
      strcpy(explorer_path, "/");
      explorer_refresh();
    } else if (sidebar_sel == 2) {
      at_this_pc = 0;
      strcpy(search_query, ".txt");
      explorer_do_search();
    } else if (sidebar_sel == 3) {
      at_this_pc = 0;
      strcpy(search_query, ".png");
      explorer_do_search();
    } else if (sidebar_sel == 4) {
      at_this_pc = 0;
      strcpy(search_query, ".app");
      explorer_do_search();
    }
    return;
  }

  // Navigation bar (y: 24-52)
  if (ry >= 27 && ry < 49) {
    // Back button (x: 4-32)
    if (rx >= 4 && rx < 32 && !at_this_pc) {
      explorer_go_up();
      explorer_refresh();
      search_active = 0; // Exit search on back
    }

    // Search Box Hit Test (sw=150, pos from right)
    int sw = 150;
    int sx = win->width - sw - 10;
    if (rx >= sx && rx < sx + sw) {
      search_focus = 1;
      dialog_active = 0; // Close other dialogs
      win->needs_redraw = 1;
    } else {
      search_focus = 0;
      win->needs_redraw = 1;
    }

    // Clear Button Hit Test
    if (search_active && rx >= sx - 25 && rx < sx - 5) {
      search_active = 0;
      search_query[0] = 0;
      search_cursor = 0;
      sidebar_sel = 1; // switch to "All Files"
      strcpy(explorer_path, "/");
      explorer_refresh();
      win->needs_redraw = 1;
    }

    return;
  }

  // Action toolbar (y: 52-76, only when not at This PC)
  int tx = SIDEBAR_WIDTH + 5;
  if (!at_this_pc && ry >= 54 && ry < 73) {
    if (rx >= tx + 6 && rx < tx + 66) { // + Folder
      dialog_active = 1;
      dialog_input[0] = 0;
      dialog_cursor = 0;
      win->needs_redraw = 1;
    } else if (rx >= tx + 70 && rx < tx + 125) { // Refresh
      // Invalidate all caches
      for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
        dir_cache[i].valid = 0;
      explorer_refresh();
    }
    return;
  }

  // Content area
  int content_x = SIDEBAR_WIDTH + 10;
  int content_y = 60;
  int content_h = win->height - 78 - 24;
  if (rx >= content_x && ry >= content_y) {
    if (at_this_pc) {
      // Check if C: drive was clicked (Simplified & Robust)
      if (rx >= SIDEBAR_WIDTH + 5 && rx < win->width - 10 && ry >= 75 &&
          ry < 165) {
        selected_index = 0;
        at_this_pc = 0;
        strcpy(explorer_path, "/");
        explorer_refresh();
        win->needs_redraw = 1;
      }

    } else {
      // Grid hit test using content_x offset
      int cell_w = 110, cell_h = 90;
      int cols = (win->width - content_x - 10) / cell_w;
      if (cols < 1)
        cols = 1;
      int grid_x = rx - content_x;
      int grid_y =
          ry - (78 + 8) + win->scroll_position; // add scroll offset back
      if (grid_y >= 0 && ry < 78 + content_h) { // ensure within content area
        int col = grid_x / cell_w;
        int row = grid_y / cell_h;
        int idx = row * cols + col;

        int slot = -1;
        for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
          if (dir_cache[i].valid &&
              strcmp(dir_cache[i].path, explorer_path) == 0) {
            slot = i;
            break;
          }
        }

        if (slot != -1 && idx >= 0 && idx < dir_cache[slot].count) {
          if (selected_index == idx)
            explorer_open_file(idx);
          else
            selected_index = idx;
          win->needs_redraw = 1;
        }
      }
    }
  }
}

static void explorer_on_close(void *w) {
  (void)w;
  explorer_win = 0;
}

void explorer_init() {
  window_t *existing_win = winmgr_get_window_by_app_type(5);
  if (existing_win) {
    existing_win->is_minimized = 0;
    extern window_t *active_window;
    active_window = existing_win;
    winmgr_bring_to_front(existing_win);
    explorer_refresh();
    existing_win->needs_redraw = 1;
    return;
  }

  explorer_win = winmgr_create_window(100, 60, 480, 420, "Explorer");
  if (!explorer_win) {
    print_serial("EXPLORER: Failed to create window (OOM)\n");
    return;
  }
  explorer_win->draw = (void (*)(void *))explorer_draw;
  explorer_win->on_mouse =
      (void (*)(void *, int, int, int))explorer_handle_mouse;
  explorer_win->on_key = (void (*)(void *, int, char))explorer_handle_key;
  explorer_win->on_scroll = (void (*)(void *, int))explorer_on_scroll;
  explorer_win->on_copy = explorer_on_copy;
  explorer_win->on_cut = explorer_on_cut;
  explorer_win->on_paste = explorer_on_paste;
  explorer_win->on_close = explorer_on_close;
  explorer_win->bg_color = 0xFFFFFFFF; // Pure White
  explorer_win->app_type = 5;
  explorer_win->blur_enabled = 0;        // Opaque
  explorer_win->scroll_line_height = 30; // smooth scroll

  // Reset state
  at_this_pc = 1;
  strcpy(explorer_path, "/");
  dialog_active = 0;

  explorer_refresh();
  explorer_win->needs_redraw = 1;
}
