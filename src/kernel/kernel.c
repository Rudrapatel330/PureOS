#include "../drivers/bga.h"
#include "../drivers/es1370.h"
#include "../drivers/keyboard.h"
#include "../drivers/mouse.h"
#include "../drivers/pci.h"
#include "../drivers/pcnet.h"
#include "../drivers/speaker.h"
#include "../drivers/timer.h"
#include "../drivers/vmsvga.h"
#include "../fs/fs.h"
#include "../net/net.h"
#include "clipboard.h"
#include "compositor.h"
#include "config.h"
#include "desktop.h"
#include "hal/hal.h"
#include "hal/gfx_device.h"
#include "heap.h"
#include "profiler.h"
#include "screen.h"
#include "string.h"
#include "task.h"
#include "theme.h"
#include "window.h"
#include <stdint.h>

extern void videoplayer_update();
extern void camera_init();
extern void camera_update();
extern void camera_app_update();
extern void usb_init();
extern void usb_poll();

// Forward declarations
void exit(int status);
void process_input_queue(void);

// Globals
uint32_t *framebuffer = (uint32_t *)0xE0000000;
window_t *sysmon_win = 0;
uint32_t *real_lfb = (uint32_t *)0xE0000000;
uint32_t *backbuffer = 0;

int screen_width = SCREEN_WIDTH;
int screen_height = SCREEN_HEIGHT;
int screen_bpp = SCREEN_BPP;
int ui_dirty = 1;

// Optimization globals
int last_mouse_x = -1;
int last_mouse_y = -1;
int redraw_pending = 0;

// FPS Counter
int frame_count = 0;
int last_second = 0;

// Mouse globals
int mouse_sensitivity = 1; // Reduced for VirtualBox stability
int mouse_x = 800;
int mouse_y = 450;
int current_cursor_type = 0; // 0 = default, 1 = resize

// Scroll state
#define SCROLL_WHEEL_DELTA 120
#define SCROLL_TIMEOUT_MS 200

typedef struct {
  int accumulated_delta;
  uint32_t last_event_time;
  int scroll_direction;
} scroll_state_t;

static scroll_state_t g_scroll_state = {0, 0, 0};

void flush_scroll_accumulator() {
  g_scroll_state.accumulated_delta = 0;
  g_scroll_state.scroll_direction = 0;
}

void process_scroll_event(int raw_delta) {
  g_scroll_state.accumulated_delta += raw_delta;

  int direction = (raw_delta > 0) ? 1 : -1;
  if (direction != g_scroll_state.scroll_direction) {
    flush_scroll_accumulator();
    g_scroll_state.accumulated_delta = raw_delta;
    g_scroll_state.scroll_direction = direction;
  }

  while (abs(g_scroll_state.accumulated_delta) >= SCROLL_WHEEL_DELTA) {
    int step = (g_scroll_state.accumulated_delta > 0) ? SCROLL_WHEEL_DELTA
                                                      : -SCROLL_WHEEL_DELTA;

    winmgr_handle_scroll(mouse_x, mouse_y, step);

    g_scroll_state.accumulated_delta -= step;
  }
}
// Software Cursor Backbuffer (Backups from BACKBUFFER, not VRAM)
uint32_t mouse_bg[144]; // 12x12
int mouse_visible = 1;
int mouse_moved = 0;
int mouse_prev_x = 800;
int mouse_prev_y = 450;

// 12x12 arrow cursor bitmap (1=white, 2=black outline, 0=transparent)
static const uint8_t cursor_bitmap[12][12] = {
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0}, {2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0}, {2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 2, 2, 2, 0, 0, 0, 0}, {2, 1, 1, 2, 1, 2, 0, 0, 0, 0, 0, 0},
    {2, 1, 2, 0, 2, 1, 2, 0, 0, 0, 0, 0}, {2, 2, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0},
};

// 20x20 diagonal resize cursor (NW-SE arrow)
static const uint8_t cursor_resize_bitmap[20][20] = {
    {2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 2, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 2, 2, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 2, 1, 2, 2},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 1, 2},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 2},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 2},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 1, 1, 2},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2},
};

