#include "../kernel/ui_layout.h"
#include "phone.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../net/net.h"
#include "../drivers/ac97.h"
#include "../lib/speexdsp/include/speex/speex_echo.h"
#include "../lib/speexdsp/include/speex/speex_preprocess.h"
#include "contacts.h"

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
    CALL_STATE_INCALL,
    CALL_STATE_ADD_CONTACT
} call_state_t;

typedef struct {
    char target_username[32];
    char contact_name[32];
    tcp_conn_t conn;
    int connected;
    int connecting;
    int conn_in_progress;
    uint32_t server_ip;
    call_state_t state;
    
    int ui_tab;
    int add_step;
    
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
    // Disabled Speex Preprocess: WebRTC in browser already handles it, 
    // and sending raw pristine 48kHz PCM provides much clearer audio.
}

static void phone_destroy_aec(phone_state_t *s) {
    if (s->preprocess_state) { speex_preprocess_state_destroy(s->preprocess_state); s->preprocess_state = NULL; }
}

typedef struct {
    char number[32];
    char name[32];
} contact_entry_t;
extern contact_entry_t _global_contacts[];
extern int _global_contacts_count;

static phone_state_t* get_state(window_t* win) {
    return (phone_state_t*)win->user_data;
}

static void draw_fill_circle(window_t *win, int cx, int cy, int r, uint32_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                winmgr_put_pixel(win, cx+x, cy+y, color);
            }
        }
    }
}

static void draw_rounded_rect(window_t *win, int x, int y, int w, int h, int r, uint32_t color) {
    winmgr_fill_rect(win, x + r, y, w - 2*r, h, color);
    winmgr_fill_rect(win, x, y + r, r, h - 2*r, color);
    winmgr_fill_rect(win, x + w - r, y + r, r, h - 2*r, color);
    
    int cx1 = x + r;
    int cy1 = y + r;
    int cx2 = x + w - r - 1;
    int cy2 = y + r;
    int cx3 = x + r;
    int cy3 = y + h - r - 1;
    int cx4 = x + w - r - 1;
    int cy4 = y + h - r - 1;

    for (int dy = -r; dy <= 0; dy++) {
        for (int dx = -r; dx <= 0; dx++) {
            if (dx*dx + dy*dy <= r*r) {
                winmgr_put_pixel(win, cx1 + dx, cy1 + dy, color);
                winmgr_put_pixel(win, cx2 - dx, cy2 + dy, color);
                winmgr_put_pixel(win, cx3 + dx, cy3 - dy, color);
                winmgr_put_pixel(win, cx4 - dx, cy4 - dy, color);
            }
        }
    }
}

static int approx_text_width(const char *text) {
    int w = 0;
    while (*text) {
        char c = *text;
        if (c == '1' || c == 'i' || c == 'l' || c == 'I') w += 6;
        else if (c == 'W' || c == 'M') w += 11;
        else if (c == 'w' || c == 'm') w += 9;
        else if (c == 'o') w += 7;
        else if (c == '_') w += 6;
        else w += 8;
        text++;
    }
    return w;
}

