#include "window.h"
#include "../drivers/font.h"
#include "../drivers/mouse.h"
#include "../drivers/rtc.h"
#include "../drivers/vga.h"
#include "../fs/fat12.h"
#include "../kernel/animation.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "animation.c"   // Include ThorVG-inspired Animation Engine
#include "editor_plus.c" // Include Enhanced Editor
#include "icons_data.h"
#include "paint.c" // Include Paint Implementation

// Globals from kernel.c
extern int mouse_x;
extern int mouse_y;
extern int mouse_buttons;
extern uint16_t *real_lfb;
extern uint32_t *backbuffer;
extern int screen_width;
extern int screen_height;

// Legacy support functions that were missing
int get_mouse_x() { return mouse_x; }
int get_mouse_y() { return mouse_y; }
int get_mouse_buttons() { return mouse_buttons; }
#include "sysmon.c" // Include SysMon Implementation

// Prototypes

// Prototypes
void draw_num(int x, int y, int num);
void launch_editor(const char *filename);
void launch_calculator();
void launch_paint();
void draw_calculator_content(Window *win);
void calc_process_input(Window *win, int key);

#ifndef NULL
#define NULL ((void *)0)
#endif

// Colors
// Colors (32-bit RGB)
#define COLOR_DESKTOP 0x00008080        // Teal
#define COLOR_WIN_BG 0x00C0C0C0         // Silver
#define COLOR_WIN_TITLE_BG 0x00000080   // Navy Blue
#define COLOR_WIN_TITLE_TEXT 0x00FFFFFF // White
#define COLOR_WIN_BORDER 0x00404040     // Dark Grey

// ================= STRUCTS =================

// Structs moved to header

void shell_execute(void *win, char *cmd);

// GLOBALS
#define MAX_WINDOWS 10
#define ICON_SIZE 64
#define ICON_SPACING 120
Window windows[MAX_WINDOWS];
Window *active_window = NULL;

#define MAX_ICONS 5
static Icon icons[MAX_ICONS];
static int num_icons = 0;

// Mouse Optimization State
static int last_mouse_x = -1;
static int last_mouse_y = -1;

void mouse_draw_lfb(int mx, int my) {
  // Hardcoded 2x Scale Cursor (10x10) from mouse.c logic but using direct LFB
  // write We can't access cursor_bitmap from here easily unless we export it or
  // move this logic. Ideally mouse.c should export "mouse_draw_at(x,y)". For
  // now, let's just make mouse_draw() in mouse.c use LFB if a flag is set? OR,
  // just use a simple block cursor here for speed. Let's use a White block with
  // Black border.

  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      // Simple Box Cursor
      uint32_t col = 0x00FFFFFF; // White
      if (x == 0 || x == 9 || y == 0 || y == 9)
        col = 0x00000000; // Black Border
      vga_put_pixel_lfb(mx + x, my + y, col, real_lfb);
    }
  }
}

// FORWARD DECLARATIONS
void draw_window_struct(Window *win);
void render_screen();
void render_scene();
void update_mouse_overlay(int mx, int my); // Optimization
int my_strlen(const char *str);
void terminal_print(Window *win, const char *str);
void draw_clock();

// ================= HELPERS =================

int my_strlen(const char *str) {
  int len = 0;
  while (str[len])
    len++;
  return len;
}

int my_strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void winmgr_minimize_window(Window *win) {
  if (!win)
    return;

  animation_start_window_minimize(win, win->x + win->width / 2, 768, 12);
}

void winmgr_maximize_window(Window *win) {
  if (!win)
    return;

  extern int screen_width;
  extern int screen_height;
  extern void animation_start_bounds_transition(Window *, int, int, int, int,
                                                int);

  if (!win->is_maximized) {
    int old_x = win->x;
    int old_y = win->y;
    int old_w = win->width;
    int old_h = win->height;

    // Save for recovery
    win->saved_x = win->x;
    win->saved_y = win->y;
    win->saved_w = win->width;
    win->saved_h = win->height;

    win->x = 0;
    win->y = 0;
    win->width = screen_width;
    win->height = screen_height - 40; // Desktop area
    win->is_maximized = 1;

    animation_start_bounds_transition(win, old_x, old_y, old_w, old_h, 30);
  } else {
    int old_x = win->x;
    int old_y = win->y;
    int old_w = win->width;
    int old_h = win->height;

    // Restore
    win->x = win->saved_x;
    win->y = win->saved_y;
    win->width = win->saved_w;
    win->height = win->saved_h;
    win->is_maximized = 0;

    animation_start_bounds_transition(win, old_x, old_y, old_w, old_h, 12);
  }
}

void init_window_manager() {
  for (int i = 0; i < MAX_WINDOWS; i++) {
    windows[i].is_valid = 0;
    windows[i].is_visible = 0;
    windows[i].id = i;
  }
  active_window = NULL;
}

