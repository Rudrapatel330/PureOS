#include "chat.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../net/net.h"

// --- Chat State Structures ---

#define MAX_CHAT_HISTORY 50
#define MSG_LEN 256
#define USER_LEN 32

typedef struct {
    char sender[USER_LEN];
    char text[MSG_LEN];
    int is_self;
} chat_msg_t;

typedef struct {
    char my_username[USER_LEN];
    char target_username[USER_LEN];
    char input_buf[MSG_LEN];
    int input_pos;
    
    chat_msg_t history[MAX_CHAT_HISTORY];
    int history_count;
    
    tcp_conn_t conn;
    int connected;
    int connecting;
    uint32_t server_ip;
    
    char debug_buf[128]; // For troubleshooting incoming data
} chat_state_t;

// --- UI Constants ---
#define SIDE_WIDTH 150
#define CHAT_PADDING 10

// --- Implementation ---

static chat_state_t* get_state(window_t* win) {
    return (chat_state_t*)win->user_data;
}

static void chat_draw(window_t *win) {
    chat_state_t *s = get_state(win);
    const theme_t *th = theme_get();

    // Background
    winmgr_fill_rect(win, 0, 24, win->width, win->height - 24, th->bg);

    // Sidebar (Contacts)
    winmgr_fill_rect(win, 0, 24, SIDE_WIDTH, win->height - 24, th->menu_bg);
    winmgr_fill_rect(win, SIDE_WIDTH - 1, 24, 1, win->height - 24, th->border);
    winmgr_draw_text(win, 10, 35, "Contacts", th->fg);
    winmgr_draw_text(win, 10, 60, s->target_username[0] ? s->target_username : "No active chat", th->accent);

    // Chat Area
    int chat_x = SIDE_WIDTH + 10;
    int chat_y = 40;
    int chat_w = win->width - SIDE_WIDTH - 20;

    // Status
    if (!s->connected) {
        winmgr_draw_text(win, chat_x, chat_y, s->connecting ? "Connecting..." : "Disconnected. Click to connect.", 0xFF888888);
    } else {
        winmgr_draw_text(win, chat_x, chat_y, "Connected to Relay", 0xFF00AA00);
    }

    // history
    int cur_y = win->height - 80;
    for (int i = s->history_count - 1; i >= 0 && cur_y > 60; i--) {
        chat_msg_t *m = &s->history[i];
        int bubble_h = 24;
        int text_len = strlen(m->text);
        int bubble_w = text_len * 8 + 20;
        if (bubble_w > chat_w - 40)
            bubble_w = chat_w - 40;

        int bx = m->is_self ? (win->width - bubble_w - 10) : (chat_x);

        uint32_t b_color = m->is_self ? th->accent : 0xFF333333; // Darker bubble for 'other'
        winmgr_fill_rect(win, bx, cur_y - bubble_h, bubble_w, bubble_h, b_color);
        winmgr_draw_text(win, bx + 10, cur_y - bubble_h + 8, m->text, 0xFFFFFFFF); // Always use white for contrast

        cur_y -= (bubble_h + 10);
    }

    // Input Box
    int input_y = win->height - 40;
    winmgr_fill_rect(win, chat_x, input_y, chat_w - 60, 30, th->input_bg);
    winmgr_draw_rect(win, chat_x, input_y, chat_w - 60, 30, th->border);
    winmgr_draw_text(win, chat_x + 10, input_y + 8, s->input_buf, th->fg);

    // Send Button
    winmgr_fill_rect(win, win->width - 60, input_y, 50, 30, th->accent);
    winmgr_draw_text(win, win->width - 55, input_y + 8, "Send", 0xFFFFFFFF);

    // Debug Overlay (Top)
    if (s->debug_buf[0]) {
        winmgr_draw_text(win, SIDE_WIDTH + 10, 26, s->debug_buf, 0xFFFFFF00);
    }
}

// Simple JSON-ish parser for our relay messages
static void chat_process_packet(window_t *win, const char *buf) {
    chat_state_t *s = get_state(win);
    
    // Copy to debug buffer (first 60 chars)
    int blen = strlen(buf);
    k_itoa(blen, s->debug_buf);
    strcat(s->debug_buf, " recv: ");
    strncat(s->debug_buf, buf, 60);
    
    // Safety: ensure we have space in history
    if (s->history_count >= MAX_CHAT_HISTORY) return;

    // Safer JSON-ish parsing
    const char *text_key = "\"text\"";
    const char *from_key = "\"from\"";
    
    const char *text_ptr = strstr(buf, text_key);
    const char *from_ptr = strstr(buf, from_key);

    if (text_ptr && from_ptr) {
        chat_msg_t *m = &s->history[s->history_count++];
        
        // Find opening quote for Text
        text_ptr += 6; // skip "text"
        while (*text_ptr && *text_ptr != ':') text_ptr++; // find :
        if (*text_ptr == ':') text_ptr++;
        while (*text_ptr && *text_ptr != '\"') text_ptr++; // find opening "
        if (*text_ptr == '\"') text_ptr++;

        int i = 0;
        while (*text_ptr && *text_ptr != '\"' && i < MSG_LEN - 1) {
            m->text[i++] = *text_ptr++;
        }
        m->text[i] = 0;

        // Find opening quote for Sender
        from_ptr += 6; // skip "from"
        while (*from_ptr && *from_ptr != ':') from_ptr++; // find :
        if (*from_ptr == ':') from_ptr++;
        while (*from_ptr && *from_ptr != '\"') from_ptr++; // find opening "
        if (*from_ptr == '\"') from_ptr++;
        
        i = 0;
        while (*from_ptr && *from_ptr != '\"' && i < USER_LEN - 1) {
            m->sender[i++] = *from_ptr++;
        }
        m->sender[i] = 0;
        
        m->is_self = (strcmp(m->sender, s->my_username) == 0);
    }
}

