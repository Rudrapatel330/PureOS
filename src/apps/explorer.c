// explorer.c - Windows "This PC" Style File Explorer
#include "../fs/fat.h"
#include "../fs/fs.h"
#include "../fs/vfs.h"
#include "../gui/explorer_icons.h"
#include "../kernel/clipboard.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"
<<<<<<< HEAD
=======
#include "../gui/explorer_icons.h"
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
#include "../ui/ctxmenu.h"
#include <stdint.h>
#include <string.h>

extern window_t *active_window;
extern int fat_mkdir(const char *path);
extern void print_serial(const char *);
extern int ui_dirty;

// ======================== STATE ========================
#define EXPLORER_CACHE_SIZE 8
typedef struct {
  char path[128];
  FileInfo files[64];
  int count;
  uint32_t last_used;
  int valid;
} explorer_cache_t;

typedef struct {
  char name[32];
  char full_path[160];
  int is_dir;
  int size;
} search_result_t;

#define MAX_SEARCH_RESULTS 64
#define SIDEBAR_WIDTH 180 // Wider sidebar for cloud/tags

typedef struct {
  window_t *win;
  int selected_index;
  char explorer_path[128];
  int scroll_offset;
  int at_this_pc;
  explorer_cache_t dir_cache[EXPLORER_CACHE_SIZE];
  uint32_t explorer_access_counter;
  int sidebar_sel;
  int prev_mouse_buttons;
  int dialog_active;
  char dialog_input[32];
  char popup_msg[64];
  int dialog_cursor;
  search_result_t search_results[MAX_SEARCH_RESULTS];
  int search_count;
  char search_query[32];
  int search_cursor;
  int search_active;
  int search_focus;
  int hover_btn;
  int hovered_index;
  int hovered_sidebar;

  // Pinned Folders
  char pinned_paths[8][128];
  char pinned_names[8][32];
  int pinned_count;
<<<<<<< HEAD

=======
  
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  // Added Features
  int sort_mode; // 0 = Name, 1 = Size
} explorer_app_t;

// Forward Declarations
void explorer_refresh(window_t *win);
static void explorer_show_popup(explorer_app_t *app, const char *msg);
static void action_pin_selected(void);
static void action_copy_selected(void);
static void action_cut_selected(void);
static void action_paste_here(void);
static void action_delete_selected(void);
static void action_new_folder(void);
static void action_rename_selected(void);
static void action_refresh_view(void);

static inline explorer_app_t *get_explorer_app(void *w) {
  if (!w)
    return NULL;
  return (explorer_app_t *)((window_t *)w)->user_data;
}

static void explorer_show_popup(explorer_app_t *app, const char *msg) {
  if (!app)
    return;
  strcpy(app->popup_msg, msg);
  app->dialog_active = 3;
  extern int ui_dirty;
  ui_dirty = 1;
}

// Hover tracking for toolbar
static int hover_btn = -1;

// ======================== COLORS (AURA DARK THEME) ========================
<<<<<<< HEAD
#define COL_APP_BG 0xFF161616     // Deep dark, almost black
#define COL_SIDEBAR_BG 0xFF1A1A1A // Slightly lighter than app bg
#define COL_CONTENT_BG 0xFF1E1E1E // Content area bg
#define COL_CARD_BG 0xFF252525    // File card bg
#define COL_CARD_HOVER 0xFF303030 // File card hover
#define COL_CARD_SEL 0xFF3A3A3A   // File card selected
#define COL_ACCENT 0xFF2F82E2     // macOS blue accent
#define COL_TEXT_WHT 0xFFFFFFFF   // Primary text
#define COL_TEXT_MUTED 0xFF8B8F96 // Secondary text
#define COL_TEXT_DARK 0xFFD0D0D0
#define COL_DIVIDER 0xFF2A2A2A // Subtle borders
#define COL_ADDR_BG 0xFF2A2A2A // Search/Address bar
#define COL_ADDR_FIELD 0xFF2A2A2A
#define COL_ADDR_EDGE 0xFF404040
#define COL_SEL_BORDER 0xFF4A90D9
=======
#define COL_APP_BG       0xFF161616 // Deep dark, almost black
#define COL_SIDEBAR_BG   0xFF1A1A1A // Slightly lighter than app bg
#define COL_CONTENT_BG   0xFF1E1E1E // Content area bg
#define COL_CARD_BG      0xFF252525 // File card bg
#define COL_CARD_HOVER   0xFF303030 // File card hover
#define COL_CARD_SEL     0xFF3A3A3A // File card selected
#define COL_ACCENT       0xFF2F82E2 // macOS blue accent
#define COL_TEXT_WHT     0xFFFFFFFF // Primary text
#define COL_TEXT_MUTED   0xFF8B8F96 // Secondary text
#define COL_TEXT_DARK    0xFFD0D0D0
#define COL_DIVIDER      0xFF2A2A2A // Subtle borders
#define COL_ADDR_BG      0xFF2A2A2A // Search/Address bar
#define COL_ADDR_FIELD   0xFF2A2A2A
#define COL_ADDR_EDGE    0xFF404040
#define COL_SEL_BORDER   0xFF4A90D9
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

// ======================== HELPERS ========================

static void explorer_go_up(explorer_app_t *app) {
  if (strcmp(app->explorer_path, "/") == 0)
    return;
  char *last_slash = 0;
  for (int i = 0; app->explorer_path[i]; i++)
    if (app->explorer_path[i] == '/')
      last_slash = app->explorer_path + i;
  if (last_slash == app->explorer_path)
    app->explorer_path[1] = 0;
  else if (last_slash)
    *last_slash = 0;
}

static void build_display_path(explorer_app_t *app, char *out, int max) {
<<<<<<< HEAD
=======
  // Convert /DOCS/PROJECTS to "Documents > Projects"
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  if (app->at_this_pc) {
    strcpy(out, "This PC");
    return;
  }
<<<<<<< HEAD

  strcpy(out, "Home");
  if (app->explorer_path[0] == '/' && app->explorer_path[1] == 0)
    return;

  strcat(out, " > ");
=======
  strcpy(out, "Documents"); // Assuming base is documents for the mockup style
  if (app->explorer_path[0] == '/' && app->explorer_path[1] == 0)
    return;
    
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  int start = strlen(out);
  char *p = app->explorer_path;
  if (*p == '/')
    p++;
  while (*p) {
    if (start < max - 4) {
      if (*p == '/') {
        strcat(out, " > ");
        start = strlen(out);
      } else {
        out[start++] = *p;
        out[start] = 0;
      }
    }
    p++;
  }
}

// ======================== PINNING ========================

<<<<<<< HEAD
static void explorer_pin_folder(explorer_app_t *app, const char *name,
                                const char *path) {
  if (app->pinned_count >= 8)
    return;
  // Check if already pinned
  for (int i = 0; i < app->pinned_count; i++) {
    if (strcmp(app->pinned_paths[i], path) == 0)
      return;
=======
static void explorer_pin_folder(explorer_app_t *app, const char *name, const char *path) {
  if (app->pinned_count >= 8) return;
  // Check if already pinned
  for (int i = 0; i < app->pinned_count; i++) {
    if (strcmp(app->pinned_paths[i], path) == 0) return;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  }
  strcpy(app->pinned_names[app->pinned_count], name);
  strcpy(app->pinned_paths[app->pinned_count], path);
  app->pinned_count++;
}

static void action_pin_selected(void) {
  // Find current explorer app
  window_t *win = active_window;
  explorer_app_t *app = get_explorer_app(win);
<<<<<<< HEAD
  if (!app || app->selected_index < 0)
    return;

  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid &&
        strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i;
      break;
    }
  }
  if (slot == -1)
    return;
=======
  if (!app || app->selected_index < 0) return;

  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid && strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i; break;
    }
  }
  if (slot == -1) return;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

  FileInfo *fi = &app->dir_cache[slot].files[app->selected_index];
  if (!fi->is_dir) {
    explorer_show_popup(app, "Only folders can be pinned");
    return;
  }

  char full_path[160];
  strcpy(full_path, app->explorer_path);
<<<<<<< HEAD
  if (full_path[strlen(full_path) - 1] != '/')
    strcat(full_path, "/");
=======
  if (full_path[strlen(full_path)-1] != '/') strcat(full_path, "/");
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  strcat(full_path, fi->name);

  explorer_pin_folder(app, fi->name, full_path);
  win->needs_redraw = 1;
}

static void action_copy_selected(void) {
  window_t *win = active_window;
<<<<<<< HEAD
  if (win && win->on_copy)
    win->on_copy(win);
=======
  if (win && win->on_copy) win->on_copy(win);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
}

