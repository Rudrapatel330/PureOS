#include "recorder.h"
#include "../kernel/window.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../drivers/ac97.h"

extern void print_serial(const char *str);

#define RECORDER_BUFFER_SIZE (2 * 1024 * 1024)
#define REC_SAMPLE_RATE 48000
#define REC_BITS 16
#define REC_CHANNELS 2

// 256-entry sine table, values are 16-bit signed PCM (-32767 to +32767)
// Represents one full cycle of a sine wave
static const int16_t sine_table[256] = {
        0,   804,  1608,  2410,  3212,  4011,  4808,  5602,
     6393,  7179,  7962,  8739,  9512, 10278, 11039, 11793,
    12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530,
    18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
    27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971,
    32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
    32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285,
    32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
    30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683,
    27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868,
    18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279,
    12539, 11793, 11039, 10278,  9512,  8739,  7962,  7179,
     6393,  5602,  4808,  4011,  3212,  2410,  1608,   804,
        0,  -804, -1608, -2410, -3212, -4011, -4808, -5602,
    -6393, -7179, -7962, -8739, -9512,-10278,-11039,-11793,
   -12539,-13279,-14010,-14732,-15446,-16151,-16846,-17530,
   -18204,-18868,-19519,-20159,-20787,-21403,-22005,-22594,
   -23170,-23731,-24279,-24811,-25329,-25832,-26319,-26790,
   -27245,-27683,-28105,-28510,-28898,-29268,-29621,-29956,
   -30273,-30571,-30852,-31113,-31356,-31580,-31785,-31971,
   -32137,-32285,-32412,-32521,-32609,-32678,-32728,-32757,
   -32767,-32757,-32728,-32678,-32609,-32521,-32412,-32285,
   -32137,-31971,-31785,-31580,-31356,-31113,-30852,-30571,
   -30273,-29956,-29621,-29268,-28898,-28510,-28105,-27683,
   -27245,-26790,-26319,-25832,-25329,-24811,-24279,-23731,
   -23170,-22594,-22005,-21403,-20787,-20159,-19519,-18868,
   -18204,-17530,-16846,-16151,-15446,-14732,-14010,-13279,
   -12539,-11793,-11039,-10278, -9512, -8739, -7962, -7179,
    -6393, -5602, -4808, -4011, -3212, -2410, -1608,  -804
};

// Phase accumulator for sine synthesis (16.16 fixed-point, wraps at 256.0)
static uint32_t synth_phase = 0;

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

static recorder_app_t* get_state(window_t* win) {
    return (recorder_app_t*)win->user_data;
}

extern uint32_t get_timer_ticks(void);

static void recorder_draw(window_t *win) {
    recorder_app_t *s = get_state(win);
    const theme_t *th = theme_get();

    // Background
    winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, th->bg);

    int cx = win->width / 2;
    int cy = 120;

    // Status Circle Background (Glassy effect)
    uint32_t status_col = 0xFF444444;
    if (s->state == STATE_RECORDING) status_col = 0xFFAA0000;
    else if (s->state == STATE_PLAYING) status_col = 0xFF00AA00;
    
    // Draw a "vis" area
    winmgr_fill_rect(win, 20, 50, win->width - 40, 80, 0xFF18181B);
    winmgr_draw_rect(win, 20, 50, win->width - 40, 80, th->border);

    // Waveform simulation
    if (s->state == STATE_RECORDING || s->state == STATE_PLAYING) {
        uint32_t ticks = get_timer_ticks();
        for (int i = 0; i < win->width - 50; i += 4) {
            int h = (ticks + i) % 30 + 5;
            if (s->state == STATE_RECORDING) h = (get_timer_ticks() * (i%7)) % 40 + 10;
            winmgr_fill_rect(win, 25 + i, 90 - h/2, 2, h, th->accent);
        }
    }

    // Status Text
    char status_str[32];
    if (s->state == STATE_RECORDING) strcpy(status_str, "RECORDING...");
    else if (s->state == STATE_PLAYING) strcpy(status_str, "PLAYING...");
    else strcpy(status_str, "READY");
    winmgr_draw_text(win, cx - 40, 140, status_str, (s->state == STATE_IDLE) ? th->fg_secondary : th->accent);

    // Time/Size Info
    char info[64];
    uint32_t seconds = s->current_size / (REC_SAMPLE_RATE * (REC_BITS/8) * REC_CHANNELS);
    strcpy(info, "Duration: ");
    char buf[16];
    k_itoa(seconds, buf);
    strcat(info, buf);
    strcat(info, "s | Size: ");
    k_itoa(s->current_size / 1024, buf);
    strcat(info, buf);
    strcat(info, " KB");
    winmgr_draw_text(win, 30, 170, info, th->fg);

    // Buttons
    // Record Button (Circle-ish)
    uint32_t rec_btn_col = (s->state == STATE_RECORDING) ? 0xFFFF0000 : 0xFFAA0000;
    winmgr_fill_rect(win, cx - 60, 220, 40, 40, rec_btn_col);
    winmgr_draw_text(win, cx - 55, 265, "REC", (s->state == STATE_RECORDING) ? 0xFFFF5555 : th->fg);

    // Stop Button
    winmgr_fill_rect(win, cx - 20, 220, 40, 40, 0xFF888888);
    winmgr_draw_text(win, cx - 15, 265, "STOP", th->fg);

    // Play Button
    uint32_t play_btn_col = (s->state == STATE_PLAYING) ? 0xFF00FF00 : 0xFF00AA00;
    winmgr_fill_rect(win, cx + 20, 220, 40, 40, play_btn_col);
    winmgr_draw_text(win, cx + 25, 265, "PLAY", (s->state == STATE_PLAYING) ? 0xFF55FF55 : th->fg);
    
    // Aesthetic footer
    winmgr_draw_text(win, 10, win->height - 20, "PureOS Voice Engine v1.0", 0xFF666666);
}

