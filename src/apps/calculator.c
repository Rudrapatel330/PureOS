#include "../kernel/ui_layout.h"
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
  int is_error;
  animation_t btn_anim[19];
  animation_t bg_flash_anims[19];
  animation_t error_shake_anim;
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
  if (calc->is_error && c != 0) {
    calc->is_error = 0;
    strcpy(calc->display, "0");
    calc->new_entry = 1;
  }
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
      if (val != 0) {
        result = calc->accumulator / val;
      } else {
        calc->is_error = 1;
        anim_start_spring(&calc->error_shake_anim, 20.0f, 0.0f, 150.0f, 8.0f);
        strcpy(calc->display, "Error");
        calc->new_entry = 1;
        calc->op = 0;
        return;
      }
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
  
  // Dynamic Layout Calculations
  int display_h = win->height * 20 / 100; // 20% of height
  if (display_h < 60) display_h = 60;
  if (display_h > 120) display_h = 120;
  
  int start_y = 32 + display_h;
  int bw = win->width / 4;
  int bh = (win->height - start_y) / 5;
  if (bh < 20) bh = 20;

  // Draw Background
  winmgr_fill_rect(win, 0, 32, win->width, win->height - 32, theme->bg);

  // 1. Draw Display Area (Top part)
  winmgr_fill_rect(win, 0, 32, win->width, display_h, theme->bg);

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
  // Auto-adjust scale if text is too wide
  while (len * (8 * scale) > win->width - 40 && scale > 1) {
    scale--;
  }
  
  int char_w = 8 * scale;
  int txt_x = win->width - (len * char_w) - 20;
  if (txt_x < 10) txt_x = 10;
  
  // Center text vertically in display area
  int txt_y = 32 + (display_h - (8 * scale)) / 2;

  anim_tick(&calc->error_shake_anim, 0.04f);
  if (calc->error_shake_anim.active) {
      win->needs_redraw = 1;
      extern int ui_dirty;
      ui_dirty = 1;
  }

  txt_x += (int)calc->error_shake_anim.current_val;
  uint32_t text_color = theme->fg;
  if (calc->is_error || calc->error_shake_anim.current_val > 1.0f || calc->error_shake_anim.current_val < -1.0f) {
      text_color = 0xFFFF5555; // Soft red
  }

  for (int i = 0; i < len; i++) {
    uint8_t c = (uint8_t)display_str[i];
    for (int py = 0; py < 8; py++) {
      uint8_t row = font8x8_basic[c][py];
      for (int px = 0; px < 8; px++) {
        if (row & (1 << (7 - px))) {
          winmgr_fill_rect(win, txt_x + i * char_w + px * scale,
                           txt_y + py * scale, scale, scale, text_color);
        }
      }
    }
  }

  // 2. Draw Button Grid
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

      // Tick animation
      anim_tick(&calc->btn_anim[label_idx], 0.04f);
      if (calc->btn_anim[label_idx].active) {
          win->needs_redraw = 1;
          extern int ui_dirty;
          ui_dirty = 1;
      }
      
      // Tick background flash animation
      anim_tick(&calc->bg_flash_anims[label_idx], 0.04f);
      if (calc->bg_flash_anims[label_idx].active) {
          win->needs_redraw = 1;
          extern int ui_dirty;
          ui_dirty = 1;
      }
      
      float btn_scale = calc->btn_anim[label_idx].current_val;
      if (btn_scale == 0.0f) btn_scale = 1.0f; // safety
      
      int base_size = (bw < bh ? bw : bh) - 16;
      if (base_size < 10) base_size = 10;
      
      int target_w = (current_bw > bw) ? current_bw - 16 : base_size;
      int target_h = base_size;

      int dw = (int)(target_w * btn_scale);
      int dh = (int)(target_h * btn_scale);
      int dx = bx + (current_bw - dw) / 2;
      int dy = by + (bh - dh) / 2;

      float flash_val = calc->bg_flash_anims[label_idx].current_val;
      uint32_t render_color = colors[label_idx];
      if (flash_val > 0.0f) {
          // --- GLOW EFFECT ---
          uint32_t glow_base = theme->accent;
          uint32_t g_r = (((glow_base >> 16) & 0xFF) + 255) / 2;
          uint32_t g_g = (((glow_base >> 8) & 0xFF) + 255) / 2;
          uint32_t g_b = ((glow_base & 0xFF) + 255) / 2;

          int halo_dw = target_w;
          int halo_dh = target_h;
          int halo_dx = bx + (current_bw - halo_dw) / 2;
          int halo_dy = by + (bh - halo_dh) / 2;

          for (int i = 4; i >= 1; i--) {
              int g_w = halo_dw + i * 4;
              int g_h = halo_dh + i * 4;
              int g_x = halo_dx - i * 2;
              int g_y = halo_dy - i * 2;
              
              int glow_alpha = (int)(flash_val * 160 / (i + 1)); 
              if (glow_alpha > 255) glow_alpha = 255;
              if (glow_alpha > 0) {
                  uint32_t glow_color = (glow_alpha << 24) | (g_r << 16) | (g_g << 8) | g_b;
                  winmgr_draw_rounded_rect_ex(win, g_x, g_y, g_w, g_h, glow_color, 0, 0, g_h / 2);
              }
          }
          // --- END GLOW ---

          uint32_t a = (render_color >> 24) & 0xFF;
          uint32_t r = (render_color >> 16) & 0xFF;
          uint32_t g = (render_color >> 8) & 0xFF;
          uint32_t b = render_color & 0xFF;
          r = (uint32_t)(r * (1.0f - 0.4f * flash_val));
          g = (uint32_t)(g * (1.0f - 0.4f * flash_val));
          b = (uint32_t)(b * (1.0f - 0.4f * flash_val));
          render_color = (a << 24) | (r << 16) | (g << 8) | b;
      }

      winmgr_draw_rounded_rect_ex(win, dx, dy, dw, dh, render_color, 0, 0, dh / 2);

      uint32_t txt_col = (colors[label_idx] == theme->accent)
                             ? theme->button_text
                             : theme->fg;
      int lbl_len = strlen(labels[label_idx]);
      
      int y_shift = (int)((1.0f - btn_scale) * 10) + (flash_val > 0.1f ? 1 : 0);
      winmgr_draw_text(win, dx + (dw - lbl_len * 8) / 2,
                       dy + (dh - 8) / 2 + y_shift, labels[label_idx], txt_col);

      label_idx++;
    }
  }
}