static void action_cut_selected(void) {
  window_t *win = active_window;
<<<<<<< HEAD
  if (win && win->on_cut)
    win->on_cut(win);
=======
  if (win && win->on_cut) win->on_cut(win);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
}

static void action_paste_here(void) {
  window_t *win = active_window;
<<<<<<< HEAD
  if (win && win->on_paste)
    win->on_paste(win, clipboard_paste());
=======
  if (win && win->on_paste) win->on_paste(win, clipboard_paste());
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
}

static void action_delete_selected(void) {
  window_t *win = active_window;
  explorer_app_t *app = get_explorer_app(win);
<<<<<<< HEAD
  if (!app || app->selected_index < 0)
    return;

  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid &&
        strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i;
      break;
    }
  }
  if (slot == -1)
    return;
=======
  if (!app || app->selected_index < 0) return;
  
  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid && strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i; break;
    }
  }
  if (slot == -1) return;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

  FileInfo *fi = &app->dir_cache[slot].files[app->selected_index];
  char full_path[160];
  strcpy(full_path, (app->explorer_path[0] == 0) ? "/" : app->explorer_path);
  if (full_path[strlen(full_path) - 1] != '/')
    strcat(full_path, "/");
  strcat(full_path, fi->name);

<<<<<<< HEAD
  print_serial("EXPLORER DELETE: path='");
  print_serial(full_path);
  print_serial("'\n");
  int res = fat_delete_recursive(full_path);
  print_serial("EXPLORER DELETE: result=");
  char rb[12];
  k_itoa(res, rb);
  print_serial(rb);
  print_serial("\n");
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
    app->dir_cache[i].valid = 0;
=======
  print_serial("EXPLORER DELETE: path='"); print_serial(full_path); print_serial("'\n");
  int res = fat_delete_recursive(full_path);
  print_serial("EXPLORER DELETE: result=");
  char rb[12]; k_itoa(res, rb); print_serial(rb); print_serial("\n");
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) app->dir_cache[i].valid = 0;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  explorer_refresh(win);
  win->needs_redraw = 1;
}

static void action_new_folder(void) {
  window_t *win = active_window;
  explorer_app_t *app = get_explorer_app(win);
<<<<<<< HEAD
  if (!app)
    return;
=======
  if (!app) return;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  app->dialog_active = 1;
  app->dialog_input[0] = 0;
  app->dialog_cursor = 0;
  win->needs_redraw = 1;
}

static void action_refresh_view(void) {
  window_t *win = active_window;
  if (win) {
    explorer_app_t *app = get_explorer_app(win);
    if (app) {
<<<<<<< HEAD
      for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
        app->dir_cache[i].valid = 0;
=======
      for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) app->dir_cache[i].valid = 0;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      explorer_refresh(win);
    }
  }
}

static void action_rename_selected(void) {
  window_t *win = active_window;
  explorer_app_t *app = get_explorer_app(win);
<<<<<<< HEAD
  if (!app)
    return;

  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid &&
        strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i;
      break;
    }
  }
  if (slot == -1 || app->selected_index < 0)
    return;

  app->dialog_active = 4;
  strcpy(app->dialog_input,
         app->dir_cache[slot].files[app->selected_index].name);
=======
  if (!app) return;
  
  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid && strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i; break;
    }
  }
  if (slot == -1 || app->selected_index < 0) return;
  
  app->dialog_active = 4;
  strcpy(app->dialog_input, app->dir_cache[slot].files[app->selected_index].name);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  app->dialog_cursor = strlen(app->dialog_input);
  win->needs_redraw = 1;
}

static void explorer_sort_cache(explorer_app_t *app, int slot) {
<<<<<<< HEAD
  if (slot < 0 || slot >= EXPLORER_CACHE_SIZE)
    return;
  int count = app->dir_cache[slot].count;
  FileInfo *files = app->dir_cache[slot].files;

  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      int swap = 0;
      if (files[j].is_dir != files[j + 1].is_dir) {
        swap = files[j].is_dir ? 0 : 1;
      } else {
        if (app->sort_mode == 0) { // By name
          swap = (strcmp(files[j].name, files[j + 1].name) > 0);
        } else { // By size
          swap = (files[j].size < files[j + 1].size);
=======
  if (slot < 0 || slot >= EXPLORER_CACHE_SIZE) return;
  int count = app->dir_cache[slot].count;
  FileInfo *files = app->dir_cache[slot].files;
  
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      int swap = 0;
      if (files[j].is_dir != files[j+1].is_dir) {
        swap = files[j].is_dir ? 0 : 1;
      } else {
        if (app->sort_mode == 0) { // By name
          swap = (strcmp(files[j].name, files[j+1].name) > 0);
        } else { // By size
          swap = (files[j].size < files[j+1].size);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
        }
      }
      if (swap) {
        FileInfo tmp = files[j];
<<<<<<< HEAD
        files[j] = files[j + 1];
        files[j + 1] = tmp;
=======
        files[j] = files[j+1];
        files[j+1] = tmp;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      }
    }
  }
}

// ======================== SEARCH ========================
static void explorer_search_recursive(explorer_app_t *app, const char *path,
                                      const char *query) {
  if (app->search_count >= MAX_SEARCH_RESULTS)
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
      search_result_t *res = &app->search_results[app->search_count++];
      strncpy(res->name, node->name, 31);
      res->name[31] = 0;
      res->is_dir = (node->flags & 2) ? 1 : 0;
      res->size = node->length;

      // Build full path
      strcpy(res->full_path, path);
      if (res->full_path[strlen(res->full_path) - 1] != '/')
        strcat(res->full_path, "/");
      strcat(res->full_path, node->name);

      if (app->search_count >= MAX_SEARCH_RESULTS) {
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
      explorer_search_recursive(app, sub, query);
    } else {
      kfree(node);
    }

    if (app->search_count >= MAX_SEARCH_RESULTS)
      break;
  }

  vfs_close(fd);
}

static void explorer_do_search(explorer_app_t *app) {
  if (strlen(app->search_query) < 1) {
    app->search_active = 0;
    return;
  }
  app->search_count = 0;
  app->search_active = 1;
  explorer_search_recursive(app, "/", app->search_query);
  app->selected_index = -1;
  app->scroll_offset = 0;
  extern int ui_dirty;
  ui_dirty = 1;
}

// ======================== REFRESH ========================

void explorer_refresh(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;

  if (app->at_this_pc) {
    app->selected_index = -1;
    app->scroll_offset = 0;
    win->scroll_position = 0;
    win->needs_redraw = 1;
    ui_dirty = 1;
    return;
  }

  // Check multi-dir cache
  int cache_slot = -1;
  int lru_slot = 0;
  uint32_t oldest = 0xFFFFFFFF;

  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid &&
        strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      cache_slot = i;
      break;
    }
    if (!app->dir_cache[i].valid) {
      lru_slot = i;
    } else if (app->dir_cache[i].last_used < oldest) {
      oldest = app->dir_cache[i].last_used;
      lru_slot = i;
    }
  }

  if (cache_slot != -1) {
    // Cache Hit
    app->dir_cache[cache_slot].last_used = ++app->explorer_access_counter;
  } else {
    // Cache Miss - read from disk
    cache_slot = lru_slot;
    app->dir_cache[cache_slot].count =
        fs_list_files(app->explorer_path, app->dir_cache[cache_slot].files, 64);
    strncpy(app->dir_cache[cache_slot].path, app->explorer_path, 127);
    app->dir_cache[cache_slot].path[127] = 0;
    app->dir_cache[cache_slot].valid = 1;
    app->dir_cache[cache_slot].last_used = ++app->explorer_access_counter;
  }
  
  explorer_sort_cache(app, cache_slot);

  explorer_sort_cache(app, cache_slot);

  app->selected_index = -1;
  app->scroll_offset = 0;
  win->scroll_position = 0;
  win->needs_redraw = 1;
  ui_dirty = 1;
}

// ======================== DRAWING UI ========================

