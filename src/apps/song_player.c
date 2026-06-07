#include "../kernel/window.h"
#include "../kernel/string.h"
#include "../kernel/heap.h"
#include "../fs/fs.h"
#include "../drivers/mp3.h"
#include "../kernel/image.h"

// Colors matching the Spotify theme
#define COL_SP_BG           0xFF121212
#define COL_SP_SIDEBAR      0xFF000000
#define COL_SP_BOTTOM       0xFF181818
#define COL_SP_CARD         0xFF181818
#define COL_SP_CARD_HOVER   0xFF282828
#define COL_SP_TEXT_WHT     0xFFFFFFFF
#define COL_SP_TEXT_MUTED   0xFFAAAAAA
#define COL_SP_ACCENT       0xFF1DB954
#define COL_SP_GRAD_TOP     0xFF2A1C3D

typedef struct {
    char title[64];
    char filename_mp3[16];
    char filename_png[16];
    uint32_t *cover_img;
    int cover_w;
    int cover_h;
} song_entry_t;

#define MAX_SONGS 32

typedef struct {
    window_t *win;
    song_entry_t songs[MAX_SONGS];
    int song_count;
    
    int current_song_idx;
    int is_playing;
    
    int hover_idx; // for sidebar
    int hover_btn; // 1=prev, 2=play, 3=next
} song_app_t;

window_t *song_player_win = 0;

extern unsigned int get_timer_ticks(void);

void song_player_update(window_t *win) {
    song_app_t *app = (song_app_t *)win->user_data;
    if (!app) return;
    
    static unsigned int last_song_update = 0;
    unsigned int now = get_timer_ticks();
    
    // Update progress bar approx 25 times per second (every 10 ticks if 1 tick = ~4ms)
    if (app->is_playing && (now - last_song_update >= 10)) {
        last_song_update = now;
        win->needs_redraw = 1;
        extern int ui_dirty;
        ui_dirty = 1;
        int bw = win->width;
        int by = win->height - 90;
        winmgr_invalidate_rect(win, 0, by, bw, 90);
    }
}

// Helper to get app state
static song_app_t *get_app(window_t *win) {
    return (song_app_t *)win->user_data;
}

// Helper to read file entirely into memory
static uint8_t *song_read_file(const char *filename, int *out_size) {
    file_entry_t *f = fs_find(filename);
    if (!f) return NULL;
    uint8_t *buf = (uint8_t *)kmalloc(f->size);
    if (!buf) return NULL;
    int rs = fs_read(filename, buf);
    if (rs <= 0) { kfree(buf); return NULL; }
    *out_size = rs;
    return buf;
}

