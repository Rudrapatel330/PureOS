#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"

extern uint32_t *backbuffer;
extern int screen_width, screen_height;
extern const uint8_t font8x8_basic[256][8];
extern void print_serial(const char *s);

typedef struct {
  window_t *win;
  char display[32];
  int accumulator;
  char op;
  int new_entry;
  int font_scale;
} calc_app_t;

static inline calc_app_t *get_calc(void *w) {
  return (calc_app_t *)((window_t *)w)->user_data;
}

// Colors matching the reference image approx
#define CALC_COL_DISPLAY_BG 0xFF3A3A3A
#define CALC_COL_BTN_FUNC 0xFFA5A5A5 // AC, +/-, %
#define CALC_COL_BTN_NUM 0xFF333333  // Numbers, .
#define CALC_COL_BTN_OP 0xFFFF9F0A   // Operators
#define CALC_COL_TEXT_BLK 0xFF000000
#define CALC_COL_TEXT_WHT 0xFFFFFFFF

#define MAX_DIGITS 9

static void calc_process_input(calc_app_t *calc, char c) {
  if (c >= '0' && c <= '9') {
    int len = 0;
    while (calc->display[len])
      len++;
    if (calc->new_entry) {
      calc->display[0] = c;
      calc->display[1] = 0;
      calc->new_entry = 0;
    } else {
      if (len < MAX_DIGITS) {
        calc->display[len] = c;
        calc->display[len + 1] = 0;
      }
    }
  } else if (c == '\b') { // Backspace
    if (calc->op != 0 && calc->new_entry) {
      calc->op = 0; // Erase the operator
    } else {
      int len = 0;
      while (calc->display[len])
        len++;
      if (len > 1) {
        calc->display[len - 1] = 0; // Erase last digit
      } else if (len == 1) {
        calc->display[0] = '0'; // Reset to 0
        calc->display[1] = 0;
        calc->new_entry = 1;
      }
    }
  } else if (c == '+' || c == '-' || c == '*' || c == '/') {
    if (calc->op != 0 && !calc->new_entry) {
      calc_process_input(calc, '=');
    }
    calc->accumulator = atoi(calc->display);
    calc->op = c;
    calc->new_entry = 1;
  } else if (c == '=' || c == '\n' || c == '\r') {
    int val = atoi(calc->display);
    int result = val;
    if (calc->op == '+')
      result = calc->accumulator + val;
    if (calc->op == '-')
      result = calc->accumulator - val;
    if (calc->op == '*')
      result = calc->accumulator * val;
    if (calc->op == '/') {
      if (val != 0)
        result = calc->accumulator / val;
      else
        result = 0;
    }

    // Convert result to string using kernel itoa
    k_itoa(result, calc->display);
    calc->new_entry = 1;
    calc->op = 0;
  } else if (c == 'C' || c == 'A') { // AC
    calc->display[0] = '0';
    calc->display[1] = 0;
    calc->accumulator = 0;
    calc->op = 0;
    calc->new_entry = 1;
  } else if (c == 'n') { // Negation +/-
    int val = atoi(calc->display);
    k_itoa(-val, calc->display);
  } else if (c == '%') { // Percentage
    int val = atoi(calc->display);
    k_itoa(val / 100, calc->display);
  }
}