static void draw_sidebar(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;
  int h = win->height;
<<<<<<< HEAD

  // Solid dark sidebar
  winmgr_fill_rect(win, 0, 0, SIDEBAR_WIDTH, h, COL_SIDEBAR_BG);
  winmgr_fill_rect(win, SIDEBAR_WIDTH - 1, 0, 1, h, COL_DIVIDER);

  struct {
    const char *section;
    const char *items[4];
    int count;
  } menu[] = {
      {"Quick Access", {"Recents", "Downloads", "Desktop", "Documents"}, 4},
      {"My Storage", {"Local Disk (C:)", "External Drive", 0, 0}, 2},
      {"Cloud", {"iCloud Drive", "Google Drive", 0, 0}, 2}};

  int y = 50;
  int sel_idx = 0;

  for (int s = 0; s < 3; s++) {
    winmgr_draw_text(win, 15, y, menu[s].section, COL_TEXT_MUTED);
    y += 20;

    for (int i = 0; i < menu[s].count; i++) {
      if (sel_idx == app->sidebar_sel) {
        winmgr_fill_rect(win, 8, y - 4, SIDEBAR_WIDTH - 16, 24, COL_CARD_SEL);
      } else if (sel_idx == app->hovered_sidebar) {
        winmgr_fill_rect(win, 8, y - 4, SIDEBAR_WIDTH - 16, 24, COL_CARD_HOVER);
      }

      winmgr_draw_text(win, 25, y + 2, menu[s].items[i], COL_TEXT_WHT);
      y += 28;
      sel_idx++;
=======
  
  // Solid dark sidebar
  winmgr_fill_rect(win, 0, 0, SIDEBAR_WIDTH, h, COL_SIDEBAR_BG);
  winmgr_fill_rect(win, SIDEBAR_WIDTH - 1, 0, 1, h, COL_DIVIDER);

  struct { const char* section; const char* items[4]; int count; } menu[] = {
    {"Quick Access", {"Recents", "Downloads", "Desktop", "Documents"}, 4},
    {"My Storage", {"Local Disk (C:)", "External Drive", 0, 0}, 2},
    {"Cloud", {"iCloud Drive", "Google Drive", 0, 0}, 2}
  };

  int y = 50;
  int sel_idx = 0;
  
  for (int s = 0; s < 3; s++) {
    winmgr_draw_text(win, 15, y, menu[s].section, COL_TEXT_MUTED);
    y += 20;
    
    for (int i = 0; i < menu[s].count; i++) {
        if (sel_idx == app->sidebar_sel) {
            winmgr_fill_rect(win, 8, y - 4, SIDEBAR_WIDTH - 16, 24, COL_CARD_SEL);
        } else if (sel_idx == app->hovered_sidebar) {
            winmgr_fill_rect(win, 8, y - 4, SIDEBAR_WIDTH - 16, 24, COL_CARD_HOVER);
        }
        
        winmgr_draw_text(win, 25, y + 2, menu[s].items[i], COL_TEXT_WHT);
        y += 28;
        sel_idx++;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    }
    y += 10;
  }

  // Dynamic Pinned Folders
  winmgr_draw_text(win, 15, y, "Pinned", COL_TEXT_MUTED);
  y += 20;
  for (int i = 0; i < app->pinned_count; i++) {
<<<<<<< HEAD
    if (sel_idx == app->sidebar_sel) {
      winmgr_fill_rect(win, 8, y - 4, SIDEBAR_WIDTH - 16, 24, COL_CARD_SEL);
    } else if (sel_idx == app->hovered_sidebar) {
      winmgr_fill_rect(win, 8, y - 4, SIDEBAR_WIDTH - 16, 24, COL_CARD_HOVER);
    }
    winmgr_draw_text(win, 25, y + 2, app->pinned_names[i], COL_TEXT_WHT);
    y += 28;
    sel_idx++;
=======
      if (sel_idx == app->sidebar_sel) {
          winmgr_fill_rect(win, 8, y - 4, SIDEBAR_WIDTH - 16, 24, COL_CARD_SEL);
      } else if (sel_idx == app->hovered_sidebar) {
          winmgr_fill_rect(win, 8, y - 4, SIDEBAR_WIDTH - 16, 24, COL_CARD_HOVER);
      }
      winmgr_draw_text(win, 25, y + 2, app->pinned_names[i], COL_TEXT_WHT);
      y += 28;
      sel_idx++;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  }
}

static void draw_header(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;
  int w = win->width;

  // Header Background
<<<<<<< HEAD
  winmgr_fill_rect(win, SIDEBAR_WIDTH, 0, w - SIDEBAR_WIDTH, 50,
                   COL_CONTENT_BG);
=======
  winmgr_fill_rect(win, SIDEBAR_WIDTH, 0, w - SIDEBAR_WIDTH, 50, COL_CONTENT_BG);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  winmgr_fill_rect(win, SIDEBAR_WIDTH, 49, w - SIDEBAR_WIDTH, 1, COL_DIVIDER);

  // App Title
  winmgr_draw_text(win, SIDEBAR_WIDTH + 20, 18, "Aura Files", COL_TEXT_WHT);

  // Search Bar (Center-ish)
  int sw = 250;
  int sx = SIDEBAR_WIDTH + (w - SIDEBAR_WIDTH - sw) / 2;
<<<<<<< HEAD

  // Rounded-like search bar (drawn as standard rect due to primitive engine
  // limitations)
  winmgr_fill_rect(win, sx, 12, sw, 26,
                   app->search_focus ? 0xFF353535 : COL_ADDR_BG);
  winmgr_draw_rect(win, sx, 12, sw, 26, COL_DIVIDER);

  if (strlen(app->search_query) == 0 && !app->search_focus) {
    winmgr_draw_text(win, sx + 10, 18, "Search folders, files...",
                     COL_TEXT_MUTED);
=======
  
  // Rounded-like search bar (drawn as standard rect due to primitive engine limitations)
  winmgr_fill_rect(win, sx, 12, sw, 26, app->search_focus ? 0xFF353535 : COL_ADDR_BG);
  winmgr_draw_rect(win, sx, 12, sw, 26, COL_DIVIDER);

  if (strlen(app->search_query) == 0 && !app->search_focus) {
    winmgr_draw_text(win, sx + 10, 18, "Search folders, files...", COL_TEXT_MUTED);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  } else {
    winmgr_draw_text(win, sx + 10, 18, app->search_query, COL_TEXT_WHT);
    if (app->search_focus) {
      int cur_x = sx + 10 + strlen(app->search_query) * 8;
      winmgr_fill_rect(win, cur_x, 18, 1, 12, COL_TEXT_WHT);
    }
  }

  // Fake Profile/Notification icons on far right
  winmgr_draw_text(win, w - 70, 18, "O", COL_TEXT_MUTED); // Bell
<<<<<<< HEAD
  winmgr_draw_text(win, w - 40, 18, "U", COL_TEXT_WHT);   // Profile circle
=======
  winmgr_draw_text(win, w - 40, 18, "U", COL_TEXT_WHT); // Profile circle
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
}

static void draw_navigation_toolbar(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
<<<<<<< HEAD
  if (!app)
    return;

=======
  if (!app) return;
  
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  int tx = SIDEBAR_WIDTH;
  int ty = 50;
  int w = win->width;

  // Toolbar area
  winmgr_fill_rect(win, tx, ty, w - tx, 60, COL_CONTENT_BG);
<<<<<<< HEAD

  // Breadcrumb
  char title[128];
  build_display_path(app, title, 128);
  winmgr_draw_text(win, tx + 20, ty + 15, "H",
                   COL_TEXT_MUTED); // Home icon placeholder
  winmgr_draw_text(win, tx + 35, ty + 15, ">", COL_TEXT_MUTED);
  winmgr_draw_text(win, tx + 50, ty + 15, title, COL_TEXT_WHT);

=======
  
  // Breadcrumb
  char title[128];
  build_display_path(app, title, 128);
  winmgr_draw_text(win, tx + 20, ty + 15, "H", COL_TEXT_MUTED); // Home icon placeholder
  winmgr_draw_text(win, tx + 35, ty + 15, ">", COL_TEXT_MUTED);
  winmgr_draw_text(win, tx + 50, ty + 15, title, COL_TEXT_WHT);

>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  // Action Buttons
  int btn_y = ty + 35;
  winmgr_fill_rect(win, tx + 20, btn_y, 90, 24, COL_ACCENT); // Primary button
  winmgr_draw_text(win, tx + 25, btn_y + 6, "+ New Folder", COL_TEXT_WHT);
<<<<<<< HEAD

  winmgr_fill_rect(win, tx + 120, btn_y, 70, 24, COL_CARD_BG);
  winmgr_draw_text(win, tx + 130, btn_y + 6, "^ Upload", COL_TEXT_WHT);

  winmgr_fill_rect(win, tx + 200, btn_y, 60, 24, COL_CARD_BG);
  winmgr_draw_text(win, tx + 210, btn_y + 6, "Share", COL_TEXT_WHT);

  winmgr_fill_rect(win, tx + 270, btn_y, 70, 24, COL_CARD_BG);
  winmgr_draw_text(win, tx + 280, btn_y + 6, "Sort", COL_TEXT_WHT);
}

static void draw_column_headers(window_t *win) {
  int tx = SIDEBAR_WIDTH;
  int ty = 110;
  int w = win->width - tx;

  winmgr_fill_rect(win, tx, ty, w, 24, COL_CONTENT_BG);
  winmgr_fill_rect(win, tx, ty + 23, w, 1, COL_DIVIDER);

  winmgr_draw_text(win, tx + 20, ty + 6, "Name", COL_TEXT_MUTED);
  winmgr_draw_text(win, tx + (w / 2) - 30, ty + 6, "Size", COL_TEXT_MUTED);
  winmgr_draw_text(win, tx + (w / 2) + 50, ty + 6, "Type", COL_TEXT_MUTED);
  winmgr_draw_text(win, tx + w - 150, ty + 6, "Date Modified", COL_TEXT_MUTED);
}

static void draw_file_card(window_t *win, FileInfo *fi, int x, int y, int w,
                           int h, int selected, int hovered) {
  if (selected) {
    winmgr_fill_rect(win, x, y, w, h, COL_CARD_SEL);
    winmgr_draw_rect(win, x, y, w, h, COL_ACCENT);
  } else if (hovered) {
    winmgr_fill_rect(win, x, y, w, h, COL_CARD_HOVER);
    winmgr_draw_rect(win, x, y, w, h, COL_DIVIDER);
  } else {
    winmgr_fill_rect(win, x, y, w, h, COL_CARD_BG);
    winmgr_draw_rect(win, x, y, w, h, COL_DIVIDER);
  }

  // Determine type & icon
  const char *type_name = "File";
  if (fi->is_dir) {
    if (strcmp(fi->name, "..") == 0) {
      draw_icon_folder_48(win, x + w / 2 - 24, y + 10);
      winmgr_draw_text(win, x + w / 2 - 8, y + 65, "..", COL_TEXT_WHT);
      return;
    }
    type_name = "Folder";
    draw_icon_folder_48(win, x + w / 2 - 24, y + 10);
  } else {
    int len = strlen(fi->name);
    if (len > 4) {
      if (strcasestr(fi->name, ".pdf")) {
        type_name = "PDF Document";
        draw_icon_pdf_48(win, x + w / 2 - 24, y + 10);
      } else if (strcasestr(fi->name, ".png") || strcasestr(fi->name, ".bmp")) {
        type_name = "Image";
        draw_icon_image_48(win, x + w / 2 - 24, y + 10);
      } else if (strcasestr(fi->name, ".app")) {
        type_name = "Application";
        draw_icon_app_48(win, x + w / 2 - 24, y + 10);
      } else if (strcasestr(fi->name, ".txt")) {
        type_name = "Text Document";
        draw_icon_file_48(win, x + w / 2 - 24, y + 10);
      } else {
        draw_icon_file_48(win, x + w / 2 - 24, y + 10);
      }
    } else {
      draw_icon_file_48(win, x + w / 2 - 24, y + 10);
    }
  }

  // Name truncation
  char name_buf[20] = {0};
  strncpy(name_buf, fi->name, 15);
  if (strlen(fi->name) > 15) {
    strcpy(name_buf + 12, "...");
  }
  int tx = x + (w - strlen(name_buf) * 8) / 2;
  winmgr_draw_text(win, tx, y + 65, name_buf, COL_TEXT_WHT);

  // Type label
  int ty_x = x + (w - strlen(type_name) * 8) / 2;
  winmgr_draw_text(win, ty_x, y + h - 26, type_name, COL_TEXT_MUTED);

  // Size label
  char size_buf[32];
  if (fi->is_dir) {
    strcpy(size_buf, "--");
  } else {
    if (fi->size < 1024) {
      char num[16];
      k_itoa(fi->size, num);
      strcpy(size_buf, num);
      strcat(size_buf, " B");
    } else if (fi->size < 1024 * 1024) {
      char num[16];
      k_itoa(fi->size / 1024, num);
      strcpy(size_buf, num);
      strcat(size_buf, " KB");
    } else {
      char num[16];
      k_itoa(fi->size / (1024 * 1024), num);
      strcpy(size_buf, num);
      strcat(size_buf, " MB");
    }
  }
  int sz_x = x + (w - strlen(size_buf) * 8) / 2;
  winmgr_draw_text(win, sz_x, y + h - 14, size_buf, COL_TEXT_MUTED);
=======
  
  winmgr_fill_rect(win, tx + 120, btn_y, 70, 24, COL_CARD_BG);
  winmgr_draw_text(win, tx + 130, btn_y + 6, "^ Upload", COL_TEXT_WHT);
  
  winmgr_fill_rect(win, tx + 200, btn_y, 60, 24, COL_CARD_BG);
  winmgr_draw_text(win, tx + 210, btn_y + 6, "Share", COL_TEXT_WHT);
  
  winmgr_fill_rect(win, tx + 270, btn_y, 70, 24, COL_CARD_BG);
  winmgr_draw_text(win, tx + 280, btn_y + 6, "Sort", COL_TEXT_WHT);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
}

static void draw_column_headers(window_t *win) {
  int tx = SIDEBAR_WIDTH;
  int ty = 110;
  int w = win->width - tx;
  
  winmgr_fill_rect(win, tx, ty, w, 24, COL_CONTENT_BG);
  winmgr_fill_rect(win, tx, ty + 23, w, 1, COL_DIVIDER);
  
  winmgr_draw_text(win, tx + 20, ty + 6, "Name", COL_TEXT_MUTED);
  winmgr_draw_text(win, tx + (w / 2) - 30, ty + 6, "Size", COL_TEXT_MUTED);
  winmgr_draw_text(win, tx + (w / 2) + 50, ty + 6, "Type", COL_TEXT_MUTED);
  winmgr_draw_text(win, tx + w - 150, ty + 6, "Date Modified", COL_TEXT_MUTED);
}


static void draw_file_card(window_t *win, FileInfo *fi, int x, int y, int w, int h, int selected, int hovered) {
    if (selected) {
        winmgr_fill_rect(win, x, y, w, h, COL_CARD_SEL);
        winmgr_draw_rect(win, x, y, w, h, COL_ACCENT);
    } else if (hovered) {
        winmgr_fill_rect(win, x, y, w, h, COL_CARD_HOVER);
        winmgr_draw_rect(win, x, y, w, h, COL_DIVIDER);
    } else {
        winmgr_fill_rect(win, x, y, w, h, COL_CARD_BG);
        winmgr_draw_rect(win, x, y, w, h, COL_DIVIDER);
    }

    // Determine type & icon
    const char *type_name = "File";
    if (fi->is_dir) {
        if (strcmp(fi->name, "..") == 0) {
            draw_icon_folder_48(win, x + w / 2 - 24, y + 10);
            winmgr_draw_text(win, x + w / 2 - 8, y + 65, "..", COL_TEXT_WHT);
            return;
        }
        type_name = "Folder";
        draw_icon_folder_48(win, x + w / 2 - 24, y + 10);
    } else {
        int len = strlen(fi->name);
        if (len > 4) {
            if (strcasestr(fi->name, ".pdf")) {
                type_name = "PDF Document";
                draw_icon_pdf_48(win, x + w / 2 - 24, y + 10);
            } else if (strcasestr(fi->name, ".png") || strcasestr(fi->name, ".bmp")) {
                type_name = "Image";
                draw_icon_image_48(win, x + w / 2 - 24, y + 10);
            } else if (strcasestr(fi->name, ".app")) {
                type_name = "Application";
                draw_icon_app_48(win, x + w / 2 - 24, y + 10);
            } else if (strcasestr(fi->name, ".txt")) {
                type_name = "Text Document";
                draw_icon_file_48(win, x + w / 2 - 24, y + 10);
            } else {
                draw_icon_file_48(win, x + w / 2 - 24, y + 10);
            }
        } else {
            draw_icon_file_48(win, x + w / 2 - 24, y + 10);
        }
    }

    // Name truncation
    char name_buf[20] = {0};
    strncpy(name_buf, fi->name, 15);
    if (strlen(fi->name) > 15) {
        strcpy(name_buf + 12, "...");
    }
    int tx = x + (w - strlen(name_buf) * 8) / 2;
    winmgr_draw_text(win, tx, y + 65, name_buf, COL_TEXT_WHT);

    // Type label
    int ty_x = x + (w - strlen(type_name) * 8) / 2;
    winmgr_draw_text(win, ty_x, y + h - 26, type_name, COL_TEXT_MUTED);

    // Size label
    char size_buf[32];
    if (fi->is_dir) {
        strcpy(size_buf, "--");
    } else {
        if (fi->size < 1024) {
            char num[16];
            k_itoa(fi->size, num);
            strcpy(size_buf, num);
            strcat(size_buf, " B");
        } else if (fi->size < 1024 * 1024) {
            char num[16];
            k_itoa(fi->size / 1024, num);
            strcpy(size_buf, num);
            strcat(size_buf, " KB");
        } else {
            char num[16];
            k_itoa(fi->size / (1024 * 1024), num);
            strcpy(size_buf, num);
            strcat(size_buf, " MB");
        }
    }
    int sz_x = x + (w - strlen(size_buf) * 8) / 2;
    winmgr_draw_text(win, sz_x, y + h - 14, size_buf, COL_TEXT_MUTED);
}


static void draw_file_grid(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;
  int w = win->width;
  int content_y = 135;
  int content_h = win->height - content_y - 30; // 30 for status bar
  int content_x = SIDEBAR_WIDTH;
<<<<<<< HEAD

  winmgr_fill_rect(win, content_x, content_y, w - content_x, content_h,
                   COL_CONTENT_BG);

=======

  winmgr_fill_rect(win, content_x, content_y, w - content_x, content_h, COL_CONTENT_BG);

>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid &&
        strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i;
      break;
    }
  }

  if (slot == -1 || app->dir_cache[slot].count == 0) {
    win->max_scroll = 0;
<<<<<<< HEAD
    winmgr_draw_text(win, content_x + (w - content_x) / 2 - 100,
                     content_y + 100, "This folder is empty", COL_TEXT_MUTED);
=======
    winmgr_draw_text(win, content_x + (w - content_x)/2 - 100, content_y + 100,
                     "This folder is empty", COL_TEXT_MUTED);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    return;
  }

  int f_count = app->dir_cache[slot].count;
  FileInfo *f_cache = app->dir_cache[slot].files;

  // Grid layout (using File Cards)
  int start_x = content_x + 20;
  int start_y = content_y + 10;
<<<<<<< HEAD
  int cell_w = 140;
  int cell_h = 120;
  int padding = 15;
  int cols = (w - content_x - 40) / (cell_w + padding);
  if (cols < 1)
    cols = 1;
=======
  int cell_w = 140; 
  int cell_h = 120;  
  int padding = 15;
  int cols = (w - content_x - 40) / (cell_w + padding);
  if (cols < 1) cols = 1;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

  int total_rows = (f_count + cols - 1) / cols;
  int max_scroll = (total_rows * (cell_h + padding)) - content_h + 20;
  win->max_scroll = (max_scroll > 0) ? max_scroll : 0;

  for (int i = 0; i < f_count && i < 64; i++) {
<<<<<<< HEAD
    if (strcmp(f_cache[i].name, ".") == 0)
      continue;
=======
    if (strcmp(f_cache[i].name, ".") == 0) continue;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

    int col = i % cols;
    int row = i / cols;

    int fx = start_x + col * (cell_w + padding);
    int fy = start_y + row * (cell_h + padding) - win->scroll_position;

<<<<<<< HEAD
    if (fy + cell_h < content_y || fy > win->height - 30)
      continue;

    draw_file_card(win, &f_cache[i], fx, fy, cell_w, cell_h,
                   (i == app->selected_index), (i == app->hovered_index));
=======
    if (fy + cell_h < content_y || fy > win->height - 30) continue;

    draw_file_card(win, &f_cache[i], fx, fy, cell_w, cell_h, 
                  (i == app->selected_index), (i == app->hovered_index));
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  }
}

static void draw_search_results(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;
  int w = win->width;
  int content_y = 135;
  int content_h = win->height - content_y - 30;
  int content_x = SIDEBAR_WIDTH;

  winmgr_fill_rect(win, content_x, content_y, w - content_x, content_h, COL_CONTENT_BG);

  if (app->search_count == 0) {
<<<<<<< HEAD
    winmgr_draw_text(win, content_x + 20, content_y + 30, "No results found.",
                     COL_TEXT_MUTED);
=======
    winmgr_draw_text(win, content_x + 20, content_y + 30, "No results found.", COL_TEXT_MUTED);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    return;
  }

  int start_x = content_x + 20;
  int start_y = content_y + 10;
<<<<<<< HEAD
  int cell_w = 140;
  int cell_h = 120;
  int padding = 15;
  int cols = (w - content_x - 40) / (cell_w + padding);
  if (cols < 1)
    cols = 1;
=======
  int cell_w = 140; 
  int cell_h = 120;  
  int padding = 15;
  int cols = (w - content_x - 40) / (cell_w + padding);
  if (cols < 1) cols = 1;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

  int total_rows = (app->search_count + cols - 1) / cols;
  int max_scroll = (total_rows * (cell_h + padding)) - content_h + 20;
  win->max_scroll = (max_scroll > 0) ? max_scroll : 0;

  for (int i = 0; i < app->search_count; i++) {
    int col = i % cols;
    int row = i / cols;
    int fx = start_x + col * (cell_w + padding);
    int fy = start_y + row * (cell_h + padding) - win->scroll_position;

<<<<<<< HEAD
    if (fy + cell_h < content_y || fy > win->height - 30)
      continue;
=======
    if (fy + cell_h < content_y || fy > win->height - 30) continue;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

    FileInfo fi;
    strcpy(fi.name, app->search_results[i].name);
    fi.is_dir = app->search_results[i].is_dir;
    fi.size = app->search_results[i].size;
<<<<<<< HEAD

    draw_file_card(win, &fi, fx, fy, cell_w, cell_h, (i == app->selected_index),
                   (i == app->hovered_index));
=======
    
    draw_file_card(win, &fi, fx, fy, cell_w, cell_h, 
                  (i == app->selected_index), (i == app->hovered_index));
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  }
}

static void draw_status_bar(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;
  int w = win->width;
  int sy = win->height - 30;

<<<<<<< HEAD
  winmgr_fill_rect(win, SIDEBAR_WIDTH, sy, w - SIDEBAR_WIDTH, 30,
                   COL_CONTENT_BG);
=======
  winmgr_fill_rect(win, SIDEBAR_WIDTH, sy, w - SIDEBAR_WIDTH, 30, COL_CONTENT_BG);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  winmgr_fill_rect(win, SIDEBAR_WIDTH, sy, w - SIDEBAR_WIDTH, 1, COL_DIVIDER);

  char status[128];
  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
<<<<<<< HEAD
    if (app->dir_cache[i].valid &&
        strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
=======
    if (app->dir_cache[i].valid && strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      slot = i;
      break;
    }
  }
  int f_count = (slot != -1) ? app->dir_cache[slot].count : 0;

  char num[12];
  k_itoa(f_count, num);
  strcpy(status, num);
  strcat(status, " items");
<<<<<<< HEAD

  winmgr_draw_text(win, SIDEBAR_WIDTH + 20, sy + 10, status, COL_TEXT_MUTED);

=======
  
  winmgr_draw_text(win, SIDEBAR_WIDTH + 20, sy + 10, status, COL_TEXT_MUTED);
  
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  // OS Name branding
  winmgr_draw_text(win, w - 80, sy + 10, "Pure OS", COL_TEXT_MUTED);
}

static void draw_input_dialog(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app || !app->dialog_active)
    return;

  int dw = 280, dh = 110;
  int dx = (win->width - dw) / 2;
  int dy = (win->height - dh) / 2;

  // Dialog background (Opaque)
  winmgr_fill_rect(win, dx, dy, dw, dh, 0xFF2C313A);
  winmgr_draw_rect(win, dx, dy, dw, dh, COL_SEL_BORDER);

  if (app->dialog_active == 3) {
    winmgr_draw_text(win, dx + 10, dy + 8, "Transfer Status", COL_TEXT_DARK);
    winmgr_draw_text(win, dx + 16, dy + 44, app->popup_msg, COL_TEXT_DARK);

    winmgr_fill_rect(win, dx + 105, dy + 75, 70, 24, 0xFF98C379); // Green
    winmgr_draw_text(win, dx + 130, dy + 81, "OK", COL_TEXT_DARK);
    // Draw white highlight text for better contrast against dark popup
    // background if needed, but we use dark here over pale? Wait, the popup is
    // 0xFF2C313A which is dark gray! Let's redraw text in white.
    winmgr_draw_text(win, dx + 10, dy + 8, "Transfer Status", 0xFFFFFFFF);
    winmgr_draw_text(win, dx + 16, dy + 44, app->popup_msg, 0xFFFFFFFF);
    return;
  }

  // Title
  const char *title = "";
<<<<<<< HEAD
  if (app->dialog_active == 1)
    title = "New Folder";
  else if (app->dialog_active == 2)
    title = "New File";
  else if (app->dialog_active == 4)
    title = "Rename Item";
=======
  if (app->dialog_active == 1) title = "New Folder";
  else if (app->dialog_active == 2) title = "New File";
  else if (app->dialog_active == 4) title = "Rename Item";
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  winmgr_draw_text(win, dx + 10, dy + 8, title, 0xFFFFFFFF); // Fixed contrast

  // Input field
  winmgr_fill_rect(win, dx + 12, dy + 40, dw - 24, 22, COL_ADDR_FIELD);
  winmgr_draw_rect(win, dx + 12, dy + 40, dw - 24, 22, COL_ADDR_EDGE);
  winmgr_draw_text(win, dx + 16, dy + 44, app->dialog_input, COL_TEXT_DARK);

  // Actions
  winmgr_fill_rect(win, dx + 60, dy + 75, 70, 24, COL_APP_BG);
  winmgr_draw_text(win, dx + 85, dy + 79, "OK", COL_TEXT_WHT);

  winmgr_fill_rect(win, dx + 150, dy + 75, 70, 24, COL_CARD_SEL); // Sakura
  winmgr_draw_text(win, dx + 162, dy + 79, "Cancel", COL_TEXT_WHT);
}