// Smart Drag Globals
int drag_dirty_x = 0;
int drag_dirty_y = 0;
int drag_dirty_w = 0;
int drag_dirty_h = 0;

// External Font Data (8x8)
extern const uint8_t font8x8_basic[256][8];
extern uint32_t tick;

// Serial helpers
void serial_init(void);
void print_serial(const char *str);

// itoa moved to string.c

// 32bpp text drawing
void draw_char_32bpp(int x, int y, char c, uint32_t color);
void draw_text_32bpp(int x, int y, const char *text, uint32_t color);
void kernel_draw_mouse(int erase_only);

// Function prototypes
void window_handle_key(int key, char c);
void receive_mouse_byte(uint8_t byte);
void receive_keyboard_byte(uint8_t scancode);
void kernel_draw_mouse_to_buffer(uint32_t *target, int mx, int my);

// Fast screen fill
void fast_fill(uint32_t *buffer, uint32_t color, int count) {
  // Fill in chunks for speed
  for (int i = 0; i < count; i += 4) {
    buffer[i] = color;
    if (i + 1 < count)
      buffer[i + 1] = color;
    if (i + 2 < count)
      buffer[i + 2] = color;
    if (i + 3 < count)
      buffer[i + 3] = color;
  }
}

// Reboot
void reboot_system(void) {
  uint8_t good = 0x02;
  while (good & 0x02)
    good = inb(0x64);
  outb(0x64, 0xFE);
  __asm__ volatile("hlt");
}

#include "config.h" 
// This includes os_config_t and extern global_config

extern void desktop_invalidate(void);
extern void compositor_init(void);
extern void desktop_draw(void);
extern uint32_t bga_width, bga_height;

void screen_set_resolution(int width, int height) {
  if (width < 640 || height < 480)
    return;
  if (width > 1920 || height > 1080)
    return;

  print_serial("KERNEL: Changing resolution to ");
  char buf[16];
  k_itoa(width, buf);
  print_serial(buf);
  print_serial("x");
  k_itoa(height, buf);
  print_serial(buf);
  print_serial("\n");

  // 1. Update hardware
  bga_set_video_mode(width, height, 32, 1, 1);

  // 2. Update globals
  screen_width = width;
  screen_height = height;
  bga_width = width;
  bga_height = height;

  if (current_gfx_device) {
    current_gfx_device->width = width;
    current_gfx_device->height = height;
    current_gfx_device->pitch = width;
  }

  // 3. Reallocate backbuffer
  if (backbuffer)
    kfree(backbuffer);
  backbuffer = (uint32_t *)kmalloc(width * height * 4);
  if (backbuffer) {
    memset(backbuffer, 0, width * height * 4);
  }

  // 4. Update config & theme
  global_config.screen_width = width;
  global_config.screen_height = height;
  config_save();

  // 5. Notify systems
  compositor_init();
  desktop_invalidate();
  desktop_draw(); // Re-render wallpaper to new buffer size

  // 6. Ensure windows are within bounds
  extern window_t windows[];
  extern int window_count;
  for (int i = 0; i < window_count; i++) {
    if (windows[i].id != 0) {
      if (windows[i].x + windows[i].width > width) {
          windows[i].x = width - windows[i].width;
          if (windows[i].x < 0) windows[i].x = 0;
      }
      if (windows[i].y + windows[i].height > height) {
          windows[i].y = height - windows[i].height;
          if (windows[i].y < 24) windows[i].y = 100; // Below menubar
      }
      windows[i].needs_redraw = 1;
    }
  }

  // 7. Reposition mouse if needed
  if (mouse_x > width - 12)
    mouse_x = width - 12;
  if (mouse_y > height - 12)
    mouse_y = height - 12;

  ui_dirty = 1;
  print_serial("KERNEL: Resolution changed successfully.\n");
}

// Mouse state (Desktop Context)
int mouse_buttons = 0;

// Internal helpers for desktop_process_messages
extern int winmgr_handle_mouse_global(int mx, int my, int buttons);
extern void desktop_click(int mx, int my, int buttons);
extern void desktop_mouse_move(int, int);
extern void desktop_mouse_up(int, int);
extern void winmgr_handle_scroll(int mx, int my, int scroll);

