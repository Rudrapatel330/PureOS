#include "../kernel/ui_layout.h"
#include "audio_recv_test.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../net/net.h"
#include "../drivers/ac97.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);

static int b64_rev(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static int b64_decode(const char *src, int len, uint8_t *dst) {
    int i = 0, j = 0;
    while (i < len) {
        int v[4], count = 0;
        while (count < 4 && i < len) {
            if (src[i] == '=') { i = len; break; }
            int val = b64_rev(src[i++]);
            if (val != -1) v[count++] = val;
        }
        if (count >= 2) dst[j++] = (v[0] << 2) | (v[1] >> 4);
        if (count >= 3) dst[j++] = (v[1] << 4) | (v[2] >> 2);
        if (count >= 4) dst[j++] = (v[2] << 6) | v[3];
    }
    return j;
}

typedef struct {
    tcp_conn_t conn;
    int connected;
    int connecting;
    uint32_t server_ip;
    
    // Stats for UI
    int packets_rx;
    int bytes_rx;
    
    // RX buffers
    uint8_t rx_json_buf[65536];
    int rx_json_len;
    uint8_t rx_pcm_mono[16384];
    int16_t rx_stereo_buf[16384];
} audio_test_state_t;

static void audio_test_draw(window_t *win) {
    audio_test_state_t *s = (audio_test_state_t*)win->user_data;
    const theme_t *th = theme_get();

    winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, th->bg);

    if (s->connecting) {
        winmgr_draw_text(win, 10, 40, "Connecting to 10.0.2.2:7870...", th->fg);
        return;
    }
    
    if (!s->connected) {
        winmgr_draw_text(win, 10, 40, "Disconnected.", 0xFFAA0000);
        return;
    }
    
    winmgr_draw_text(win, 10, 40, "Connected to Test Relay: RX ONLY", 0xFF00AA00);
    
    // Print stats
    char buf[64];
    strcpy(buf, "Packets Received: ");
    char num[16];
    k_itoa(s->packets_rx, num);
    strcat(buf, num);
    winmgr_draw_text(win, 10, 70, buf, th->fg);

    strcpy(buf, "Audio Bytes RX: ");
    k_itoa(s->bytes_rx, num);
    strcat(buf, num);
    winmgr_draw_text(win, 10, 90, buf, th->fg);
}

static void audio_test_process_packet(window_t *win, const char *buf) {
    audio_test_state_t *s = (audio_test_state_t*)win->user_data;
    
    if (strstr(buf, "\"type\":\"audio\"")) {
        char *d = strstr((char*)buf, "\"data\":\"");
        if (d) {
            d += 8;
            char *end = strchr(d, '"');
            if (end) {
                *end = 0;
                int b64len = end - d;
                int plen = b64_decode(d, b64len, s->rx_pcm_mono);
                
                s->packets_rx++;
                s->bytes_rx += plen;
                
                // Print detailed debug to help isolate drops
                char dbg[128];
                strcpy(dbg, "AUDIO TEST: RX b64=");
                char num[12];
                k_itoa(b64len, num);
                strcat(dbg, num);
                strcat(dbg, " pcm=");
                k_itoa(plen, num);
                strcat(dbg, num);
                
                if (plen > 0) {
                    // Upmix to stereo
                    int16_t *mono = (int16_t *)s->rx_pcm_mono;
                    int mono_samples = plen / 2;
                    if (mono_samples > 8192) mono_samples = 8192;
                    
                    int16_t max_val = 0;
                    for (int i = 0; i < mono_samples; i++) {
                        int32_t sample = (int32_t)mono[i]; 
                        if (sample > 32767) sample = 32767;
                        if (sample < -32768) sample = -32768;
                        
                        s->rx_stereo_buf[i * 2] = (int16_t)sample;     
                        s->rx_stereo_buf[i * 2 + 1] = (int16_t)sample;
                        
                        int16_t abs_val = (mono[i] < 0) ? -mono[i] : mono[i];
                        if (abs_val > max_val) max_val = abs_val;
                    }
                    
                    strcat(dbg, " samp=");
                    k_itoa(mono_samples, num);
                    strcat(dbg, num);
                    strcat(dbg, " max_amp=");
                    k_itoa(max_val, num);
                    strcat(dbg, num);
                    strcat(dbg, "\n");
                    print_serial(dbg);
                    
                    // Force AC97 start if it's dead
                    ac97_stream_pcm((uint8_t*)s->rx_stereo_buf, mono_samples * 4, 48000, 16, 2);
                    
                    if (s->packets_rx % 50 == 0 && mono_samples > 4) {
                        print_serial("AUDIO TEST: [DEBUG PCM] ");
                        for(int k=0; k<4; k++) {
                            k_itoa(mono[k], num);
                            print_serial(num); print_serial(" ");
                        }
                        print_serial("\n");
                    }
                } else {
                    strcat(dbg, " EMPTY!\n");
                    print_serial(dbg);
                }
            }
        }
    }
    // Rate limit redraws
    if (s->packets_rx % 10 == 0) {
        win->needs_redraw = 1;
    }
}

