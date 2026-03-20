#include "../drivers/keyboard.h"
#include "../drivers/vga.h"
#include "../fs/fat12.h"
#include "../kernel/mem.h"
#include "../kernel/string.h"
#include "../kernel/window.h"

```c
#include "../drivers/keyboard.h"
#include "../fs/fat12.h"
#include "../kernel/mem.h"
#include "../kernel/string.h"
#include "../kernel/window.h" // Already included or redundant if window.h has everything

#define EDITOR_MAX_LINES 1024
#define EDITOR_GUTTER_W 40
#define EDITOR_LINE_H 16
#define EDITOR_CHAR_W 10

// Colors
#define COL_BG 0xFFFFFFFF      // White
#define COL_GUTTER 0xFFE0E0E0  // Grey
#define COL_TEXT 0xFF000000    // Black
#define COL_KEYWORD 0xFF0000FF // Blue
#define COL_COMMENT 0xFF008000 // Green
#define COL_STRING 0xFFA00000  // Dark Red
#define COL_NUMBER 0xFF800080  // Purple

    void editor_init(window_t *win) {
  // Allocate pointer array
  win->editor_lines = (char **)malloc(EDITOR_MAX_LINES * sizeof(char *));
  win->editor_max_lines = EDITOR_MAX_LINES;

  // Create first empty line
  win->editor_lines[0] = (char *)malloc(1);
  win->editor_lines[0][0] = 0;

  win->editor_line_count = 1;
  win->editor_scroll_x = 0;
  win->editor_scroll_y = 0;
  win->editor_cursor_row = 0;
  win->editor_cursor_col = 0;
  win->editor_syntax_mode = 1; // Default to C for now
}

// Helper: Check if word is a keyword
int is_keyword(const char *word) {
  const char *keywords[] = {"int",     "char",     "void",     "return",
                            "if",      "else",     "while",    "for",
                            "struct",  "typedef",  "include",  "define",
                            "uint8_t", "uint32_t", "extern",   "static",
                            "const",   "break",    "continue", 0};
  int i = 0;
  while (keywords[i]) {
    if (strcmp(word, keywords[i]) == 0)
      return 1;
    i++;
  }
  return 0;
}

// Draw a single line with syntax highlighting
void editor_draw_line(window_t *win, int x, int y, const char *line) {
  int i = 0;
  int draw_x = x;

  while (line[i]) {
    // Simple Parser
    uint32_t color = COL_TEXT;

    // Comments //
    if (line[i] == '/' && line[i + 1] == '/') {
      // Rest of line is comment
      // We'll need a way to draw to window from here, or just draw to screen if
      // temporary But better draw to win->surface if possible. For now, assume
      // window-relative drawing.
      winmgr_draw_text(win, draw_x, y, line + i, COL_COMMENT);
      return;
    }

    // Strings "..."
    if (line[i] == '"') {
      // Draw string
      // For now just char by char
      color = COL_STRING;
    }

    // Keywords (Start of word)
    if ((i == 0 || line[i - 1] == ' ') && (line[i] >= 'a' && line[i] <= 'z')) {
      // Scan word
      char word[32];
      int k = 0;
      int j = i;
      while (line[j] && (line[j] >= 'a' && line[j] <= 'z') && k < 31) {
        word[k++] = line[j++];
      }
      word[k] = 0;
      if (is_keyword(word)) {
        // Draw keyword
        winmgr_draw_text(win, draw_x, y, word, COL_KEYWORD);
        draw_x += k * 10;
        i = j;
        continue;
      }
    }

    // Draw Char
    char c_str[2];
    c_str[0] = line[i];
    c_str[1] = 0;
    winmgr_draw_text(win, draw_x, y, c_str, color);
    draw_x += 10;
    i++;
  }
}

void editor_draw(window_t *win) {
  int w = win->width - 4;
  int h = win->height - 35;

  // Background (Relative to window content)
  winmgr_draw_rect(win, 2, 22, w, h, COL_BG);

  // Gutter
  winmgr_draw_rect(win, 2, 22, EDITOR_GUTTER_W, h, COL_GUTTER);

  int visible_lines = h / EDITOR_LINE_H;

  for (int i = 0; i < visible_lines; i++) {
    int line_idx = win->editor_scroll_y + i;
    if (line_idx >= win->editor_line_count)
      break;

    int draw_y = 22 + (i * EDITOR_LINE_H);

    // Draw Line Num
    char numbuf[8];
    int t = line_idx + 1;
    int k = 0;
    if (t == 0)
      numbuf[k++] = '0';
    while (t > 0) {
      numbuf[k++] = (t % 10) + '0';
      t /= 10;
    }
    // reverse
    for (int r = 0; r < k / 2; r++) {
      char tmp = numbuf[r];
      numbuf[r] = numbuf[k - 1 - r];
      numbuf[k - 1 - r] = tmp;
    }
    numbuf[k] = 0;
    winmgr_draw_text(win, 5, draw_y, numbuf, 0xFF404040);

    // Draw Line Content (Simple for now, syntax highlight needs more work for
    // surface)
    if (win->editor_lines[line_idx]) {
      editor_draw_line(win, EDITOR_GUTTER_W + 5, draw_y,
                       win->editor_lines[line_idx]);
    }
  }

  // Draw Cursor
  int cursor_scr_y =
      (win->editor_cursor_row - win->editor_scroll_y) * EDITOR_LINE_H;
  int cursor_scr_x = EDITOR_GUTTER_W + 5 + (win->editor_cursor_col * 10);

  if (cursor_scr_y >= 0 && cursor_scr_y < h) {
    int cx = cursor_scr_x;
    int cy = 22 + cursor_scr_y;
    winmgr_draw_rect(win, cx, cy, 2, EDITOR_LINE_H, 0xFF000000);
  }

  // Scrollbar (Simple indicator)
  if (win->editor_line_count > visible_lines) {
    int sb_h = (visible_lines * h) / win->editor_line_count;
    int sb_y = 22 + (win->editor_scroll_y * h) / win->editor_line_count;
    winmgr_draw_rect(win, w - 10, sb_y, 8, sb_h, 0xFF808080);
  }
}

void editor_toggle_comment(void) {
  extern window_t *active_window;
  window_t *win = active_window;
  if (!win || win->app_type != 4)
    return;

  int row = win->editor_cursor_row;
  if (row >= win->editor_line_count)
    return;

  char *line = win->editor_lines[row];
  int len = strlen(line);

  if (len >= 2 && line[0] == '/' && line[1] == '/') {
    // Remove comment
    char *new_line = (char *)malloc(len - 1);
    for (int i = 2; i <= len; i++)
      new_line[i - 2] = line[i];
    win->editor_lines[row] = new_line;
    if (win->editor_cursor_col >= 2)
      win->editor_cursor_col -= 2;
  } else {
    // Add comment
    char *new_line = (char *)malloc(len + 3);
    new_line[0] = '/';
    new_line[1] = '/';
    new_line[2] = ' ';
    for (int i = 0; i <= len; i++)
      new_line[i + 3] = line[i];
    win->editor_lines[row] = new_line;
    win->editor_cursor_col += 3;
  }
}

void editor_handle_key(window_t *win, int key) {
  if (key == 0x48) { // UP
    if (win->editor_cursor_row > 0)
      win->editor_cursor_row--;
  } else if (key == 0x50) { // DOWN
    if (win->editor_cursor_row < win->editor_line_count - 1)
      win->editor_cursor_row++;
  } else if (key == 0x4B) { // LEFT
    if (win->editor_cursor_col > 0)
      win->editor_cursor_col--;
  } else if (key == 0x4D) { // RIGHT
    int len = strlen(win->editor_lines[win->editor_cursor_row]);
    if (win->editor_cursor_col < len)
      win->editor_cursor_col++;
  } else if (key == '\n') { // ENTER
    if (win->editor_line_count < win->editor_max_lines - 1) {
      // Split Line
      // For MVP: Just insert empty line below and move cursor
      // TODO: Handle splitting text

      // Shift lines down
      for (int i = win->editor_line_count; i > win->editor_cursor_row + 1;
           i--) {
        win->editor_lines[i] = win->editor_lines[i - 1];
      }

      // Allocate new line
      win->editor_lines[win->editor_cursor_row + 1] = (char *)malloc(1);
      win->editor_lines[win->editor_cursor_row + 1][0] = 0;

      win->editor_line_count++;
      win->editor_cursor_row++;
      win->editor_cursor_col = 0;
    }
  } else if (key == '\b') { // BACKSPACE
    if (win->editor_cursor_col > 0) {
      // Delete char
      char *line = win->editor_lines[win->editor_cursor_row];
      int len = strlen(line);
      int pos = win->editor_cursor_col;

      // Shift left
      for (int i = pos - 1; i < len; i++) {
        line[i] = line[i + 1];
      }
      win->editor_cursor_col--;
    } else if (win->editor_cursor_row > 0) {
      // Select previous line (Merge TODO)
      win->editor_cursor_row--;
      win->editor_cursor_col =
          strlen(win->editor_lines[win->editor_cursor_row]);
    }
  } else if (key >= 32 && key <= 126) { // Typing
    // Insert char
    // Need to realloc string. Since we can't realloc, we malloc new and forget
    // old.
    char *old_line = win->editor_lines[win->editor_cursor_row];
    int len = strlen(old_line);

    char *new_line = (char *)malloc(len + 2);

    int pos = win->editor_cursor_col;
    for (int i = 0; i < pos; i++)
      new_line[i] = old_line[i];
    new_line[pos] = (char)key;
    for (int i = pos; i <= len; i++)
      new_line[i + 1] = old_line[i];

    win->editor_lines[win->editor_cursor_row] = new_line;
    win->editor_cursor_col++;
  }

  // Adjust Scroll
  int visible_lines = (win->height - 35) / EDITOR_LINE_H;
  if (win->editor_cursor_row < win->editor_scroll_y) {
    win->editor_scroll_y = win->editor_cursor_row;
  }
  if (win->editor_cursor_row >= win->editor_scroll_y + visible_lines) {
    win->editor_scroll_y = win->editor_cursor_row - visible_lines + 1;
  }

  // Save Command (F2 -> 128)
  if (key == 128) {
    // Parse filename from title. Title format: "Editing: <filename>" or just
    // "<filename>"? launch_editor creates title: "Editing: <filename>" or just
    // filename if reused logic? Let's check launch_editor in window.c It does:
    // `const char* p = "Editing: "; ... title` So we skip 9 chars.

    // Wait, title is inside win->title.
    // We need a helper to save.

    char *fname = win->title;
    if (strlen(fname) > 9 && fname[0] == 'E')
      fname += 9;

    // Serialize
    // Calculate total size
    int total_size = 0;
    for (int i = 0; i < win->editor_line_count; i++) {
      total_size += strlen(win->editor_lines[i]) + 1; // +1 for \n
    }

    char *buf = (char *)malloc(total_size + 1);
    int p = 0;
    for (int i = 0; i < win->editor_line_count; i++) {
      char *line = win->editor_lines[i];
      int l = strlen(line);
      for (int k = 0; k < l; k++)
        buf[p++] = line[k];
      buf[p++] = '\n';
    }
    // Remove last \n? usually text files end with \n
    // If file content had no \n at end, this adds one. That's fine.

    if (fs_write(fname, (uint8_t *)buf, p)) {
      // Visual feedback?
      // Flash gutter green?
      // For now, assume success.
      // Maybe draw "Saved" in status bar next draw
    }

    // free(buf); // No-op
  }
}

// Load file into editor buffer
void editor_load(window_t *win, const char *filename) {
  editor_init(win);

  // Read file content
  extern uint8_t disk_buffer_sector[512]; // Needed? No, use malloc buffer
  // fs_read to a large temp buffer then split?
  // Max file size?
  // For MVP, just assume small file

  // Let's use simple logic: if we have fs_read...
  // But fs_read reads to buffer.
  char *big_buf = (char *)malloc(32000); // 32KB buffer
  if (!big_buf)
    return;

  if (fs_read(filename, (uint8_t *)big_buf)) {
    // Split by \n
    win->editor_line_count = 0; // Reset

    int i = 0;
    int line_start = 0;
    while (big_buf[i]) {
      if (big_buf[i] == '\n') {
        // Determine length
        int len = i - line_start;
        // Copy
        char *line = (char *)malloc(len + 1);
        for (int k = 0; k < len; k++)
          line[k] = big_buf[line_start + k];
        line[len] = 0;

        win->editor_lines[win->editor_line_count++] = line;

        line_start = i + 1;
      }
      i++;
    }
    // Last line?
    if (i > line_start) {
      int len = i - line_start;
      char *line = (char *)malloc(len + 1);
      for (int k = 0; k < len; k++)
        line[k] = big_buf[line_start + k];
      line[len] = 0;
      win->editor_lines[win->editor_line_count++] = line;
    }

    // Ensure at least 1 line
    if (win->editor_line_count == 0) {
      win->editor_lines[0] = (char *)malloc(1);
      win->editor_lines[0][0] = 0;
      win->editor_line_count = 1;
    }
  }
}
