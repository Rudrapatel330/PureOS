#include "../kernel/ui_layout.h"
#include "recorder.h"
#include "../drivers/ac97.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../drivers/timer.h"

#define RECORDER_BUFFER_SIZE (2 * 1024 * 1024)
#define REC_SAMPLE_RATE 44100
#define REC_CHANNELS 2
#define REC_BITS 16

static int get_sidebar_width(void) { return ui_get_font_scale() * 15; }
#define BOTTOM_BAR_HEIGHT 90

typedef enum {
    STATE_IDLE,
    STATE_RECORDING,
    STATE_PLAYING
} recorder_state_t;

typedef struct {
    uint8_t *buffer;
    uint32_t current_size;
    recorder_state_t state;
    uint32_t play_pos;
    uint32_t start_ticks;
} recorder_app_t;

extern void print_serial(const char *);

static recorder_app_t* get_state(window_t* win) {
    if (!win) return 0;
    return (recorder_app_t*)win->user_data;
}

static void draw_waveform(window_t *win, int x, int y, int w, int h, recorder_app_t *s, const theme_t *th) {
    if (!win || !s || !th) return;
    
    // Background of waveform area
    winmgr_fill_rect(win, x, y, w, h, 0xFF121212);
    
    // Middle horizontal line (dotted simulation)
    for (int i = 0; i < w; i += 4) {
        winmgr_put_pixel(win, x + i, y + h / 2, 0xFF444444);
    }
    
    // Time markers at the top
    for (int i = 0; i < w; i += 80) {
        uint32_t total_secs = i / 10; // Simplified scale
        uint32_t mins = total_secs / 60;
        uint32_t secs = total_secs % 60;
        
        char m_buf[12], s_buf[12];
        k_itoa(mins, m_buf);
        k_itoa(secs, s_buf);
        
        char time_str[16];
        time_str[0] = (mins < 10) ? '0' : m_buf[0];
        time_str[1] = (mins < 10) ? m_buf[0] : m_buf[1];
        time_str[2] = ':';
        time_str[3] = (secs < 10) ? '0' : s_buf[0];
        time_str[4] = (secs < 10) ? s_buf[0] : s_buf[1];
        time_str[5] = '\0';
        
        winmgr_draw_text(win, x + i, y + 5, time_str, 0xFF666666);
        winmgr_draw_line(win, x + i, y + 20, x + i, y + 25, 0xFF444444);
    }

    // Draw bars if we have data or are recording
    if (s->state == STATE_RECORDING || s->state == STATE_PLAYING || s->current_size > 0) {
        int num_bars = (w - 20) / 4;
        uint32_t ticks = get_timer_ticks();
        
        for (int i = 0; i < num_bars; i++) {
            int bar_x = x + i * 4 + 10;
            // Simulated waveform based on state and time
            int bar_h = 4;
            if (s->state == STATE_RECORDING) {
                bar_h = (ticks * (i % 7 + 1)) % (h / 2) + 5;
            } else if (s->current_size > 0) {
                // Static wave for existing recording
                bar_h = (i * i * 13 + 53) % (h / 3) + 10;
            }
            
            if (bar_h > h - 40) bar_h = h - 40;
            winmgr_fill_rect(win, bar_x, y + h/2 - bar_h/2, 2, bar_h, th->accent);
        }
    }
    
    // Playhead
    int ph_x = x + 10;
    if (s->state == STATE_PLAYING && s->current_size > 0) {
        ph_x = x + 10 + ((get_timer_ticks() % 500) * (w - 20) / 500);
    }
    
    winmgr_draw_line_aa(win, ph_x, y + 30, ph_x, y + h - 10, 0xFFFFFFFF);
    winmgr_fill_rect(win, ph_x - 3, y + 27, 7, 7, 0xFFFFFFFF); // Handle
}