// Helper to load image
static uint32_t *load_cover_image(const char *filename, int *w, int *h) {
    int file_size = 0;
    uint8_t *file_data = song_read_file(filename, &file_size);
    if (!file_data) return NULL;
    
    int channels;
    uint8_t *img = stbi_load_from_memory(file_data, file_size, w, h, &channels, 4);
    kfree(file_data);
    
    if (!img) return NULL;
    
    int total_pixels = (*w) * (*h);
    uint32_t *argb = (uint32_t *)kmalloc(total_pixels * 4);
    for (int i = 0; i < total_pixels; i++) {
        uint8_t r = img[i*4 + 0];
        uint8_t g = img[i*4 + 1];
        uint8_t b = img[i*4 + 2];
        uint8_t a = img[i*4 + 3];
        argb[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    stbi_image_free(img);
    return argb;
}

static void song_stretch_blit(window_t *win, int dx, int dy, int dw, int dh, const uint32_t *src, int sw, int sh) {
    if (!src || dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0) return;
    for (int y = 0; y < dh; y++) {
        int sy = (y * sh) / dh;
        for (int x = 0; x < dw; x++) {
            int sx = (x * sw) / dw;
            uint32_t color = src[sy * sw + sx];
            uint8_t a = (color >> 24) & 0xFF;
            if (a == 255) {
                winmgr_put_pixel(win, dx + x, dy + y, color);
            }
        }
    }
}

static void draw_sidebar(window_t *win, song_app_t *app) {
    int sw = 250;
    int sh = win->height - 90; // Bottom bar is 90
    
    // Deep black sidebar with a slight transparency effect if supported, but here just black
    winmgr_fill_rect(win, 0, 0, sw, sh, COL_SP_SIDEBAR);
    
    // Top Nav with subtle highlighting
    winmgr_draw_text(win, 30, 30, "Home", COL_SP_TEXT_WHT);
    winmgr_draw_text(win, 30, 65, "Search", COL_SP_TEXT_MUTED);
    
    // Elegant divider
    winmgr_draw_rounded_rect_ex(win, 20, 110, sw - 40, 2, 0xFF282828, 0, 0, 1);
    
    winmgr_draw_text(win, 30, 130, "Your Library", COL_SP_TEXT_WHT);
    
    // Song list in sidebar (Scrollable looking)
    int y = 180;
    
    if (app->song_count == 0) {
         winmgr_draw_text(win, 30, y, "No songs found in OS.", COL_SP_TEXT_MUTED);
         winmgr_draw_text(win, 30, y + 20, "Add to Songs/ folder", COL_SP_TEXT_MUTED);
    }
    
    for (int i = 0; i < app->song_count; i++) {
        if (app->hover_idx == i) {
            winmgr_draw_rounded_rect_ex(win, 15, y - 10, sw - 30, 60, COL_SP_CARD_HOVER, 0, 0, 8);
        }
        
        if (app->songs[i].cover_img) {
            song_stretch_blit(win, 25, y, 40, 40, app->songs[i].cover_img, app->songs[i].cover_w, app->songs[i].cover_h);
        } else {
            winmgr_draw_rounded_rect_ex(win, 25, y, 40, 40, 0xFF333333, 0, 0, 4);
        }
        
        uint32_t t_color = (app->current_song_idx == i) ? COL_SP_ACCENT : COL_SP_TEXT_WHT;
        
        // Truncate title if too long
        char display_title[24];
        strncpy(display_title, app->songs[i].title, 20);
        display_title[20] = 0;
        if (strlen(app->songs[i].title) > 20) strcat(display_title, "...");
        
        winmgr_draw_text(win, 80, y + 5, display_title, t_color);
        winmgr_draw_text(win, 80, y + 25, "Song • PureOS", COL_SP_TEXT_MUTED);
        
        y += 65;
        if (y > sh - 60) break;
    }
}

static void draw_main_area(window_t *win, song_app_t *app) {
    int mx = 250;
    int mw = win->width - mx;
    int mh = win->height - 90;
    
    // 1. Draw premium gradient background
    // We will draw a series of horizontal rects to simulate a smooth deep purple-to-black fade
    for (int y = 0; y < 350; y += 5) {
        int alpha = 255 - (y * 255 / 350);
        // Base bg is 0x121212. Target is 0x210B3B (deep purple)
        int r = (0x21 * alpha + 0x12 * (255 - alpha)) / 255;
        int g = (0x0B * alpha + 0x12 * (255 - alpha)) / 255;
        int b = (0x3B * alpha + 0x12 * (255 - alpha)) / 255;
        uint32_t c = 0xFF000000 | (r << 16) | (g << 8) | b;
        winmgr_fill_rect(win, mx, y, mw, 5, c);
    }
    winmgr_fill_rect(win, mx, 350, mw, mh - 350, COL_SP_BG);
    
    // Greeting
    winmgr_draw_text(win, mx + 40, 40, "Good evening", COL_SP_TEXT_WHT);
    
    // Top 6 recent items
    // If we have songs, we use them, otherwise we use beautiful placeholder data
    const char* mock_mixes[] = {"Pop Mix", "Chill Vibes", "Daily Mix 1", "2010s Hits", "Lofi Beats", "Discover Weekly"};
    uint32_t mock_colors[] = {0xFFE91E63, 0xFF4FC3F7, 0xFFFFC107, 0xFF9C27B0, 0xFF00BCD4, 0xFF4CAF50};
    
    for (int i = 0; i < 6; i++) {
        int r = i / 2;
        int c = i % 2;
        int cx = mx + 40 + c * 340;
        int cy = 90 + r * 80;
        
        winmgr_draw_rounded_rect_ex(win, cx, cy, 310, 64, 0xFF282828, 0, 0, 6); // Card background
        
        // Use actual song data if available
        if (i < app->song_count) {
             if (app->songs[i].cover_img) {
                 // Thumbnail
                 song_stretch_blit(win, cx, cy, 64, 64, app->songs[i].cover_img, app->songs[i].cover_w, app->songs[i].cover_h);
             } else {
                 winmgr_draw_rounded_rect_ex(win, cx, cy, 64, 64, mock_colors[i], 0, 0, 6);
                 winmgr_fill_rect(win, cx + 58, cy, 6, 64, mock_colors[i]); // Straighten right edge
             }
             
             char disp[32];
             strncpy(disp, app->songs[i].title, 28); disp[28] = 0;
             winmgr_draw_text(win, cx + 80, cy + 24, disp, COL_SP_TEXT_WHT);
        } else {
             // Mock Data
             winmgr_draw_rounded_rect_ex(win, cx, cy, 64, 64, mock_colors[i], 0, 0, 6);
             winmgr_fill_rect(win, cx + 58, cy, 6, 64, mock_colors[i]); // Straighten right edge
             winmgr_draw_text(win, cx + 80, cy + 24, mock_mixes[i], COL_SP_TEXT_WHT);
        }
    }
    
    // "Made for You" or "Your Songs"
    winmgr_draw_text(win, mx + 40, 360, (app->song_count > 0) ? "Your Songs" : "Made for You", COL_SP_TEXT_WHT);
    
    int list_count = (app->song_count > 0) ? app->song_count : 5;
    for (int i = 0; i < list_count && i < 5; i++) {
        int cx = mx + 40 + i * 170;
        int cy = 410;
        
        winmgr_draw_rounded_rect_ex(win, cx, cy, 150, 220, COL_SP_CARD, 0, 0, 8);
        
        if (i < app->song_count) {
            if (app->songs[i].cover_img) {
                 song_stretch_blit(win, cx + 15, cy + 15, 120, 120, app->songs[i].cover_img, app->songs[i].cover_w, app->songs[i].cover_h);
            } else {
                 winmgr_draw_rounded_rect_ex(win, cx + 15, cy + 15, 120, 120, mock_colors[i], 0, 0, 8);
            }
            char disp[20];
            strncpy(disp, app->songs[i].title, 16); disp[16] = 0;
            winmgr_draw_text(win, cx + 15, cy + 155, disp, COL_SP_TEXT_WHT);
            winmgr_draw_text(win, cx + 15, cy + 180, "PureOS Audio", COL_SP_TEXT_MUTED);
        } else {
            winmgr_draw_rounded_rect_ex(win, cx + 15, cy + 15, 120, 120, mock_colors[i], 0, 0, 8);
            winmgr_draw_text(win, cx + 15, cy + 155, mock_mixes[i], COL_SP_TEXT_WHT);
            winmgr_draw_text(win, cx + 15, cy + 180, "Daily Mix", COL_SP_TEXT_MUTED);
        }
    }
}

static void draw_bottom_bar(window_t *win, song_app_t *app) {
    int bw = win->width;
    int by = win->height - 90;
    
    winmgr_fill_rect(win, 0, by, bw, 90, COL_SP_BOTTOM);
    winmgr_fill_rect(win, 0, by, bw, 1, 0xFF282828); // Top border
    
    // Now Playing Left Side
    if (app->current_song_idx >= 0 && app->current_song_idx < app->song_count) {
        song_entry_t *s = &app->songs[app->current_song_idx];
        if (s->cover_img) {
            song_stretch_blit(win, 30, by + 15, 60, 60, s->cover_img, s->cover_w, s->cover_h);
        } else {
            winmgr_draw_rounded_rect_ex(win, 30, by + 15, 60, 60, 0xFF333333, 0, 0, 4);
        }
        
        char disp[30];
        strncpy(disp, s->title, 26); disp[26] = 0;
        winmgr_draw_text(win, 110, by + 25, disp, COL_SP_TEXT_WHT);
        winmgr_draw_text(win, 110, by + 45, "PureOS Audio", COL_SP_TEXT_MUTED);
    }
    
    // Controls Center
    int cx = bw / 2;
    int cy = by + 25;
    
    // Prev
    winmgr_draw_text(win, cx - 60, cy, "|<", (app->hover_btn == 1) ? COL_SP_TEXT_WHT : COL_SP_TEXT_MUTED);
    
    // Play/Pause
    winmgr_draw_rounded_rect_ex(win, cx - 18, cy - 8, 36, 36, COL_SP_TEXT_WHT, 0, 0, 18); // Circle
    if (app->is_playing) {
        winmgr_fill_rect(win, cx - 6, cy + 2, 4, 12, COL_SP_SIDEBAR);
        winmgr_fill_rect(win, cx + 2, cy + 2, 4, 12, COL_SP_SIDEBAR);
    } else {
        winmgr_draw_text(win, cx - 5, cy + 2, ">", COL_SP_SIDEBAR);
    }
    
    // Next
    winmgr_draw_text(win, cx + 50, cy, ">|", (app->hover_btn == 3) ? COL_SP_TEXT_WHT : COL_SP_TEXT_MUTED);
    
    // Progress Bar
    uint32_t pos_ms = 0;
    uint32_t dur_ms = 0;
    
    // Always get progress so we show the time even when paused
    mp3_get_progress(&pos_ms, &dur_ms);
    
    if (app->is_playing) {
        // Check if playback has finished
        extern int ac97_is_playback_done(void);
        if (dur_ms > 0 && pos_ms >= dur_ms) {
            app->is_playing = 0;
        } else if (ac97_is_playback_done() && dur_ms > 0) {
            app->is_playing = 0;
        }
    }
    
    char pos_str[16] = "0:00";
    char dur_str[16] = "0:00";
    
    if (dur_ms > 0) {
        int pm = (pos_ms / 1000) / 60;
        int ps = (pos_ms / 1000) % 60;
        int dm = (dur_ms / 1000) / 60;
        int ds = (dur_ms / 1000) % 60;
        
        pos_str[0] = '0' + (pm % 10);
        pos_str[1] = ':';
        pos_str[2] = '0' + (ps / 10);
        pos_str[3] = '0' + (ps % 10);
        pos_str[4] = 0;
        
        dur_str[0] = '0' + (dm % 10);
        dur_str[1] = ':';
        dur_str[2] = '0' + (ds / 10);
        dur_str[3] = '0' + (ds % 10);
        dur_str[4] = 0;
    }
    
    winmgr_draw_text(win, cx - 250, by + 65, pos_str, COL_SP_TEXT_MUTED);
    winmgr_draw_rounded_rect_ex(win, cx - 200, by + 70, 400, 6, 0xFF3E3E3E, 0, 0, 3);
    
    
    if (dur_ms > 0) {
        int w = (pos_ms * 400) / dur_ms;
        if (w > 400) w = 400;
        if (w < 0) w = 0;
        winmgr_draw_rounded_rect_ex(win, cx - 200, by + 70, w, 6, COL_SP_TEXT_WHT, 0, 0, 3);
        winmgr_draw_rounded_rect_ex(win, cx - 200 + w - 6, by + 67, 12, 12, COL_SP_TEXT_WHT, 0, 0, 6); // thumb
    }
    
    winmgr_draw_text(win, cx + 220, by + 65, dur_str, COL_SP_TEXT_MUTED);
}

void song_draw(window_t *win) {
    song_app_t *app = get_app(win);
    if (!app) return;
    
    draw_sidebar(win, app);
    draw_main_area(win, app);
    draw_bottom_bar(win, app);
}

void song_handle_mouse(window_t *win, int mx, int my, int buttons) {
    song_app_t *app = get_app(win);
    
    int new_hover_idx = -1;
    int new_hover_btn = 0;
    
    // Sidebar list check
    if (mx >= 10 && mx <= 240 && my >= 180 && my <= win->height - 90) {
        new_hover_idx = (my - 180) / 65;
        if (new_hover_idx >= app->song_count) new_hover_idx = -1;
    }
    
    // Main Area Card check
    if (mx >= 280 && mx <= 430 && my >= 360 && my <= 530) {
        if (app->song_count > 0) new_hover_idx = 0; // The first card
    }
    
    // Bottom bar controls
    int bw = win->width;
    int by = win->height - 90;
    int cx = bw / 2;
    int cy = by + 25;
    
    if (my >= by && my <= by + 60) {
        if (mx >= cx - 50 && mx <= cx - 20) new_hover_btn = 1; // Prev
        else if (mx >= cx - 15 && mx <= cx + 15) new_hover_btn = 2; // Play
        else if (mx >= cx + 20 && mx <= cx + 50) new_hover_btn = 3; // Next
    }
    
    if (new_hover_idx != app->hover_idx || new_hover_btn != app->hover_btn) {
        app->hover_idx = new_hover_idx;
        app->hover_btn = new_hover_btn;
        winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
    }
    
    if (buttons & 1) { // Left click
        if (app->hover_idx != -1) {
            app->current_song_idx = app->hover_idx;
            app->is_playing = 1;
            
            // Trigger playback
            int fsize = 0;
            uint8_t *fdata = song_read_file(app->songs[app->current_song_idx].filename_mp3, &fsize);
            if (fdata) {
                mp3_play(fdata, fsize);
                kfree(fdata);
            }
            
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        } else if (app->hover_btn == 2) {
            app->is_playing = !app->is_playing;
            if (app->is_playing) {
                // Replay
                int fsize = 0;
                uint8_t *fdata = song_read_file(app->songs[app->current_song_idx].filename_mp3, &fsize);
                if (fdata) {
                    mp3_play(fdata, fsize);
                    kfree(fdata);
                }
            } else {
                mp3_stop();
            }
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        } else if (app->hover_btn == 1) {
            if (app->current_song_idx > 0) app->current_song_idx--;
            app->is_playing = 1;
            int fsize = 0;
            uint8_t *fdata = song_read_file(app->songs[app->current_song_idx].filename_mp3, &fsize);
            if (fdata) {
                mp3_play(fdata, fsize);
                kfree(fdata);
            }
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        } else if (app->hover_btn == 3) {
            if (app->current_song_idx < app->song_count - 1) app->current_song_idx++;
            app->is_playing = 1;
            int fsize = 0;
            uint8_t *fdata = song_read_file(app->songs[app->current_song_idx].filename_mp3, &fsize);
            if (fdata) {
                mp3_play(fdata, fsize);
                kfree(fdata);
            }
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        }
    }
}

static void song_on_close(void *w) {
    window_t *win = (window_t *)w;
    if (song_player_win == win) song_player_win = 0; // Prevent background updates before freeing
    song_app_t *app = get_app(win);
    mp3_stop();
    for (int i = 0; i < app->song_count; i++) {
        if (app->songs[i].cover_img) {
            kfree(app->songs[i].cover_img);
        }
    }
    kfree(app);
    winmgr_close_window(win);
}

void song_app_init() {
    window_t *win = winmgr_create_window(-1, -1, 1024, 768, "Music");
    if (!win) return;
    
    song_app_t *app = (song_app_t *)kmalloc(sizeof(song_app_t));
    memset(app, 0, sizeof(song_app_t));
    
    app->win = win;
    app->current_song_idx = -1;
    app->hover_idx = -1;
    win->user_data = app;
    win->flags |= WINDOW_FLAG_NO_TITLEBAR;
    song_player_win = win;
    
    win->draw = (void (*)(void *))song_draw;
    win->on_mouse = (void (*)(void *, int, int, int))song_handle_mouse;
    win->on_close = song_on_close;
    
    // Scan for MP3s in root directory
    FileInfo list[64];
    int count = 0;
    fs_list_files("/", list, 64);
    
    for (int i = 0; i < 64 && count < MAX_SONGS; i++) {
        if (list[i].name[0] == 0) continue;
        if (strstr(list[i].name, ".MP3") || strstr(list[i].name, ".mp3")) {
            // Copy filename
            strcpy(app->songs[count].filename_mp3, list[i].name);
            
            // Derive title
            strcpy(app->songs[count].title, list[i].name);
            char *dot = strstr(app->songs[count].title, ".");
            if (dot) *dot = 0; // Strip extension
            
            // Derive cover filename
            strcpy(app->songs[count].filename_png, app->songs[count].title);
            strcat(app->songs[count].filename_png, ".PNG");
            
            // Load cover if exists
            app->songs[count].cover_img = load_cover_image(app->songs[count].filename_png, &app->songs[count].cover_w, &app->songs[count].cover_h);
            
            count++;
        }
    }
    app->song_count = count;
    
    winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
}
