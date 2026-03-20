#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/window.h"

extern uint32_t *backbuffer;
extern int screen_width, screen_height;
extern const uint8_t font8x8_basic[256][8];
extern void print_serial(const char *s);

static window_t *calc_win = 0;
static char calc_display[32] = "0";
static int calc_accumulator = 0;
static char calc_op = 0;
static int calc_new_entry = 1;
static int calc_font_scale = 2; // Default 2x scale

// Colors matching the reference image approx
#define CALC_COL_DISPLAY_BG 0xFF3A3A3A
#define CALC_COL_BTN_FUNC 0xFFA5A5A5 // AC, +/-, %
#define CALC_COL_BTN_NUM 0xFF333333  // Numbers, .
#define CALC_COL_BTN_OP 0xFFFF9F0A   // Operators
#define CALC_COL_TEXT_BLK 0xFF000000
#define CALC_COL_TEXT_WHT 0xFFFFFFFF

// Draw a single character DIRECTLY to backbuffer
void calculator_draw(window_t *win) {
  if (!win)
    return;

  // Draw Dark Background
  winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, 0xFF1C1C1C);

  // 1. Draw Display Area (Top part)
  // Dark background for display
  winmgr_fill_rect(win, 0, 24, win->width, 80, 0xFF1C1C1C);

  // Draw display text (large, right-aligned)
  char display_str[64];
  if (calc_op != 0) {
    char accum_str[32];
    k_itoa(calc_accumulator, accum_str);
    int i = 0;
    while (accum_str[i]) {
      display_str[i] = accum_str[i];
      i++;
    }
    display_str[i++] = ' ';
    display_str[i++] = calc_op;
    display_str[i++] = ' ';
    if (!calc_new_entry) {
      int j = 0;
      while (calc_display[j]) {
        display_str[i++] = calc_display[j++];
      }
    }
    display_str[i] = 0;
  } else {
    int i = 0;
    while (calc_display[i] && i < 63) {
      display_str[i] = calc_display[i];
      i++;
    }
    display_str[i] = 0;
  }

  int len = 0;
  while (display_str[len])
    len++;

  // Custom scaled drawing for 'Wow' factor
  int scale = calc_font_scale;
  int char_w = 8 * scale;
  int txt_x = win->width - (len * char_w) - 15;
  if (txt_x < 10)
    txt_x = 10;

  // Manual scaling loop
  for (int i = 0; i < len; i++) {
    uint8_t c = (uint8_t)display_str[i];
    for (int py = 0; py < 8; py++) {
      uint8_t row = font8x8_basic[c][py];
      for (int px = 0; px < 8; px++) {
        if (row & (1 << (7 - px))) {
          winmgr_fill_rect(win, txt_x + i * char_w + px * scale,
                           45 + py * scale, scale, scale, CALC_COL_TEXT_WHT);
        }
      }
    }
  }

  // 2. Draw Button Grid (Starting below display)
  int start_y = 104;
  int bw = win->width / 4;
  int bh = (win->height - start_y) / 5;

  const char *labels[] = {"AC", "+/-", "%", "/", "7", "8", "9", "*", "4", "5",
                          "6",  "-",   "1", "2", "3", "+", "0", ".", "="};

  uint32_t colors[] = {
      CALC_COL_BTN_FUNC, CALC_COL_BTN_FUNC, CALC_COL_BTN_FUNC, CALC_COL_BTN_OP,
      CALC_COL_BTN_NUM,  CALC_COL_BTN_NUM,  CALC_COL_BTN_NUM,  CALC_COL_BTN_OP,
      CALC_COL_BTN_NUM,  CALC_COL_BTN_NUM,  CALC_COL_BTN_NUM,  CALC_COL_BTN_OP,
      CALC_COL_BTN_NUM,  CALC_COL_BTN_NUM,  CALC_COL_BTN_NUM,  CALC_COL_BTN_OP,
      CALC_COL_BTN_NUM,  CALC_COL_BTN_NUM,  CALC_COL_BTN_OP};

  int label_idx = 0;
  for (int r = 0; r < 5; r++) {
    for (int c = 0; c < 4; c++) {
      if (label_idx >= 19)
        break;

      int bx = c * bw;
      int by = start_y + r * bh;
      int current_bw = bw;

      // Special case for '0' button (spans 2 columns)
      if (r == 4 && c == 0) {
        current_bw = bw * 2;
        c++; // Skip next column
      }

      // Draw Button Base (Flat)
      winmgr_fill_rect(win, bx + 1, by + 1, current_bw - 2, bh - 2,
                       colors[label_idx]);

      // Draw Label
      uint32_t txt_col = (colors[label_idx] == CALC_COL_BTN_FUNC)
                             ? CALC_COL_TEXT_BLK
                             : CALC_COL_TEXT_WHT;
      int lbl_len = 0;
      while (labels[label_idx][lbl_len])
        lbl_len++;
      winmgr_draw_text(win, bx + (current_bw - lbl_len * 8) / 2,
                       by + (bh - 8) / 2, labels[label_idx], txt_col);

      label_idx++;
    }
  }
}