static void recorder_draw(window_t *win) {
    if (!win) return;
    recorder_app_t *s = get_state(win);
    if (!s) return;
    
    const theme_t *th = theme_get();
    
    // 1. Sidebar
    winmgr_fill_rect(win, 0, 0, get_sidebar_width(), win->height, 0xFF1F1F1F);
    winmgr_draw_line(win, get_sidebar_width() - 1, 0, get_sidebar_width() - 1, win->height, 0xFF333333);
    
    // Sidebar Header
    winmgr_draw_text(win, 15, 15, "Recordings", 0xFFFFFFFF);
    winmgr_draw_rounded_rect_ex(win, 15, 45, 120, 32, 0xFF2D2D2D, 1, 0xFF444444, 4);
    winmgr_draw_text(win, 25, 53, "+ Import", 0xFFBBBBBB);
    
    // Sidebar Content
    if (s->current_size == 0 && s->state != STATE_RECORDING) {
        winmgr_draw_text(win, 30, win->height / 2 - 10, "Start recording", 0xFF888888);
        winmgr_draw_text(win, 30, win->height / 2 + 5, "to create files", 0xFF888888);
    } else {
        winmgr_draw_rounded_rect_ex(win, 10, 100, get_sidebar_width() - 20, 50, 0xFF2D2D2D, 1, th->accent, 6);
        winmgr_draw_text(win, 20, 110, "Recording 1", 0xFFFFFFFF);
        winmgr_draw_text(win, 20, 128, "Just now", 0xFF888888);
    }
    
    // Bottom Sidebar
    winmgr_draw_text(win, 15, win->height - 35, "Mic Array (Active)", 0xFF888888);
    
    // 2. Main Area (Waveform)
    int main_x = get_sidebar_width();
    int main_y = 0;
    int main_w = win->width - get_sidebar_width();
    int main_h = win->height - BOTTOM_BAR_HEIGHT;
    
    draw_waveform(win, main_x, main_y, main_w, main_h, s, th);
    
    // 3. Bottom Control Bar
    int bb_y = win->height - BOTTOM_BAR_HEIGHT;
    winmgr_fill_rect(win, main_x, bb_y, main_w, BOTTOM_BAR_HEIGHT, 0xFF1A1A1A);
    winmgr_draw_line(win, main_x, bb_y, win->width, bb_y, 0xFF333333);
    
    // Timer display
    uint32_t total_secs = 0;
    if (s->state == STATE_RECORDING) {
        total_secs = (get_timer_ticks() - s->start_ticks) / 250;
    } else if (s->state == STATE_PLAYING) {
        total_secs = s->play_pos / (REC_SAMPLE_RATE * 2 * 2);
    }
    
    uint32_t mins = total_secs / 60;
    uint32_t secs = total_secs % 60;
    char m_buf[12], s_buf[12];
    k_itoa(mins, m_buf);
    k_itoa(secs, s_buf);
    
    char time_str[64];
    strcpy(time_str, "00:");
    if (mins < 10) { strcat(time_str, "0"); strcat(time_str, m_buf); }
    else { strcat(time_str, m_buf); }
    strcat(time_str, ":");
    if (secs < 10) { strcat(time_str, "0"); strcat(time_str, s_buf); }
    else { strcat(time_str, s_buf); }
    strcat(time_str, ".00 / 00:00:00");
    
    int cx = main_x + main_w / 2;
    int cy = bb_y + BOTTOM_BAR_HEIGHT / 2;
    
    winmgr_draw_text(win, cx + 10, cy - 5, time_str, 0xFFFFFFFF);
    
    // Controls
    // Record Button (Red Circle)
    uint32_t rec_col = (s->state == STATE_RECORDING) ? 0xFFFF0000 : 0xFFE81123;
    winmgr_draw_rounded_rect_ex(win, cx - 50, cy - 20, 40, 40, rec_col, 0, 0, 20);
    if (s->state == STATE_RECORDING) {
        winmgr_fill_rect(win, cx - 40, cy - 10, 20, 20, 0xFFFFFFFF); // Square stop
    } else {
        // Dot in circle
        winmgr_draw_rounded_rect_ex(win, cx - 38, cy - 8, 16, 16, 0xFFFFFFFF, 0, 0, 8);
    }
    
    // Play Button
    uint32_t play_btn_bg = (s->current_size > 0) ? 0xFF333333 : 0xFF252525;
    winmgr_draw_rounded_rect_ex(win, cx + 180, cy - 18, 36, 36, play_btn_bg, 1, 0xFF444444, 18);
    winmgr_draw_text(win, cx + 192, cy - 8, ">", (s->current_size > 0) ? 0xFFFFFFFF : 0xFF666666);
    
    winmgr_draw_text(win, win->width - 120, cy - 5, "1.0 x", 0xFFBBBBBB);
    winmgr_draw_text(win, win->width - 60, cy - 5, "Mark", 0xFFBBBBBB);
}

