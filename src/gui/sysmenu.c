#include "sysmenu.h"
#include "../drivers/vga.h"
#include "../kernel/anim.h"
#include "../kernel/screen.h"

#define PANEL_W 400
#define PANEL_H 480
#define PAD 16
#define CORNER_R 14

// Colors - Ubuntu/GNOME dark style
#define SM_BG       0xFF2D2D2D
#define SM_BG2      0xFF3D3D3D
#define SM_ACTIVE   0xFF3584E4  // GNOME blue
#define SM_INACTIVE 0xFF4A4A4A
#define SM_TEXT      0xFFFFFFFF
#define SM_DIM       0xFFA0A0A0
#define SM_SLIDER_BG 0xFF4A4A4A
#define SM_SLIDER_FG 0xFFFFFFFF
#define SM_DIVIDER   0xFF404040

// Toggle item dimensions
#define TOGGLE_W   ((PANEL_W - PAD * 3) / 2)
#define TOGGLE_H   52
#define TOGGLE_GAP 10

// Slider dimensions
#define SLIDER_H    36
#define SLIDER_KNOB 16
#define SLIDER_BAR_H 4

// Top button row
#define BTN_SIZE    34
#define BTN_GAP     10

// State
static int sysmenu_active = 0;
static int sysmenu_closing = 0;
static animation_t anim_y;
static int panel_x = 0;
static uint32_t *sysmenu_surface = 0;
static int surface_dirty = 1;

// Toggle states (visual only)
static int wifi_on = 1;
static int bt_on = 0;
static int power_balanced = 1;
static int nightlight_on = 0;
static int darkstyle_on = 1;
static int keyboard_on = 1;
static int airplane_on = 0;

// Slider values (0-100)
static int volume_val = 60;
static int brightness_val = 50;

// Slider dragging
static int dragging_volume = 0;
static int dragging_brightness = 0;

extern int screen_width, screen_height;
extern void *kmalloc(uint32_t size);
extern void compositor_invalidate_rect(int x, int y, int w, int h);
extern void print_serial(const char *);

static void sysmenu_render_to_cache(void);

// ====== OPEN / CLOSE ======
static void sysmenu_close_done(void *data) {
  (void)data;
  sysmenu_active = 0;
  sysmenu_closing = 0;
  extern int ui_dirty;
  ui_dirty = 1;
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
}

void sysmenu_toggle(void) {
  extern int ui_dirty;
  float y_open = 28.0f; // Just below menubar

  if (!sysmenu_surface) {
    sysmenu_surface = (uint32_t *)kmalloc(PANEL_W * PANEL_H * 4);
  }

  if (!sysmenu_active && !sysmenu_closing) {
    // Open — show instantly (no animation for stability)
    print_serial("[SYSMENU] Opening\n");
    sysmenu_active = 1;
    sysmenu_closing = 0;
    panel_x = screen_width - PANEL_W - 12;
    surface_dirty = 1;
    sysmenu_render_to_cache();
    if (!sysmenu_surface) {
      print_serial("[SYSMENU] ERROR: surface is NULL after render!\n");
    }
    // Set position directly — no animation
    anim_y.current_val = y_open;
    anim_y.active = 0;
  } else {
    // Close — instant
    print_serial("[SYSMENU] Closing\n");
    sysmenu_active = 0;
    sysmenu_closing = 0;
    anim_y.active = 0;
  }

  ui_dirty = 1;
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
}

int sysmenu_is_active(void) { return sysmenu_active || sysmenu_closing; }
int sysmenu_is_animating(void) { return anim_y.active; }

void sysmenu_tick_animation(float dt) {
  if (anim_y.active) {
    int old_y = (int)anim_y.current_val;
    anim_tick(&anim_y, dt);
    int new_y = (int)anim_y.current_val;
    int min_y = (old_y < new_y) ? old_y : new_y;
    int max_y = (old_y > new_y) ? old_y : new_y;
    compositor_invalidate_rect(panel_x - 2, min_y - 2,
                               PANEL_W + 4, (max_y - min_y) + PANEL_H + 4);
    extern int ui_dirty;
    ui_dirty = 1;
  }
}