Window *create_window(int x, int y, int w, int h, const char *title, int type) {
  // Find free slot
  for (int i = 0; i < MAX_WINDOWS; i++) {
    if (!windows[i].is_valid) {
      Window *win = &windows[i];
      win->is_valid = 1;
      win->is_visible = 1;
      win->type = type;
      win->x = x;
      win->y = y;
      win->width = w;
      win->height = h;
      win->is_dragging = 0;

      // Set Title
      int j = 0;
      while (title[j] && j < 31) {
        win->title[j] = title[j];
        j++;
      }
      win->title[j] = 0;

      // Init Type Specifics
      if (type == 1) { // Terminal
        win->term_history[0] = 0;
        win->term_pos = 0;
        win->term_buffer[0] = 0;
        win->history_count = 0;
        win->history_curr = 0;

        const char *welcome =
            "Welcome to PureOS Terminal\n--------------------------\nType "
            "'help'.\n\nPureOS /> ";
        int len = 0;
        while (welcome[len]) {
          win->term_history[len] = welcome[len];
          len++;
        }
        win->term_history[len] = 0;
      } else if (type == 2) { // File Manager
        win->file_count = fs_list_files("/", win->files, MAX_FILES_PER_WINDOW);
        win->selected_file_idx = -1;
      } else if (type == 3) { // Text Editor (Enhanced)
        editor_init(win);
      } else if (type == 4) { // Calculator
        win->calc_display[0] = '0';
        win->calc_display[1] = 0;
        win->calc_acc = 0;
        win->calc_op = 0;
        win->calc_new_entry = 1;
      } else if (type == 5) { // Paint
        paint_init(win);
      } else if (type == 6) { // SysMon
        sysmon_init(win);
      }

      // ThorVG Phase 3: Start Open Animation
      // Find the icon that matches this type to get start coordinates
      int start_x = screen_width / 2;
      int start_y = screen_height / 2;

      // Attempt to find the clicked icon
      for (int k = 0; k < num_icons; k++) {
        if (icons[k].selected) {
          start_x = icons[k].x + 16;
          start_y = icons[k].y + 16;
          break;
        }
      }

      // Start the scale-up animation (approx 300ms)
      animation_start_window_open(win, start_x, start_y,
                                  12); // 12 ticks ~ 50ms at 250Hz

      return win;
    }
  }
  return NULL;
}

// ================= DRAWING PRIMITIVES =================

void draw_bitmap_16(int x, int y, const uint8_t *bitmap, int selected,
                    int transparent_bg) {
  for (int r = 0; r < 16; r++) {
    for (int c = 0; c < 16; c++) {
      uint8_t pixel = bitmap[r * 16 + c];

      if (pixel == 0) {
        // Transparent
        if (!transparent_bg) {
          uint32_t bg = selected ? 0x00000080 : COLOR_DESKTOP;
          // Draw 2x2 BG
          vga_put_pixel(x + c * 2, y + r * 2, bg);
          vga_put_pixel(x + c * 2 + 1, y + r * 2, bg);
          vga_put_pixel(x + c * 2, y + r * 2 + 1, bg);
          vga_put_pixel(x + c * 2 + 1, y + r * 2 + 1, bg);
        }
      } else {
        // Draw Pixel
        uint32_t color = 0x00FFFFFF; // Default White
        switch (pixel) {
        case 1:
          color = 0x00000000;
          break; // Black
        case 2:
          color = 0x00FFFFFF;
          break; // White
        case 3:
          color = 0x000000AA;
          break; // Cyan/Blueish
        case 4:
          color = 0x00C0C0C0;
          break; // Grey
        case 6:
          color = 0x00FFFF55;
          break; // Yellow
        default:
          color = 0x00FFFFFF;
          break;
        }

        // Draw 2x2 Pixel
        vga_put_pixel(x + c * 2, y + r * 2, color);
        vga_put_pixel(x + c * 2 + 1, y + r * 2, color);
        vga_put_pixel(x + c * 2, y + r * 2 + 1, color);
        vga_put_pixel(x + c * 2 + 1, y + r * 2 + 1, color);
      }
    }
  }
}

// Defines for 32x32 Icons
void draw_text_terminal(Window *win, int x, int y, const char *str,
                        uint32_t color) {
  if (!str)
    return;
  int start_x = x;

  while (*str) {
    char c = *str;
    if (c == '\n') {
      x = start_x; // Reset to initial X
      y += 16;     // Line height (8 * 2)
    } else {
      // Draw if within bounds
      if (x < win->x + win->width - 10 && y < win->y + win->height - 16) {
        vga_draw_char(x, y, c, color);
      }
      x += 10; // 5*2 char + gap
    }
    str++;
  }
}

// Defines for 32x32 Icons
#define ICON_COLOR_BACKGROUND 0x00000000
#define ICON_COLOR_FRAME 0x00FFFFFF
#define ICON_COLOR_SCREEN 0x000000AA
#define ICON_COLOR_PROMPT 0x0055FF55 // Green
#define ICON_COLOR_TEXT 0x00FFFFFF
#define ICON_COLOR_TITLE 0x0000AAAA // Cyan

void draw_computer_icon_32x32(int x, int y) {
  // SCALED 2X (64x64)
  // Monitor (top part)
  vga_draw_rect(x + 12, y + 4, 40, 32, ICON_COLOR_FRAME);
  vga_draw_rect(x + 16, y + 8, 32, 24, ICON_COLOR_SCREEN);

  // Draw "PC" on screen
  vga_draw_char(x + 22, y + 14, 'P', ICON_COLOR_TEXT);
  vga_draw_char(x + 30, y + 14, 'C', ICON_COLOR_TEXT);

  // Draw @ symbol on screen
  vga_draw_char(x + 26, y + 18, '@', ICON_COLOR_PROMPT);

  // Monitor stand
  vga_draw_rect(x + 26, y + 36, 12, 8, ICON_COLOR_FRAME);

  // Keyboard (bottom part)
  vga_draw_rect(x + 8, y + 44, 48, 12, ICON_COLOR_FRAME);

  // Keyboard keys
  for (int i = 0; i < 3; i++) {
    vga_draw_rect(x + 14 + i * 12, y + 48, 8, 4, ICON_COLOR_BACKGROUND);
  }

  // Add glow/shadow effect
  vga_draw_rect(x + 10, y + 6, 2, 28, 0x00404040);
  vga_draw_rect(x + 52, y + 6, 2, 28, 0x00404040);
}

void draw_terminal_icon_32x32(int x, int y) {
  // SCALED 2X (64x64)
  vga_draw_rect(x + 8, y + 4, 48, 40, ICON_COLOR_FRAME);
  vga_draw_rect(x + 8, y + 4, 48, 8, ICON_COLOR_TITLE);

  vga_draw_char(x + 12, y + 4, 'T', ICON_COLOR_TEXT);
  vga_draw_char(x + 20, y + 4, 'E', ICON_COLOR_TEXT);
  vga_draw_char(x + 28, y + 4, 'R', ICON_COLOR_TEXT);
  vga_draw_char(x + 36, y + 4, 'M', ICON_COLOR_TEXT);

  vga_draw_rect(x + 12, y + 16, 40, 24, ICON_COLOR_BACKGROUND);

  vga_draw_char(x + 16, y + 20, '$', ICON_COLOR_PROMPT);
  vga_draw_char(x + 24, y + 20, '>', ICON_COLOR_PROMPT);
  vga_draw_char(x + 32, y + 20, 'l', ICON_COLOR_TEXT);
  vga_draw_char(x + 40, y + 20, 's', ICON_COLOR_TEXT);

  vga_draw_rect(x + 48, y + 20, 4, 16, ICON_COLOR_TEXT);
}