void chat_update(void *w) {
    window_t *win = (window_t *)w;
    chat_state_t *s = get_state(win);

    if (s->connecting) {
        // Our server IP - user should update this to their HF Space IP or domain
        // For testing, we can use 10.0.2.2 (alias for host machine in Bochs/QEMU)
        s->server_ip = make_ip(10, 0, 2, 2); 
        if (tcp_connect(&s->conn, s->server_ip, 7860) == 0) {
            s->connected = 1;
            // Send Auth
            char auth[128];
            strcpy(auth, "{\"type\":\"auth\",\"username\":\"");
            strcat(auth, s->my_username);
            strcat(auth, "\"} \n");
            tcp_send(&s->conn, auth, strlen(auth));
        }
        s->connecting = 0;
        win->needs_redraw = 1;
    }

    if (s->connected && s->conn.rx_ready) {
        char buf[512];
        int n = tcp_recv(&s->conn, buf, 511);
        if (n > 0) {
            buf[n] = 0;
            chat_process_packet(win, buf);
            win->needs_redraw = 1;
        } else if (n < 0) {
            s->connected = 0;
            win->needs_redraw = 1;
        }
    }
}

static void chat_send_message(window_t *win) {
    chat_state_t *s = get_state(win);
    if (!s->connected || s->input_pos == 0) return;

    // Send: {"type":"chat","from":"...","to":"...","message":"..."}
    char packet[512];
    strcpy(packet, "{\"type\":\"chat\",\"from\":\"");
    strcat(packet, s->my_username);
    strcat(packet, "\",\"to\":\"");
    strcat(packet, s->target_username);
    strcat(packet, "\",\"message\":\"");
    strcat(packet, s->input_buf);
    strcat(packet, "\"} \n");

    tcp_send(&s->conn, packet, strlen(packet));
    
    // Add to local history
    if (s->history_count < MAX_CHAT_HISTORY) {
        chat_msg_t *m = &s->history[s->history_count++];
        strcpy(m->sender, s->my_username);
        strcpy(m->text, s->input_buf);
        m->is_self = 1;
    }

    s->input_pos = 0;
    s->input_buf[0] = 0;
    win->needs_redraw = 1;
}

static void chat_on_key(void *w, int key, char c) {
    window_t *win = (window_t *)w;
    chat_state_t *s = get_state(win);

    if (c >= 32 && c <= 126 && s->input_pos < MSG_LEN - 1) {
        s->input_buf[s->input_pos++] = c;
        s->input_buf[s->input_pos] = 0;
        win->needs_redraw = 1;
    } else if (c == '\b' && s->input_pos > 0) {
        s->input_buf[--s->input_pos] = 0;
        win->needs_redraw = 1;
    } else if (c == '\n') {
        chat_send_message(win);
    }
}

static void chat_on_mouse(void *w, int x, int y, int buttons) {
    window_t *win = (window_t *)w;
    chat_state_t *s = get_state(win);
    static int last_buttons = 0;
    int click = (buttons & 1) && !(last_buttons & 1);
    last_buttons = buttons;

    if (click) {
        int input_y = win->height - 40;
        // Send Button
        if (x >= win->width - 60 && x <= win->width - 10 && y >= input_y && y <= input_y + 30) {
            chat_send_message(win);
        }
        // Connect click (if disconnected)
        if (!s->connected && !s->connecting && x < SIDE_WIDTH) {
            s->connecting = 1;
            win->needs_redraw = 1;
        }
    }
}

window_t *chat_win = 0;

void chat_on_close(void *w) {
    window_t *win = (window_t *)w;
    chat_state_t *s = get_state(win);
    if (s->connected) {
        tcp_close(&s->conn);
    }
    kfree(s);
    chat_win = 0; // Clear global pointer so kernel stops calling update
}

void chat_init(void) {
    window_t *win = winmgr_create_window(-1, -1, 500, 400, "PureChat");
    if (!win)
        return;

    chat_state_t *s = (chat_state_t *)kmalloc(sizeof(chat_state_t));
    memset(s, 0, sizeof(chat_state_t));

    strcpy(s->my_username, "PureOS_User");
    strcpy(s->target_username, "Android_User");

    win->user_data = s;
    win->draw = (void (*)(void *))chat_draw;
    win->on_key = (void (*)(void *, int, char))chat_on_key;
    win->on_mouse = (void (*)(void *, int, int, int))chat_on_mouse;
    win->on_close = (void (*)(void *))chat_on_close;
    
    chat_win = win;
}