void recorder_update(window_t *win) {
    if (!win) return;
    recorder_app_t *s = get_state(win);
    if (!s) return;

    if (s->state == STATE_RECORDING) {
        // Read ALL available chunks from the AC97 driver
        while (s->current_size + 1024 <= RECORDER_BUFFER_SIZE) {
            uint32_t read = ac97_read_capture(s->buffer + s->current_size, 1024);
            if (read == 0) break;
            s->current_size += read;
        }

        if (s->current_size >= RECORDER_BUFFER_SIZE - 1024) {
            s->state = STATE_IDLE;
            ac97_stop_capture();
        }
        win->needs_redraw = 1;
    } else if (s->state == STATE_PLAYING) {
        // Real playback streaming
        if (s->play_pos < s->current_size) {
            // Feed approx 20ms of audio per update (at 48kHz Stereo 16-bit, that's ~4k bytes)
            uint32_t chunk = 4096;
            if (s->play_pos + chunk > s->current_size) chunk = s->current_size - s->play_pos;
            
            ac97_stream_pcm(s->buffer + s->play_pos, chunk, 48000, 16, 2);
            s->play_pos += chunk;
        } else {
            s->state = STATE_IDLE;
            s->play_pos = 0;
            ac97_stop_playback();
        }
        win->needs_redraw = 1;
    }
}

static void recorder_on_mouse(window_t *win, int mx, int my, int btns) {
    if (!win || (btns & 1) == 0) return;
    recorder_app_t *s = get_state(win);
    if (!s) return;
    
    int main_x = get_sidebar_width();
    int main_w = win->width - get_sidebar_width();
    int bb_y = win->height - BOTTOM_BAR_HEIGHT;
    int cx = main_x + main_w / 2;
    int cy = bb_y + BOTTOM_BAR_HEIGHT / 2;
    
    // Record Button Click
    if (mx >= cx - 60 && mx <= cx - 10 && my >= cy - 30 && my <= cy + 30) {
        if (s->state == STATE_IDLE) {
            s->state = STATE_RECORDING;
            s->start_ticks = get_timer_ticks();
            s->current_size = 0;
            ac97_start_capture(s->buffer, RECORDER_BUFFER_SIZE);
        } else if (s->state == STATE_RECORDING) {
            s->state = STATE_IDLE;
            ac97_stop_capture();
        }
        win->needs_redraw = 1;
    }
    
    // Play Button Click
    if (mx >= cx + 170 && mx <= cx + 220 && my >= cy - 30 && my <= cy + 30) {
        if (s->state == STATE_IDLE && s->current_size > 0) {
            s->state = STATE_PLAYING;
            s->play_pos = 0;
        } else if (s->state == STATE_PLAYING) {
            s->state = STATE_IDLE;
            ac97_stop_playback();
        }
        win->needs_redraw = 1;
    }
}

window_t *recorder_win = 0;

static void recorder_on_close(window_t *win) {
    if (!win) return;
    recorder_app_t *s = get_state(win);
    if (s) {
        if (s->state == STATE_RECORDING) ac97_stop_capture();
        if (s->state == STATE_PLAYING) ac97_stop_playback();
        if (s->buffer) {
            kfree(s->buffer);
            s->buffer = 0;
        }
        kfree(s);
        win->user_data = 0;
    }
    if (win == recorder_win) {
        recorder_win = 0;
    }
}

void recorder_init(void) {
    print_serial("RECORDER: init start\n");
    
    recorder_app_t *s = (recorder_app_t *)kmalloc(sizeof(recorder_app_t));
    if (!s) {
        print_serial("RECORDER: state allocation FAILED\n");
        return;
    }
    memset(s, 0, sizeof(recorder_app_t));
    
    s->buffer = (uint8_t *)kmalloc(RECORDER_BUFFER_SIZE);
    if (!s->buffer) {
        print_serial("RECORDER: buffer allocation FAILED\n");
        kfree(s);
        return;
    }
    
    print_serial("RECORDER: allocations OK\n");

    window_t *win = winmgr_create_window(-1, -1, 800, 500, "Sound Recorder");
    if (!win) {
        print_serial("RECORDER: window creation FAILED\n");
        kfree(s->buffer);
        kfree(s);
        return;
    }
    
    win->user_data = s;
    win->app_type = 14; 
    win->draw = (void (*)(void *))recorder_draw;
    win->on_mouse = (void (*)(void *, int, int, int))recorder_on_mouse;
    win->on_close = (void (*)(void *))recorder_on_close;
    
    recorder_win = win;
    win->needs_redraw = 1;
    
    print_serial("RECORDER: init complete\n");
}