// ====== DRAWING HELPERS ======
static void draw_rounded_rect_sm(int x, int y, int w, int h, int r,
                                  uint32_t col, uint32_t *buf) {
  for (int py = y; py < y + h; py++) {
    if (py < 0 || py >= PANEL_H) continue;
    for (int px = x; px < x + w; px++) {
      if (px < 0 || px >= PANEL_W) continue;
      int draw = 1;
      // Top-left
      if (px < x + r && py < y + r) {
        int dx = (x + r) - px - 1, dy = (y + r) - py - 1;
        if (dx * dx + dy * dy >= r * r) draw = 0;
      }
      // Top-right
      else if (px >= x + w - r && py < y + r) {
        int dx = px - (x + w - r), dy = (y + r) - py - 1;
        if (dx * dx + dy * dy >= r * r) draw = 0;
      }
      // Bottom-left
      else if (px < x + r && py >= y + h - r) {
        int dx = (x + r) - px - 1, dy = py - (y + h - r);
        if (dx * dx + dy * dy >= r * r) draw = 0;
      }
      // Bottom-right
      else if (px >= x + w - r && py >= y + h - r) {
        int dx = px - (x + w - r), dy = py - (y + h - r);
        if (dx * dx + dy * dy >= r * r) draw = 0;
      }
      if (draw)
        buf[py * PANEL_W + px] = col;
    }
  }
}

static void draw_circle_sm(int cx, int cy, int r, uint32_t col,
                            uint32_t *buf) {
  for (int py = cy - r; py <= cy + r; py++) {
    if (py < 0 || py >= PANEL_H) continue;
    for (int px = cx - r; px <= cx + r; px++) {
      if (px < 0 || px >= PANEL_W) continue;
      int dx = px - cx, dy = py - cy;
      if (dx * dx + dy * dy <= r * r)
        buf[py * PANEL_W + px] = col;
    }
  }
}

