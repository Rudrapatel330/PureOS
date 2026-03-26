#include "phone.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../net/net.h"
#include "../drivers/ac97.h"

static const char b64table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int b64_encode(const uint8_t *src, int len, char *dst) {
    int i = 0, j = 0;
    for (i = 0; i < len - 2; i += 3) {
        uint32_t v = (src[i] << 16) | (src[i+1] << 8) | src[i+2];
        dst[j++] = b64table[(v >> 18) & 63];
        dst[j++] = b64table[(v >> 12) & 63];
        dst[j++] = b64table[(v >> 6) & 63];
        dst[j++] = b64table[v & 63];
    }
    if (i < len) {
        uint32_t v = src[i] << 16;
        if (i + 1 < len) v |= src[i+1] << 8;
        dst[j++] = b64table[(v >> 18) & 63];
        dst[j++] = b64table[(v >> 12) & 63];
        dst[j++] = (i + 1 < len) ? b64table[(v >> 6) & 63] : '=';
        dst[j++] = '=';
    }
    dst[j] = 0;
    return j;
}

static int b64_rev(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return 0;
}

static int b64_decode(const char *src, int len, uint8_t *dst) {
    int i = 0, j = 0;
    while (i < len) {
        if (src[i] == '=') break;
        uint32_t a = b64_rev(src[i++]);
        uint32_t b = i < len && src[i] != '=' ? b64_rev(src[i++]) : 0;
        uint32_t c = i < len && src[i] != '=' ? b64_rev(src[i++]) : 0;
        uint32_t d = i < len && src[i] != '=' ? b64_rev(src[i++]) : 0;
        dst[j++] = (a << 2) | (b >> 4);
        if (i <= len && src[i - 2] != '=') dst[j++] = (b << 4) | (c >> 2);
        if (i <= len && src[i - 1] != '=') dst[j++] = (c << 6) | d;
    }
    return j;
}

static void get_json_val(const char *json, const char *key, char *out, int max_len) {
    out[0] = 0;
    char search[32];
    strcpy(search, "\"");
    strcat(search, key);
    strcat(search, "\":\"");
    char *p = strstr(json, search);
    if (p) {
        p += strlen(search);
        int i = 0;
        while (*p && *p != '"' && i < max_len - 1) {
            out[i++] = *p++;
        }
        out[i] = 0;
    }
}

typedef enum {
    CALL_STATE_IDLE,
    CALL_STATE_CALLING,
    CALL_STATE_RINGING,
    CALL_STATE_INCALL
} call_state_t;

typedef struct {
    char target_username[32];
    tcp_conn_t conn;
    int connected;
    int connecting;
    uint32_t server_ip;
    call_state_t state;
    
    uint8_t mic_buf[1024];
    uint8_t mono_buf[8192];
    uint8_t rx_json_buf[32768];
    int rx_json_len;
    
    // RX audio buffers moved off stack to prevent overflow
    uint8_t rx_pcm_mono[16384];
    int16_t rx_stereo_buf[16384];
    
    uint8_t mic_accum[16384]; // ~170ms stereo audio chunk
    int mic_accum_len;
} phone_state_t;

static phone_state_t* get_state(window_t* win) {
    return (phone_state_t*)win->user_data;
}