void draw_computer_content(Window *win) {
  int content_x = win->x + 2;
  int content_y = win->y + 20;
  int content_w = win->width - 4;   // 196
  int content_h = win->height - 22; // 98
  vga_draw_rect(content_x, content_y, content_w, content_h,
                0x00C0C0C0); // Light Grey

  int start_x = content_x + 10;
  int start_y = content_y + 10;
  int item_w = 110; // Increased from 60 to fit 10 chars * 10px
  int item_h = 60;  // Increased from 40
  int cols = 3; // Might need to adjust cols calculation based on window width?
  // Win width 200 -> 3 cols fits? 110*3 = 330. No.
  // Win width is usually 200. We need to make window bigger or cols fewer.
  // Let's set cols = 1 for now or resizing window.
  // Wait, with 1024 width, we can make the default window larger.

  cols = (content_w - 20) / item_w;
  if (cols < 1)
    cols = 1;

  for (int i = 0; i < win->file_count; i++) {
    int col = i % cols;
    int row = i / cols;

    int ix = start_x + (col * item_w);
    int iy = start_y + (row * item_h);

    // ... (Icon drawing remains same, offsets relative to ix/iy) ...

    // Check Selection
    int selected = (i == win->selected_file_idx);
    if (selected) {
      vga_draw_rect(ix - 2, iy - 2, 40, 56,
                    0x00000080); // Blue Hit Box (Taller)
    }

    // Draw Icon Procedurally (Scaled 2.5x approx)
    // Center in 110px width. Center roughly at offset 35-40.
    int icon_x = ix + 35;
    int icon_y = iy;

    if (win->files[i].is_dir) {
      // Folder: Yellow Rect (30x24)
      vga_draw_rect(icon_x, icon_y + 6, 30, 24, 0x00FFFF55);
      vga_draw_rect(icon_x, icon_y, 12, 6, 0x00FFFF55); // Tab
    } else {
      int len = my_strlen(win->files[i].name);
      int is_bin = (len > 4 && win->files[i].name[len - 3] == 'B' &&
                    win->files[i].name[len - 2] == 'I' &&
                    win->files[i].name[len - 1] == 'N');

      if (is_bin) {
        // Executable (30x34)
        vga_draw_rect(icon_x, icon_y, 28, 32, 0x00FFFFFF); // White Box
        vga_draw_rect(icon_x + 4, icon_y + 4, 20, 16,
                      0x000000AA); // Blue Screen
      } else {
        // File (26x32)
        vga_draw_rect(icon_x, icon_y, 24, 32, 0x00FFFFFF);    // White Page
        vga_draw_rect(icon_x + 16, icon_y, 8, 8, 0x00C0C0C0); // Fold
      }
    }

    // Draw Filename
    int lbl_x = ix + 5;  // Left padding
    int lbl_y = iy + 36; // Below icon (32 + 4)
    const char *name = win->files[i].name;

    int k = 0;
    while (name[k] && k < 12) { // Show a bit more
      uint32_t text_col = selected ? 0x00FFFFFF : 0x00000000;
      if (selected)
        text_col = 0x00FFFFFF;
      vga_draw_char(lbl_x + (k * 10), lbl_y, name[k], text_col); // Spacing 10
      k++;
    }
  }
}

void draw_icon(Icon *icon, int type) {
  if (type == 0) { // Computer
    draw_computer_icon_32x32(icon->x, icon->y);
  } else if (type == 1) { // Terminal
    draw_terminal_icon_32x32(icon->x, icon->y);
  } else if (type == 3) { // Paint (new icon)
    // Simple paint palette icon
    vga_draw_rect(icon->x + 16, icon->y + 16, 32, 32,
                  0x00C0C0C0); // Grey palette base
    vga_draw_rect(icon->x + 20, icon->y + 20, 8, 8, 0x00FF0000); // Red dot
    vga_draw_rect(icon->x + 30, icon->y + 20, 8, 8, 0x0000FF00); // Green dot
    vga_draw_rect(icon->x + 20, icon->y + 30, 8, 8, 0x000000FF); // Blue dot
    vga_draw_rect(icon->x + 30, icon->y + 30, 8, 8, 0x00FFFF00); // Yellow dot
    vga_draw_rect(icon->x + 40, icon->y + 16, 4, 16,
                  0x00804000); // Brush handle
  } else {
    // Fallback (Bitmap 16x16 -> Scaled 32x32)
    // Center in 64px slot. Offset = (64-32)/2 = 16.
    draw_bitmap_16(icon->x + 16, icon->y, icon->icon_data, icon->selected, 0);
  }

  // Label centered for 64px icon: x + 32
  int icon_center = icon->x + 32;
  int label_len = my_strlen(icon->label);
  int label_x = icon_center - ((label_len * 10) / 2); // 10px char width
  int label_y = icon->y + 48;                         // Closer to icon (was 70)

  const char *p = icon->label;
  int lx = label_x;
  while (*p) {
    if (icon->selected) {
      vga_draw_rect(lx, label_y, 10, 16, 0x00000080); // Blue BG
    }
    vga_draw_char(lx, label_y, *p, 0x00FFFFFF);
    lx += 10;
    p++;
  }
}

void redraw_icons() {
  for (int i = 0; i < num_icons; i++) {
    draw_icon(&icons[i], i);
  }
}
// ================= TERMINAL LOGIC =================