// ======================== MAIN DRAW ========================

void explorer_draw(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
<<<<<<< HEAD
  if (!app)
    return;

  // App Background (entire window)
  winmgr_fill_rect(win, 0, 0, win->width, win->height, COL_APP_BG);

=======
  if (!app) return;
  
  // App Background (entire window)
  winmgr_fill_rect(win, 0, 0, win->width, win->height, COL_APP_BG);

>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  // Note: draw order matters. Background -> layout components -> overlays
  draw_sidebar(win);
  draw_header(win);
  draw_navigation_toolbar(win);
  draw_column_headers(win);

  if (app->search_active) {
<<<<<<< HEAD
    draw_search_results(win);
  } else {
    draw_file_grid(win);
=======
      draw_search_results(win);
  } else {
      draw_file_grid(win);
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  }

  draw_status_bar(win);
  draw_input_dialog(win);
}

// ======================== FILE ACTIONS ========================

void explorer_open_file(window_t *win, int index) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;
  char final_path[160];
  int is_dir = 0;

  if (app->search_active) {
    if (index < 0 || index >= app->search_count)
      return;
    strcpy(final_path, app->search_results[index].full_path);
    is_dir = app->search_results[index].is_dir;
  } else {
    int slot = -1;
    for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
      if (app->dir_cache[i].valid &&
          strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
        slot = i;
        break;
      }
    }
    if (slot == -1 || index < 0 || index >= app->dir_cache[slot].count)
      return;

    char *name = app->dir_cache[slot].files[index].name;

    // . and ..
    if (strcmp(name, ".") == 0) {
      explorer_refresh(win);
      return;
    }
    if (strcmp(name, "..") == 0) {
      explorer_go_up(app);
      explorer_refresh(win);
      return;
    }

    is_dir = app->dir_cache[slot].files[index].is_dir;
    strcpy(final_path, (app->explorer_path[0] == 0) ? "/" : app->explorer_path);
    int path_len = strlen(final_path);
    if (path_len > 0 && final_path[path_len - 1] != '/')
      strcat(final_path, "/");
    strcat(final_path, name);
  }

  if (is_dir) {
    strcpy(app->explorer_path, final_path);
    app->search_active = 0; // Exit search when entering folder
    app->search_query[0] = 0;
    app->search_cursor = 0;
    app->at_this_pc = 0;
    explorer_refresh(win);
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

static void dialog_confirm(window_t *win) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;
  if (app->dialog_cursor == 0) {
    app->dialog_active = 0;
    return;
  }

  char full_path[160];
  strcpy(full_path, app->explorer_path);
  if (full_path[strlen(full_path) - 1] != '/')
    strcat(full_path, "/");
  strcat(full_path, app->dialog_input);

  if (app->dialog_active == 1) {
    fs_mkdir(full_path);
  } else if (app->dialog_active == 2) {
    fs_write(full_path, (const uint8_t *)"", 0);
  } else if (app->dialog_active == 4) {
    // Rename operation
    int slot = -1;
    for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
<<<<<<< HEAD
      if (app->dir_cache[i].valid &&
          strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
        slot = i;
        break;
=======
      if (app->dir_cache[i].valid && strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
        slot = i; break;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      }
    }
    if (slot != -1 && app->selected_index >= 0) {
      char old_path[160];
      strcpy(old_path, app->explorer_path);
<<<<<<< HEAD
      if (old_path[strlen(old_path) - 1] != '/')
        strcat(old_path, "/");
      strcat(old_path, app->dir_cache[slot].files[app->selected_index].name);

=======
      if (old_path[strlen(old_path) - 1] != '/') strcat(old_path, "/");
      strcat(old_path, app->dir_cache[slot].files[app->selected_index].name);
      
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      fat_move_file(old_path, full_path);
    }
  }

  app->dialog_active = 0;
  app->dialog_input[0] = 0;
  app->dialog_cursor = 0;
  // Invalidate all caches after mutation
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
    app->dir_cache[i].valid = 0;
  explorer_refresh(win);
}