static void phone_draw(window_t *win) {
    phone_state_t *s = get_state(win);
    const theme_t *th = theme_get();

    winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, th->bg);

    int cx = win->width / 2;
    int cy = (win->height + 24) / 2;

    if (!s->connected) {
        winmgr_draw_text(win, cx - 60, cy, s->connecting ? "Connecting to network..." : "Network Offline", 0xFF888888);
        return;
    }
    
    winmgr_draw_text(win, 10, 30, "KABUTAR PHONE", th->accent);

    if (s->state == CALL_STATE_IDLE) {
        winmgr_draw_text(win, cx - 80, cy - 60, "Enter target to dial:", th->fg);
        winmgr_fill_rect(win, cx - 80, cy - 40, 160, 30, th->input_bg);
        winmgr_draw_rect(win, cx - 80, cy - 40, 160, 30, th->border);
        winmgr_draw_text(win, cx - 70, cy - 32, s->target_username, th->fg);

        winmgr_fill_rect(win, cx - 40, cy + 10, 80, 40, 0xFF00AA00);
        winmgr_draw_text(win, cx - 15, cy + 22, "Dial", 0xFFFFFFFF);
    } else if (s->state == CALL_STATE_CALLING) {
        winmgr_draw_text(win, cx - 40, cy - 30, "Calling...", th->fg);
        winmgr_draw_text(win, cx - 40, cy - 10, s->target_username, th->accent);

        winmgr_fill_rect(win, cx - 40, cy + 20, 80, 40, 0xFFAA0000);
        winmgr_draw_text(win, cx - 25, cy + 32, "Cancel", 0xFFFFFFFF);
    } else if (s->state == CALL_STATE_RINGING) {
        winmgr_draw_text(win, cx - 50, cy - 30, "Incoming call!", 0xFF00AA00);
        winmgr_draw_text(win, cx - 40, cy - 10, s->target_username, th->accent);

        winmgr_fill_rect(win, cx - 90, cy + 20, 80, 40, 0xFF00AA00);
        winmgr_draw_text(win, cx - 75, cy + 32, "Accept", 0xFFFFFFFF);

        winmgr_fill_rect(win, cx + 10, cy + 20, 80, 40, 0xFFAA0000);
        winmgr_draw_text(win, cx + 25, cy + 32, "Reject", 0xFFFFFFFF);
    } else if (s->state == CALL_STATE_INCALL) {
        winmgr_draw_text(win, cx - 30, cy - 30, "In Call", 0xFF00AA00);
        winmgr_draw_text(win, cx - 40, cy - 10, s->target_username, th->accent);

        winmgr_fill_rect(win, cx - 40, cy + 20, 80, 40, 0xFFAA0000);
        winmgr_draw_text(win, cx - 30, cy + 32, "Hang Up", 0xFFFFFFFF);
    }
}

static void phone_process_packet(window_t *win, const char *buf) {
    phone_state_t *s = get_state(win);
    
    if (strstr(buf, "\"type\":\"call_request\"")) {
        char from[32];
        get_json_val(buf, "from", from, 32);
        if (s->state == CALL_STATE_IDLE) {
            strcpy(s->target_username, from);
            s->state = CALL_STATE_RINGING;
        }
    } else if (strstr(buf, "\"type\":\"call_accept\"")) {
        if (s->state == CALL_STATE_CALLING) {
            s->state = CALL_STATE_INCALL;
            ac97_start_capture(NULL, 0);
        }
    } else if (strstr(buf, "\"type\":\"call_reject\"") || strstr(buf, "\"type\":\"call_end\"")) {
        s->state = CALL_STATE_IDLE;
        ac97_stop_capture();
        ac97_stop_playback();
    } else if (strstr(buf, "\"type\":\"audio\"")) {
        if (s->state == CALL_STATE_INCALL) {
            char *d = strstr((char*)buf, "\"data\":\"");
            if (d) {
                d += 8;
                char *end = strchr(d, '"');
                if (end) {
                    *end = 0;
                    int plen = b64_decode(d, end - d, s->rx_pcm_mono);
                    if (plen > 0) {
                        // Upmix mono to stereo for AC97
                        int16_t *mono = (int16_t *)s->rx_pcm_mono;
                        int mono_samples = plen / 2;
                        
                        // Prevent overflow
                        if (mono_samples > 16384) mono_samples = 16384;
                        
                        for (int i = 0; i < mono_samples; i++) {
                            s->rx_stereo_buf[i * 2] = mono[i];     // Left
                            s->rx_stereo_buf[i * 2 + 1] = mono[i]; // Right
                        }
                        ac97_stream_pcm((uint8_t*)s->rx_stereo_buf, mono_samples * 4, 48000, 16, 2);
                    }
                }
            }
        }
    }
    win->needs_redraw = 1;
}