void desktop_process_messages(void) {
  msg_t m;
  while (msg_receive(&m)) {
    if (m.type == MSG_KEYBOARD) {
      receive_keyboard_byte(m.d1);
    } else if (m.type == MSG_MOUSE_MOVE) {
      // Delta logic
      int dx = m.d1 * mouse_sensitivity;
      int dy = m.d2 * mouse_sensitivity;
      int curr_btns = m.d3;
      int curr_scroll = m.d4;

      // Clamp deltas
      if (dx > 127)
        dx = 127;
      if (dx < -127)
        dx = -127;
      if (dy > 127)
        dy = 127;
      if (dy < -127)
        dy = -127;

      mouse_prev_x = mouse_x;
      mouse_prev_y = mouse_y;
      mouse_x += dx;
      mouse_y -= dy; // Inverted Y

      // Clamp screen space
      if (mouse_x < 0)
        mouse_x = 0;
      if (mouse_x > screen_width - 12)
        mouse_x = screen_width - 12;
      if (mouse_y < 0)
        mouse_y = 0;
      if (mouse_y > screen_height - 12)
        mouse_y = screen_height - 12;

      if (dx != 0 || dy != 0)
        mouse_moved = 1;

      if (curr_scroll != 0) {
        g_scroll_state.last_event_time = get_timer_ticks();
        process_scroll_event(curr_scroll * SCROLL_WHEEL_DELTA);
        ui_dirty = 1;
      }

      if (mouse_moved) {
        desktop_mouse_move(mouse_x, mouse_y);
      }

      static int local_prev_buttons = 0;
      int wm_consumed = 0;
      if (curr_btns != local_prev_buttons || dx != 0 || dy != 0) {
        wm_consumed = winmgr_handle_mouse_global(mouse_x, mouse_y, curr_btns);
        if (wm_consumed)
          ui_dirty = 1;
      }

      if (curr_btns && !local_prev_buttons) {
        if (!wm_consumed) {
          desktop_click(mouse_x, mouse_y, curr_btns);
          ui_dirty = 1;
        }
      }
      if (!curr_btns && local_prev_buttons) {
        desktop_mouse_up(mouse_x, mouse_y);
        ui_dirty = 1;
      }
      local_prev_buttons = curr_btns;
      mouse_buttons = curr_btns;
    } else if (m.type == MSG_MOUSE_CLICK) {
      // Direct click event from message queue
      winmgr_handle_mouse_global(m.d1 + mouse_x, m.d2 + mouse_y, m.d3);
    } else if (m.type == MSG_SHOW_SEARCH) {
      extern void desktop_activate_search(void);
      desktop_activate_search();
    } else if (m.type == MSG_SCREENSHOT) {
      extern void capture_screenshot(void);
      capture_screenshot();
    }
  }
}