void calculator_click(window_t *win, int mx, int my, int buttons) {
  calc_app_t *calc = get_calc(win);
  if (!calc || !(buttons & 1))
    return;

  int display_h = win->height * 20 / 100;
  if (display_h < 60) display_h = 60;
  if (display_h > 120) display_h = 120;
  int start_y = 32 + display_h;

  if (my < start_y)
    return;

  int bw = win->width / 4;
  int bh = (win->height - start_y) / 5;
  if (bh <= 0) return;
  int r = (my - start_y) / bh;
  int c = mx / bw;

  if (r < 0 || r >= 5 || c < 0 || c >= 4) return;

  char *btn_chars = "An%/789*456-123+0.=";

  if (r == 4) {
    if (c <= 1) c = 0;
    else if (c == 2) c = 1;
    else if (c == 3) c = 2;
  }

  int idx = r * 4 + c;
  char cmd = btn_chars[idx];
  
  // Start spring animation (jump to 0.85 scale, spring back to 1.0)
  anim_start_spring(&calc->btn_anim[idx], 0.85f, 1.0f, SPRING_BOUNCY_K, SPRING_BOUNCY_D);
  // Start flash animation
  anim_start_spring(&calc->bg_flash_anims[idx], 1.0f, 0.0f, 150.0f, 15.0f);
  
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
  calc->is_error = 0;
  for (int i = 0; i < 19; i++) {
      anim_init_val(&calc->btn_anim[i], 1.0f);
      anim_init_val(&calc->bg_flash_anims[i], 0.0f);
  }
  anim_init_val(&calc->error_shake_anim, 0.0f);

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