// ======================== SHORTCUTS ========================

static void explorer_on_copy(void *w) {
  window_t *win = (window_t *)w;
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;

  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid &&
        strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i;
      break;
    }
  }
  if (app->at_this_pc || slot == -1 || app->selected_index < 0 ||
      app->selected_index >= app->dir_cache[slot].count)
    return;
  if (strcmp(app->dir_cache[slot].files[app->selected_index].name, ".") == 0 ||
      strcmp(app->dir_cache[slot].files[app->selected_index].name, "..") == 0)
    return;

  char full_path[160];
  strcpy(full_path, (app->explorer_path[0] == 0) ? "/" : app->explorer_path);
  if (full_path[strlen(full_path) - 1] != '/')
    strcat(full_path, "/");
  strcat(full_path, app->dir_cache[slot].files[app->selected_index].name);

  clipboard_copy(full_path);
  clipboard_set_operation(CLIPBOARD_OP_COPY);

  char msg[64];
  strcpy(msg, "Copied ");
  strcat(msg, app->dir_cache[slot].files[app->selected_index].name);
  explorer_show_popup(app, msg);
  win->needs_redraw = 1;
}

static void explorer_on_cut(void *w) {
  window_t *win = (window_t *)w;
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;

  int slot = -1;
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
    if (app->dir_cache[i].valid &&
        strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
      slot = i;
      break;
    }
  }
  if (app->at_this_pc || slot == -1 || app->selected_index < 0 ||
      app->selected_index >= app->dir_cache[slot].count)
    return;
  if (strcmp(app->dir_cache[slot].files[app->selected_index].name, ".") == 0 ||
      strcmp(app->dir_cache[slot].files[app->selected_index].name, "..") == 0)
    return;

  char full_path[160];
  strcpy(full_path, (app->explorer_path[0] == 0) ? "/" : app->explorer_path);
  if (full_path[strlen(full_path) - 1] != '/')
    strcat(full_path, "/");
  strcat(full_path, app->dir_cache[slot].files[app->selected_index].name);

  clipboard_copy(full_path);
  clipboard_set_operation(CLIPBOARD_OP_CUT);

  char msg[64];
  strcpy(msg, "Cut ");
  strcat(msg, app->dir_cache[slot].files[app->selected_index].name);
  explorer_show_popup(app, msg);
  win->needs_redraw = 1;
}