void desktop_task() {
  unsigned int last_render_tick = get_timer_ticks();
  uint64_t last_anim_ms = get_timer_ms_hires();
  unsigned int last_second = get_timer_ticks();
  unsigned int frame_count = 0;

  print_serial("DESKTOP THREAD STARTED\n");
  __asm__ volatile("sti");

  extern void lockscreen_show(void);
  extern void compositor_render(void);
  extern void compositor_reset_history(void);

  // Initialize lockscreen BEFORE the first render
  print_serial("Initializing lockscreen...\n");
  lockscreen_show();

  // Initialize compositor state and render
  compositor_reset_history();
  compositor_render();
  print_serial("First frame rendered.\n");

  ui_dirty = 1;

  // Startup Sound
  play_sound(440);
  sleep(5);
  play_sound(554);
  sleep(5);
  play_sound(659);
  sleep(10);
  nosound();

  while (1) {
    desktop_process_messages();
    usb_poll();

    // Background Network Polling — drain ALL pending packets per frame
    // Audio packets from the web span multiple TCP segments; polling only
    // one per frame starves phone_update and causes audio packets to never
    // fully arrive in the TCP receive buffer.
    {
        static uint8_t net_poll_buf[1600];
        uint16_t net_poll_len;
        int polls = 0;
        while (polls < 128 && pcnet_poll(net_poll_buf, &net_poll_len) > 0) {
            net_receive(net_poll_buf, net_poll_len);
            polls++;
        }
    }

    static uint64_t last_anim_tick_ms = 0;
    unsigned int now_tick = get_timer_ticks();
    uint64_t now_ms = get_timer_ms_hires();

    if (last_anim_tick_ms == 0)
      last_anim_tick_ms = now_ms;

    float dt = 0.0f;
    if (now_ms != last_anim_ms) {
      dt = (float)(now_ms - last_anim_ms) * 0.001f;
      if (dt > 0.1f)
        dt = 0.1f;
      last_anim_ms = now_ms;
    }

    int tick_elapsed = (now_tick - last_render_tick >= 4);

    // Convert 250 ticks/sec to ms (approx 4ms per tick)
    if ((now_tick - g_scroll_state.last_event_time) * 4 > SCROLL_TIMEOUT_MS) {
      flush_scroll_accumulator();
    }

    int any_anim = 0;
    {
      extern window_t windows[];
      for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id != 0 && (windows[i].is_animating || windows[i].fading_mode != 0 ||
            windows[i].vel_x != 0 || windows[i].vel_y != 0 ||
            windows[i].anim_scale.active)) {
          any_anim = 1;
        }
      }
      extern int startmenu_is_animating();
      if (startmenu_is_animating())
        any_anim = 1;

      extern int taskbar_is_animating(void);
      if (taskbar_is_animating())
        any_anim = 1;
    }

    if (mouse_moved) {
      extern int magnifier_enabled;
      int inv_size = magnifier_enabled ? 160 : 32;
      int half_size = inv_size / 2;

      if (last_mouse_x != -1) {
        compositor_invalidate_rect(last_mouse_x - half_size,
                                   last_mouse_y - half_size, inv_size,
                                   inv_size);
      }
      compositor_invalidate_rect(mouse_x - half_size, mouse_y - half_size,
                                  inv_size, inv_size);

      // Ensure sysmenu area is always redrawn when mouse moves while active
      extern void sysmenu_invalidate_rect(void);
      sysmenu_invalidate_rect();
    }



    if (tick_elapsed && (redraw_pending || ui_dirty > 0 || mouse_moved ||
                         any_anim || compositor_is_dirty())) {
      last_render_tick = now_tick;
      frame_count++;
      if (now_tick - last_second > 250) {
        frame_count = 0;
        last_second = now_tick;
      }

      if (any_anim) {
        float anim_dt = (float)(now_ms - last_anim_tick_ms) * 0.001f;
        // Tighter clamp for physical stability: max 33ms (approx 30fps dip)
        if (anim_dt > 0.033f)
          anim_dt = 0.033f;

        if (anim_dt > 0.0f) {
          extern void winmgr_tick_animations(float dt);
          winmgr_tick_animations(anim_dt);
          extern void taskbar_tick_animations(float dt);
          taskbar_tick_animations(anim_dt);
          last_anim_tick_ms = now_ms;
        }
      } else {
        // CRITICAL: Keep timer fresh even when not animating
        last_anim_tick_ms = now_ms;
      }

      winmgr_flush_updates();
      extern void winmgr_draw_drag_overlay(void);
      winmgr_draw_drag_overlay();
      compositor_render();

      // Periodic heap audit to catch corruption early
      if ((frame_count & 63) == 0) {
        extern void heap_audit(void);
        heap_audit();
      }

      last_mouse_x = mouse_x;
      last_mouse_y = mouse_y;
      mouse_moved = 0;
      redraw_pending = 0;
      ui_dirty = 0;
    }

    if (tick_elapsed) {
      extern window_t *sysmon_win, *chat_win, *phone_win, *recorder_win;
      // We will add audio test win here
      extern window_t *audio_test_win, *pong_win, *song_player_win;
      extern void sysmon_update(window_t *), chat_update(window_t *), phone_update(window_t *), recorder_update(window_t *);
      extern void audio_test_update(window_t *), pong_update(window_t *), song_player_update(window_t *);
      if (sysmon_win) sysmon_update(sysmon_win);
      if (chat_win) chat_update(chat_win);
      if (phone_win) phone_update(phone_win);
      if (recorder_win) recorder_update(recorder_win);
      if (audio_test_win) audio_test_update(audio_test_win);
      if (pong_win) pong_update(pong_win);
      if (song_player_win) song_player_update(song_player_win);
    }

    videoplayer_update();
    camera_update();
    camera_app_update();
    /* Yield to scheduler — do NOT use hlt here!
       This is the master render loop. hlt halts the entire CPU
       and blocks all rendering/input processing until next IRQ. */
    __asm__ volatile("int $32");
  }
}