#define MAX_DIGITS 9

void calc_process_input(char c) {
  if (c >= '0' && c <= '9') {
    int len = 0;
    while (calc_display[len])
      len++;
    if (calc_new_entry) {
      calc_display[0] = c;
      calc_display[1] = 0;
      calc_new_entry = 0;
    } else {
      if (len < MAX_DIGITS) {
        calc_display[len] = c;
        calc_display[len + 1] = 0;
      }
    }
  } else if (c == '\b') { // Backspace
    if (calc_op != 0 && calc_new_entry) {
      calc_op = 0; // Erase the operator
    } else {
      int len = 0;
      while (calc_display[len])
        len++;
      if (len > 1) {
        calc_display[len - 1] = 0; // Erase last digit
      } else if (len == 1) {
        calc_display[0] = '0'; // Reset to 0
        calc_display[1] = 0;
        calc_new_entry = 1;
      }
    }
  } else if (c == '+' || c == '-' || c == '*' || c == '/') {
    if (calc_op != 0 && !calc_new_entry) {
      calc_process_input('=');
    }
    calc_accumulator = atoi(calc_display);
    calc_op = c;
    calc_new_entry = 1;
  } else if (c == '=' || c == '\n' || c == '\r') {
    int val = atoi(calc_display);
    int result = val;
    if (calc_op == '+')
      result = calc_accumulator + val;
    if (calc_op == '-')
      result = calc_accumulator - val;
    if (calc_op == '*')
      result = calc_accumulator * val;
    if (calc_op == '/') {
      if (val != 0)
        result = calc_accumulator / val;
      else
        result = 0;
    }

    // Convert result to string using kernel itoa
    k_itoa(result, calc_display);
    calc_new_entry = 1;
    calc_op = 0;
  } else if (c == 'C' || c == 'A') { // AC
    calc_display[0] = '0';
    calc_display[1] = 0;
    calc_accumulator = 0;
    calc_op = 0;
    calc_new_entry = 1;
  } else if (c == 'n') { // Negation +/-
    int val = atoi(calc_display);
    k_itoa(-val, calc_display);
  } else if (c == '%') { // Percentage
    int val = atoi(calc_display);
    k_itoa(val / 100, calc_display);
  }
}

void calculator_click(window_t *win, int mx, int my, int buttons) {
  if (!(buttons & 1))
    return;

  int rx = mx;
  int ry = my;

  int start_y = 104;
  if (ry < start_y)
    return;

  int bw = win->width / 4;
  int bh = (win->height - start_y) / 5;

  int r = (ry - start_y) / bh;
  int c = rx / bw;

  // Match grid logic from draw
  char *btn_chars = "An%/789*456-123+0.="; // A=AC, n=+/-

  // Check for spans
  if (r == 4) {
    if (c <= 1)
      c = 0; // '0' spans 1st and 2nd column
    else if (c == 2)
      c = 1; // '.' is next
    else if (c == 3)
      c = 2; // '=' is last
  }

  int idx = r * 4 + c;
  char cmd = btn_chars[idx];
  if (cmd != ' ') {
    calc_process_input(cmd);
    win->needs_redraw = 1;
    extern int ui_dirty;
    ui_dirty = 1;
  }
}

void calculator_on_key(window_t *win, int key, char ascii) {
  if (ascii == 0)
    return;
  calc_process_input(ascii);
  win->needs_redraw = 1;
  extern int ui_dirty;
  ui_dirty = 1;
}

void calculator_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  // Wheel UP (direction > 0) -> Increase font size
  if (direction > 0) {
    if (calc_font_scale < 5)
      calc_font_scale++;
  } else {
    if (calc_font_scale > 1)
      calc_font_scale--;
  }
  win->needs_redraw = 1;
}

static void calculator_on_close(void *w) {
  (void)w;
  calc_win = 0;
}

void calculator_init() {
  // Modern vertical layout matching reference
  calc_win = winmgr_create_window(300, 150, 240, 320, "Calculator");
  if (!calc_win) {
    print_serial("CALC: Failed to create window (OOM)\n");
    return;
  }
  calc_win->draw = (void (*)(void *))calculator_draw;
  calc_win->on_mouse = (void (*)(void *, int, int, int))calculator_click;
  calc_win->on_key = (void (*)(void *, int, char))calculator_on_key;
  calc_win->on_scroll = (void (*)(void *, int))calculator_on_scroll;
  calc_win->on_close = calculator_on_close;
  calc_win->bg_color = 0xFF1C1C1C;
  calc_win->blur_enabled = 0; // No blur as requested
  calc_win->app_type = 1;
  calc_win->cursor_pos = -1;

  calc_display[0] = '0';
  calc_display[1] = 0;
  calc_accumulator = 0;
  calc_op = 0;
  calc_new_entry = 1;
}