static void phone_draw(window_t *win) {
    phone_state_t *s = get_state(win);
    
    int left_w = 200;
    int mid_w = 400;
    int right_w = 300;
    
    winmgr_fill_rect(win, 0, 24, left_w, win->height - 24, 0xFF171A21);
    winmgr_fill_rect(win, left_w, 24, mid_w, win->height - 24, 0xFF111319);
    winmgr_fill_rect(win, left_w + mid_w, 24, right_w, win->height - 24, 0xFF171A21);

    if (!s->connected) {
        winmgr_draw_text(win, win->width/2 - 60, win->height/2, s->connecting ? "Connecting to network..." : "Offline. Click to connect", 0xFF888888);
        return;
    }
    
    winmgr_draw_text(win, 30, 60, "Phone", 0xFFFFFFFF);
    winmgr_draw_text(win, 31, 60, "Phone", 0xFFFFFFFF);
    
    int menu_y = 120;
    const char* tabs[] = {"   Keypad", "   Recents", "   Contacts"};
    for(int i=0; i<3; i++) {
        if(s->ui_tab == i) {
            draw_rounded_rect(win, 15, menu_y - 8, 170, 36, 18, 0xFF2D4E9A);
            winmgr_draw_text(win, 45, menu_y + 2, tabs[i], 0xFFFFFFFF);
        } else {
            winmgr_draw_text(win, 45, menu_y + 2, tabs[i], 0xFF868C96);
        }
        menu_y += 50;
    }
    
    if(s->ui_tab == 1 || s->ui_tab == 0) {
        winmgr_draw_text(win, left_w + 30, 60, "Recents", 0xFFFFFFFF);
        winmgr_draw_text(win, left_w + 31, 60, "Recents", 0xFFFFFFFF);
        
        int ry = 110;
        
        draw_fill_circle(win, left_w + 50, ry + 16, 20, 0xFF353A45);
        winmgr_draw_text(win, left_w + 46, ry + 10, "E", 0xFFFFFFFF);
        winmgr_draw_text(win, left_w + 90, ry, "Emma Watson", 0xFFFFFFFF);
        winmgr_draw_text(win, left_w + 90, ry + 16, "\xFB 10:42 AM", 0xFF868C96);
        
        ry += 60;
        draw_fill_circle(win, left_w + 50, ry + 16, 20, 0xFF353A45);
        winmgr_draw_text(win, left_w + 46, ry + 10, "#", 0xFFFFFFFF);
        winmgr_draw_text(win, left_w + 90, ry, "+1 555-3921", 0xFFFFFFFF);
        winmgr_draw_text(win, left_w + 90, ry + 16, "\xFB Yesterday", 0xFF868C96);
        
        ry += 60;
        draw_fill_circle(win, left_w + 50, ry + 16, 20, 0xFF353A45);
        winmgr_draw_text(win, left_w + 46, ry + 10, "A", 0xFFFFFFFF);
        winmgr_draw_text(win, left_w + 90, ry, "Alex Thompson", 0xFFFFFFFF);
        winmgr_draw_text(win, left_w + 90, ry + 16, "\xFB Oct 14", 0xFF868C96);
    } else if (s->ui_tab == 2) {
        winmgr_draw_text(win, left_w + 30, 60, "Contacts", 0xFFFFFFFF);
        winmgr_draw_text(win, left_w + 31, 60, "Contacts", 0xFFFFFFFF);

        draw_rounded_rect(win, left_w + 30, 110, 150, 36, 18, 0xFF232630);
        winmgr_draw_text(win, left_w + 50, 120, "+ Add Contact", 0xFF3C9B4A);
        
        int cy_c = 170;
        for (int i=0; i<_global_contacts_count; i++) {
            draw_fill_circle(win, left_w + 50, cy_c + 16, 20, 0xFF353A45);
            char initial[2] = { _global_contacts[i].name[0], 0 };
            winmgr_draw_text(win, left_w + 46, cy_c + 10, initial, 0xFFFFFFFF);
            
            winmgr_draw_text(win, left_w + 90, cy_c, _global_contacts[i].name, 0xFFFFFFFF);
            winmgr_draw_text(win, left_w + 90, cy_c + 16, _global_contacts[i].number, 0xFF868C96);
            
            draw_rounded_rect(win, left_w + 310, cy_c, 60, 32, 16, 0xFF3C9B4A);
            winmgr_draw_text(win, left_w + 326, cy_c + 10, "Call", 0xFFFFFFFF);
            cy_c += 60;
        }
    }
    
    draw_fill_circle(win, 840, 60, 15, 0xFF171A21);
    winmgr_draw_text(win, 836, 54, "Q", 0xFF868C96); 
    winmgr_draw_text(win, 870, 54, "...", 0xFF868C96);
    
    int keypad_cx = 750;
    int keypad_cy = 200;
    
    int num_len = strlen(s->target_username);
    if (num_len > 0) {
        winmgr_draw_text(win, 650, 120, s->target_username, 0xFFFFFFFF);
        winmgr_draw_text(win, 850, 120, "X", 0xFF868C96);
    }
    
    const char *keys[12] = {"1","2","3","4","5","6","7","8","9","*","0","#"};
    const char *subkeys[12] = {"\x01","ABC","DEF","GHI","JKL","MNO","PQRS","TUV","WXYZ","","+",""};
    for(int i=0; i<12; i++) {
        int row = i / 3;
        int col = i % 3;
        int kx = keypad_cx + (col - 1) * 82;
        int ky = keypad_cy + row * 82;
        draw_fill_circle(win, kx, ky, 38, 0xFF232630);
        
        int key_w = approx_text_width(keys[i]);
        if (subkeys[i][0] != '\0' && subkeys[i][0] != '\x01') {
            int sub_w = approx_text_width(subkeys[i]);
            winmgr_draw_text(win, kx - (key_w / 2), ky - 8, keys[i], 0xFFFFFFFF);
            winmgr_draw_text(win, kx - (sub_w / 2), ky + 8, subkeys[i], 0xFF868C96);
        } else {
            winmgr_draw_text(win, kx - (key_w / 2), ky - 6, keys[i], 0xFFFFFFFF);
            if (subkeys[i][0] == '\x01') {
               int o_w = approx_text_width("o_o");
               winmgr_draw_text(win, kx - (o_w / 2), ky + 8, "o_o", 0xFF868C96); 
            }
        }
    }
    
    int call_y = keypad_cy + 4 * 82;
    draw_fill_circle(win, keypad_cx, call_y, 38, 0xFF3C9B4A);
    int call_w = approx_text_width("Call");
    winmgr_draw_text(win, keypad_cx - (call_w / 2), call_y - 6, "Call", 0xFFFFFFFF);

    if (s->state != CALL_STATE_IDLE) {
        int pw = 400;
        int ph = 300;
        int px = (win->width - pw) / 2;
        int py = (win->height + 24 - ph) / 2;
        draw_rounded_rect(win, px, py, pw, ph, 20, 0xFF171A21);
        
        int pcx = px + pw / 2;
        int pcy = py + ph / 2;
        
        if (s->state == CALL_STATE_ADD_CONTACT) {
            if (s->add_step == 0) {
                winmgr_draw_text(win, pcx - 80, pcy - 60, "Enter phone number:", 0xFF868C96);
                draw_rounded_rect(win, pcx - 100, pcy - 40, 200, 36, 10, 0xFF111319);
                winmgr_draw_text(win, pcx - 90, pcy - 30, s->target_username, 0xFFFFFFFF);
                
                draw_rounded_rect(win, pcx - 50, pcy + 20, 100, 40, 10, 0xFF3C9B4A);
                winmgr_draw_text(win, pcx - 16, pcy + 32, "Next", 0xFFFFFFFF);
            } else {
                winmgr_draw_text(win, pcx - 80, pcy - 60, "Enter contact name:", 0xFF868C96);
                draw_rounded_rect(win, pcx - 100, pcy - 40, 200, 36, 10, 0xFF111319);
                winmgr_draw_text(win, pcx - 90, pcy - 30, s->contact_name, 0xFFFFFFFF);
                
                draw_rounded_rect(win, pcx - 50, pcy + 20, 100, 40, 10, 0xFF3C9B4A);
                winmgr_draw_text(win, pcx - 16, pcy + 32, "Save", 0xFFFFFFFF);
            }
        } else if (s->state == CALL_STATE_CALLING) {
            winmgr_draw_text(win, pcx - 40, pcy - 40, "Calling...", 0xFFFFFFFF);
            winmgr_draw_text(win, pcx - 40, pcy - 20, contacts_get_name(s->target_username), 0xFF3C9B4A);

            draw_rounded_rect(win, pcx - 60, pcy + 20, 120, 40, 10, 0xFFAA0000);
            winmgr_draw_text(win, pcx - 24, pcy + 32, "Cancel", 0xFFFFFFFF);
        } else if (s->state == CALL_STATE_RINGING) {
            winmgr_draw_text(win, pcx - 50, pcy - 40, "Incoming call!", 0xFF3C9B4A);
            winmgr_draw_text(win, pcx - 40, pcy - 20, contacts_get_name(s->target_username), 0xFFFFFFFF);

            draw_rounded_rect(win, pcx - 100, pcy + 20, 80, 40, 10, 0xFF3C9B4A);
            winmgr_draw_text(win, pcx - 80, pcy + 32, "Accept", 0xFFFFFFFF);

            draw_rounded_rect(win, pcx + 20, pcy + 20, 80, 40, 10, 0xFFAA0000);
            winmgr_draw_text(win, pcx + 40, pcy + 32, "Reject", 0xFFFFFFFF);
        } else if (s->state == CALL_STATE_INCALL) {
            winmgr_draw_text(win, pcx - 30, pcy - 40, "In Call", 0xFF3C9B4A);
            winmgr_draw_text(win, pcx - 40, pcy - 20, contacts_get_name(s->target_username), 0xFFFFFFFF);

            draw_rounded_rect(win, pcx - 60, pcy + 20, 120, 40, 10, 0xFFAA0000);
            winmgr_draw_text(win, pcx - 28, pcy + 32, "Hang Up", 0xFFFFFFFF);
        }
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

static void phone_send_json(phone_state_t *s, const char *json) {
    uint32_t len = strlen(json);
    if (len + 4 > sizeof(s->packet_send_buf)) return;
    *(uint32_t*)s->packet_send_buf = len;
    memcpy(s->packet_send_buf + 4, json, len);
    tcp_send(&s->conn, s->packet_send_buf, len + 4);
}

void phone_update(void *w) {
    window_t *win = (window_t *)w;
    phone_state_t *s = get_state(win);

    if (s->connecting) {
        if (!s->conn_in_progress) {
            s->server_ip = make_ip(10, 0, 2, 2); 
            int ret = tcp_connect(&s->conn, s->server_ip, 7860);
            if (ret == 0) {
                s->connected = 1;
                s->connecting = 0;
                s->conn_in_progress = 0;
                char auth[128];
                strcpy(auth, "{\"type\":\"auth\",\"username\":\"PureOS_Phone\"}");
                phone_send_json(s, auth);
                s->rx_json_len = 0;
            } else if (ret == 1) {
                s->conn_in_progress = 1;
            } else {
                s->connecting = 0;
                s->conn_in_progress = 0;
            }
            win->needs_redraw = 1;
        } else {
            int ret = tcp_check_connect(&s->conn);
            if (ret == 0) {
                s->connected = 1;
                s->connecting = 0;
                s->conn_in_progress = 0;
                char auth[128];
                strcpy(auth, "{\"type\":\"auth\",\"username\":\"PureOS_Phone\"}");
                phone_send_json(s, auth);
                s->rx_json_len = 0;
                win->needs_redraw = 1;
            } else if (ret == 1) {
                // Still waiting
            } else {
                s->connecting = 0;
                s->conn_in_progress = 0;
                win->needs_redraw = 1;
            }
        }
    }

    if (s->connected) {
        extern int pcnet_poll(uint8_t *buf, uint16_t *len_out);
        extern void net_receive(const uint8_t *packet, uint16_t len);
        
        int work_done = 0;
        int max_work = 24; 
        
        while (work_done < max_work) {
            int activity = 0;
            
            if (s->conn.tx_len > 0) {
                int old_len = s->conn.tx_len;
                tcp_flush(&s->conn);
                if (s->conn.tx_len < old_len) activity = 1;
            }
            
            // 1. Poll NIC (Priority) - 4 polls to keep up with audio packets
            for (int p = 0; p < 4; p++) {
                static uint8_t phone_poll_buf[1600];
                uint16_t plen;
                if (pcnet_poll(phone_poll_buf, &plen)) {
                    net_receive(phone_poll_buf, plen);
                    activity = 1;
                }
            }
            
            // 2. Drain TCP via length-prefixed protocol
            if (s->conn.rx_ready) {
                // Read 4-byte length prefix
                if (s->rx_json_len < 4) {
                    int n = tcp_recv(&s->conn, s->rx_json_buf + s->rx_json_len, 4 - s->rx_json_len);
                    if (n > 0) { s->rx_json_len += n; activity = 1; }
                    else if (n < 0) { s->connected = 0; return; }
                }
                
                if (s->rx_json_len >= 4) {
                    uint32_t expected_len = *(uint32_t*)s->rx_json_buf;
                    if (expected_len > sizeof(s->rx_json_buf) - 5) {
                        // Protocol error, drop connection
                        s->connected = 0;
                        return;
                    }
                    int current_data = s->rx_json_len - 4;
                    if (current_data < (int)expected_len) {
                        int n = tcp_recv(&s->conn, s->rx_json_buf + s->rx_json_len, expected_len - current_data);
                        if (n > 0) { s->rx_json_len += n; activity = 1; }
                        else if (n < 0) { s->connected = 0; return; }
                        current_data = s->rx_json_len - 4;
                    }
                    
                    if (current_data == (int)expected_len) {
                        // Full packet received
                        uint8_t *payload = s->rx_json_buf + 4;
                        if (payload[0] == '{') {
                            payload[expected_len] = 0;
                            phone_process_packet(win, (char*)payload);
                        } else if (payload[0] == 0x01) {
                            // Binary audio
                            int pcm_len = expected_len - 1;
                            int16_t *mono = (int16_t *)(payload + 1);
                            int mono_samples = pcm_len / 2;
                            if (mono_samples > 8192) mono_samples = 8192;
                            for (int i = 0; i < mono_samples; i++) {
                                int16_t sample = mono[i];
                                s->rx_stereo_buf[i * 2] = sample;
                                s->rx_stereo_buf[i * 2 + 1] = sample;
                            }
                            ac97_stream_pcm((uint8_t*)s->rx_stereo_buf, mono_samples * 4, 48000, 16, 2);
                        }
                        // Reset buffer for next packet
                        s->rx_json_len = 0;
                        activity = 1;
                    }
                }
            }
            
            // 4. Send all available mic chunks
            if (s->state == CALL_STATE_INCALL) {
                while (1) {
                    int r_mic = ac97_read_capture(s->mic_buf, 1024);
                    if (r_mic <= 0) break;
                    
                    activity = 1;
                    int16_t *samples = (int16_t *)s->mic_buf;
                    int stereo_frames = r_mic / 4; // each frame = 2 samples (L+R) = 4 bytes
                    for (int i = 0; i < stereo_frames; i++) {
                        // Extract left channel only from stereo pairs
                        int16_t sample = samples[i * 2];
                        s->mic_accum[s->mic_accum_len++] = (uint8_t)(sample & 0xFF);
                        s->mic_accum[s->mic_accum_len++] = (uint8_t)((sample >> 8) & 0xFF);
                        
                        if (s->mic_accum_len >= 2048) {
                            uint32_t pkt_len = 1 + 32 + 2048;
                            *(uint32_t*)s->packet_send_buf = pkt_len;
                            s->packet_send_buf[4] = 0x01;
                            memset(s->packet_send_buf + 5, 0, 32);
                            strncpy((char*)s->packet_send_buf + 5, s->target_username, 31);
                            memcpy(s->packet_send_buf + 37, s->mic_accum, 2048);
                            tcp_send(&s->conn, s->packet_send_buf, 4 + pkt_len);
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
        } else if (c == '\n' || c == '\r') {
            if (len > 0) {
                char packet[128];
                strcpy(packet, "{\"type\":\"call_request\",\"from\":\"PureOS_Phone\",\"to\":\"");
                strcat(packet, s->target_username);
                strcat(packet, "\"}");
                phone_send_json(s, packet);
                s->state = CALL_STATE_CALLING;
                win->needs_redraw = 1;
            }
        }
    } else if (s->state == CALL_STATE_ADD_CONTACT) {
        if (s->add_step == 0) {
            int len = strlen(s->target_username);
            if (c >= 32 && c <= 126 && len < 31) {
                s->target_username[len] = c;
                s->target_username[len+1] = 0;
                win->needs_redraw = 1;
            } else if (c == '\b' && len > 0) {
                s->target_username[len-1] = 0;
                win->needs_redraw = 1;
            } else if (c == '\n' || c == '\r') {
                s->add_step = 1;
                win->needs_redraw = 1;
            }
        } else {
            int len = strlen(s->contact_name);
            if (c >= 32 && c <= 126 && len < 31) {
                s->contact_name[len] = c;
                s->contact_name[len+1] = 0;
                win->needs_redraw = 1;
            } else if (c == '\b' && len > 0) {
                s->contact_name[len-1] = 0;
                win->needs_redraw = 1;
            } else if (c == '\n' || c == '\r') {
                contacts_add(s->target_username, s->contact_name);
                s->state = CALL_STATE_IDLE;
                s->ui_tab = 2;
                win->needs_redraw = 1;
            }
        }
    }
}

static void phone_on_mouse(void *w, int x, int y, int buttons) {
    window_t *win = (window_t *)w;
    phone_state_t *s = get_state(win);
    static int last_buttons = 0;
    int click = (buttons & 1) && !(last_buttons & 1);
    last_buttons = buttons;

    if (!click) return;

    if (!s->connected) {
        if (!s->connecting) {
            s->connecting = 1;
            win->needs_redraw = 1;
        }
        return;
    }

    if (s->state != CALL_STATE_IDLE) {
        int pw = 400;
        int ph = 300;
        int px = (win->width - pw) / 2;
        int py = (win->height + 24 - ph) / 2;
        int pcx = px + pw / 2;
        int pcy = py + ph / 2;

        if (s->state == CALL_STATE_ADD_CONTACT) {
            if (x >= pcx - 50 && x <= pcx + 50 && y >= pcy + 20 && y <= pcy + 60) {
                if (s->add_step == 0) s->add_step = 1;
                else {
                    contacts_add(s->target_username, s->contact_name);
                    s->state = CALL_STATE_IDLE;
                    s->ui_tab = 2;
                }
                win->needs_redraw = 1;
            }
        } else if (s->state == CALL_STATE_CALLING || s->state == CALL_STATE_INCALL) {
            if (x >= pcx - 60 && x <= pcx + 60 && y >= pcy + 20 && y <= pcy + 60) {
                char packet[128];
                strcpy(packet, "{\"type\":\"call_end\",\"from\":\"PureOS_Phone\",\"to\":\"");
                strcat(packet, s->target_username);
                strcat(packet, "\"}");
                phone_send_json(s, packet);
                s->state = CALL_STATE_IDLE;
                ac97_stop_capture();
                ac97_stop_playback();
                phone_destroy_aec(s);
                win->needs_redraw = 1;
            }
        } else if (s->state == CALL_STATE_RINGING) {
            if (x >= pcx - 100 && x <= pcx - 20 && y >= pcy + 20 && y <= pcy + 60) {
                char packet[128];
                strcpy(packet, "{\"type\":\"call_accept\",\"from\":\"PureOS_Phone\",\"to\":\"");
                strcat(packet, s->target_username);
                strcat(packet, "\"}");
                phone_send_json(s, packet);
                s->state = CALL_STATE_INCALL;
                ac97_start_capture(NULL, 0);
                phone_init_aec(s);
                win->needs_redraw = 1;
            } else if (x >= pcx + 20 && x <= pcx + 100 && y >= pcy + 20 && y <= pcy + 60) {
                char packet[128];
                strcpy(packet, "{\"type\":\"call_reject\",\"from\":\"PureOS_Phone\",\"to\":\"");
                strcat(packet, s->target_username);
                strcat(packet, "\"}");
                phone_send_json(s, packet);
                s->state = CALL_STATE_IDLE;
                win->needs_redraw = 1;
            }
        }
        return;
    }

    if (x >= 15 && x <= 185 && y >= 112 && y <= 148) { s->ui_tab = 0; win->needs_redraw = 1; }
    if (x >= 15 && x <= 185 && y >= 162 && y <= 198) { s->ui_tab = 1; win->needs_redraw = 1; }
    if (x >= 15 && x <= 185 && y >= 212 && y <= 248) { s->ui_tab = 2; win->needs_redraw = 1; }

    if (s->ui_tab == 2) {
        if (x >= 230 && x <= 380 && y >= 110 && y <= 146) {
            s->contact_name[0] = 0;
            s->target_username[0] = 0;
            s->add_step = 0;
            s->state = CALL_STATE_ADD_CONTACT;
            win->needs_redraw = 1;
            return;
        }
        
        int cy_c = 170;
        for (int i=0; i<_global_contacts_count; i++) {
            if (x >= 510 && x <= 570 && y >= cy_c && y <= cy_c + 32) {
                strcpy(s->target_username, _global_contacts[i].number);
                char packet[128];
                strcpy(packet, "{\"type\":\"call_request\",\"from\":\"PureOS_Phone\",\"to\":\"");
                strcat(packet, s->target_username);
                strcat(packet, "\"}");
                phone_send_json(s, packet);
                s->state = CALL_STATE_CALLING;
                win->needs_redraw = 1;
                return;
            }
            cy_c += 60;
        }
    } else if (s->ui_tab == 0 || s->ui_tab == 1) {
        int ry = 110;
        if (x >= 230 && x <= 500) {
            if (y >= ry && y <= ry+40) { strcpy(s->target_username, "Emma Watson"); win->needs_redraw=1; }
            ry += 60;
            if (y >= ry && y <= ry+40) { strcpy(s->target_username, "+1 555-3921"); win->needs_redraw=1; }
            ry += 60;
            if (y >= ry && y <= ry+40) { strcpy(s->target_username, "Alex Thompson"); win->needs_redraw=1; }
        }
    }

    int keypad_cx = 750;
    int keypad_cy = 200;
    
    for(int i=0; i<12; i++) {
        int row = i / 3;
        int col = i % 3;
        int kx = keypad_cx + (col - 1) * 82;
        int ky = keypad_cy + row * 82;
        if ((x-kx)*(x-kx) + (y-ky)*(y-ky) <= 38*38) {
            const char *keys = "123456789*0#";
            int len = strlen(s->target_username);
            if (len < 31) {
                s->target_username[len] = keys[i];
                s->target_username[len+1] = 0;
                win->needs_redraw = 1;
            }
            return;
        }
    }

    int call_y = keypad_cy + 4 * 82;
    if ((x-keypad_cx)*(x-keypad_cx) + (y-call_y)*(y-call_y) <= 38*38) {
        if (strlen(s->target_username) > 0) {
            char packet[128];
            strcpy(packet, "{\"type\":\"call_request\",\"from\":\"PureOS_Phone\",\"to\":\"");
            strcat(packet, s->target_username);
            strcat(packet, "\"}");
            phone_send_json(s, packet);
            s->state = CALL_STATE_CALLING;
            win->needs_redraw = 1;
        }
        return;
    }

    if (x >= 840 && x <= 860 && y >= 110 && y <= 130) {
        int len = strlen(s->target_username);
        if (len > 0) {
            s->target_username[len-1] = 0;
            win->needs_redraw = 1;
        }
        return;
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
    window_t *win = winmgr_create_window(-1, -1, 900, 600, "Phone");
    if (!win) return;

    phone_state_t *s = (phone_state_t *)kmalloc(sizeof(phone_state_t));
    memset(s, 0, sizeof(phone_state_t));

    strcpy(s->target_username, "");
    s->connecting = 0;
    s->ui_tab = 1;

    win->user_data = s;
    win->app_type = 16;
    win->draw = (void (*)(void *))phone_draw;
    win->on_key = (void (*)(void *, int, char))phone_on_key;
    win->on_mouse = (void (*)(void *, int, int, int))phone_on_mouse;
    win->on_close = (void (*)(void *))phone_on_close;
    
    phone_win = win;
}