// Stubs and drivers alignment
void kernel_draw_mouse_to_buffer(uint32_t *target, int mx, int my) {
  if (!target)
    return;

  if (current_cursor_type == 0) {
    for (int cy = 0; cy < 12; cy++) {
      for (int cx = 0; cx < 12; cx++) {
        uint8_t p = cursor_bitmap[cy][cx];
        if (p == 0)
          continue;
        int px = mx + cx;
        int py = my + cy;
        if (px >= 0 && px < screen_width && py >= 0 && py < screen_height) {
          target[py * screen_width + px] = (p == 1) ? 0xFFFFFFFF : 0xFF000000;
        }
      }
    }
  } else if (current_cursor_type == 1) {
    for (int cy = 0; cy < 20; cy++) {
      for (int cx = 0; cx < 20; cx++) {
        uint8_t p = cursor_resize_bitmap[cy][cx];
        if (p == 0)
          continue;
        int px = mx + cx - 10;
        int py = my + cy - 10;
        if (px >= 0 && px < screen_width && py >= 0 && py < screen_height) {
          target[py * screen_width + px] = (p == 1) ? 0xFFFFFFFF : 0xFF000000;
        }
      }
    }
  }
}

void kernel_draw_mouse(int erase_only) {
  if (erase_only)
    return;
  extern int lockscreen_active;
  if (!lockscreen_active)
    return;
  uint32_t *target = gfx_device_get_render_buffer();
  if (!target)
    target = real_lfb;
  kernel_draw_mouse_to_buffer(target, mouse_x, mouse_y);
}

void kernel_poll_events(void) {
  static int in_poll = 0;
  if (in_poll)
    return;
  in_poll = 1;

  desktop_process_messages();

  // If we are in a blocking loop (e.g. browser navigation), we still want the
  // screen to update.
  extern int ui_dirty, mouse_moved;
  extern int compositor_needs_render(void);
  if (ui_dirty || mouse_moved || compositor_needs_render()) {
    if (mouse_moved) {
      extern int last_mouse_x, last_mouse_y, mouse_x, mouse_y;
      extern void compositor_invalidate_rect(int x, int y, int w, int h);
      int inv_size = 32;
      int half_size = inv_size / 2;
      if (last_mouse_x != -1) {
        compositor_invalidate_rect(last_mouse_x - half_size,
                                   last_mouse_y - half_size, inv_size, inv_size);
      }
      compositor_invalidate_rect(mouse_x - half_size, mouse_y - half_size,
                                  inv_size, inv_size);

      // Ensure sysmenu area is always redrawn when mouse moves while active
      extern void sysmenu_invalidate_rect(void);
      sysmenu_invalidate_rect();

      last_mouse_x = mouse_x;
      last_mouse_y = mouse_y;
      mouse_moved = 0;
    }

    extern void winmgr_flush_updates(void);
    winmgr_flush_updates();
    extern void compositor_render(void);
    compositor_render();
    ui_dirty = 0;
  }

  in_poll = 0;
}

void kernel_poll_events_only(void) { desktop_process_messages(); }

extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);
extern void (*__ctors_start[])(void);
extern void (*__ctors_end[])(void);

void call_cpp_constructors(void) {
  print_serial("Calling C++ Global Constructors...\n");
  for (void (**p)() = __init_array_start; p < __init_array_end; p++) {
    if (*p) {
      (*p)();
    }
  }
  for (void (**p)() = __ctors_start; p < __ctors_end; p++) {
    if (*p && *p != (void (*)())-1) {
      (*p)();
    }
  }
  print_serial("C++ Global Constructors initialized.\n");
}