static void explorer_on_paste(void *w, const char *path) {
  window_t *win = (window_t *)w;
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;

  if (app->at_this_pc || !path || path[0] == 0)
    return;

  const char *filename = path;
  for (int i = 0; path[i]; i++) {
    if (path[i] == '/')
      filename = &path[i + 1];
  }

  char dest_path[160];
  strcpy(dest_path, (app->explorer_path[0] == 0) ? "/" : app->explorer_path);
  if (dest_path[strlen(dest_path) - 1] != '/')
    strcat(dest_path, "/");
  strcat(dest_path, filename);

  if (strcmp(path, dest_path) == 0) {
    explorer_show_popup(app, "Error: Same location!");
    win->needs_redraw = 1;
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
    int res = fat_copy_recursive(path, dest_path);
    if (res == 0) {
      strcpy(msg, "Copy completed!");
    }
  } else if (op == CLIPBOARD_OP_CUT) {
    int res = fat_move_file(path, dest_path);
    if (res == 0) {
      strcpy(msg, "Move completed!");
      clipboard_set_operation(CLIPBOARD_OP_NONE);
    }
  } else {
    strcpy(msg, "Nothing to paste.");
  }

  explorer_show_popup(app, msg);
  for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
    app->dir_cache[i].valid = 0;
  explorer_refresh(win);
  win->needs_redraw = 1;
}

// ======================== INPUT HANDLERS ========================

