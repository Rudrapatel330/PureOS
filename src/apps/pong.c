#include "../kernel/ui_layout.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"

extern void *kmalloc_ap(size_t size, uint32_t *phys);
extern void ac97_play_pcm(const void *data, uint32_t size, uint32_t sample_rate, uint8_t bits, uint8_t channels);
extern uint8_t key_state[256]; // From keyboard.c - tracks held keys

static uint32_t beep_wall_phys = 0;
static uint32_t beep_paddle_phys = 0;
static uint32_t beep_score_phys = 0;
static int beep_wall_size = 0;
static int beep_paddle_size = 0;
static int beep_score_size = 0;

static void generate_beep(int freq, int duration_ms, uint32_t *out_phys, int *out_size) {
    int sample_rate = 48000;
    int samples = (duration_ms * sample_rate) / 1000;
    int size = samples * 4; // 16-bit stereo = 4 bytes per sample
    
    uint32_t phys;
    int16_t *virt = (int16_t *)kmalloc_ap(size, &phys);
    if (!virt) return;
    
    int period = sample_rate / freq;
    int half_period = period / 2;
    
    for (int i = 0; i < samples; i++) {
        int16_t val = (i % period < half_period) ? 8000 : -8000;
        virt[i * 2] = val;     // Left
        virt[i * 2 + 1] = val; // Right
    }
    
    *out_phys = phys;
    *out_size = size;
}

typedef struct {
  window_t *win;
  int mode; // 0 = Menu, 1 = 1P (vs Bot), 2 = 2P, 3 = Bot vs Bot
  
  float ball_x, ball_y;
  float ball_vx, ball_vy;
  
  float p1_y, p2_y;
  int p1_score, p2_score;
  
  int paddle_w, paddle_h;
  int ball_size;
  
  unsigned int sound_end_tick;
  int sound_playing;
} pong_app_t;

window_t *pong_win = 0;

static inline pong_app_t *get_pong(void *w) {
  return (pong_app_t *)((window_t *)w)->user_data;
}

static void reset_ball(pong_app_t *app) {
  app->ball_x = app->win->width / 2 - app->ball_size / 2;
  app->ball_y = app->win->height / 2 - app->ball_size / 2;
  app->ball_vx = (app->p1_score + app->p2_score) % 2 == 0 ? 4.0f : -4.0f;
  app->ball_vy = 2.0f;
}

static void trigger_sound(pong_app_t *app, uint32_t freq_type) {
    (void)app;
    if (freq_type == 226 && beep_wall_phys) {
        ac97_play_pcm((const void *)(uintptr_t)beep_wall_phys, beep_wall_size, 48000, 16, 2);
    } else if (freq_type == 459 && beep_paddle_phys) {
        ac97_play_pcm((const void *)(uintptr_t)beep_paddle_phys, beep_paddle_size, 48000, 16, 2);
    } else if (freq_type == 490 && beep_score_phys) {
        ac97_play_pcm((const void *)(uintptr_t)beep_score_phys, beep_score_size, 48000, 16, 2);
    }
}