void kernel_main(unsigned int magic, unsigned int addr) {
  (void)magic;
  (void)addr;
  serial_init();
  print_serial("\n[K1] KERNEL MAIN REACHED\n");

  // 0. VISUAL DEBUG & INIT (Blue Screen Test)
  uint32_t *fb_addr_ptr = (uint32_t *)0x6000;
  uint32_t fb_val = *fb_addr_ptr;

  if (fb_val != 0 && fb_val != 0xFFFFFFFF) {
    real_lfb = (uint32_t *)(uintptr_t)fb_val;
  } else {
    real_lfb = (uint32_t *)0xE0000000;
  }
  framebuffer = real_lfb;

  // 1. Initialize FPU & SSE (Standard SSE2/3/4 support in 64-bit mode)
  __asm__ volatile(
      "fninit\n"
      "mov %%cr0, %%rax\n"
      "and $0xFFFFFFFFFFFFFFF7, %%rax\n" // Clear TS (bit 3)
      "and $0xFFFFFFFFFFFFFFFB, %%rax\n" // Clear EM (bit 2)
      "or $0x22, %%rax\n"                // Set MP (bit 1), NE (bit 5)
      "mov %%rax, %%cr0\n"
      "mov %%cr4, %%rax\n"
      "or $0x600, %%rax\n" // Set OSFXSR (bit 9), OSXMMEXCPT (bit 10)
      "mov %%rax, %%cr4\n"
      :
      :
      : "rax");

  // Mask floating-point exceptions to prevent Guru Meditation on instability
  uint16_t fpu_cw = 0x37F;
  __asm__ volatile("fldcw %0" : : "m"(fpu_cw));
  uint32_t mxcsr = 0x1F80;
  __asm__ volatile("ldmxcsr %0" : : "m"(mxcsr));

  print_serial("\n\n=== PUREOS BOOTING (PHASE 7 - HIGHER HALF) ===\n");

  char addr_str[12];
  print_serial("LFB Phys: 0x");
  k_itoa_hex((uint32_t)(uintptr_t)real_lfb, addr_str);
  print_serial(addr_str);
  print_serial("\n");

  // 1. Initialize HAL (GDT, IDT, ISR)
  hal_init_early();

  // Initialize Profiler
  profiler_init();

  // 2. Memory (Heap)
  heap_init();
  print_serial("HEAP OK\n");
  
  // 2.5 C++ Constructors (MUST be after Heap)
  call_cpp_constructors();

  // 3. Full Hardware Init (Paging, ACPI, APIC, PIC, SMP)
  hal_init();

  // 7. Multitasking & Timer - START BEFORE DRIVERS to support sleep()
  init_multitasking();
  init_timer(250);
  __asm__ volatile("sti"); // Enable interrupts early for timer but keep IRQs
                           // masked at PIC if needed
  print_serial("TASKING & TIMER (EARLY) OK\n");

  // 4. Filesystem
  print_serial("[INIT 1] FS START\n");
  fs_init();
  print_serial("[INIT 2] FS OK\n");

  // Load persistent configuration
  extern void config_init(void);
  extern void config_load(void);
  config_init(); // Initialize defaults first
  config_load();
  theme_init();
  print_serial("[INIT 3] CONFIG & THEME OK\n");

  // 5. PCI & Hardware Drivers
  print_serial("[INIT 4] PCI START\n");
  usb_init();
  pci_init();
  camera_init();
  print_serial("[INIT 5] BGA START\n");
  bga_init();
  
  print_serial("[INIT 5.5] VMSVGA SCAN\n");
  // vmsvga_init(); // Disabled to use BGA true page-flipping

  if (bga_lfb) {
    real_lfb = bga_lfb;
    framebuffer = bga_lfb;
    print_serial("KERNEL: Updated real_lfb to bga_lfb\n");
  }

  // es1370_play_test_beep(); // Skip during diagnostic

  print_serial("[INIT 6] KB/MOUSE START\n");
  init_keyboard();

  init_ps2_mouse();

  extern int mouse_sensitivity;
  mouse_sensitivity = 1;

  print_serial("[INIT 7] NE2000 START\n");
  pcnet_init();
  print_serial("NE2000 OK\n");

  net_init();
  print_serial("NET STACK OK\n");

  // 6. GUI Systems - ALLOCATE BACKBUFFER FIRST!
  print_serial("[INIT 8] GUI START\n");
  backbuffer = (uint32_t *)kmalloc(screen_width * screen_height * 4);
  if (backbuffer) {
    memset(backbuffer, 0, screen_width * screen_height * 4);
  } else {
    print_serial("CRITICAL: backbuffer allocation FAILED!\n");
  }

  print_serial("Initializing WinMgr...\n");
  winmgr_init();

  print_serial("Initializing Fonts...\n");
  extern void ttf_init(void);
  extern int ttf_load_browser_font(void);
  ttf_init();
  ttf_load_browser_font();

  print_serial("Initializing Clipboard...\n");
  clipboard_init();

  print_serial("Initializing Desktop...\n");
  desktop_init();

  compositor_init();
  print_serial("[INIT 9] GUI OK\n");

  // Force lockscreen state active immediately to block desktop rendering while it loads
  extern int lockscreen_active;
  lockscreen_active = 1;

  // Clear backbuffer once to ensure no garbage/desktop is shown initially
  if (backbuffer) memset(backbuffer, 0, screen_width * screen_height * 4);

  extern void desktop_task();
  create_task(desktop_task, "desktop");

  extern window_t *sysmon_create(void);
  if (global_config.show_sysmon_widget) {
    sysmon_win = sysmon_create();
  }

  extern void gfx_test_run(void);
  gfx_test_run();

  print_serial("KERNEL INIT COMPLETE\n");
  heap_stats();

  // Calibrate TSC
  calibrate_tsc();

  // IDLE LOOP (Kernel Task)
  while (1) {
    __asm__ volatile("hlt");
  }
}

