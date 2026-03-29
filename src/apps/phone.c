#include "phone.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../net/net.h"
#include "../drivers/ac97.h"
#include "../lib/speexdsp/include/speex/speex_echo.h"
#include "../lib/speexdsp/include/speex/speex_preprocess.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);

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
        uint32_t a = (i < len && src[i] != '=') ? b64_rev(src[i++]) : 0;
        uint32_t b = (i < len && src[i] != '=') ? b64_rev(src[i++]) : 0;
        uint32_t c = (i < len && src[i] != '=') ? b64_rev(src[i++]) : 0;
        uint32_t d = (i < len && src[i] != '=') ? b64_rev(src[i++]) : 0;

        if (i > 0) {
            dst[j++] = (a << 2) | (b >> 4);
            if (i >= 3 && src[i-2] != '=') dst[j++] = (b << 4) | (c >> 2);
            if (i >= 4 && src[i-1] != '=') dst[j++] = (c << 6) | d;
        }
        
        // If we hit padding, skip to next 4-char block
        while (i < len && src[i] == '=') i++;
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
    
    // Networking & Parsing (Lazy memmove support)
    uint8_t rx_json_buf[65536]; 
    int rx_json_len;
    int rx_json_idx;
    
    // Audio Buffers
    uint8_t mic_buf[1024];
    uint8_t mono_buf[8192]; 
    int16_t rx_stereo_buf[16384];
    uint8_t mic_accum[8192];
    int mic_accum_len;
    
    // Pre-allocated scratchpads
    char b64_send_buf[12000]; 
    char packet_send_buf[16384];
    uint8_t pcm_decode_buf[16384];

    SpeexEchoState *echo_state;
    SpeexPreprocessState *preprocess_state;
} phone_state_t;

static void phone_init_aec(phone_state_t *s) {
    if (s->echo_state) return;
    int sample_rate = 48000;
    int frame_size = 1024;
    int filter_length = 4800; // 100ms
    s->echo_state = speex_echo_state_init(frame_size, filter_length);
    speex_echo_ctl(s->echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &sample_rate);
    s->preprocess_state = speex_preprocess_state_init(frame_size, sample_rate);
    speex_preprocess_ctl(s->preprocess_state, SPEEX_PREPROCESS_SET_ECHO_STATE, s->echo_state);
    int enable_ns = 1;
    speex_preprocess_ctl(s->preprocess_state, SPEEX_PREPROCESS_SET_DENOISE, &enable_ns);
    int enable_agc = 1;
    speex_preprocess_ctl(s->preprocess_state, SPEEX_PREPROCESS_SET_AGC, &enable_agc);
}

static void phone_destroy_aec(phone_state_t *s) {
    if (s->echo_state) { speex_echo_state_destroy(s->echo_state); s->echo_state = NULL; }
    if (s->preprocess_state) { speex_preprocess_state_destroy(s->preprocess_state); s->preprocess_state = NULL; }
}

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