void phone_update(void *w) {
    window_t *win = (window_t *)w;
    phone_state_t *s = get_state(win);

    if (s->connecting) {
        s->server_ip = make_ip(10, 0, 2, 2); 
        if (tcp_connect(&s->conn, s->server_ip, 7860) == 0) {
            s->connected = 1;
            char auth[128];
            strcpy(auth, "{\"type\":\"auth\",\"username\":\"PureOS_User\"} \n");
            tcp_send(&s->conn, auth, strlen(auth));
        }
        s->connecting = 0;
        win->needs_redraw = 1;
    }

    if (s->connected) {
        char buf[512];
        while (s->conn.rx_ready) {
            int n = tcp_recv(&s->conn, buf, 511);
            if (n == 0) break; // No more data available right now
            
            if (n > 0) {
                buf[n] = 0;
                if (s->rx_json_len + n < sizeof(s->rx_json_buf)) {
                    memcpy(s->rx_json_buf + s->rx_json_len, buf, n);
                    s->rx_json_len += n;
                    s->rx_json_buf[s->rx_json_len] = 0;
                    
                    char *lines = (char*)s->rx_json_buf;
                    char *nl;
                    while ((nl = strchr(lines, '\n')) != 0) {
                        *nl = 0;
                        phone_process_packet(win, lines);
                        lines = nl + 1;
                    }
                    
                    int remain = s->rx_json_len - (lines - (char*)s->rx_json_buf);
                    if (remain > 0) {
                        memmove(s->rx_json_buf, lines, remain);
                        s->rx_json_len = remain;
                    } else {
                        s->rx_json_len = 0;
                    }
                } else {
                    // Buffer full without newline? Discard to prevent deadlock
                    s->rx_json_len = 0;
                }
            } else if (n < 0) {
                s->connected = 0;
                s->state = CALL_STATE_IDLE;
                ac97_stop_capture();
                ac97_stop_playback();
                win->needs_redraw = 1;
                break;
            }
        }
    }

    if (s->state == CALL_STATE_INCALL && s->connected) {
        while (1) {
            int r = ac97_read_capture(s->mic_buf, 1024);
            if (r <= 0) break;
            
            // Append to accumulator
            int to_copy = r;
            if (s->mic_accum_len + to_copy > sizeof(s->mic_accum)) {
                to_copy = sizeof(s->mic_accum) - s->mic_accum_len;
            }
            memcpy(s->mic_accum + s->mic_accum_len, s->mic_buf, to_copy);
            s->mic_accum_len += to_copy;
            
            // Send when accumulated chunk is full
            if (s->mic_accum_len == sizeof(s->mic_accum)) {
                // Downsample accumulated stereo to mono
                int16_t *stereo = (int16_t *)s->mic_accum;
                int16_t *mono = (int16_t *)s->mono_buf;
                int stereo_samples = sizeof(s->mic_accum) / 4;
                for (int i = 0; i < stereo_samples; i++) {
                    mono[i] = (stereo[i * 2] / 2) + (stereo[i * 2 + 1] / 2);
                }
                int mono_bytes = stereo_samples * 2;
                
                char b64[16384]; // large enough for base64 of 8KB
                b64_encode(s->mono_buf, mono_bytes, b64);
                
                // Construct JSON packet
                char packet[18000]; // large enough for JSON + 11KB base64
                strcpy(packet, "{\"type\":\"audio\",\"from\":\"PureOS_User\",\"to\":\"");
                strcat(packet, s->target_username);
                strcat(packet, "\",\"data\":\"");
                strcat(packet, b64);
                strcat(packet, "\"} \n");
                
                tcp_send(&s->conn, packet, strlen(packet));
                
                // Reset accumulator
                s->mic_accum_len = 0;
            }
        }
    }
}