void recorder_update(void *w) {
    window_t *win = (window_t *)w;
    recorder_app_t *s = get_state(win);

    if (s->state == STATE_RECORDING) {
        uint32_t max_to_read = RECORDER_BUFFER_SIZE - s->current_size;
        if (max_to_read > 0) {
            int r = ac97_read_capture(s->buffer + s->current_size, max_to_read);
            if (r > 0) {
                s->current_size += r;
                win->needs_redraw = 1;
            }
        } else {
            // Buffer full
            ac97_stop_capture();
            s->state = STATE_IDLE;
            win->needs_redraw = 1;
        }
    } else if (s->state == STATE_PLAYING) {
        // Check if playback finished
        if (ac97_is_playback_done()) {
            s->state = STATE_IDLE;
            win->needs_redraw = 1;
            return;
        }
        static int frame_count = 0;
        if (frame_count++ % 20 == 0) {
            uint8_t civ; uint16_t picb, sr;
            ac97_get_playback_status(&civ, &picb, &sr);
            print_serial("AC97 Playback: CIV=");
            char b[16]; k_itoa(civ, b); print_serial(b);
            print_serial(" PICB=");
            k_itoa(picb, b); print_serial(b);
            print_serial(" SR=");
            k_itoa_hex(sr, b); print_serial(b);
            print_serial("\n");
        }
    }
}

static void recorder_on_mouse(void *w, int x, int y, int buttons) {
    window_t *win = (window_t *)w;
    recorder_app_t *s = get_state(win);
    static int last_buttons = 0;
    int click = (buttons & 1) && !(last_buttons & 1);
    last_buttons = buttons;

    if (!click) return;

    int cx = win->width / 2;

    // Record Button
    if (x >= cx - 60 && x <= cx - 20 && y >= 220 && y <= 260) {
        if (s->state == STATE_IDLE) {
            print_serial("RECORDER: Clicked REC\n");
            s->current_size = 0;
            s->state = STATE_RECORDING;
            ac97_start_capture(s->buffer, RECORDER_BUFFER_SIZE);
            win->needs_redraw = 1;
        }
    }
    // Stop Button
    else if (x >= cx - 20 && x <= cx + 20 && y >= 220 && y <= 260) {
        if (s->state == STATE_RECORDING) ac97_stop_capture();
        s->state = STATE_IDLE;
        win->needs_redraw = 1;
    }
    // Play Button
    else if (x >= cx + 20 && x <= cx + 60 && y >= 220 && y <= 260) {
        if (s->state == STATE_IDLE && s->current_size > 0) {
            s->state = STATE_PLAYING;
            ac97_play_pcm(s->buffer, s->current_size, REC_SAMPLE_RATE, REC_BITS, REC_CHANNELS);
            win->needs_redraw = 1;
        }
    }
}
 
window_t *recorder_win = 0;
 
void recorder_on_close(void *w) {
    window_t *win = (window_t *)w;
    recorder_app_t *s = get_state(win);
    if (s->state == STATE_RECORDING) ac97_stop_capture();
    
    if (s->buffer) kfree(s->buffer);
    kfree(s);
    recorder_win = 0;
}

void recorder_init(void) {
    window_t *win = winmgr_create_window(-1, -1, 300, 320, "Voice Recorder");
    if (!win) return;

    recorder_app_t *s = (recorder_app_t *)kmalloc(sizeof(recorder_app_t));
    memset(s, 0, sizeof(recorder_app_t));
    s->buffer = (uint8_t *)kmalloc(RECORDER_BUFFER_SIZE);
    s->state = STATE_IDLE;

    win->user_data = s;
    win->draw = (void (*)(void *))recorder_draw;
    win->on_mouse = (void (*)(void *, int, int, int))recorder_on_mouse;
    win->on_close = (void (*)(void *))recorder_on_close;
 
    recorder_win = win;
}