void pong_update(window_t *win) {
  pong_app_t *app = get_pong(win);
  if (!app) return;

  if (app->mode == 0) return;
  
  // Continuous paddle movement by polling held keys directly
  float paddle_speed = 8.0f;
  
  if (app->mode == 1) {
      // 1P: Arrow keys for Player 1
      if (key_state[0x48]) app->p1_y -= paddle_speed; // Up Arrow held
      if (key_state[0x50]) app->p1_y += paddle_speed; // Down Arrow held
  } else if (app->mode == 2) {
      // 2P: W/S for Player 1, Arrows for Player 2
      if (key_state[0x11]) app->p1_y -= paddle_speed; // W held (scancode 0x11)
      if (key_state[0x1F]) app->p1_y += paddle_speed; // S held (scancode 0x1F)
      if (key_state[0x48]) app->p2_y -= paddle_speed; // Up Arrow held
      if (key_state[0x50]) app->p2_y += paddle_speed; // Down Arrow held
  }
  // Mode 3 (Bot vs Bot): both paddles handled by AI below
  
  // Physics update
  app->ball_x += app->ball_vx;
  app->ball_y += app->ball_vy;
  
  // Bounce top/bottom
  if (app->ball_y <= 32) { // 32 is titlebar height
      app->ball_y = 32;
      app->ball_vy = -app->ball_vy;
      trigger_sound(app, 226); // Wall
  }
  if (app->ball_y >= win->height - app->ball_size) {
      app->ball_y = win->height - app->ball_size;
      app->ball_vy = -app->ball_vy;
      trigger_sound(app, 226); // Wall
  }
  
  // Check paddles
  // P1 (Left)
  if (app->ball_x <= 20 + app->paddle_w) {
      if (app->ball_y + app->ball_size >= app->p1_y && app->ball_y <= app->p1_y + app->paddle_h) {
          app->ball_x = 20 + app->paddle_w;
          app->ball_vx = -app->ball_vx;
          float diff = (app->ball_y + app->ball_size/2.0f) - (app->p1_y + app->paddle_h/2.0f);
          app->ball_vy = diff * 0.15f;
          if (app->ball_vx < 12.0f) app->ball_vx *= 1.05f; // Speed up cap
          trigger_sound(app, 459); // Paddle
      } else if (app->ball_x < 0) {
          app->p2_score++;
          trigger_sound(app, 490); // Score
          reset_ball(app);
      }
  }
  
  // P2 (Right)
  if (app->ball_x + app->ball_size >= win->width - 20 - app->paddle_w) {
      if (app->ball_y + app->ball_size >= app->p2_y && app->ball_y <= app->p2_y + app->paddle_h) {
          app->ball_x = win->width - 20 - app->paddle_w - app->ball_size;
          app->ball_vx = -app->ball_vx;
          float diff = (app->ball_y + app->ball_size/2.0f) - (app->p2_y + app->paddle_h/2.0f);
          app->ball_vy = diff * 0.15f;
          if (app->ball_vx > -12.0f) app->ball_vx *= 1.05f; // Speed up cap
          trigger_sound(app, 459);
      } else if (app->ball_x > win->width) {
          app->p1_score++;
          trigger_sound(app, 490);
          reset_ball(app);
      }
  }
  
  // Bot AI for P2 (modes 1 and 3)
  if (app->mode == 1 || app->mode == 3) {
      if (app->ball_vx > 0) {
          // Predict where it's going
          float time_to_reach = (win->width - 20 - app->paddle_w - app->ball_x) / app->ball_vx;
          float target_y = app->ball_y + app->ball_vy * time_to_reach;
          
          // Handle bounces in prediction
          int max_bounces = 5;
          while ((target_y < 32 || target_y > win->height) && max_bounces > 0) {
              if (target_y < 32) target_y = 32 + (32 - target_y);
              if (target_y > win->height) target_y = win->height - (target_y - win->height);
              max_bounces--;
          }
          
          float center_p2 = app->p2_y + app->paddle_h / 2.0f;
          if (center_p2 < target_y - 10.0f) app->p2_y += 5.0f;
          else if (center_p2 > target_y + 10.0f) app->p2_y -= 5.0f;
      } else {
          // Slowly return to center when ball is moving away
          float center_p2 = app->p2_y + app->paddle_h / 2.0f;
          float screen_center = win->height / 2.0f;
          if (center_p2 < screen_center - 10.0f) app->p2_y += 2.0f;
          else if (center_p2 > screen_center + 10.0f) app->p2_y -= 2.0f;
      }
  }
  
  // Bot AI for P1 (mode 3 only)
  if (app->mode == 3) {
      if (app->ball_vx < 0) {
          // Predict where it's going toward P1
          float time_to_reach = (app->ball_x - 20 - app->paddle_w) / (-app->ball_vx);
          float target_y = app->ball_y + app->ball_vy * time_to_reach;
          
          // Handle bounces in prediction
          int max_bounces = 5;
          while ((target_y < 32 || target_y > win->height) && max_bounces > 0) {
              if (target_y < 32) target_y = 32 + (32 - target_y);
              if (target_y > win->height) target_y = win->height - (target_y - win->height);
              max_bounces--;
          }
          
          float center_p1 = app->p1_y + app->paddle_h / 2.0f;
          // Slightly faster reaction than P2 to make games interesting
          if (center_p1 < target_y - 8.0f) app->p1_y += 5.5f;
          else if (center_p1 > target_y + 8.0f) app->p1_y -= 5.5f;
      } else {
          // Slowly return to center when ball is moving away
          float center_p1 = app->p1_y + app->paddle_h / 2.0f;
          float screen_center = win->height / 2.0f;
          if (center_p1 < screen_center - 10.0f) app->p1_y += 2.5f;
          else if (center_p1 > screen_center + 10.0f) app->p1_y -= 2.5f;
      }
  }
  
  // Clamp paddles
  if (app->p1_y < 32) app->p1_y = 32;
  if (app->p1_y > win->height - app->paddle_h) app->p1_y = win->height - app->paddle_h;
  if (app->p2_y < 32) app->p2_y = 32;
  if (app->p2_y > win->height - app->paddle_h) app->p2_y = win->height - app->paddle_h;

  win->needs_redraw = 1;
  extern int ui_dirty;
  ui_dirty = 1;
}