void terminal_print(Window *win, const char *str) {
  if (!win)
    return;
  int len = my_strlen(win->term_history);
  int add_len = my_strlen(str);

  if (len + add_len >= TERM_HIST_SIZE - 1) {
    win->term_history[0] = 0;
    len = 0;
  }

  char *dest = win->term_history + len;
  while (*str)
    *dest++ = *str++;
  *dest = 0;
}

void terminal_execute(Window *win) {
  if (!win)
    return;

  terminal_print(win, win->term_buffer);
  terminal_print(win, "\n");

  // Save to History
  if (my_strlen(win->term_buffer) > 0) {
    // Shift if full? Or ring buffer?
    // Simple: just fill up to 8, then rotate?
    if (win->history_count < 8) {
      // Copy to [count]
      int len = 0;
      while (win->term_buffer[len] && len < 63) {
        win->cmd_history[win->history_count][len] = win->term_buffer[len];
        len++;
      }
      win->cmd_history[win->history_count][len] = 0;
      win->history_count++;
    } else {
      // Rotate
      for (int i = 0; i < 7; i++) {
        // copy i+1 to i
        char *dst = win->cmd_history[i];
        char *src = win->cmd_history[i + 1];
        while (*src)
          *dst++ = *src++;
        *dst = 0;
      }
      // Copy to [7]
      int len = 0;
      while (win->term_buffer[len] && len < 63) {
        win->cmd_history[7][len] = win->term_buffer[len];
        len++;
      }
      win->cmd_history[7][len] = 0;
    }
  }
  win->history_curr = win->history_count; // Reset view to bottom

  // EXECUTE
  shell_execute(win, win->term_buffer);

  // prompt
  terminal_print(win, "\nPureOS ");
  char cwd[64];
  fs_pwd(cwd);
  terminal_print(win, cwd);
  terminal_print(win, "> ");

  win->term_pos = 0;
  win->term_buffer[0] = 0;
}

void draw_terminal_content(Window *win) {
  int content_x = win->x + 2;
  int content_y = win->y + 20;
  int content_w = win->width - 4;
  int content_h = win->height - 22;
  vga_draw_rect(content_x, content_y, content_w, content_h,
                0x00000000); // Black

  // 1. Draw History
  draw_text_terminal(win, content_x + 4, content_y + 4, win->term_history,
                     0x00FFFFFF);

  // 2. Find Current Cursor Position
  int lines = 0;
  const char *p = win->term_history;
  const char *line_start = p;

  while (*p) {
    if (*p == '\n') {
      lines++;
      line_start = p + 1;
    }
    p++;
  }

  int last_line_len = (int)(p - line_start);
  int prompt_y = content_y + 4 + (lines * 16);        // Line height 16
  int input_x = content_x + 4 + (last_line_len * 10); // Char width 10

  // 3. Draw Buffer
  draw_text_terminal(win, input_x, prompt_y, win->term_buffer, 0x00FFFFFF);

  // 4. Draw Cursor
  if (active_window == win) {
    int cursor_len = my_strlen(win->term_buffer);
    int cursor_visual_x = input_x + (cursor_len * 10);
    vga_draw_rect(cursor_visual_x, prompt_y, 8, 12, 0x00FFFFFF); // Cursor 8x12
  }
}

// Editor content drawing moved to editor_plus.c
void draw_calculator_content(Window *win) {
  // Window Width 240.
  // Display Width 200. Center x = (240-200)/2 = 20.
  int cx_disp = win->x + 20;
  int cy = win->y + 24;

  // Display Field
  vga_draw_rect(cx_disp, cy, 200, 32, 0x00FFFFFF); // White BG
  draw_text_terminal(win, cx_disp + 8, cy + 8, win->calc_display, 0x00000000);

  // Buttons
  // Grid Width: 4*40 + 3*8 = 160 + 24 = 184.
  // Center x = (240 - 184)/2 = 28.
  int cx_grid = win->x + 28;
  // Layout:
  // 7 8 9 /
  // 4 5 6 *
  // 1 2 3 -
  // C 0 = +

  const char *labels[] = {"7", "8", "9", "/", "4", "5", "6", "*",
                          "1", "2", "3", "-", "C", "0", "=", "+"};

  int btn_w = 40;        // Doubled
  int btn_h = 40;        // Doubled
  int gap = 8;           // Doubled
  int start_y = cy + 40; // Lower start

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      int bx = cx_grid + (col * (btn_w + gap));
      int by = start_y + (row * (btn_h + gap));

      // Draw Button
      vga_draw_rect(bx, by, btn_w, btn_h, 0x00C0C0C0); // Light Grey
      // Border
      vga_draw_rect(bx, by, btn_w, 2, 0x00FFFFFF);
      vga_draw_rect(bx, by, 2, btn_h, 0x00FFFFFF);
      vga_draw_rect(bx + btn_w - 2, by, 2, btn_h, 0x00404040);
      vga_draw_rect(bx, by + btn_h - 2, btn_w, 2, 0x00404040);

      // Label
      int idx = row * 4 + col;
      // Center char (10x16) in 40x40 btn.
      // x = 40/2 - 5 = 15
      // y = 40/2 - 8 = 12
      vga_draw_char(bx + 15, by + 12, labels[idx][0], 0x00000000);
    }
  }
}

// ================= INPUT HANDLERS =================

void winmgr_close_active(void) {
  if (active_window) {
    // ThorVG Phase 3: Start Close Animation instead of instant closing
    int dest_x = screen_width / 2;
    int dest_y = screen_height; // Taskbar area

    animation_start_window_close(active_window, dest_x, dest_y, 12); // 12 ticks

    // We don't NULL out active_window yet, animation.c will complete the
    // closure.
  }
}

// Tick animations from kernel loop
void winmgr_tick_animations(float dt) {
  (void)dt; // We use sys_ticks in animation.c mostly, but we can call update
  extern uint32_t sys_ticks;

  if (has_active_animations()) {
    animation_update(sys_ticks);
    // We'll trust the kernel's 'any_anim' check to trigger render_screen or
    // compositor For now, since Phase 1 doesn't have partial redraw fully wired
    // up securely, we'll just set ui_dirty so the kernel draws the next frame.
    extern int ui_dirty;
    ui_dirty = 1;
  }
}

