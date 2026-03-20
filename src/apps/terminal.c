#include "terminal.h"
#include "../fs/fs.h"
#include "../kernel/clipboard.h"
#include "../kernel/heap.h"
#include "../kernel/window.h"
#include "autocomplete.h"

// Helper to get length
int term_strlen(const char *str) {
  int len = 0;
  while (str[len])
    len++;
  return len;
}

// Helper to string compare
int term_strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

// Advanced Terminal State
#define MAX_LINES 200
#define LINE_LEN 128

typedef struct {
  char lines[MAX_LINES][LINE_LEN];
  int count;
  char input_buf[256];
  int input_pos;
  int scroll_offset;
  // Command History
  char history[32][256];
  int history_count;
  int history_browse;
  char saved_input[256];
  int saved_input_pos;
  int tab_count;
  char current_line[LINE_LEN];
  int current_lp;
} term_app_t;

// Track the most recently focused terminal window
static window_t *last_active_term = 0;

// Helper: get term_app_t from window
static term_app_t *get_term(void *win) {
  return (term_app_t *)((window_t *)win)->user_data;
}

void term_render_win(window_t *win) {
  if (!win)
    return;
  win->needs_redraw = 1;
  extern int ui_dirty;
  ui_dirty = 1;
}

void term_print_line_to(window_t *win, const char *text) {
  term_app_t *t = get_term(win);
  if (!t)
    return;

  if (t->count < MAX_LINES) {
    int idx = t->count++;
    int i = 0;
    while (text[i] && i < LINE_LEN - 1) {
      t->lines[idx][i] = text[i];
      i++;
    }
    t->lines[idx][i] = 0;
  } else {
    for (int i = 0; i < MAX_LINES - 1; i++) {
      for (int j = 0; j < LINE_LEN; j++)
        t->lines[i][j] = t->lines[i + 1][j];
    }
    int i = 0;
    while (text[i] && i < LINE_LEN - 1) {
      t->lines[MAX_LINES - 1][i] = text[i];
      i++;
    }
    t->lines[MAX_LINES - 1][i] = 0;
  }
  term_render_win(win);
}

#include "../shell/shell.h"

void terminal_handle_key_win(window_t *win, char c) {
  term_app_t *t = get_term(win);
  if (!t)
    return;

  // Tab completion
  if (c == '\t') {
    t->tab_count++;
    autocomplete(t->input_buf, &t->input_pos, t->tab_count - 1);
    term_render_win(win);
    return;
  }
  // Reset tab count on any other key
  t->tab_count = 0;

  if (c == '\n') {
    t->input_buf[t->input_pos] = 0;
    term_print_line_to(win, t->input_buf);

    // Save to history (if non-empty)
    if (t->input_pos > 0) {
      int idx = t->history_count % 32;
      for (int i = 0; i < 256; i++) {
        t->history[idx][i] = t->input_buf[i];
        if (!t->input_buf[i])
          break;
      }
      t->history_count++;
    }
    t->history_browse = -1;

    shell_execute(win, t->input_buf);

    t->input_pos = 0;
    t->input_buf[0] = 0;
  } else if (c == '\b') {
    if (t->input_pos > 0) {
      t->input_pos--;
      t->input_buf[t->input_pos] = 0;
    }
  } else {
    if (t->input_pos < 200) {
      t->input_buf[t->input_pos++] = c;
      t->input_buf[t->input_pos] = 0;
    }
  }
  term_render_win(win);
}

void terminal_on_key(void *win, int key, char c) {
  term_app_t *t = get_term(win);
  if (!t)
    return;

  // Track last active terminal
  last_active_term = (window_t *)win;

  // Arrow Up = History previous
  if (key == 0x48) {
    if (t->history_count == 0)
      return;

    if (t->history_browse == -1) {
      for (int i = 0; i < 256; i++) {
        t->saved_input[i] = t->input_buf[i];
        if (!t->input_buf[i])
          break;
      }
      t->saved_input_pos = t->input_pos;
      t->history_browse = t->history_count;
    }

    if (t->history_browse > 0 && t->history_browse > t->history_count - 32) {
      t->history_browse--;
      int idx = t->history_browse % 32;
      t->input_pos = 0;
      while (t->history[idx][t->input_pos]) {
        t->input_buf[t->input_pos] = t->history[idx][t->input_pos];
        t->input_pos++;
      }
      t->input_buf[t->input_pos] = 0;
    }
    term_render_win((window_t *)win);
    return;
  }

  // Arrow Down = History next
  if (key == 0x50) {
    if (t->history_browse == -1)
      return;

    t->history_browse++;
    if (t->history_browse >= t->history_count) {
      t->history_browse = -1;
      for (int i = 0; i < 256; i++) {
        t->input_buf[i] = t->saved_input[i];
        if (!t->saved_input[i])
          break;
      }
      t->input_pos = t->saved_input_pos;
    } else {
      int idx = t->history_browse % 32;
      t->input_pos = 0;
      while (t->history[idx][t->input_pos]) {
        t->input_buf[t->input_pos] = t->history[idx][t->input_pos];
        t->input_pos++;
      }
      t->input_buf[t->input_pos] = 0;
    }
    term_render_win((window_t *)win);
    return;
  }

  // Page Up = Scroll up
  if (key == 0x49) {
    t->scroll_offset += 5;
    int visible_lines = ((window_t *)win)->height > 30
                            ? (((window_t *)win)->height - 30) / 10
                            : 16;
    int max_scroll = t->count > visible_lines ? t->count - visible_lines : 0;
    if (t->scroll_offset > max_scroll)
      t->scroll_offset = max_scroll;
    term_render_win((window_t *)win);
    return;
  }
  // Page Down = Scroll down
  if (key == 0x51) {
    t->scroll_offset -= 5;
    if (t->scroll_offset < 0)
      t->scroll_offset = 0;
    term_render_win((window_t *)win);
    return;
  }
  if (c != 0) {
    t->scroll_offset = 0;
    t->history_browse = -1;
    terminal_handle_key_win((window_t *)win, c);
  }
}