void pong_draw(window_t *win) {
  pong_app_t *app = get_pong(win);
  if (!win || !app) return;
  
  const theme_t *theme = theme_get();
  
  // Dark Background
  winmgr_fill_rect(win, 0, 32, win->width, win->height - 32, 0xFF121212);
  
  if (app->mode == 0) {
      // Draw Menu
      winmgr_draw_text(win, win->width/2 - 24, win->height/2 - 60, "PONG", 0xFF00A3FF);
      
      // Button 1
      winmgr_fill_rect(win, win->width/2 - 70, win->height/2 - 10, 140, 30, theme->button);
      winmgr_draw_text(win, win->width/2 - 55, win->height/2 - 2, "1 Player (Bot)", theme->button_text);
      
      // Button 2
      winmgr_fill_rect(win, win->width/2 - 70, win->height/2 + 30, 140, 30, theme->button);
      winmgr_draw_text(win, win->width/2 - 35, win->height/2 + 38, "2 Players", theme->button_text);
      
      // Button 3
      winmgr_fill_rect(win, win->width/2 - 70, win->height/2 + 70, 140, 30, theme->button);
      winmgr_draw_text(win, win->width/2 - 43, win->height/2 + 78, "Bot vs Bot", theme->button_text);
      return;
  }
  
  // Mid line
  for(int i = 32; i < win->height; i += 20) {
      winmgr_fill_rect(win, win->width/2 - 2, i, 4, 10, 0xFF333333);
  }
  
  // Paddles
  winmgr_fill_rect(win, 20, (int)app->p1_y, app->paddle_w, app->paddle_h, 0xFF00A3FF); // Blue P1
  winmgr_fill_rect(win, win->width - 20 - app->paddle_w, (int)app->p2_y, app->paddle_w, app->paddle_h, 0xFFFF6B6B); // Red P2
  
  // Ball
  winmgr_fill_rect(win, (int)app->ball_x, (int)app->ball_y, app->ball_size, app->ball_size, 0xFFFFFFFF);
  
  // Scores
  char score_str[16];
  k_itoa(app->p1_score, score_str);
  winmgr_draw_text(win, win->width/4, 50, score_str, 0xFFFFFFFF);
  k_itoa(app->p2_score, score_str);
  winmgr_draw_text(win, win->width*3/4, 50, score_str, 0xFFFFFFFF);
  
  // Mode labels
  if (app->mode == 1) {
      winmgr_draw_text(win, 10, win->height - 16, "YOU", 0xFF00A3FF);
      winmgr_draw_text(win, win->width - 30, win->height - 16, "BOT", 0xFFFF6B6B);
  } else if (app->mode == 2) {
      winmgr_draw_text(win, 10, win->height - 16, "P1 [W/S]", 0xFF00A3FF);
      winmgr_draw_text(win, win->width - 70, win->height - 16, "P2 [Up/Dn]", 0xFFFF6B6B);
  } else if (app->mode == 3) {
      winmgr_draw_text(win, 10, win->height - 16, "BOT 1", 0xFF00A3FF);
      winmgr_draw_text(win, win->width - 42, win->height - 16, "BOT 2", 0xFFFF6B6B);
  }
}

void pong_on_mouse(window_t *win, int mx, int my, int buttons) {
  pong_app_t *app = get_pong(win);
  if (!app) return;
  
  if (app->mode == 0) {
      if (buttons & 1) { // Left click
          if (mx >= win->width/2 - 70 && mx <= win->width/2 + 70) {
              if (my >= win->height/2 - 10 && my <= win->height/2 + 20) {
                  app->mode = 1;
                  app->p1_score = 0;
                  app->p2_score = 0;
                  reset_ball(app);
              }
              if (my >= win->height/2 + 30 && my <= win->height/2 + 60) {
                  app->mode = 2;
                  app->p1_score = 0;
                  app->p2_score = 0;
                  reset_ball(app);
              }
              if (my >= win->height/2 + 70 && my <= win->height/2 + 100) {
                  app->mode = 3;
                  app->p1_score = 0;
                  app->p2_score = 0;
                  reset_ball(app);
              }
          }
      }
  }
}

void pong_on_key(window_t *win, int key, char ascii) {
  (void)key; (void)ascii;
  // Movement is now handled by polling key_state[] in pong_update
  // This callback is kept for future use (pause, etc.)
}

static void pong_on_close(void *w) {
  window_t *win = (window_t *)w;
  pong_app_t *app = get_pong(win);
  if (app) {
      kfree(app);
      win->user_data = 0;
  }
  if (pong_win == win) {
      pong_win = 0;
  }
}

void pong_init() {
  window_t *win = winmgr_create_window(-1, -1, 500, 350, "Pong");
  if (!win) return;

  pong_app_t *app = (pong_app_t *)kmalloc(sizeof(pong_app_t));
  if (!app) {
      winmgr_close_window(win);
      return;
  }

  if (!beep_wall_phys) {
      generate_beep(226, 16, &beep_wall_phys, &beep_wall_size);
      generate_beep(459, 96, &beep_paddle_phys, &beep_paddle_size);
      generate_beep(490, 257, &beep_score_phys, &beep_score_size);
  }

  for (int i = 0; i < (int)sizeof(pong_app_t); i++) ((char *)app)[i] = 0;
  
  app->win = win;
  app->mode = 0;
  app->paddle_w = 10;
  app->paddle_h = 60;
  app->ball_size = 10;
  app->p1_y = win->height/2 - app->paddle_h/2;
  app->p2_y = win->height/2 - app->paddle_h/2;

  win->user_data = app;
  win->draw = (void (*)(void *))pong_draw;
  win->on_mouse = (void (*)(void *, int, int, int))pong_on_mouse;
  win->on_key = (void (*)(void *, int, char))pong_on_key;
  win->on_close = pong_on_close;
  win->bg_color = 0xFF121212;
  win->app_type = 20; // Some unique ID
  win->needs_redraw = 1;
  
  pong_win = win;
}