void winmgr_cycle_window(void) {
  if (!active_window) {
    // Find first valid
    for (int i = 0; i < MAX_WINDOWS; i++) {
      if (windows[i].is_valid) {
        active_window = &windows[i];
        render_screen();
        break;
      }
    }
    return;
  }

  int current_id = active_window->id;
  for (int i = 1; i <= MAX_WINDOWS; i++) {
    int next_idx = (current_id + i) % MAX_WINDOWS;
    if (windows[next_idx].is_valid) {
      active_window = &windows[next_idx];
      render_screen();
      break;
    }
  }
}

// ================= INPUT HANDLERS =================

void window_handle_key(uint8_t scancode, char ascii) {
  if (!active_window)
    return;

  // Process Active Window
  Window *win = active_window;
  char c = ascii;

  if (win->type == 1) { // TERMINAL

    if (scancode == 0x0E) { // Backspace
      if (win->term_pos > 0) {
        win->term_pos--;
        win->term_buffer[win->term_pos] = 0;
        render_screen();
      }
    } else if (scancode == 0x1C) { // Enter
      terminal_execute(win);
      render_screen();
    } else if (scancode == 0x48) { // UP ARROW
      if (win->history_curr > 0) {
        win->history_curr--;
        // Copy history to buffer
        char *src = win->cmd_history[win->history_curr];
        int i = 0;
        while (src[i] && i < TERM_BUF_SIZE - 1) {
          win->term_buffer[i] = src[i];
          i++;
        }
        win->term_buffer[i] = 0;
        win->term_pos = i;
        render_screen();
      }
    } else if (scancode == 0x50) { // DOWN ARROW
      if (win->history_curr < win->history_count) {
        win->history_curr++;
        if (win->history_curr == win->history_count) {
          // New Line handling (Empty)
          win->term_buffer[0] = 0;
          win->term_pos = 0;
        } else {
          char *src = win->cmd_history[win->history_curr];
          int i = 0;
          while (src[i] && i < TERM_BUF_SIZE - 1) {
            win->term_buffer[i] = src[i];
            i++;
          }
          win->term_buffer[i] = 0;
          win->term_pos = i;
        }
        render_screen();
      }
    } else if (ascii != 0) {
      if (win->term_pos < TERM_BUF_SIZE - 1) {
        win->term_buffer[win->term_pos++] = ascii;
        win->term_buffer[win->term_pos] = 0;
        render_screen();
      }
    }
  } else if (win->type == 3) { // EDITOR
    // EDITOR INPUT
    editor_handle_key(
        win, ascii); // Editor might need updating too but keep ascii for now
    render_screen();
  } else if (win->type == 4) { // CALCULATOR
    // Filter keys
    if ((ascii >= '0' && ascii <= '9') || ascii == '+' || ascii == '-' ||
        ascii == '*' || ascii == '/' || ascii == '=' || ascii == '\n' ||
        ascii == 'c' || ascii == 'C' || ascii == '\b') {
      calc_process_input(win, ascii);
    }
  }
}

// ================= RENDERER =================

void draw_window_struct(Window *win) {
  if (!win->is_visible)
    return;

  vga_draw_rect(win->x, win->y, win->width, win->height, COLOR_WIN_BG);

  // Draw Border
  vga_draw_rect(win->x, win->y, win->width, 2, 0x00000000); // Top
  vga_draw_rect(win->x, win->y + win->height - 2, win->width, 2,
                0x00000000);                                 // Bottom
  vga_draw_rect(win->x, win->y, 2, win->height, 0x00000000); // Left
  vga_draw_rect(win->x + win->width - 2, win->y, 2, win->height,
                0x00000000); // Right

  // Scaled Title Bar (30px height)
  uint32_t title_color =
      (win == active_window)
          ? (win->is_dragging ? 0x00000040 : COLOR_WIN_TITLE_BG)
          : 0x00404040;
  vga_draw_rect(win->x + 2, win->y + 2, win->width - 4, 30, title_color);

  // Draw Title
  vga_draw_char(win->x + 6, win->y + 8, ' ', COLOR_WIN_TITLE_TEXT); // Padding
  vga_draw_string(win->x + 14, win->y + 8, win->title, COLOR_WIN_TITLE_TEXT);

  extern int is_window_animating(Window * win);
  if (is_window_animating(win))
    return;

  int btn_size = 20;
  int btn_spacing = 24;

  // Close Button (Red)
  int close_x = win->x + win->width - btn_size - 6;
  int close_y = win->y + 6;
  vga_draw_rect(close_x, close_y, btn_size, btn_size, 0x00AA0000);
  vga_draw_char(close_x + 6, close_y + 2, 'x', 0x00FFFFFF);

  // Maximize Button (Green)
  int max_x = close_x - btn_spacing;
  int max_y = win->y + 6;
  vga_draw_rect(max_x, max_y, btn_size, btn_size, 0x0000AA00);
  vga_draw_char(max_x + 6, max_y + 2, '^', 0x00FFFFFF);

  // Minimize Button (Yellow/Orange)
  int min_x = max_x - btn_spacing;
  int min_y = win->y + 6;
  vga_draw_rect(min_x, min_y, btn_size, btn_size, 0x00AAAA00);
  vga_draw_char(min_x + 6, min_y + 2, '_', 0x00FFFFFF);

  if (win->type == 1)
    draw_terminal_content(win);
  else if (win->type == 2)
    draw_computer_content(win);
  else if (win->type == 3)
    editor_draw(win);
  else if (win->type == 4)
    draw_calculator_content(win);
  else if (win->type == 5)
    paint_draw(win);
  else if (win->type == 6)
    sysmon_draw(win);
}