void audio_test_update(void *w) {
    window_t *win = (window_t *)w;
    audio_test_state_t *s = (audio_test_state_t*)win->user_data;

    if (s->connecting) {
        s->server_ip = make_ip(10, 0, 2, 2); 
        // Port 7870 is the Test Relay
        if (tcp_connect(&s->conn, s->server_ip, 7870) == 0) {
            s->connected = 1;
            char auth[128];
            strcpy(auth, "{\"type\":\"auth\",\"username\":\"PureOS_Test\"} \n");
            tcp_send(&s->conn, auth, strlen(auth));
            print_serial("AUDIO TEST: Connected to relay\n");
        }
        s->connecting = 0;
        win->needs_redraw = 1;
    }

    if (s->connected) {
        // Local self-test: play a 1-second 440Hz tone upon connection
        static int self_test_done = 0;
        if (!self_test_done) {
            static int16_t test_buf[4096 * 2];
            for (int i = 0; i < 4096; i++) {
                int16_t v = (i % 100 < 50) ? 8000 : -8000; 
                test_buf[i * 2] = v;
                test_buf[i * 2 + 1] = v;
            }
            ac97_stream_pcm((uint8_t*)test_buf, sizeof(test_buf), 48000, 16, 2);
            print_serial("AUDIO TEST: Local Self-Test Played (Stack Fixed)\n");
            self_test_done = 1;
        }

        // Poll NIC aggressive to drain TCP RX
        extern int pcnet_poll(uint8_t *buf, uint16_t *len_out);
        extern void net_receive(const uint8_t *packet, uint16_t len);
        static uint8_t test_poll_buf[1600];
        uint16_t plen;
        int polls = 0;
        
        char buf[8192]; // Large read buffer
        while (polls < 64) {
            // Drain TCP rx_buf into JSON buffer
            while (s->conn.rx_ready) {
                int n = tcp_recv(&s->conn, buf, sizeof(buf) - 1);
                if (n == 0) break;
                
                if (n > 0) {
                    buf[n] = 0;
                    if (s->rx_json_len + n < (int)sizeof(s->rx_json_buf)) {
                        memcpy(s->rx_json_buf + s->rx_json_len, buf, n);
                        s->rx_json_len += n;
                        s->rx_json_buf[s->rx_json_len] = 0;
                        
                        char *lines = (char*)s->rx_json_buf;
                        char *nl;
                        while ((nl = strchr(lines, '\n')) != 0) {
                            *nl = 0;
                            audio_test_process_packet(win, lines);
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
                        print_serial("AUDIO TEST: JSON Buffer Overflow! Dropping.\n");
                        s->rx_json_len = 0;
                    }
                } else if (n < 0) {
                    s->connected = 0;
                    ac97_stop_playback();
                    win->needs_redraw = 1;
                    print_serial("AUDIO TEST: Connection lost.\n");
                    break;
                }
            }
            
            // Poll next packet
            if (s->connected && pcnet_poll(test_poll_buf, &plen) > 0) {
                net_receive(test_poll_buf, plen);
                polls++;
            } else {
                break; // No more NIC packets
            }
        }
    }
}

void audio_test_on_close(void *w) {
    window_t *win = (window_t *)w;
    audio_test_state_t *s = (audio_test_state_t*)win->user_data;
    
    ac97_stop_playback();
    if (s->connected) {
        tcp_close(&s->conn);
    }
    
    extern window_t *audio_test_win;
    audio_test_win = 0;
    
    kfree(s);
    win->user_data = 0;
}

window_t *audio_test_win = 0;

void audio_recv_test_init(void) {
    window_t *win = winmgr_create_window(-1, -1, 400, 200, "Audio RX Test");
    if (!win) return;

    audio_test_state_t *s = (audio_test_state_t *)kmalloc(sizeof(audio_test_state_t));
    memset(s, 0, sizeof(audio_test_state_t));

    s->connecting = 1;

    win->user_data = s;
    win->draw = (void (*)(void *))audio_test_draw;
    win->on_close = (void (*)(void *))audio_test_on_close;
    
    audio_test_win = win;
}