void explorer_handle_key(window_t *win, int key, char ascii) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;

  if (app->dialog_active == 3) {
    if (key == 0x1C || key == 0x01 || ascii == '\n') {
      app->dialog_active = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  if (app->dialog_active) {
    if (key == 0x1C) {
      dialog_confirm(win);
      win->needs_redraw = 1;
      return;
    }
    if (key == 0x01) {
      app->dialog_active = 0;
      app->dialog_input[0] = 0;
      app->dialog_cursor = 0;
      win->needs_redraw = 1;
      return;
    }
    if (key == 0x0E && app->dialog_cursor > 0) {
      app->dialog_input[--app->dialog_cursor] = 0;
      win->needs_redraw = 1;
      return;
    }
    if (ascii >= 32 && ascii < 127 && app->dialog_cursor < 24) {
      app->dialog_input[app->dialog_cursor++] =
          (ascii >= 'a' && ascii <= 'z') ? ascii - 32 : ascii;
      app->dialog_input[app->dialog_cursor] = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  if (key == 0x1E) { // 'A' key - test for redraw issue
    print_serial("EXPLORER: Manually triggering redraw\n");
    win->needs_redraw = 1;
  }

  if (key == 0x53) { // Delete key
    print_serial("EXPLORER: Delete key pressed\n");
    action_delete_selected();
    return;
  }

  if (key == 0x0E || ascii == '\b') {
    if (app->search_focus) {
      if (app->search_cursor > 0) {
        app->search_query[--app->search_cursor] = 0;
        explorer_do_search(app);
      }
      return;
    }
    if (!app->at_this_pc) {
      explorer_go_up(app);
      explorer_refresh(win);
    }
    if (app->search_focus) {
      explorer_do_search(app);
      app->search_focus = 0;
      return;
    }
    if (app->at_this_pc && app->selected_index == 0) {
      app->at_this_pc = 0;
      strcpy(app->explorer_path, "/");
      explorer_refresh(win);
    } else if (!app->at_this_pc && app->selected_index >= 0) {
      explorer_open_file(win, app->selected_index);
    }
  } else if (app->search_focus && ascii >= 32 && ascii < 127 &&
             app->search_cursor < 31) {
    app->search_query[app->search_cursor++] = ascii;
    app->search_query[app->search_cursor] = 0;
    explorer_do_search(app); // Instant search
    win->needs_redraw = 1;
  }
}

void explorer_on_scroll(window_t *win, int direction) {
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
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
  explorer_app_t *app = get_explorer_app(win);
  if (!app)
    return;

  int rx = mx;
  int ry = my;
  int w = win->width;

  // Layout parameters matching drawing functions
  int content_x = SIDEBAR_WIDTH;
  int content_y = 135;
  int content_h = win->height - content_y - 30; // 30 for status bar
  int cell_w = 140, cell_h = 120, padding = 15;
  int start_x = content_x + 20;
  int start_y = content_y + 10;
  int cols = (w - content_x - 40) / (cell_w + padding);
<<<<<<< HEAD
  if (cols < 1)
    cols = 1;
=======
  if (cols < 1) cols = 1;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)

  // HOVER DETECTION
  int new_hover_sb = -1;
  if (rx < SIDEBAR_WIDTH) {
    int y = 50;
    int sel_idx = 0;
    int counts[4] = {4, 2, 2, 3};
    for (int s = 0; s < 4; s++) {
      y += 20; // section header
      for (int i = 0; i < counts[s]; i++) {
        if (ry >= y - 4 && ry < y + 24) {
          new_hover_sb = sel_idx;
        }
        y += 28;
        sel_idx++;
      }
      y += 10;
    }
  }

  if (new_hover_sb != app->hovered_sidebar) {
    app->hovered_sidebar = new_hover_sb;
    win->needs_redraw = 1;
  }

  int new_hover = -1;
  if (rx >= content_x && ry >= content_y && ry < win->height - 30) {
<<<<<<< HEAD
    int grid_x = rx - start_x;
    int grid_y = ry - start_y + win->scroll_position;
    if (grid_x >= 0 && grid_y >= 0) {
      int col = grid_x / (cell_w + padding);
      int row = grid_y / (cell_h + padding);
      if ((grid_x % (cell_w + padding)) < cell_w &&
          (grid_y % (cell_h + padding)) < cell_h) {
        new_hover = row * cols + col;

        int slot = -1;
        for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
          if (app->dir_cache[i].valid &&
              strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
            slot = i;
            break;
          }
        }
        int max_items = (slot != -1) ? app->dir_cache[slot].count : 0;
        if (app->search_active)
          max_items = app->search_count;
        if (new_hover >= max_items)
          new_hover = -1;
      }
    }
=======
      int grid_x = rx - start_x;
      int grid_y = ry - start_y + win->scroll_position;
      if (grid_x >= 0 && grid_y >= 0) {
          int col = grid_x / (cell_w + padding);
          int row = grid_y / (cell_h + padding);
          if ((grid_x % (cell_w + padding)) < cell_w && (grid_y % (cell_h + padding)) < cell_h) {
              new_hover = row * cols + col;
              
              int slot = -1;
              for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) {
                if (app->dir_cache[i].valid && strcmp(app->dir_cache[i].path, app->explorer_path) == 0) {
                  slot = i; break;
                }
              }
              int max_items = (slot != -1) ? app->dir_cache[slot].count : 0;
              if (app->search_active) max_items = app->search_count;
              if (new_hover >= max_items) new_hover = -1;
          }
      }
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  }
  if (new_hover != app->hovered_index) {
    app->hovered_index = new_hover;
    win->needs_redraw = 1;
  }

  // CLICK DETECTION (Rising edge only)
  if (!(buttons & 3)) { // Track both left & right
    app->prev_mouse_buttons = 0;
    return;
  }
  if (app->prev_mouse_buttons)
    return;
  app->prev_mouse_buttons = buttons;

  // Handle Right Click
  if (buttons & 2) {
    // If hovering over a file/folder in the grid
    if (new_hover != -1) {
      app->selected_index = new_hover;
      win->needs_redraw = 1;

      static ctxmenu_item_t folder_items[] = {
<<<<<<< HEAD
          {"Open", 0},
          {"Pin to Sidebar", action_pin_selected},
          {0, 0},
          {"Copy", action_copy_selected},
          {"Cut", action_cut_selected},
          {"Rename", action_rename_selected},
          {"Delete", action_delete_selected}};

=======
        {"Open", 0}, 
        {"Pin to Sidebar", action_pin_selected},
        {0, 0},
        {"Copy", action_copy_selected},
        {"Cut", action_cut_selected},
        {"Rename", action_rename_selected},
        {"Delete", action_delete_selected}
      };
      
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      // Global coordinates for ctxmenu_show
      ctxmenu_show(win->x + mx, win->y + my, folder_items, 7);
      return;
    }
    // Right click on empty area
    else if (rx >= content_x && ry >= content_y) {
<<<<<<< HEAD
      static ctxmenu_item_t empty_items[] = {{"New Folder", action_new_folder},
                                             {"Paste", action_paste_here},
                                             {0, 0},
                                             {"Refresh", action_refresh_view},
                                             {"Sort By Name", 0}};
      ctxmenu_show(win->x + mx, win->y + my, empty_items, 5);
      return;
=======
       static ctxmenu_item_t empty_items[] = {
         {"New Folder", action_new_folder},
         {"Paste", action_paste_here},
         {0, 0},
         {"Refresh", action_refresh_view},
         {"Sort By Name", 0}
       };
       ctxmenu_show(win->x + mx, win->y + my, empty_items, 5);
       return;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    }
  }

  if (app->dialog_active == 3) {
    int dw = 280, dh = 110;
    int dx = (win->width - dw) / 2;
    int dy = (win->height - dh) / 2;
    if (rx >= dx + 105 && rx < dx + 175 && ry >= dy + 75 && ry < dy + 99) {
      app->dialog_active = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  if (app->dialog_active) {
    int dw = 280, dh = 110;
    int dx = (win->width - dw) / 2;
    int dy = (win->height - dh) / 2;

    if (rx >= dx + 60 && rx < dx + 130 && ry >= dy + 75 && ry < dy + 99) {
      dialog_confirm(win);
      win->needs_redraw = 1;
<<<<<<< HEAD
    } else if (rx >= dx + 150 && rx < dx + 220 && ry >= dy + 75 &&
               ry < dy + 99) {
=======
    }
    else if (rx >= dx + 150 && rx < dx + 220 && ry >= dy + 75 && ry < dy + 99) {
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      app->dialog_active = 0;
      app->dialog_input[0] = 0;
      app->dialog_cursor = 0;
      win->needs_redraw = 1;
    }
    return;
  }

  // Handle Sidebar Click
  if (new_hover_sb != -1) {
    app->sidebar_sel = new_hover_sb;
    win->needs_redraw = 1;
    app->search_active = 0;
    app->at_this_pc = 0;
    app->selected_index = -1;
<<<<<<< HEAD

    if (new_hover_sb == 0)
      strcpy(app->explorer_path, "/Recents");
    else if (new_hover_sb == 1)
      strcpy(app->explorer_path, "/Downloads");
    else if (new_hover_sb == 2)
      strcpy(app->explorer_path, "/Desktop");
    else if (new_hover_sb == 3)
      strcpy(app->explorer_path, "/Documents");
    else if (new_hover_sb == 4)
      strcpy(app->explorer_path, "/"); // Local Disk C:
    else if (new_hover_sb == 5)
      strcpy(app->explorer_path, "/"); // External Drive
    else if (new_hover_sb == 6)
      strcpy(app->explorer_path, "/"); // iCloud
    else if (new_hover_sb == 7)
      strcpy(app->explorer_path, "/"); // Google Drive
    else if (new_hover_sb >= 8 && new_hover_sb < 8 + app->pinned_count) {
      strcpy(app->explorer_path, app->pinned_paths[new_hover_sb - 8]);
    } else {
      strcpy(app->explorer_path, "/");
    }
=======
    
    if (new_hover_sb == 0) strcpy(app->explorer_path, "/Recents");
    else if (new_hover_sb == 1) strcpy(app->explorer_path, "/Downloads");
    else if (new_hover_sb == 2) strcpy(app->explorer_path, "/Desktop");
    else if (new_hover_sb == 3) strcpy(app->explorer_path, "/Documents");
    else if (new_hover_sb == 4) strcpy(app->explorer_path, "/"); // Local Disk C:
    else if (new_hover_sb == 5) strcpy(app->explorer_path, "/"); // External Drive
    else if (new_hover_sb == 6) strcpy(app->explorer_path, "/"); // iCloud
    else if (new_hover_sb == 7) strcpy(app->explorer_path, "/"); // Google Drive
    else if (new_hover_sb >= 8 && new_hover_sb < 8 + app->pinned_count) {
       strcpy(app->explorer_path, app->pinned_paths[new_hover_sb - 8]);
    }
    else {
       strcpy(app->explorer_path, "/");
    }
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    explorer_refresh(win);
    return;
  }

  // Header / Search Bar Hit Test (y: 0-50)
  if (ry < 50 && rx >= SIDEBAR_WIDTH) {
    int sw = 250;
    int sx = SIDEBAR_WIDTH + (w - SIDEBAR_WIDTH - sw) / 2;
<<<<<<< HEAD

=======
    
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    if (rx >= sx && rx < sx + sw) {
      app->search_focus = 1;
      app->dialog_active = 0;
      win->needs_redraw = 1;
    } else {
      app->search_focus = 0;
      win->needs_redraw = 1;
    }
<<<<<<< HEAD

=======
    
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    // Clear search
    if (app->search_active && rx >= sx + sw - 20) {
      app->search_active = 0;
      app->search_query[0] = 0;
      app->search_cursor = 0;
      explorer_refresh(win);
    }
    return;
  }

  // Navigation Toolbar Hit Test (y: 50-110)
  if (ry >= 50 && ry < 110 && rx >= SIDEBAR_WIDTH) {
    app->search_focus = 0;
<<<<<<< HEAD

    int tx = SIDEBAR_WIDTH;
    int ty = 50;

=======
    
    int tx = SIDEBAR_WIDTH;
    int ty = 50;
    
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
    // Back navigation
    if (rx >= tx + 15 && rx < tx + 40 && ry >= ty + 5 && ry < ty + 30) {
      explorer_go_up(app);
      explorer_refresh(win);
      return;
    }

    int btn_y = ty + 35; // 85
    if (ry >= btn_y && ry < btn_y + 24) {
      if (rx >= tx + 20 && rx < tx + 110) { // New Folder
        app->dialog_active = 1;
        app->dialog_input[0] = 0;
        app->dialog_cursor = 0;
        win->needs_redraw = 1;
        return;
      }
      if (rx >= tx + 120 && rx < tx + 190) { // Upload / Refresh
        for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
          app->dir_cache[i].valid = 0;
        explorer_refresh(win);
        return;
      }
      if (rx >= tx + 200 && rx < tx + 260) { // Share
<<<<<<< HEAD
        explorer_show_popup(app, "Share not implemented");
        return;
      }
      if (rx >= tx + 270 && rx < tx + 340) { // Sort
        app->sort_mode = !app->sort_mode;
        for (int i = 0; i < EXPLORER_CACHE_SIZE; i++)
          app->dir_cache[i].valid = 0;
        explorer_refresh(win);
        return;
=======
         explorer_show_popup(app, "Share not implemented");
         return;
      }
      if (rx >= tx + 270 && rx < tx + 340) { // Sort
         app->sort_mode = !app->sort_mode;
         for (int i = 0; i < EXPLORER_CACHE_SIZE; i++) app->dir_cache[i].valid = 0;
         explorer_refresh(win);
         return;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
      }
    }
    return;
  }

  // Column Headers (y: 110-135)
  if (ry >= 110 && ry < 135 && rx >= SIDEBAR_WIDTH) {
    return; // No-op for now
  }

  // Content Area Hit Test
  if (new_hover != -1) {
<<<<<<< HEAD
    app->search_focus = 0;
    if (app->selected_index == new_hover)
      explorer_open_file(win, new_hover);
    else
      app->selected_index = new_hover;
    win->needs_redraw = 1;
  } else if (rx >= content_x && ry >= content_y && ry < win->height - 30) {
    app->search_focus = 0;
    app->selected_index = -1; // Deselect
    win->needs_redraw = 1;
=======
      app->search_focus = 0;
      if (app->selected_index == new_hover)
          explorer_open_file(win, new_hover);
      else
          app->selected_index = new_hover;
      win->needs_redraw = 1;
  } else if (rx >= content_x && ry >= content_y && ry < win->height - 30) {
      app->search_focus = 0;
      app->selected_index = -1; // Deselect
      win->needs_redraw = 1;
>>>>>>> a9f8805 (Integrate TinyExpr math engine, Duktape JS engine, and UI modernizations)
  }
}

static void explorer_on_close(void *w) {
  window_t *win = (window_t *)w;
  if (win->user_data) {
    kfree(win->user_data);
    win->user_data = 0;
  }
}

void explorer_init() {
  window_t *win = winmgr_create_window(-1, -1, 800, 600, "Explorer");
  if (!win) {
    print_serial("EXPLORER: Failed to create window (OOM)\n");
    return;
  }

  explorer_app_t *app = (explorer_app_t *)kmalloc(sizeof(explorer_app_t));
  if (!app) {
    winmgr_close_window(win);
    return;
  }
  for (int i = 0; i < (int)sizeof(explorer_app_t); i++)
    ((char *)app)[i] = 0;

  app->win = win;
  win->user_data = app;

  win->draw = (void (*)(void *))explorer_draw;
  win->on_mouse = (void (*)(void *, int, int, int))explorer_handle_mouse;
  win->on_key = (void (*)(void *, int, char))explorer_handle_key;
  win->on_scroll = (void (*)(void *, int))explorer_on_scroll;
  win->on_copy = explorer_on_copy;
  win->on_cut = explorer_on_cut;
  win->on_paste = explorer_on_paste;
  win->on_close = explorer_on_close;
  win->bg_color = 0xFFFFFFFF; // Pure White
  win->app_type = 5;
  win->blur_enabled = 0;        // Opaque
  win->scroll_line_height = 30; // smooth scroll

  // Reset state
  app->at_this_pc = 0;
  strcpy(app->explorer_path, "/");
  app->dialog_active = 0;
  app->selected_index = -1;
  app->hover_btn = -1;
  app->hovered_index = -1;
  app->hovered_sidebar = -1;
  app->sort_mode = 0;

  // Ensure standard folders exist
  fat_mkdir("/Downloads");
  fat_mkdir("/Desktop");
  fat_mkdir("/Documents");
  fat_mkdir("/Recents");

  // Default Pins
  strcpy(app->pinned_names[0], "Work");
  strcpy(app->pinned_paths[0], "/Documents/Work");
  strcpy(app->pinned_names[1], "Personal");
  strcpy(app->pinned_paths[1], "/Documents/Personal");
  app->pinned_count = 2;
  fat_mkdir("/Documents/Work");
  fat_mkdir("/Documents/Personal");

  explorer_refresh(win);
  win->needs_redraw = 1;
}