// Separated Scene Rendering (To Back Buffer)
void render_scene() {
  vga_clear_screen(COLOR_DESKTOP);

  // Draw Taskbar
  vga_draw_rect(0, 768 - 28, 1024, 28, 0x00C0C0C0);

  redraw_icons();

  for (int i = 0; i < MAX_WINDOWS; i++) {
    if (windows[i].is_valid && windows[i].is_visible) {
      // During animation, we might want to just draw the wireframe or draw the
      // window scaled. For now, if it's animating, we let draw_window_struct
      // handle the scaled bounds.

      // We need to override the window bounds temporarily to the animation
      // bounds during draw to achieve the scaling effect.
      int orig_x = windows[i].x;
      int orig_y = windows[i].y;
      int orig_w = windows[i].width;
      int orig_h = windows[i].height;

      // Check if this window is animating
      if (has_active_animations()) {
        // Find if this window is specifically animating
        for (int a = 0; a < MAX_ANIMATIONS; a++) {
          extern Animation animations[];
          if (animations[a].active && animations[a].target_win == &windows[i]) {
            windows[i].x = animations[a].curr_x;
            windows[i].y = animations[a].curr_y;
            windows[i].width = animations[a].curr_w;
            windows[i].height = animations[a].curr_h;
            break;
          }
        }
      }

      draw_window_struct(&windows[i]);

      // Restore original bounds for logic (clicks, etc.)
      windows[i].x = orig_x;
      windows[i].y = orig_y;
      windows[i].width = orig_w;
      windows[i].height = orig_h;
    }
  }

  // Clock
  draw_clock();
}

// Full Render: Scene -> BackBuffer -> Flip -> Mouse on Top
void render_screen() {
  render_scene();
  vga_flip(); // Copy Backbuffer to LFB

  int mx = get_mouse_x();
  int my = get_mouse_y();

  mouse_draw_lfb(mx, my);

  last_mouse_x = mx;
  last_mouse_y = my;
}

// Optimized Mouse Update (No Flip)
void update_mouse_overlay(int mx, int my) {
  if (last_mouse_x != -1) {
    // Restore background from Backbuffer (which has clean scene)
    // Cursor is 10x10
    vga_restore_rect(last_mouse_x, last_mouse_y, 10, 10);
  }

  mouse_draw_lfb(mx, my);

  last_mouse_x = mx;
  last_mouse_y = my;
}

void draw_clock() {
  rtc_time_t t;
  rtc_read(&t);
  char time_str[6];
  time_str[0] = (t.hour / 10) + '0';
  time_str[1] = (t.hour % 10) + '0';
  time_str[2] = ':';
  time_str[3] = (t.minute / 10) + '0';
  time_str[4] = (t.minute % 10) + '0';
  time_str[5] = 0;

  // Draw string at bottom right
  int cx = 950;
  int cy = 740; // Desktop clock
  for (int k = 0; k < 5; k++) {
    vga_draw_char(cx + (k * 8), cy, time_str[k], 0x00000000); // Black text
  }

  // Note: mouse_draw() and vga_flip() removed from here because render_screen
  // handles it
}

// ================= LOGIC =================

void init_icons() {
  num_icons = 0;

  // Computer Icon (32x32)
  icons[num_icons].x = 60; // Moved right
  icons[num_icons].y = 30;
  icons[num_icons].w = 32;
  icons[num_icons].h = 32;
  icons[num_icons].label = "Computer";
  icons[num_icons].icon_data =
      (const uint8_t *)icon_computer; // Kept for generic ID
  icons[num_icons].selected = 0;
  num_icons++;

  // Terminal Icon (32x32)
  icons[num_icons].x = 60;  // Moved right
  icons[num_icons].y = 150; // Spacing 120px (30 + 120)
  icons[num_icons].w = 32;
  icons[num_icons].h = 32;
  icons[num_icons].label = "Terminal";
  icons[num_icons].icon_data = (const uint8_t *)icon_terminal;
  icons[num_icons].selected = 0;
  num_icons++;

  // Calculator Icon (32x32 Placeholder for now, centered 16x16)
  // We can use the 16x16 bitmap directly or make a draw function.
  // Let's use the bitmap for now (Type 2).
  icons[num_icons].x = 60;  // Moved right
  icons[num_icons].y = 270; // Spacing 120px (150 + 120)
  icons[num_icons].w = 16;
  icons[num_icons].h = 16;
  icons[num_icons].label = "Calc";
  icons[num_icons].icon_data = (const uint8_t *)icon_calculator;
  icons[num_icons].selected = 0;
  num_icons++;

  // Paint Icon
  icons[num_icons].x = 60;
  icons[num_icons].y = 30 + (3 * ICON_SPACING);
  icons[num_icons].w = 32;
  icons[num_icons].h = 32;
  icons[num_icons].label = "Paint";
  icons[num_icons].selected = 0;
  num_icons++;

  // SysMon Icon
  icons[num_icons].x = 60;
  icons[num_icons].y = 30 + (4 * ICON_SPACING);
  icons[num_icons].w = 32;
  icons[num_icons].h = 32;
  icons[num_icons].label = "SysMon";
  icons[num_icons].selected = 0;
  num_icons++;
}

void launch_terminal() {
  // 20 lines high * 16 = 320.
  Window *w = create_window(100, 100, 600, 400, "Terminal", 1);
  if (w) {
    active_window = w;
    render_screen();
  }
}

void launch_computer() {
  // Scaled for larger icons
  Window *w = create_window(150, 150, 500, 400, "My Computer", 2);
  if (w) {
    active_window = w;
    render_screen();
  }
}

void launch_editor(const char *filename) {
  char title[64];
  const char *p = "Editing: ";
  int i = 0;
  while (p[i]) {
    title[i] = p[i];
    i++;
  }
  int j = 0;
  while (filename[j] && i < 63) {
    title[i++] = filename[j++];
  }
  title[i] = 0;

  title[i] = 0;

  Window *w = create_window(120, 120, 600, 450, title, 3);
  if (w) {
    // Load File
    editor_load(w, filename);
    active_window = w;
    render_screen();
  }
}

