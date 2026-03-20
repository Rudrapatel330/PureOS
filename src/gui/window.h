#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

#include "../fs/fat12.h" // For FileInfo

#define TERM_HIST_SIZE 2048
#define TERM_BUF_SIZE 256
#define MAX_FILES_PER_WINDOW 16

typedef struct {
  int id;            // Index in array
  int is_valid;      // 1 if active slot
  int x, y;          // Window position
  int width, height; // Window size
  char title[32];    // Window title
  int is_dragging;   // Is window being dragged?
  int drag_offset_x; // Mouse offset within window
  int drag_offset_y;
  int is_visible; // Is window visible?
  int type;       // 0=None, 1=Terminal, 2=Computer, 3=Editor

  // Terminal State
  char term_history[TERM_HIST_SIZE];
  char term_buffer[TERM_BUF_SIZE];
  int term_pos;

  // Command History
  char cmd_history[8][64];
  int history_count;
  int history_curr; // Index being viewed, -1 if new line

  // File Manager State
  FileInfo files[MAX_FILES_PER_WINDOW];
  int file_count;
  int selected_file_idx; // -1 if none

  // Editor State (Enhanced)
  char **editor_lines;    // Array of strings (pointers)
  int editor_line_count;  // Current number of lines
  int editor_max_lines;   // Max lines (capacity of array)
  int editor_scroll_y;    // Vertical scroll offset
  int editor_scroll_x;    // Horizontal scroll offset
  int editor_cursor_row;  // Cursor Line
  int editor_cursor_col;  // Cursor Column
  int editor_syntax_mode; // 0=Text, 1=C

  // Calculator State
  char calc_display[16];
  int calc_acc;       // Accumulator
  int calc_op;        // 0=None, 1=Add, 2=Sub, 3=Mul, 4=Div
  int calc_new_entry; // Flag to clear display on next digit

  // Paint State
  uint32_t *paint_canvas; // Pointer to pixel buffer (ARGB)
  uint32_t paint_color;   // Current drawing color
  int paint_brush_size;   // 1..5
  int paint_prev_x;
  int paint_prev_y;

  // System Monitor State
  int sysmon_history[60]; // Memory usage % for last 60 seconds
  uint32_t sysmon_last_tick;
  uint32_t sysmon_uptime;

  // Phase 2: Window States
  int is_minimized;
  int is_maximized;
  int saved_x, saved_y, saved_w, saved_h;
} Window;

void draw_window(int x, int y, int w, int h, const char *title);
void terminal_print(Window *win, const char *str);

// Icon Structure
typedef struct {
  int x, y;
  int w, h;
  const char *label;
  const uint8_t *icon_data; // Ptr to 16x16 or similar
  int selected;
} Icon;

void gui_init();
void gui_update();
void launch_calculator();
void window_handle_key(uint8_t scancode, char ascii); // Input Routing

#endif