// Draw a single character DIRECTLY to backbuffer
void calculator_draw(window_t *win) {
  calc_app_t *calc = get_calc(win);
  if (!win || !calc)
    return;

  const theme_t *theme = theme_get();
 
  // Draw Background
  winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, theme->bg);

  // 1. Draw Display Area (Top part)
  winmgr_fill_rect(win, 0, 24, win->width, 80, theme->bg);

  // Draw display text
  char display_str[64];
  if (calc->op != 0) {
    char accum_str[32];
    k_itoa(calc->accumulator, accum_str);
    int i = 0;
    while (accum_str[i]) {
      display_str[i] = accum_str[i];
      i++;
    }
    display_str[i++] = ' ';
    display_str[i++] = calc->op;
    display_str[i++] = ' ';
    if (!calc->new_entry) {
      int j = 0;
      while (calc->display[j]) {
        display_str[i++] = calc->display[j++];
      }
    }
    display_str[i] = 0;
  } else {
    int i = 0;
    while (calc->display[i] && i < 63) {
      display_str[i] = calc->display[i];
      i++;
    }
    display_str[i] = 0;
  }

  int len = strlen(display_str);

  // Custom scaled drawing
  int scale = calc->font_scale;
  int char_w = 8 * scale;
  int txt_x = win->width - (len * char_w) - 15;
  if (txt_x < 10)
    txt_x = 10;

  for (int i = 0; i < len; i++) {
    uint8_t c = (uint8_t)display_str[i];
    for (int py = 0; py < 8; py++) {
      uint8_t row = font8x8_basic[c][py];
      for (int px = 0; px < 8; px++) {
        if (row & (1 << (7 - px))) {
          winmgr_fill_rect(win, txt_x + i * char_w + px * scale,
                           45 + py * scale, scale, scale, theme->fg);
        }
      }
    }
  }

  // 2. Draw Button Grid
  int start_y = 104;
  int bw = win->width / 4;
  int bh = (win->height - start_y) / 5;

  const char *labels[] = {"AC", "+/-", "%", "/", "7", "8", "9", "*", "4", "5",
                          "6",  "-",   "1", "2", "3", "+", "0", ".", "="};

  uint32_t colors[] = {
      theme->button, theme->button, theme->button, theme->accent,
      theme->bg,     theme->bg,     theme->bg,     theme->accent,
      theme->bg,     theme->bg,     theme->bg,     theme->accent,
      theme->bg,     theme->bg,     theme->bg,     theme->accent,
      theme->bg,     theme->bg,     theme->accent};

  int label_idx = 0;
  for (int r = 0; r < 5; r++) {
    for (int c = 0; c < 4; c++) {
      if (label_idx >= 19)
        break;

      int bx = c * bw;
      int by = start_y + r * bh;
      int current_bw = bw;

      if (r == 4 && c == 0) {
        current_bw = bw * 2;
        c++;
      }

      winmgr_fill_rect(win, bx + 1, by + 1, current_bw - 2, bh - 2,
                       colors[label_idx]);

      uint32_t txt_col = (colors[label_idx] == theme->accent)
                             ? theme->button_text
                             : theme->fg;
      int lbl_len = strlen(labels[label_idx]);
      winmgr_draw_text(win, bx + (current_bw - lbl_len * 8) / 2,
                       by + (bh - 8) / 2, labels[label_idx], txt_col);

      label_idx++;
    }
  }
}

void calculator_click(window_t *win, int mx, int my, int buttons) {
  calc_app_t *calc = get_calc(win);
  if (!calc || !(buttons & 1))
    return;

  int start_y = 104;
  if (my < start_y)
    return;

  int bw = win->width / 4;
  int bh = (win->height - start_y) / 5;
  int r = (my - start_y) / bh;
  int c = mx / bw;

  char *btn_chars = "An%/789*456-123+0.=";

  if (r == 4) {
    if (c <= 1) c = 0;
    else if (c == 2) c = 1;
    else if (c == 3) c = 2;
  }

  int idx = r * 4 + c;
  char cmd = btn_chars[idx];
  if (cmd != ' ') {
    calc_process_input(calc, cmd);
    win->needs_redraw = 1;
    extern int ui_dirty;
    ui_dirty = 1;
  }
}

void calculator_on_key(window_t *win, int key, char ascii) {
  calc_app_t *calc = get_calc(win);
  if (!calc || ascii == 0)
    return;
  calc_process_input(calc, ascii);
  win->needs_redraw = 1;
  extern int ui_dirty;
  ui_dirty = 1;
}

void calculator_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  calc_app_t *calc = get_calc(win);
  if (!calc) return;

  if (direction > 0) {
    if (calc->font_scale < 5)
      calc->font_scale++;
  } else {
    if (calc->font_scale > 1)
      calc->font_scale--;
  }
  win->needs_redraw = 1;
}

static void calculator_on_close(void *w) {
  window_t *win = (window_t *)w;
  calc_app_t *calc = get_calc(win);
  if (calc) {
    kfree(calc);
    win->user_data = 0;
  }
}

void calculator_init() {
  window_t *win = winmgr_create_window(-1, -1, 320, 480, "Calculator");
  if (!win) return;

  calc_app_t *calc = (calc_app_t *)kmalloc(sizeof(calc_app_t));
  if (!calc) {
    winmgr_close_window(win);
    return;
  }

  for (int i = 0; i < (int)sizeof(calc_app_t); i++) ((char *)calc)[i] = 0;
  calc->win = win;
  strcpy(calc->display, "0");
  calc->new_entry = 1;
  calc->font_scale = 2;

  win->user_data = calc;
  win->draw = (void (*)(void *))calculator_draw;
  win->on_mouse = (void (*)(void *, int, int, int))calculator_click;
  win->on_key = (void (*)(void *, int, char))calculator_on_key;
  win->on_scroll = (void (*)(void *, int))calculator_on_scroll;
  win->on_close = calculator_on_close;
  win->bg_color = 0xFF1C1C1C;
  win->app_type = 1;
  win->cursor_pos = -1;
  win->needs_redraw = 1;
}