static void phone_on_key(void *w, int key, char c) {
    window_t *win = (window_t *)w;
    phone_state_t *s = get_state(win);

    if (s->state == CALL_STATE_IDLE) {
        int len = strlen(s->target_username);
        if (c >= 32 && c <= 126 && len < 31) {
            s->target_username[len] = c;
            s->target_username[len+1] = 0;
            win->needs_redraw = 1;
        } else if (c == '\b' && len > 0) {
            s->target_username[len-1] = 0;
            win->needs_redraw = 1;
        }
    }
}

static void phone_on_mouse(void *w, int x, int y, int buttons) {
    window_t *win = (window_t *)w;
    phone_state_t *s = get_state(win);
    static int last_buttons = 0;
    int click = (buttons & 1) && !(last_buttons & 1);
    last_buttons = buttons;

    if (!click || !s->connected) return;

    int cx = win->width / 2;
    int cy = (win->height + 24) / 2;

    if (s->state == CALL_STATE_IDLE) {
        if (x >= cx - 40 && x <= cx + 40 && y >= cy + 10 && y <= cy + 50) {
            char packet[128];
            strcpy(packet, "{\"type\":\"call_request\",\"from\":\"PureOS_User\",\"to\":\"");
            strcat(packet, s->target_username);
            strcat(packet, "\"} \n");
            tcp_send(&s->conn, packet, strlen(packet));
            s->state = CALL_STATE_CALLING;
            win->needs_redraw = 1;
        }
    } else if (s->state == CALL_STATE_CALLING || s->state == CALL_STATE_INCALL) {
        if (x >= cx - 40 && x <= cx + 40 && y >= cy + 20 && y <= cy + 60) {
            char packet[128];
            strcpy(packet, "{\"type\":\"call_end\",\"from\":\"PureOS_User\",\"to\":\"");
            strcat(packet, s->target_username);
            strcat(packet, "\"} \n");
            tcp_send(&s->conn, packet, strlen(packet));
            s->state = CALL_STATE_IDLE;
            ac97_stop_capture();
            ac97_stop_playback();
            win->needs_redraw = 1;
        }
    } else if (s->state == CALL_STATE_RINGING) {
        if (x >= cx - 90 && x <= cx - 10 && y >= cy + 20 && y <= cy + 60) {
            char packet[128];
            strcpy(packet, "{\"type\":\"call_accept\",\"from\":\"PureOS_User\",\"to\":\"");
            strcat(packet, s->target_username);
            strcat(packet, "\"} \n");
            tcp_send(&s->conn, packet, strlen(packet));
            s->state = CALL_STATE_INCALL;
            ac97_start_capture(NULL, 0);
            win->needs_redraw = 1;
        }
        else if (x >= cx + 10 && x <= cx + 90 && y >= cy + 20 && y <= cy + 60) {
            char packet[128];
            strcpy(packet, "{\"type\":\"call_reject\",\"from\":\"PureOS_User\",\"to\":\"");
            strcat(packet, s->target_username);
            strcat(packet, "\"} \n");
            tcp_send(&s->conn, packet, strlen(packet));
            s->state = CALL_STATE_IDLE;
            win->needs_redraw = 1;
        }
    }
}

window_t *phone_win = 0;

void phone_on_close(void *w) {
    window_t *win = (window_t *)w;
    phone_state_t *s = get_state(win);
    if (s->state == CALL_STATE_INCALL) {
        ac97_stop_capture();
        ac97_stop_playback();
    }
    if (s->connected) {
        tcp_close(&s->conn);
    }
    phone_win = 0;
    kfree(s);
}

void phone_init(void) {
    window_t *win = winmgr_create_window(-1, -1, 300, 400, "Phone");
    if (!win) return;

    phone_state_t *s = (phone_state_t *)kmalloc(sizeof(phone_state_t));
    memset(s, 0, sizeof(phone_state_t));

    strcpy(s->target_username, "Android_User");
    s->connecting = 1;

    win->user_data = s;
    win->draw = (void (*)(void *))phone_draw;
    win->on_key = (void (*)(void *, int, char))phone_on_key;
    win->on_mouse = (void (*)(void *, int, int, int))phone_on_mouse;
    win->on_close = (void (*)(void *))phone_on_close;
    
    phone_win = win;
}