void terminal_on_copy(window_t *win) {
  term_app_t *t = get_term(win);
  if (!t)
    return;
  // Copy the current input buffer content
  clipboard_copy(t->input_buf);
}

void terminal_on_paste(window_t *win, const char *text) {
  term_app_t *t = get_term(win);
  if (!t || !text)
    return;

  // Append text to input buffer
  for (int i = 0; text[i] && t->input_pos < 200; i++) {
    t->input_buf[t->input_pos++] = text[i];
  }
  t->input_buf[t->input_pos] = 0;
  term_render_win(win);
}

void terminal_on_scroll(void *win, int direction) {
  term_app_t *t = get_term(win);
  if (!t)
    return;

  if (direction > 0) {
    t->scroll_offset += 1;
    int visible_lines = ((window_t *)win)->height > 30
                            ? (((window_t *)win)->height - 30) / 10
                            : 16;
    int max_scroll = t->count > visible_lines ? t->count - visible_lines : 0;
    if (t->scroll_offset > max_scroll)
      t->scroll_offset = max_scroll;
  } else {
    t->scroll_offset -= 1;
    if (t->scroll_offset < 0)
      t->scroll_offset = 0;
  }
  term_render_win((window_t *)win);
}

void terminal_draw(window_t *win) {
  term_app_t *t = get_term(win);
  if (!t)
    return;

  int content_h = win->height - 30;
  int visible_lines = content_h / 10;

  int max_scroll = t->count > visible_lines ? t->count - visible_lines : 0;
  if (t->scroll_offset > max_scroll)
    t->scroll_offset = max_scroll;
  if (t->scroll_offset < 0)
    t->scroll_offset = 0;

  int end = t->count - t->scroll_offset;
  if (end < 0)
    end = 0;
  int start = end - (visible_lines - 1);
  if (start < 0)
    start = 0;

  int cy = 28;
  for (int i = start; i < end && i < MAX_LINES; i++) {
    winmgr_draw_text(win, 6, cy, t->lines[i], 0xFF00FF00); // Bright Green
    cy += 10;
  }

  char prompt[256] = "> ";
  int p = 2;
  for (int i = 0; t->input_buf[i] && p < 254; i++)
    prompt[p++] = t->input_buf[i];
  prompt[p] = 0;
  winmgr_draw_text(win, 6, cy, prompt, 0xFF00FF00); // Bright Green

  if (t->scroll_offset > 0) {
    winmgr_draw_text(win, win->width - 30, 28, "^^^", 0xFF00FF00); // Bright Green
  }
}

static void terminal_on_close(void *w) {
  window_t *win = (window_t *)w;
  if (win->user_data) {
    kfree(win->user_data);
    win->user_data = 0;
  }
  if (last_active_term == win) {
    last_active_term = 0;
  }
}

void terminal_init() {
  if (last_active_term && last_active_term->id != 0) {
    last_active_term->is_minimized = 0;
    winmgr_bring_to_front(last_active_term);
    return;
  }
  window_t *win = winmgr_create_window(100, 450, 400, 200, "Terminal");

  term_app_t *t = (term_app_t *)kmalloc(sizeof(term_app_t));
  for (int i = 0; i < (int)sizeof(term_app_t); i++)
    ((char *)t)[i] = 0;
  t->history_browse = -1;
  t->scroll_offset = 0;

  win->user_data = t;
  win->style = STYLE_TERMINAL;
  win->bg_color = 0xFF000000;
  win->app_type = 0; // Explicitly set terminal type
  win->blur_enabled = 0;
  win->on_key = terminal_on_key;
  win->on_copy = (void (*)(void *))terminal_on_copy;
  win->on_paste = (void (*)(void *, const char *))terminal_on_paste;
  win->on_scroll = (void (*)(void *, int))terminal_on_scroll;
  win->draw = (void (*)(void *))terminal_draw;
  win->on_close = terminal_on_close;
  win->cursor_pos = -1;

  last_active_term = win;
  t->current_lp = 0;
  t->current_line[0] = 0;
  term_print_line_to(win, "PureOS Terminal v2.0");
}

void terminal_clear() {
  // This needs to know which terminal - use last_active_term
  if (last_active_term && last_active_term->user_data) {
    term_app_t *t = (term_app_t *)last_active_term->user_data;
    t->count = 0;
    t->scroll_offset = 0;
    term_print_line_to(last_active_term, "PureOS Terminal v2.0");
  }
}

void terminal_print(window_t *win, const char *text) {
  if (!win || !win->user_data)
    return;
  term_app_t *t = (term_app_t *)win->user_data;

  for (int i = 0; text[i]; i++) {
    if (text[i] == '\n') {
      t->current_line[t->current_lp] = 0;
      term_print_line_to(win, t->current_line);
      t->current_lp = 0;
      t->current_line[0] = 0;
    } else if (text[i] >= ' ' && text[i] <= '~') {
      if (t->current_lp < LINE_LEN - 1) {
        t->current_line[t->current_lp++] = text[i];
        t->current_line[t->current_lp] = 0;
      }
    }
  }
  term_render_win(win);
}

void term_print(const char *text) {
  if (last_active_term && last_active_term->user_data) {
    terminal_print(last_active_term, text);
  }
}

void terminal_process_command(const char *c) { (void)c; }