static void phone_process_packet(window_t* win, const char* json) {
    phone_state_t* s = get_state(win);
    
    if (strstr(json, "\"type\":\"call_request\"")) {
        char from[32];
        get_json_val(json, "from", from, 32);
        if (s->state == CALL_STATE_IDLE) {
            strcpy(s->target_username, from);
            s->state = CALL_STATE_RINGING;
        }
    } else if (strstr(json, "\"type\":\"call_accept\"")) {
        if (s->state == CALL_STATE_CALLING) {
            s->state = CALL_STATE_INCALL;
            ac97_start_capture(NULL, 0);
            phone_init_aec(s);
        }
    } else if (strstr(json, "\"type\":\"call_reject\"") || strstr(json, "\"type\":\"call_end\"")) {
        s->state = CALL_STATE_IDLE;
        ac97_stop_capture();
        ac97_stop_playback();
        phone_destroy_aec(s);
    } else if (strstr(json, "\"type\":\"audio\"")) {
        if (s->state == CALL_STATE_INCALL) {
            const char *data = strstr(json, "\"data\":\"");
            if (data) {
                data += 8;
                char *end = strchr(data, '"');
                if (end) {
                    *end = 0;
                    int plen = b64_decode(data, strlen(data), s->pcm_decode_buf);
                    *end = '"';
                    
                    if (plen > 0) {
                        int16_t *mono = (int16_t *)s->pcm_decode_buf;
                        int mono_samples = plen / 2;
                        if (mono_samples > 8192) mono_samples = 8192;

                        if (s->echo_state && mono_samples == 1024) {
                            speex_echo_playback(s->echo_state, mono);
                        }

                        for (int i = 0; i < mono_samples; i++) {
                            int16_t sample = mono[i];
                            s->rx_stereo_buf[i * 2] = sample;
                            s->rx_stereo_buf[i * 2 + 1] = sample;
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
            strcpy(auth, "{\"type\":\"auth\",\"username\":\"PureOS_Phone\"} \n");
            tcp_send(&s->conn, auth, strlen(auth));
            s->rx_json_idx = 0;
            s->rx_json_len = 0;
        }
        s->connecting = 0;
        win->needs_redraw = 1;
    }

    if (s->connected) {
        extern int pcnet_poll(uint8_t *buf, uint16_t *len_out);
        extern void net_receive(const uint8_t *packet, uint16_t len);
        
        int work_done = 0;
        int max_work = 24; 
        
        while (work_done < max_work) {
            int activity = 0;
            
            // 1. Poll NIC (Priority) - 4 polls to keep up with audio packets
            for (int p = 0; p < 4; p++) {
                static uint8_t phone_poll_buf[1600];
                uint16_t plen;
                if (pcnet_poll(phone_poll_buf, &plen)) {
                    net_receive(phone_poll_buf, plen);
                    activity = 1;
                }
            }
            
            // 2. Drain TCP with safety margin
            if (s->conn.rx_ready && s->rx_json_len < (int)sizeof(s->rx_json_buf) - 2048) {
                int n = tcp_recv(&s->conn, s->rx_json_buf + s->rx_json_len, 
                                 sizeof(s->rx_json_buf) - s->rx_json_len - 1);
                if (n > 0) {
                    s->rx_json_len += n;
                    s->rx_json_buf[s->rx_json_len] = 0;
                    activity = 1;
                } else if (n < 0) {
                    s->connected = 0;
                    return;
                }
            }
            
            // 3. Process ALL available JSON packets (drain before mic work)
            for (int pkt = 0; pkt < 8; pkt++) {
                char *lines = (char*)s->rx_json_buf + s->rx_json_idx;
                char *nl = strchr(lines, '\n');
                if (!nl) break;
                *nl = 0;
                phone_process_packet(win, lines);
                s->rx_json_idx = (nl + 1) - (char*)s->rx_json_buf;
                activity = 1;
                
                // Pack only when needed (Lazy compaction)
                if (s->rx_json_idx > 32768) {
                    int remain = s->rx_json_len - s->rx_json_idx;
                    if (remain > 0) memmove(s->rx_json_buf, s->rx_json_buf + s->rx_json_idx, remain);
                    s->rx_json_len = remain;
                    s->rx_json_idx = 0;
                    s->rx_json_buf[s->rx_json_len] = 0;
                }
            }
            if (s->rx_json_len - s->rx_json_idx > 32768) {
                // Gap recovery: discard 16KB if stuck
                s->rx_json_idx += 16384; 
                activity = 1;
            }
            
            // 4. Send exactly ONE mic chunk
            if (s->state == CALL_STATE_INCALL) {
                int r_mic = ac97_read_capture(s->mic_buf, 1024);
                if (r_mic > 0) {
                    activity = 1;
                    int16_t *samples = (int16_t *)s->mic_buf;
                    int stereo_frames = r_mic / 4; // each frame = 2 samples (L+R) = 4 bytes
                    for (int i = 0; i < stereo_frames; i++) {
                        // Extract left channel only from stereo pairs
                        int16_t sample = samples[i * 2];
                        s->mic_accum[s->mic_accum_len++] = (uint8_t)(sample & 0xFF);
                        s->mic_accum[s->mic_accum_len++] = (uint8_t)((sample >> 8) & 0xFF);
                        
                        if (s->mic_accum_len >= 2048) {
                            if (s->echo_state) {
                                int16_t processed[1024];
                                speex_echo_capture(s->echo_state, (int16_t*)s->mic_accum, processed);
                                speex_preprocess_run(s->preprocess_state, processed);
                                b64_encode((uint8_t*)processed, 2048, s->b64_send_buf);
                            } else {
                                b64_encode(s->mic_accum, 2048, s->b64_send_buf);
                            }
                            strcpy(s->packet_send_buf, "{\"type\":\"audio\",\"to\":\"");
                            strcat(s->packet_send_buf, s->target_username);
                            strcat(s->packet_send_buf, "\",\"data\":\"");
                            strcat(s->packet_send_buf, s->b64_send_buf);
                            strcat(s->packet_send_buf, "\"} \n");
                            tcp_send(&s->conn, s->packet_send_buf, strlen(s->packet_send_buf));
                            s->mic_accum_len = 0;
                        }
                    }
                }
            }
            
            if (!activity) break;
            work_done++;
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
            strcpy(packet, "{\"type\":\"call_request\",\"from\":\"PureOS_Phone\",\"to\":\"");
            strcat(packet, s->target_username);
            strcat(packet, "\"} \n");
            tcp_send(&s->conn, packet, strlen(packet));
            s->state = CALL_STATE_CALLING;
            win->needs_redraw = 1;
        }
    } else if (s->state == CALL_STATE_CALLING || s->state == CALL_STATE_INCALL) {
        if (x >= cx - 40 && x <= cx + 40 && y >= cy + 20 && y <= cy + 60) {
            char packet[128];
            strcpy(packet, "{\"type\":\"call_end\",\"from\":\"PureOS_Phone\",\"to\":\"");
            strcat(packet, s->target_username);
            strcat(packet, "\"} \n");
            tcp_send(&s->conn, packet, strlen(packet));
            s->state = CALL_STATE_IDLE;
            ac97_stop_capture();
            ac97_stop_playback();
            phone_destroy_aec(s);
            win->needs_redraw = 1;
        }
    } else if (s->state == CALL_STATE_RINGING) {
        if (x >= cx - 90 && x <= cx - 10 && y >= cy + 20 && y <= cy + 60) {
            char packet[128];
            strcpy(packet, "{\"type\":\"call_accept\",\"from\":\"PureOS_Phone\",\"to\":\"");
            strcat(packet, s->target_username);
            strcat(packet, "\"} \n");
            tcp_send(&s->conn, packet, strlen(packet));
            s->state = CALL_STATE_INCALL;
            ac97_start_capture(NULL, 0);
            phone_init_aec(s);
            win->needs_redraw = 1;
        }
        else if (x >= cx + 10 && x <= cx + 90 && y >= cy + 20 && y <= cy + 60) {
            char packet[128];
            strcpy(packet, "{\"type\":\"call_reject\",\"from\":\"PureOS_Phone\",\"to\":\"");
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
        phone_destroy_aec(s);
    }
    if (s->connected) {
        tcp_close(&s->conn);
    }
    phone_win = 0;
    kfree(s);
    win->user_data = 0;
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