// Implement specific 32bpp char drawing with anti-aliased FONT
// Uses grayscale AA cache with proper alpha blending for smooth text
void draw_char_32bpp(int x, int y, char c, uint32_t color) {
  extern uint8_t font_get_aa_pixel(unsigned char c, int x, int y);

  uint32_t s_r = (color >> 16) & 0xFF;
  uint32_t s_g = (color >> 8) & 0xFF;
  uint32_t s_b = color & 0xFF;

  for (int cy = -1; cy < 9; cy++) {
    for (int cx = -1; cx < 9; cx++) {
      int px = x + cx;
      int py = y + cy;
      if (px < 0 || px >= screen_width || py < 0 || py >= screen_height)
        continue;

      uint8_t alpha = font_get_aa_pixel((unsigned char)c, cx, cy);
      if (alpha <= 4) continue;

      if (alpha >= 250) {
        // Fully opaque — skip blending
        backbuffer[py * screen_width + px] = 0xFF000000 | (s_r << 16) | (s_g << 8) | s_b;
      } else {
        // Alpha-blend with background
        uint32_t dst = backbuffer[py * screen_width + px];
        uint32_t d_r = (dst >> 16) & 0xFF;
        uint32_t d_g = (dst >> 8) & 0xFF;
        uint32_t d_b = dst & 0xFF;
        uint32_t r = d_r + (((int)s_r - (int)d_r) * alpha >> 8);
        uint32_t g = d_g + (((int)s_g - (int)d_g) * alpha >> 8);
        uint32_t b = d_b + (((int)s_b - (int)d_b) * alpha >> 8);
        backbuffer[py * screen_width + px] = 0xFF000000 | (r << 16) | (g << 8) | b;
      }
    }
  }
}


void draw_text_32bpp(int x, int y, const char *text, uint32_t color) {
  while (*text) {
    draw_char_32bpp(x, y, *text, color);
    x += 8;
    text++;
  }
}

void winmgr_toggle_sysmon(int show) {
  extern window_t *sysmon_win;
  extern window_t *sysmon_create(void);
  extern void winmgr_close_window(window_t *);
  
  if (show) {
    if (!sysmon_win) {
      sysmon_win = sysmon_create();
    }
  } else {
    if (sysmon_win) {
      sysmon_win->flags &= ~0x80; // Clear WINDOW_FLAG_NO_CLOSE
      winmgr_close_window(sysmon_win);
      sysmon_win = 0;
    }
  }
}

// End of kernel.c