void calc_process_input(Window *win, int key) {
  char k = 0;
  if (key >= '0' && key <= '9')
    k = key;
  else if (key == '+')
    k = '+';
  else if (key == '-')
    k = '-';
  else if (key == '*')
    k = '*';
  else if (key == '/')
    k = '/';
  else if (key == '=' || key == '\n')
    k = '=';
  else if (key == 'c' || key == 'C' || key == '\b')
    k = 'C';

  if (k) {
    if (k >= '0' && k <= '9') {
      if (win->calc_new_entry) {
        win->calc_display[0] = k;
        win->calc_display[1] = 0;
        win->calc_new_entry = 0;
      } else {
        int len = my_strlen(win->calc_display);
        if (len < 10) {
          win->calc_display[len] = k;
          win->calc_display[len + 1] = 0;
        }
      }
    } else if (k == 'C') {
      win->calc_acc = 0;
      win->calc_op = 0;
      win->calc_new_entry = 1;
      win->calc_display[0] = '0';
      win->calc_display[1] = 0;
    } else if (k == '=') {
      int val = 0;
      const char *p = win->calc_display;
      int sign = 1;
      if (*p == '-') {
        sign = -1;
        p++;
      }
      while (*p) {
        val = val * 10 + (*p - '0');
        p++;
      }
      val *= sign;

      if (win->calc_op == 1)
        win->calc_acc += val;
      else if (win->calc_op == 2)
        win->calc_acc -= val;
      else if (win->calc_op == 3)
        win->calc_acc *= val;
      else if (win->calc_op == 4) {
        if (val != 0)
          win->calc_acc /= val;
      } else
        win->calc_acc = val;

      win->calc_op = 0;
      win->calc_new_entry = 1;

      if (win->calc_acc == 0) {
        win->calc_display[0] = '0';
        win->calc_display[1] = 0;
      } else {
        int t = win->calc_acc;
        int is_neg = 0;
        if (t < 0) {
          is_neg = 1;
          t = -t;
        }
        int i = 0;
        char buf[16];
        if (t == 0)
          buf[i++] = '0';
        while (t > 0) {
          buf[i++] = (t % 10) + '0';
          t /= 10;
        }
        if (is_neg)
          buf[i++] = '-';

        int j = 0;
        while (i > 0) {
          win->calc_display[j++] = buf[--i];
        }
        win->calc_display[j] = 0;
      }
    } else {
      // Operator
      int val = 0;
      const char *p = win->calc_display;
      int sign = 1;
      if (*p == '-') {
        sign = -1;
        p++;
      }
      while (*p) {
        val = val * 10 + (*p - '0');
        p++;
      }
      val *= sign;

      win->calc_acc = val;
      if (key == '+')
        win->calc_op = 1;
      else if (key == '-')
        win->calc_op = 2;
      else if (key == '*')
        win->calc_op = 3;
      else if (key == '/')
        win->calc_op = 4;

      win->calc_new_entry = 1;
    }
    render_screen();
  }
}

void launch_calculator() {
  // Scaled 2x (was 120x160) -> 240x320
  Window *w = create_window(300, 200, 240, 320, "Calculator", 4);
  if (w) {
    w->calc_acc = 0;
    w->calc_op = 0;
    w->calc_new_entry = 1;
    w->calc_display[0] = '0';
    w->calc_display[1] = 0;

    active_window = w;
    render_screen();
  }
}

void launch_paint() {
  Window *w = create_window(200, 150, 600, 450, "Paint", 5);
  if (w) {
    active_window = w;
    render_screen();
  }
}

void launch_sysmon() {
  Window *w = create_window(400, 300, 400, 300, "System Monitor", 6);
  if (w) {
    active_window = w;
    render_screen();
  }
}

void gui_init() {
  init_window_manager();
  init_icons();
  render_screen();
}