// ====== RENDER TO SURFACE CACHE ======
static void sysmenu_render_to_cache(void) {
  if (!sysmenu_surface) return;
  uint32_t *buf = sysmenu_surface;

  // Clear to transparent
  for (int i = 0; i < PANEL_W * PANEL_H; i++)
    buf[i] = 0x00000000;

  // Background rounded rect
  draw_rounded_rect_sm(0, 0, PANEL_W, PANEL_H, CORNER_R, SM_BG, buf);

  int cy = PAD; // Current Y cursor

  // === ROW 1: Battery + Action Buttons ===
  // Battery pill
  draw_rounded_rect_sm(PAD, cy, 90, BTN_SIZE, 10, SM_BG2, buf);
  // Battery icon (small rect)
  vga_draw_rect_surface(PAD + 8, cy + 10, 14, 10, SM_TEXT, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(PAD + 22, cy + 13, 3, 4, SM_TEXT, buf, PANEL_W, PANEL_H);
  // Fill inside battery
  vga_draw_rect_surface(PAD + 10, cy + 12, 10, 6, 0xFF4ADE80, buf, PANEL_W, PANEL_H);
  vga_draw_string_surface(PAD + 30, cy + 10, "98 %", SM_TEXT, buf, PANEL_W, PANEL_H);

  // Right-side action buttons: Screenshot, Settings, Lock, Power
  int btn_x = PANEL_W - PAD - (BTN_SIZE * 4 + BTN_GAP * 3);

  // Screenshot button
  draw_rounded_rect_sm(btn_x, cy, BTN_SIZE, BTN_SIZE, 10, SM_BG2, buf);
  // Camera icon
  vga_draw_rect_surface(btn_x + 9, cy + 12, 16, 10, SM_TEXT, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(btn_x + 12, cy + 9, 10, 3, SM_TEXT, buf, PANEL_W, PANEL_H);
  draw_circle_sm(btn_x + 17, cy + 17, 3, SM_BG2, buf);
  btn_x += BTN_SIZE + BTN_GAP;

  // Settings button
  draw_rounded_rect_sm(btn_x, cy, BTN_SIZE, BTN_SIZE, 10, SM_BG2, buf);
  // Gear icon (simplified)
  draw_circle_sm(btn_x + 17, cy + 17, 7, SM_TEXT, buf);
  draw_circle_sm(btn_x + 17, cy + 17, 4, SM_BG2, buf);
  btn_x += BTN_SIZE + BTN_GAP;

  // Lock button
  draw_rounded_rect_sm(btn_x, cy, BTN_SIZE, BTN_SIZE, 10, SM_BG2, buf);
  // Lock icon
  vga_draw_rect_surface(btn_x + 11, cy + 16, 12, 10, SM_TEXT, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(btn_x + 13, cy + 10, 8, 8, SM_BG2, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(btn_x + 14, cy + 11, 6, 5, SM_TEXT, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(btn_x + 15, cy + 12, 4, 3, SM_BG2, buf, PANEL_W, PANEL_H);
  btn_x += BTN_SIZE + BTN_GAP;

  // Power button
  draw_rounded_rect_sm(btn_x, cy, BTN_SIZE, BTN_SIZE, 10, SM_BG2, buf);
  // Power icon
  draw_circle_sm(btn_x + 17, cy + 18, 6, SM_TEXT, buf);
  draw_circle_sm(btn_x + 17, cy + 18, 4, SM_BG2, buf);
  vga_draw_rect_surface(btn_x + 16, cy + 10, 2, 8, SM_TEXT, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(btn_x + 16, cy + 12, 2, 3, SM_BG2, buf, PANEL_W, PANEL_H);

  cy += BTN_SIZE + 18;

  // === VOLUME SLIDER ===
  // Speaker icon
  vga_draw_rect_surface(PAD + 2, cy + 10, 6, 12, SM_DIM, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(PAD + 8, cy + 6, 4, 20, SM_DIM, buf, PANEL_W, PANEL_H);
  // Volume bar
  int sl_x = PAD + 40;
  int sl_w = PANEL_W - PAD * 2 - 40;
  vga_draw_rect_surface(sl_x, cy + 14, sl_w, SLIDER_BAR_H, SM_SLIDER_BG,
                         buf, PANEL_W, PANEL_H);
  // Filled portion
  int vol_fill = (volume_val * sl_w) / 100;
  vga_draw_rect_surface(sl_x, cy + 14, vol_fill, SLIDER_BAR_H, SM_SLIDER_FG,
                         buf, PANEL_W, PANEL_H);
  // Knob
  int knob_x = sl_x + vol_fill - SLIDER_KNOB / 2;
  draw_circle_sm(knob_x + SLIDER_KNOB / 2, cy + 16, SLIDER_KNOB / 2,
                 SM_SLIDER_FG, buf);

  cy += SLIDER_H + 8;

  // === BRIGHTNESS SLIDER ===
  // Sun icon (star shape approximation)
  draw_circle_sm(PAD + 10, cy + 16, 5, SM_DIM, buf);
  draw_circle_sm(PAD + 10, cy + 16, 2, SM_BG, buf);
  // Rays
  vga_draw_rect_surface(PAD + 9, cy + 8, 2, 3, SM_DIM, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(PAD + 9, cy + 22, 2, 3, SM_DIM, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(PAD + 2, cy + 15, 3, 2, SM_DIM, buf, PANEL_W, PANEL_H);
  vga_draw_rect_surface(PAD + 15, cy + 15, 3, 2, SM_DIM, buf, PANEL_W, PANEL_H);
  // Brightness bar
  vga_draw_rect_surface(sl_x, cy + 14, sl_w, SLIDER_BAR_H, SM_SLIDER_BG,
                         buf, PANEL_W, PANEL_H);
  int brt_fill = (brightness_val * sl_w) / 100;
  vga_draw_rect_surface(sl_x, cy + 14, brt_fill, SLIDER_BAR_H, SM_SLIDER_FG,
                         buf, PANEL_W, PANEL_H);
  int bknob_x = sl_x + brt_fill - SLIDER_KNOB / 2;
  draw_circle_sm(bknob_x + SLIDER_KNOB / 2, cy + 16, SLIDER_KNOB / 2,
                 SM_SLIDER_FG, buf);

  cy += SLIDER_H + 14;

  // === TOGGLE GRID ===
  // Row 1: Wi-Fi | Bluetooth
  {
    uint32_t wc = wifi_on ? SM_ACTIVE : SM_INACTIVE;
    draw_rounded_rect_sm(PAD, cy, TOGGLE_W, TOGGLE_H, 12, wc, buf);
    // Wi-Fi icon (simplified arcs)
    draw_circle_sm(PAD + 20, cy + 20, 10, wifi_on ? 0xFF93C5FD : SM_DIM, buf);
    draw_circle_sm(PAD + 20, cy + 20, 7, wc, buf);
    draw_circle_sm(PAD + 20, cy + 20, 6, wifi_on ? 0xFF93C5FD : SM_DIM, buf);
    draw_circle_sm(PAD + 20, cy + 20, 3, wc, buf);
    draw_circle_sm(PAD + 20, cy + 22, 2, wifi_on ? 0xFF93C5FD : SM_DIM, buf);
    vga_draw_string_surface(PAD + 38, cy + 10, "Wi-Fi", SM_TEXT, buf,
                            PANEL_W, PANEL_H);
    vga_draw_string_surface(PAD + 38, cy + 28, wifi_on ? "Connected" : "Off",
                            wifi_on ? 0xFFD0E8FF : SM_DIM, buf, PANEL_W, PANEL_H);
    // ">" arrow
    vga_draw_string_surface(PAD + TOGGLE_W - 18, cy + 18, ">", SM_DIM,
                            buf, PANEL_W, PANEL_H);
  }

  {
    int tx = PAD * 2 + TOGGLE_W;
    uint32_t bc = bt_on ? SM_ACTIVE : SM_INACTIVE;
    draw_rounded_rect_sm(tx, cy, TOGGLE_W, TOGGLE_H, 12, bc, buf);
    // Bluetooth icon (B shape)
    vga_draw_rect_surface(tx + 18, cy + 12, 2, 28, bt_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    vga_draw_rect_surface(tx + 20, cy + 14, 4, 2, bt_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    vga_draw_rect_surface(tx + 20, cy + 36, 4, 2, bt_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    vga_draw_rect_surface(tx + 14, cy + 20, 4, 2, bt_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    vga_draw_rect_surface(tx + 14, cy + 30, 4, 2, bt_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    vga_draw_string_surface(tx + 38, cy + 10, "Bluetooth", SM_TEXT, buf,
                            PANEL_W, PANEL_H);
    vga_draw_string_surface(tx + 38, cy + 28, bt_on ? "On" : "Off",
                            bt_on ? 0xFFD0E8FF : SM_DIM, buf, PANEL_W, PANEL_H);
    vga_draw_string_surface(tx + TOGGLE_W - 18, cy + 18, ">", SM_DIM,
                            buf, PANEL_W, PANEL_H);
  }

  cy += TOGGLE_H + TOGGLE_GAP;

  // Row 2: Power Mode | Night Light
  {
    uint32_t pc = power_balanced ? SM_ACTIVE : SM_INACTIVE;
    draw_rounded_rect_sm(PAD, cy, TOGGLE_W, TOGGLE_H, 12, pc, buf);
    // Battery/power icon
    draw_circle_sm(PAD + 20, cy + 22, 8, power_balanced ? 0xFF93C5FD : SM_DIM, buf);
    draw_circle_sm(PAD + 20, cy + 22, 5, pc, buf);
    vga_draw_string_surface(PAD + 38, cy + 10, "Power Mode", SM_TEXT, buf,
                            PANEL_W, PANEL_H);
    vga_draw_string_surface(PAD + 38, cy + 28, "Balanced", SM_DIM, buf,
                            PANEL_W, PANEL_H);
    vga_draw_string_surface(PAD + TOGGLE_W - 18, cy + 18, ">", SM_DIM,
                            buf, PANEL_W, PANEL_H);
  }

  {
    int tx = PAD * 2 + TOGGLE_W;
    uint32_t nc = nightlight_on ? SM_ACTIVE : SM_INACTIVE;
    draw_rounded_rect_sm(tx, cy, TOGGLE_W, TOGGLE_H, 12, nc, buf);
    // Sun/moon icon
    draw_circle_sm(tx + 20, cy + 22, 7, nightlight_on ? 0xFFFBBF24 : SM_DIM, buf);
    draw_circle_sm(tx + 24, cy + 18, 6, nc, buf); // Moon cutout
    vga_draw_string_surface(tx + 38, cy + 14, "Night Light", SM_TEXT, buf,
                            PANEL_W, PANEL_H);
    vga_draw_string_surface(tx + 38, cy + 30, nightlight_on ? "On" : "Off",
                            SM_DIM, buf, PANEL_W, PANEL_H);
  }

  cy += TOGGLE_H + TOGGLE_GAP;

  // Row 3: Dark Style | Keyboard
  {
    uint32_t dc = darkstyle_on ? SM_ACTIVE : SM_INACTIVE;
    draw_rounded_rect_sm(PAD, cy, TOGGLE_W, TOGGLE_H, 12, dc, buf);
    // Half circle (dark/light theme)
    draw_circle_sm(PAD + 20, cy + 22, 8, SM_TEXT, buf);
    vga_draw_rect_surface(PAD + 20, cy + 14, 9, 17, dc, buf, PANEL_W, PANEL_H);
    vga_draw_string_surface(PAD + 38, cy + 14, "Dark Style", SM_TEXT, buf,
                            PANEL_W, PANEL_H);
    vga_draw_string_surface(PAD + 38, cy + 30, darkstyle_on ? "On" : "Off",
                            SM_DIM, buf, PANEL_W, PANEL_H);
  }

  {
    int tx = PAD * 2 + TOGGLE_W;
    uint32_t kc = keyboard_on ? SM_ACTIVE : SM_INACTIVE;
    draw_rounded_rect_sm(tx, cy, TOGGLE_W, TOGGLE_H, 12, kc, buf);
    // Keyboard icon
    vga_draw_rect_surface(tx + 8, cy + 14, 24, 16, keyboard_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    vga_draw_rect_surface(tx + 10, cy + 16, 20, 12, kc, buf, PANEL_W, PANEL_H);
    // Key dots
    for (int kr = 0; kr < 2; kr++)
      for (int kc2 = 0; kc2 < 4; kc2++)
        vga_draw_rect_surface(tx + 12 + kc2 * 5, cy + 18 + kr * 5, 3, 3,
                              keyboard_on ? 0xFF93C5FD : SM_DIM,
                              buf, PANEL_W, PANEL_H);
    // Spacebar
    vga_draw_rect_surface(tx + 14, cy + 28, 12, 2,
                          keyboard_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    vga_draw_string_surface(tx + 38, cy + 14, "Keyboard", SM_TEXT, buf,
                            PANEL_W, PANEL_H);
    vga_draw_string_surface(tx + TOGGLE_W - 18, cy + 18, ">", SM_DIM,
                            buf, PANEL_W, PANEL_H);
  }

  cy += TOGGLE_H + TOGGLE_GAP;

  // Row 4: Airplane Mode (single, half width)
  {
    uint32_t ac = airplane_on ? SM_ACTIVE : SM_INACTIVE;
    draw_rounded_rect_sm(PAD, cy, TOGGLE_W, TOGGLE_H, 12, ac, buf);
    // Airplane icon
    vga_draw_rect_surface(PAD + 17, cy + 10, 4, 22,
                          airplane_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    // Wings
    vga_draw_rect_surface(PAD + 8, cy + 18, 22, 4,
                          airplane_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    // Tail
    vga_draw_rect_surface(PAD + 14, cy + 28, 10, 3,
                          airplane_on ? 0xFF93C5FD : SM_DIM,
                          buf, PANEL_W, PANEL_H);
    vga_draw_string_surface(PAD + 38, cy + 18, "Airplane Mode", SM_TEXT, buf,
                            PANEL_W, PANEL_H);
  }

  surface_dirty = 0;
}

// ====== DRAW TO FRAMEBUFFER ======
void sysmenu_draw(uint32_t *buffer, rect_t clip) {
  if (!sysmenu_is_active() || !sysmenu_surface)
    return;

  if (surface_dirty)
    sysmenu_render_to_cache();

  int cy = (int)anim_y.current_val;
  int start_y = (cy > clip.y) ? cy : clip.y;
  int end_y = (cy + PANEL_H < clip.y + clip.h) ? (cy + PANEL_H)
                                                 : (clip.y + clip.h);
  if (start_y >= end_y) return;

  for (int y = start_y; y < end_y; y++) {
    int src_y = y - cy;
    if (src_y < 0 || src_y >= PANEL_H) continue;

    int start_x = (panel_x > clip.x) ? panel_x : clip.x;
    int end_x = (panel_x + PANEL_W < clip.x + clip.w)
                    ? (panel_x + PANEL_W) : (clip.x + clip.w);

    int dst_off = y * screen_width;
    int src_off = src_y * PANEL_W;

    for (int x = start_x; x < end_x; x++) {
      int sx = x - panel_x;
      if (sx < 0 || sx >= PANEL_W) continue;
      uint32_t px = sysmenu_surface[src_off + sx];
      if ((px >> 24) > 0)
        buffer[dst_off + x] = px;
    }
  }
}

// ====== MOUSE HANDLING ======
int sysmenu_handle_mouse(int mx, int my, int buttons) {
  static int sm_last_btn = 0;
  int down_edge = (buttons & 1) && !(sm_last_btn & 1);
  sm_last_btn = buttons;

  if (!sysmenu_is_active() || sysmenu_closing)
    return 0;

  int cy = (int)anim_y.current_val;
  int in_panel = (mx >= panel_x && mx < panel_x + PANEL_W &&
                  my >= cy && my < cy + PANEL_H);

  // Handle slider dragging (even if cursor leaves panel)
  if (dragging_volume || dragging_brightness) {
    if (buttons & 1) {
      int sl_x_abs = panel_x + PAD + 40;
      int sl_w = PANEL_W - PAD * 2 - 40;
      int val = ((mx - sl_x_abs) * 100) / sl_w;
      if (val < 0) val = 0;
      if (val > 100) val = 100;
      if (dragging_volume)  volume_val = val;
      if (dragging_brightness) brightness_val = val;
      surface_dirty = 1;
      sysmenu_render_to_cache();
      compositor_invalidate_rect(panel_x, cy, PANEL_W, PANEL_H);
      extern int ui_dirty;
      ui_dirty = 1;
      return 1;
    } else {
      dragging_volume = 0;
      dragging_brightness = 0;
      return 1;
    }
  }

  if (!in_panel) {
    if (down_edge) {
      sysmenu_toggle(); // Close on click outside
      return 1;
    }
    return 0;
  }

  if (!down_edge)
    return 1; // Absorb mouse inside panel without click

  // Local coordinates
  int lx = mx - panel_x;
  int ly = my - cy;

  // === Check sliders ===
  int vol_y = PAD + BTN_SIZE + 18;
  int brt_y = vol_y + SLIDER_H + 8;
  int sl_x = PAD + 40;
  int sl_w = PANEL_W - PAD * 2 - 40;

  // Volume slider
  if (ly >= vol_y && ly < vol_y + SLIDER_H && lx >= sl_x && lx < sl_x + sl_w) {
    dragging_volume = 1;
    volume_val = ((lx - sl_x) * 100) / sl_w;
    if (volume_val < 0) volume_val = 0;
    if (volume_val > 100) volume_val = 100;
    surface_dirty = 1;
    sysmenu_render_to_cache();
    compositor_invalidate_rect(panel_x, cy, PANEL_W, PANEL_H);
    extern int ui_dirty;
    ui_dirty = 1;
    return 1;
  }

  // Brightness slider
  if (ly >= brt_y && ly < brt_y + SLIDER_H && lx >= sl_x && lx < sl_x + sl_w) {
    dragging_brightness = 1;
    brightness_val = ((lx - sl_x) * 100) / sl_w;
    if (brightness_val < 0) brightness_val = 0;
    if (brightness_val > 100) brightness_val = 100;
    surface_dirty = 1;
    sysmenu_render_to_cache();
    compositor_invalidate_rect(panel_x, cy, PANEL_W, PANEL_H);
    extern int ui_dirty;
    ui_dirty = 1;
    return 1;
  }

  // === Check toggle grid ===
  int toggle_y_start = brt_y + SLIDER_H + 14;

  // Row 1: Wi-Fi | Bluetooth
  if (ly >= toggle_y_start && ly < toggle_y_start + TOGGLE_H) {
    if (lx >= PAD && lx < PAD + TOGGLE_W) {
      wifi_on = !wifi_on;
    } else if (lx >= PAD * 2 + TOGGLE_W && lx < PAD * 2 + TOGGLE_W * 2) {
      bt_on = !bt_on;
    }
    surface_dirty = 1;
    sysmenu_render_to_cache();
    compositor_invalidate_rect(panel_x, cy, PANEL_W, PANEL_H);
    extern int ui_dirty;
    ui_dirty = 1;
    return 1;
  }

  toggle_y_start += TOGGLE_H + TOGGLE_GAP;

  // Row 2: Power Mode | Night Light
  if (ly >= toggle_y_start && ly < toggle_y_start + TOGGLE_H) {
    if (lx >= PAD && lx < PAD + TOGGLE_W) {
      power_balanced = !power_balanced;
    } else if (lx >= PAD * 2 + TOGGLE_W && lx < PAD * 2 + TOGGLE_W * 2) {
      nightlight_on = !nightlight_on;
    }
    surface_dirty = 1;
    sysmenu_render_to_cache();
    compositor_invalidate_rect(panel_x, cy, PANEL_W, PANEL_H);
    extern int ui_dirty;
    ui_dirty = 1;
    return 1;
  }

  toggle_y_start += TOGGLE_H + TOGGLE_GAP;

  // Row 3: Dark Style | Keyboard
  if (ly >= toggle_y_start && ly < toggle_y_start + TOGGLE_H) {
    if (lx >= PAD && lx < PAD + TOGGLE_W) {
      darkstyle_on = !darkstyle_on;
    } else if (lx >= PAD * 2 + TOGGLE_W && lx < PAD * 2 + TOGGLE_W * 2) {
      keyboard_on = !keyboard_on;
    }
    surface_dirty = 1;
    sysmenu_render_to_cache();
    compositor_invalidate_rect(panel_x, cy, PANEL_W, PANEL_H);
    extern int ui_dirty;
    ui_dirty = 1;
    return 1;
  }

  toggle_y_start += TOGGLE_H + TOGGLE_GAP;

  // Row 4: Airplane Mode
  if (ly >= toggle_y_start && ly < toggle_y_start + TOGGLE_H) {
    if (lx >= PAD && lx < PAD + TOGGLE_W) {
      airplane_on = !airplane_on;
    }
    surface_dirty = 1;
    sysmenu_render_to_cache();
    compositor_invalidate_rect(panel_x, cy, PANEL_W, PANEL_H);
    extern int ui_dirty;
    ui_dirty = 1;
    return 1;
  }

  // === Check top action buttons ===
  int top_btn_x = PANEL_W - PAD - (BTN_SIZE * 4 + BTN_GAP * 3);
  if (ly >= PAD && ly < PAD + BTN_SIZE) {
    // Settings button (2nd from left)
    if (lx >= top_btn_x + BTN_SIZE + BTN_GAP &&
        lx < top_btn_x + BTN_SIZE * 2 + BTN_GAP) {
      extern void settings_init(void);
      sysmenu_toggle();
      settings_init();
      return 1;
    }
    // Lock button (3rd from left)
    if (lx >= top_btn_x + (BTN_SIZE + BTN_GAP) * 2 &&
        lx < top_btn_x + BTN_SIZE * 3 + BTN_GAP * 2) {
      extern void lockscreen_show(void);
      sysmenu_toggle();
      lockscreen_show();
      return 1;
    }
  }

  return 1; // Absorb all clicks within panel
}