void handle_mouse_click(int mouse_x, int mouse_y, int buttons) {
  (void)buttons; // Suppress unused parameter warning
  int clicked_window = 0;

  // 1. Iterate Windows (Z-Order: Top to Bottom)
  for (int i = MAX_WINDOWS - 1; i >= 0; i--) {
    Window *w = &windows[i];
    if (!w->is_valid || !w->is_visible)
      continue;

    if (mouse_x >= w->x && mouse_x <= w->x + w->width && mouse_y >= w->y &&
        mouse_y <= w->y + w->height) {

      clicked_window = 1;
      active_window = w;

      int btn_size = 20; // Match Drawing
      int btn_x = w->x + w->width - btn_size - 6;
      int btn_y = w->y + 6;

      // Close Button
      if (mouse_x >= btn_x && mouse_x <= btn_x + btn_size && mouse_y >= btn_y &&
          mouse_y <= btn_y + btn_size) {
        w->is_valid = 0;
        w->is_visible = 0;
        active_window = NULL;
        render_screen();
        return;
      }

      // Maximize Button
      int max_x = btn_x - 24;
      if (mouse_x >= max_x && mouse_x <= max_x + btn_size && mouse_y >= btn_y &&
          mouse_y <= btn_y + btn_size) {
        winmgr_maximize_window(w);
        render_screen();
        return;
      }

      // Minimize Button
      int min_x = max_x - 24;
      if (mouse_x >= min_x && mouse_x <= min_x + btn_size && mouse_y >= btn_y &&
          mouse_y <= btn_y + btn_size) {
        winmgr_minimize_window(w);
        render_screen();
        return;
      }

      // Paint Interaction (Type 5)
      if (active_window->type == 5) {
        // Check if inside Toolbar
        if (mouse_y < active_window->y + 32 + 40) {
          paint_click(active_window, mouse_x, mouse_y);
        } else {
          // Click on Canvas starts drawing (handled in drag/move logic too)
          active_window->paint_prev_x = -1;             // Reset line start
          paint_click(active_window, mouse_x, mouse_y); // Initial dot
        }
      }

      // Calculator Clicks (Type 4)
      if (w->type == 4) {
        // Match DRAWING coordinates
        int cx_grid = w->x + 28;
        int start_y = w->y + 24 + 40; // cy + 40

        if (mouse_x >= cx_grid && mouse_y >= start_y) {
          int rel_x = mouse_x - cx_grid;
          int rel_y = mouse_y - start_y;

          // Button 40px, Gap 8px -> Stride 48
          int col = rel_x / 48;
          int row = rel_y / 48;

          // Check if inside button (ignore gap)
          int in_btn_x = (rel_x % 48) < 40;
          int in_btn_y = (rel_y % 48) < 40;

          if (col < 4 && row < 4 && in_btn_x && in_btn_y) {
            const char *labels[] = {"7", "8", "9", "/", "4", "5", "6", "*",
                                    "1", "2", "3", "-", "C", "0", "=", "+"};
            int idx = row * 4 + col;
            calc_process_input(w, labels[idx][0]);
          }
        }
      }
      // File Manager Clicks (Type 2)
      else if (w->type == 2) {
        int content_x = w->x + 2;
        int content_y = w->y + 20;
        int start_x = content_x + 10;
        int start_y = content_y + 10;
        int item_w = 60;
        int item_h = 40;
        int cols = 3;

        int col = (mouse_x - start_x);
        int row = (mouse_y - start_y);

        if (col >= 0 && row >= 0) {
          col /= item_w;
          row /= item_h;

          if (col < cols) {
            int idx = row * cols + col;
            if (idx >= 0 && idx < w->file_count) {
              if (w->selected_file_idx == idx) {
                // Double Click Logic
                if (w->files[idx].is_dir) {
                  if (fs_cd(w->files[idx].name)) {
                    w->file_count =
                        fs_list_files("/", w->files, MAX_FILES_PER_WINDOW);
                    w->selected_file_idx = -1;
                    render_screen();
                    return;
                  }
                } else {
                  // File Execution
                  char *name = w->files[idx].name;
                  int len = my_strlen(name);
                  if (len > 4) {
                    if (name[len - 3] == 'B' && name[len - 2] == 'I' &&
                        name[len - 1] == 'N') {
                      if (fs_read(name, (uint8_t *)0x20000)) {
                        void (*prog)() = (void *)0x20000;
                        prog();
                        render_screen();
                        return;
                      }
                    } else if (name[len - 3] == 'T' && name[len - 2] == 'X' &&
                               name[len - 1] == 'T') {
                      launch_editor(name);
                      return;
                    }
                  }
                }
              }
              w->selected_file_idx = idx;
              render_screen();
              return;
            }
          }
        }
        w->selected_file_idx = -1;
      }

      // Dragging (Title Bar check: y <= y + 40 for scaled title bar?)
      // Title bar was 18 high, maybe 36 now?
      // Windows not scaled yet. Keep as 20.
      if (mouse_y <= w->y + 20) {
        w->is_dragging = 1;
        w->drag_offset_x = mouse_x - w->x;
        w->drag_offset_y = mouse_y - w->y;
      }

      render_screen();
      return;
    }
  }

  // 2. Desktop Icons (if no window clicked)
  if (!clicked_window) {
    active_window = NULL;
    for (int i = 0; i < num_icons; i++) {
      // Hit tests for 64x64 icons (roughly)
      // Adjusted height to include label at y+48. (48 + 16 = 64).
      // Box of 64x64 is still fine.
      if (mouse_x >= icons[i].x && mouse_x < icons[i].x + 64 &&
          mouse_y >= icons[i].y && mouse_y < icons[i].y + 64) {

        icons[i].selected = 1;
        if (i == 0)
          launch_computer();
        if (i == 1)
          launch_terminal();
        if (i == 2)
          launch_calculator();
        if (i == 3)
          launch_paint();
        if (i == 4)
          launch_sysmon();
        render_screen();
        return;

      } else {
        icons[i].selected = 0;
      }
    }
    render_screen();
  }
}

void handle_mouse_move(int mouse_x, int mouse_y) {
  if (active_window && active_window->is_dragging) {
    active_window->x = mouse_x - active_window->drag_offset_x;
    active_window->y = mouse_y - active_window->drag_offset_y;
    render_screen();
  }
}

void handle_mouse_release() {
  if (active_window) {
    active_window->is_dragging = 0;
  }
}

void draw_num(int x, int y, int num) {
  char buf[16];
  int i = 0;
  if (num == 0) {
    buf[i++] = '0';
  } else {
    int temp = num;
    if (temp < 0) {
      vga_draw_char(x, y, '-', 0x0F);
      x += 4;
      temp = -temp;
    }
    int div = 1;
    while (temp / div >= 10)
      div *= 10;
    while (div > 0) {
      buf[i++] = (temp / div) + '0';
      temp %= div;
      div /= 10;
    }
  }
  buf[i] = 0;
  for (int k = 0; k < i; k++) {
    vga_draw_char(x + (k * 4), y, buf[k], 0x0F);
  }
}

void gui_update() {
  int mx = get_mouse_x();
  int my = get_mouse_y();
  int btns = get_mouse_buttons();

  static int prev_btns = 0;
  int left_click = (btns & 1) && !(prev_btns & 1);
  int left_release = !(btns & 1) && (prev_btns & 1);
  static int last_x = -1, last_y = -1;

  if (left_click) {
    handle_mouse_click(mx, my, btns);
  }

  if (left_release) {
    handle_mouse_release();
  }

  if (mx != last_x || my != last_y) {
    handle_mouse_move(mx, my);

    if (active_window && active_window->is_dragging) {
      render_screen(); // Full redraw for dragging
    } else if (active_window && active_window->type == 5 &&
               (get_mouse_buttons() & 1)) {
      // Paint Dragging
      paint_drag(active_window, mx, my);
      render_screen(); // Redraw to show paint
    } else {
      update_mouse_overlay(mx, my); // Fast path
    }

    last_x = mx;
    last_y = my;
  }

  // Update SysMon if active
  if (active_window && active_window->type == 6) {
    sysmon_update(active_window);
    render_screen(); // Needs redraw to animate graph
  }

  prev_btns = btns;
}
