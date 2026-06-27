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
    
    // Animations
    animation_t progress_anim;
    animation_t thumb_scale_anim;
    
    int show_now_playing;
    animation_t now_playing_anim;
    
    char search_query[64];
    int search_cursor;
    int is_searching;
    int search_focused;
    int yt_fetching;
    int yt_loading_audio;
    
    char lyrics_text[8192];
    int lyrics_loaded; // 0=none, 1=loading, 2=loaded, -1=failed
    float lyrics_scroll_y;
    
    float lyrics_line_scales[512]; // Per-line smooth animation state
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
        if (app->now_playing_anim.current_val > 0.001f) {
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        } else {
            int bw = win->width;
            int by = win->height - 90;
            winmgr_invalidate_rect(win, 0, by, bw, 90);
        }
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

static void load_local_songs(song_app_t *app) {
    app->song_count = 0;
    FileInfo list[64];
    fs_list_files("/", list, 64);
    for (int i = 0; i < 64 && app->song_count < MAX_SONGS; i++) {
        if (list[i].name[0] == 0) continue;
        if (strstr(list[i].name, ".MP3") || strstr(list[i].name, ".mp3")) {
            strcpy(app->songs[app->song_count].filename_mp3, list[i].name);
            strcpy(app->songs[app->song_count].title, list[i].name);
            char *dot = strstr(app->songs[app->song_count].title, ".");
            if (dot) *dot = 0;
            strcpy(app->songs[app->song_count].filename_png, app->songs[app->song_count].title);
            strcat(app->songs[app->song_count].filename_png, ".PNG");
            app->songs[app->song_count].cover_img = load_cover_image(app->songs[app->song_count].filename_png, &app->songs[app->song_count].cover_w, &app->songs[app->song_count].cover_h);
            app->song_count++;
        }
    }
}

static char search_query_global[64];
static void yt_search_thread(void) {
    char url[256];
    strcpy(url, "http://10.0.2.2:7862/ytsearch?q=");
    int j = strlen(url);
    for (int i = 0; search_query_global[i] && j < 255; i++) {
        if (search_query_global[i] == ' ') url[j++] = '+';
        else url[j++] = search_query_global[i];
    }
    url[j] = 0;
    
    char *res = kmalloc(32768);
    if (res && song_player_win) {
        extern int http_get(const char*, char*, int);
        int len = http_get(url, res, 32767);
        if (len > 0) {
            res[len] = 0;
            song_app_t *app = get_app(song_player_win);
            if (app) {
                app->song_count = 0;
                char *line = res;
                while (line && *line && app->song_count < MAX_SONGS) {
                    char *next = strchr(line, '\n');
                    if (next) *next = 0;
                    char *sep = strchr(line, '|');
                    if (sep) {
                        *sep = 0;
                        strncpy(app->songs[app->song_count].title, line, 63);
                        app->songs[app->song_count].title[63] = 0;
                        strcpy(app->songs[app->song_count].filename_mp3, "yt:");
                        strncat(app->songs[app->song_count].filename_mp3, sep + 1, 12);
                        app->songs[app->song_count].filename_mp3[15] = 0;
                        app->songs[app->song_count].cover_img = 0;
                        app->song_count++;
                    }
                    if (next) line = next + 1;
                    else break;
                }
                app->yt_fetching = 0;
                song_player_win->needs_redraw = 1;
                extern int ui_dirty; ui_dirty = 1;

                // Download thumbnails progressively
                for (int i = 0; i < app->song_count; i++) {
                    if (strncmp(app->songs[i].filename_mp3, "yt:", 3) == 0 && !app->songs[i].cover_img) {
                        char thumb_url[256];
                        strcpy(thumb_url, "http://10.0.2.2:7862/ytthumb?id=");
                        strncat(thumb_url, app->songs[i].filename_mp3 + 3, 20);
                        
                        char *tdata = kmalloc(150000); // 150KB should be plenty for mqdefault.jpg
                        if (tdata) {
                            int tlen = http_get(thumb_url, tdata, 149999);
                            if (tlen > 0) {
                                int w, h, channels;
                                uint8_t *img = stbi_load_from_memory((uint8_t*)tdata, tlen, &w, &h, &channels, 4);
                                if (img) {
                                    int total_pixels = w * h;
                                    uint32_t *argb = (uint32_t *)kmalloc(total_pixels * 4);
                                    if (argb) {
                                        for (int p = 0; p < total_pixels; p++) {
                                            uint8_t r = img[p*4 + 0];
                                            uint8_t g = img[p*4 + 1];
                                            uint8_t b = img[p*4 + 2];
                                            uint8_t a = img[p*4 + 3];
                                            argb[p] = (a << 24) | (r << 16) | (g << 8) | b;
                                        }
                                        app->songs[i].cover_img = argb;
                                        app->songs[i].cover_w = w;
                                        app->songs[i].cover_h = h;
                                        song_player_win->needs_redraw = 1;
                                        ui_dirty = 1;
                                    }
                                    stbi_image_free(img);
                                }
                            }
                            kfree(tdata);
                        }
                    }
                }
            }
        }
        kfree(res);
    }
    if (song_player_win) {
        song_app_t *app = get_app(song_player_win);
        if (app) app->yt_fetching = 0;
    }
    extern void exit(int status);
    exit(0);
}

static char lyrics_fetch_title[128];
static void lyrics_fetch_thread(void) {
    if (song_player_win) {
        song_app_t *app = (song_app_t *)song_player_win->user_data;
        if (app) app->lyrics_loaded = 1;
        song_player_win->needs_redraw = 1;
        extern int ui_dirty; ui_dirty = 1;
    }

    char url[512];
    strcpy(url, "http://10.0.2.2:7862/ytlyrics?q=");
    int j = strlen(url);
    for (int i = 0; lyrics_fetch_title[i] && j < 511 - 3; i++) {
        unsigned char c = lyrics_fetch_title[i];
        if (c == ' ') {
            url[j++] = '+';
        } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            url[j++] = c;
        } else {
            const char *hex = "0123456789ABCDEF";
            url[j++] = '%';
            url[j++] = hex[c >> 4];
            url[j++] = hex[c & 15];
        }
    }
    url[j] = 0;
    
    char *ldata = kmalloc(8192);
    if (ldata) {
        extern int http_get(const char*, char*, int);
        int len = http_get(url, ldata, 8191);
        if (song_player_win) {
            song_app_t *app = (song_app_t *)song_player_win->user_data;
            if (app) {
                if (len > 0) {
                    ldata[len] = 0;
                    strncpy(app->lyrics_text, ldata, 8191);
                    app->lyrics_text[8191] = 0;
                    app->lyrics_loaded = 2;
                } else {
                    app->lyrics_loaded = -1;
                }
                app->lyrics_scroll_y = 0;
                song_player_win->needs_redraw = 1;
                extern int ui_dirty; ui_dirty = 1;
            }
        }
        kfree(ldata);
    }
    extern void exit(int status);
    exit(0);
}

static char yt_fetch_vid[64];
static void yt_play_thread(void) {
    if (song_player_win) {
        song_app_t *app = (song_app_t *)song_player_win->user_data;
        if (app) app->yt_loading_audio = 1;
        song_player_win->needs_redraw = 1;
        extern int ui_dirty; ui_dirty = 1;
    }

    char url[256];
    strcpy(url, "http://10.0.2.2:7862/ytplay?id=");
    strcat(url, yt_fetch_vid);
    int max_size = 5000000;
    uint8_t *fdata = kmalloc(max_size);
    if (fdata) {
        extern int http_get(const char*, char*, int);
        int rs = http_get(url, (char*)fdata, max_size);
        if (rs > 0) {
            mp3_play(fdata, rs);
        }
        kfree(fdata);
    }
    
    if (song_player_win) {
        song_app_t *app = (song_app_t *)song_player_win->user_data;
        if (app) app->yt_loading_audio = 0;
        song_player_win->needs_redraw = 1;
        extern int ui_dirty; ui_dirty = 1;
    }
    
    extern void exit(int status);
    exit(0);
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
    winmgr_draw_text(win, 30, 30, "Home", (app->is_searching == 0) ? COL_SP_TEXT_WHT : COL_SP_TEXT_MUTED);
    winmgr_draw_text(win, 30, 65, "Search", (app->is_searching == 1) ? COL_SP_TEXT_WHT : COL_SP_TEXT_MUTED);
    
    // Elegant divider
    winmgr_draw_rounded_rect_ex(win, 20, 110, sw - 40, 2, 0xFF282828, 0, 0, 1);
    
    winmgr_draw_text(win, 30, 130, "Your Library", COL_SP_TEXT_WHT);
    
    // Song list in sidebar (Scrollable looking)
    int y = 180;
    
    if (app->song_count == 0 && app->is_searching == 0) {
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
        winmgr_draw_text(win, 80, y + 25, strncmp(app->songs[i].filename_mp3, "yt:", 3) == 0 ? "YT Music" : "Song • PureOS", COL_SP_TEXT_MUTED);
        
        y += 65;
        if (y > sh - 60) break;
    }
}

static int parse_lrc_time(const char *line, char **out_text) {
    if (line[0] == '[') {
        char *end = strchr(line, ']');
        if (end) {
            *out_text = end + 1;
            if (**out_text == ' ') (*out_text)++;
            if (line[3] == ':') {
                int m = (line[1] - '0') * 10 + (line[2] - '0');
                int s = (line[4] - '0') * 10 + (line[5] - '0');
                return m * 60 + s;
            }
            return -1; // metadata tag
        }
    }
    *out_text = (char *)line;
    return -1;
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
    
    // Draw Permanent Search Bar at top center
    int search_w = 400;
    int search_x = (win->width - search_w) / 2;
    int search_y = 20;
    
    winmgr_draw_rounded_rect_ex(win, search_x, search_y, search_w, 40, 0xFF333333, 0, 0, 20);
    winmgr_draw_text(win, search_x + 20, search_y + 12, app->search_query, COL_SP_TEXT_WHT);
    if (app->search_focused) {
        int q_len = strlen(app->search_query);
        winmgr_draw_rect(win, search_x + 20 + q_len * 8, search_y + 10, 2, 20, COL_SP_TEXT_WHT);
    }
    if (app->yt_fetching) {
         winmgr_draw_text(win, search_x + search_w + 20, search_y + 12, "Searching...", COL_SP_TEXT_MUTED);
    }
    if (strlen(app->search_query) == 0 && !app->search_focused) {
         winmgr_draw_text(win, search_x + 20, search_y + 12, "Search YouTube Music", COL_SP_TEXT_MUTED);
    }

    if (!app->is_searching) {
        if (app->is_playing || app->current_song_idx != -1) {
            // Draw lyrics
            if (app->lyrics_loaded == 2) {
                int max_y = win->height - 100;
                
                // Pass 1: Find active line index
                extern void mp3_get_progress(uint32_t*, uint32_t*);
                uint32_t pos_ms = 0, dur_ms = 0;
                mp3_get_progress(&pos_ms, &dur_ms);
                int current_time = pos_ms / 1000;
                int active_line_idx = -1;
                int current_idx = 0;
                
                char *line = app->lyrics_text;
                while (line && *line) {
                    char *next = strchr(line, '\n');
                    if (next) *next = 0;
                    
                    char *text_content;
                    int t = parse_lrc_time(line, &text_content);
                    if (t != -1 && t <= current_time) {
                        active_line_idx = current_idx;
                    }
                    
                    if (strlen(text_content) > 0) {
                        current_idx++;
                    }
                    
                    if (next) { *next = '\n'; line = next + 1; }
                    else { break; }
                }
                
                // Target scroll calculation
                if (active_line_idx != -1) {
                    float target_scroll = -(active_line_idx * 60 - (max_y - 100) / 2);
                    if (target_scroll > 0) target_scroll = 0;
                    app->lyrics_scroll_y += (target_scroll - app->lyrics_scroll_y) * 0.1f;
                    winmgr_invalidate_rect(win, 250, 80, win->width - 250, win->height - 180);
                }
                
                // Pass 2: Draw lines
                int start_y = 100 + (int)app->lyrics_scroll_y;
                int current_y = start_y;
                current_idx = 0;
                
                extern int ttf_get_default_font(void);
                extern void ttf_set_size(int, float);
                extern int ttf_text_width(int, const char *);
                extern void ttf_draw_text(void *, int, int, int, const char *, uint32_t);
                int fs = ttf_get_default_font();
                
                line = app->lyrics_text;
                while (line && *line) {
                    char *next = strchr(line, '\n');
                    if (next) *next = 0;
                    
                    char *text_content;
                    parse_lrc_time(line, &text_content);
                    
                    if (strlen(text_content) > 0) {
                        if (current_y > 40 && current_y < max_y + 60) {
                            int is_active = (current_idx == active_line_idx);
                            float target_scale = is_active ? 1.0f : 0.0f;
                            
                            if (current_idx < 512) {
                                app->lyrics_line_scales[current_idx] += (target_scale - app->lyrics_line_scales[current_idx]) * 0.15f; // Fast, smooth interpolation
                            }
                            
                            float scale_factor = (current_idx < 512) ? app->lyrics_line_scales[current_idx] : target_scale;
                            
                            int r = 179 + (int)((255 - 179) * scale_factor);
                            int g = 179 + (int)((255 - 179) * scale_factor);
                            int b = 179 + (int)((255 - 179) * scale_factor);
                            uint32_t color = 0xFF000000 | (r << 16) | (g << 8) | b;
                            
                            if (fs != -1) {
                                int scale = 22 + (int)(18.0f * scale_factor); // 22 to 40
                                ttf_set_size(fs, scale);
                                int tw = ttf_text_width(fs, text_content);
                                int text_x = mx + (mw / 2) - (tw / 2);
                                
                                // Draw text sharply. If it's the active line (scale_factor > 0.5), fake a BOLD weight 
                                // to make it look incredibly solid and crystal clear instead of soft/blurry.
                                ttf_draw_text(win, fs, text_x, current_y, text_content, color);
                                if (scale_factor > 0.5f) {
                                    ttf_draw_text(win, fs, text_x + 1, current_y, text_content, color);
                                }
                            } else {
                                extern int ui_measure_text_width(const char*, int);
                                extern void ui_draw_text_scaled(window_t *win, int x, int y, const char *text, uint32_t color, int font_scale);
                                int scale = 16 + (int)(8.0f * scale_factor);
                                int tw = ui_measure_text_width(text_content, scale);
                                int text_x = mx + (mw / 2) - (tw / 2);
                                ui_draw_text_scaled(win, text_x, current_y, text_content, color, scale);
                            }
                        }
                        current_y += 60;
                        current_idx++;
                    }
                    
                    if (next) {
                        *next = '\n';
                        line = next + 1;
                    } else {
                        break;
                    }
                }
            } else if (app->lyrics_loaded == 1) {
                winmgr_draw_text(win, mx + 150, 200, "Loading Lyrics...", COL_SP_TEXT_MUTED);
            } else if (app->lyrics_loaded == -1) {
                winmgr_draw_text(win, mx + 150, 200, "Lyrics not found for this song.", COL_SP_TEXT_MUTED);
            }
        } else {
            // Greeting
            winmgr_draw_text(win, mx + 40, 80, "Good evening", COL_SP_TEXT_WHT);
        
        // Top 6 recent items
        const char* mock_mixes[] = {"Pop Mix", "Chill Vibes", "Daily Mix 1", "2010s Hits", "Lofi Beats", "Discover Weekly"};
        uint32_t mock_colors[] = {0xFFE91E63, 0xFF4FC3F7, 0xFFFFC107, 0xFF9C27B0, 0xFF00BCD4, 0xFF4CAF50};
        
        for (int i = 0; i < 6; i++) {
            int r = i / 2;
            int c = i % 2;
            int cx = mx + 40 + c * 340;
            int cy = 130 + r * 80;
            
            winmgr_draw_rounded_rect_ex(win, cx, cy, 310, 64, 0xFF282828, 0, 0, 6); // Card background
            
            if (i < app->song_count) {
                 if (app->songs[i].cover_img) {
                     song_stretch_blit(win, cx, cy, 64, 64, app->songs[i].cover_img, app->songs[i].cover_w, app->songs[i].cover_h);
                 } else {
                     winmgr_draw_rounded_rect_ex(win, cx, cy, 64, 64, mock_colors[i], 0, 0, 6);
                     winmgr_fill_rect(win, cx + 58, cy, 6, 64, mock_colors[i]);
                 }
                 char disp[32];
                 strncpy(disp, app->songs[i].title, 28); disp[28] = 0;
                 winmgr_draw_text(win, cx + 80, cy + 24, disp, COL_SP_TEXT_WHT);
            } else {
                 winmgr_draw_rounded_rect_ex(win, cx, cy, 64, 64, mock_colors[i], 0, 0, 6);
                 winmgr_fill_rect(win, cx + 58, cy, 6, 64, mock_colors[i]);
                 winmgr_draw_text(win, cx + 80, cy + 24, mock_mixes[i], COL_SP_TEXT_WHT);
            }
        }
        
        winmgr_draw_text(win, mx + 40, 400, (app->song_count > 0) ? "Your Songs" : "Made for You", COL_SP_TEXT_WHT);
        
        int list_count = (app->song_count > 0) ? app->song_count : 5;
        for (int i = 0; i < list_count && i < 5; i++) {
            int cx = mx + 40 + i * 170;
            int cy = 450;
            
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
        } // Close the else block
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
    
    if (app->yt_loading_audio) {
        // Draw moving gray striped lines on the seek bar
        unsigned int ticks = get_timer_ticks();
        int offset = (ticks / 2) % 20;
        for (int i = -20; i < 400; i += 20) {
            int draw_x = cx - 200 + i + offset;
            int draw_w = 10;
            if (draw_x >= cx + 200) continue;
            if (draw_x + draw_w > cx + 200) draw_w = (cx + 200) - draw_x;
            if (draw_x < cx - 200) {
                int shrink = (cx - 200) - draw_x;
                if (shrink >= draw_w) continue;
                draw_w -= shrink;
                draw_x = cx - 200;
            }
            if (draw_w > 0) {
                winmgr_draw_rounded_rect_ex(win, draw_x, by + 70, draw_w, 6, 0xFFAAAAAA, 0, 0, 0); 
            }
        }
        win->needs_redraw = 1;
        extern int ui_dirty; ui_dirty = 1;
    } else if (dur_ms > 0) {
        int target_w = (pos_ms * 400) / dur_ms;
        if (target_w > 400) target_w = 400;
        if (target_w < 0) target_w = 0;
        
        // Smoothly interpolate the progress bar
        int diff = target_w - (int)app->progress_anim.current_val;
        if (diff < 0) diff = -diff;
        
        if (!app->progress_anim.active || (int)app->progress_anim.end_val != target_w) {
            if (diff > 50) {
                // If it's a huge jump (e.g. song change or scrub), jump instantly
                anim_init_val(&app->progress_anim, target_w);
            } else {
                // Otherwise glide smoothly to the new second
                anim_start(&app->progress_anim, app->progress_anim.current_val, target_w, 0.5f, EASE_OUT_CUBIC);
            }
        }
        
        // Tick animation (approx 25fps based on song_player_update frequency)
        anim_tick(&app->progress_anim, 0.04f); 
        anim_tick(&app->thumb_scale_anim, 0.04f);
        if (app->thumb_scale_anim.active) {
            win->needs_redraw = 1;
            extern int ui_dirty;
            ui_dirty = 1;
        }
        
        int w = (int)app->progress_anim.current_val;
        
        float pct = (float)w / 400.0f;
        if (pct < 0.0f) pct = 0.0f;
        if (pct > 1.0f) pct = 1.0f;
        
        // Rapid RGB color cycle for the progress bar
        static float color_t = 0.0f;
        if (app->is_playing) {
            color_t += 0.25f; // Fast RGB color cycling speed
        }
        
        extern double sin(double);
        // Calculate RGB using sine waves offset by 120 degrees (2pi/3) to create a perfect rainbow
        int r = (int)((sin(color_t) * 0.5f + 0.5f) * 200 + 55);
        int g = (int)((sin(color_t + 2.094f) * 0.5f + 0.5f) * 200 + 55); 
        int b = (int)((sin(color_t + 4.188f) * 0.5f + 0.5f) * 200 + 55); 
        uint32_t fill_color = 0xFF000000 | (r << 16) | (g << 8) | b;
        
        winmgr_draw_rounded_rect_ex(win, cx - 200, by + 70, w, 6, fill_color, 0, 0, 3);
        
        // Add wavy sound track effect — separated vertical bars ("boxy waves") forming a smooth curve
        if (app->is_playing) {
            static float wave_t = 0.0f;
            wave_t += 0.08f; // Animation speed
            
            extern double sin(double);
            int base_y = by + 70; // Set exactly on top of the progress bar
            
            // Draw separated vertical bars (width 2, gap 2)
            for (int ix = 4; ix < w - 4; ix += 4) {
                // Calculate a smooth curvy height (sine wave) instead of erratic jumping
                float curvy = sin(wave_t * 2.0f + ix * 0.05f) * sin(wave_t * 0.8f + ix * 0.02f);
                float env = 1.0f + 0.3f * sin(wave_t * 0.5f);
                
                int h_bar = (int)(curvy * env * 12.0f); // Amplitude up to 15px
                if (h_bar < 0) h_bar = -h_bar; // Make it bounce up smoothly
                if (h_bar < 1) h_bar = 1;      // Minimum height
                
                int draw_y = base_y - h_bar;
                
                // Draw the separated vertical bar sitting ON TOP of the progress bar
                winmgr_fill_rect(win, cx - 200 + ix, draw_y, 2, h_bar, 0xFFFFFFFF); 
            }
            
            // Force continuous redraw while playing
            win->needs_redraw = 1;
            extern int ui_dirty;
            ui_dirty = 1;
        }
        float thumb_scale = app->thumb_scale_anim.current_val;
        if (thumb_scale < 0.1f) thumb_scale = 1.0f; // safety
        
        int tw = (int)(12 * thumb_scale);
        int th = (int)(12 * thumb_scale);
        int tx = cx - 200 + w - (tw / 2);
        int ty = by + 73 - (th / 2);
        
        winmgr_draw_rounded_rect_ex(win, tx, ty, tw, th, COL_SP_TEXT_WHT, 0, 0, tw/2); // thumb
    }
    
    winmgr_draw_text(win, cx + 220, by + 65, dur_str, COL_SP_TEXT_MUTED);
}

static void draw_now_playing(window_t *win, song_app_t *app) {
    float t = app->now_playing_anim.current_val; // 0.0 to 1.0
    if (t <= 0.001f) return;
    
    int bw = win->width;
    int bh = win->height;
    
    // Slide up from bottom
    int offset_y = (int)((1.0f - t) * bh);
    
    for (int y = 0; y < bh; y += 5) {
        if (y + offset_y >= bh) break;
        int alpha = 255 - (y * 255 / bh);
        int r = (0x33 * alpha + 0x12 * (255 - alpha)) / 255;
        int g = (0x22 * alpha + 0x12 * (255 - alpha)) / 255;
        int b = (0x44 * alpha + 0x12 * (255 - alpha)) / 255;
        uint32_t c = 0xFF000000 | (r << 16) | (g << 8) | b;
        winmgr_fill_rect(win, 0, y + offset_y, bw, 5, c);
    }
    
    winmgr_draw_text(win, 30, 30 + offset_y, "v  Back", COL_SP_TEXT_WHT);
    winmgr_draw_text(win, bw/2 - 50, 30 + offset_y, "NOW PLAYING", COL_SP_TEXT_MUTED);
    
    if (app->current_song_idx >= 0 && app->current_song_idx < app->song_count) {
        song_entry_t *s = &app->songs[app->current_song_idx];
        
        int by = bh - 90;
        
        // Initial coords
        int start_cv_size = 60;
        int start_cv_x = 30;
        int start_cv_y = by + 15;
        
        int start_px = bw/2 - 200;
        int start_py = by + 70;
        int start_p_width = 400;
        
        int start_cx_btn = bw/2;
        int start_ctrl_y = by + 25;
        
        // Target coords
        int target_cv_size = 320;
        int target_cv_x = bw/2 - target_cv_size/2;
        int target_cv_y = 120;
        
        int target_p_width = 460; 
        int target_px = bw/2 - target_p_width/2;
        int target_py = target_cv_y + target_cv_size + 110;
        
        int target_cx_btn = bw/2;
        int target_ctrl_y = target_py + 70;
        
        // Interpolate
        int cv_size = start_cv_size + (int)((target_cv_size - start_cv_size) * t);
        int cv_x = start_cv_x + (int)((target_cv_x - start_cv_x) * t);
        int cv_y = start_cv_y + (int)((target_cv_y - start_cv_y) * t);
        
        int px = start_px + (int)((target_px - start_px) * t);
        int py = start_py + (int)((target_py - start_py) * t);
        int p_width = start_p_width + (int)((target_p_width - start_p_width) * t);
        
        int cx_btn = start_cx_btn + (int)((target_cx_btn - start_cx_btn) * t);
        int ctrl_y = start_ctrl_y + (int)((target_ctrl_y - start_ctrl_y) * t);
        
        // Shadow (fix transparency bug by using solid dark opaque color)
        int shadow_size = cv_size;
        int shadow_offset = (int)(15 * t);
        if (shadow_offset > 0) {
            winmgr_fill_rect(win, cv_x + shadow_offset, cv_y + shadow_offset, shadow_size, shadow_size, 0xFF080808); 
        }
        
        if (s->cover_img) {
            song_stretch_blit(win, cv_x, cv_y, cv_size, cv_size, s->cover_img, s->cover_w, s->cover_h);
        } else {
            winmgr_draw_rounded_rect_ex(win, cv_x, cv_y, cv_size, cv_size, 0xFF333333, 0, 0, (int)(16 * t + 4 * (1.0f - t)));
        }
        
        // Text fades and moves
        int target_tx = target_cv_x;
        int target_ty = target_cv_y + target_cv_size + 40;
        int start_tx = 110;
        int start_ty = by + 25;
        
        int tx = start_tx + (int)((target_tx - start_tx) * t);
        int ty = start_ty + (int)((target_ty - start_ty) * t);
        
        winmgr_draw_text(win, tx, ty, s->title, COL_SP_TEXT_WHT);
        winmgr_draw_text(win, tx, ty + 20 + (int)(5 * t), "PureOS Audio", COL_SP_TEXT_MUTED);
        
        // Progress Bar
        uint32_t pos_ms = 0;
        uint32_t dur_ms = 0;
        mp3_get_progress(&pos_ms, &dur_ms);
        
        char pos_str[16] = "0:00";
        char dur_str[16] = "0:00";
        if (dur_ms > 0) {
            int pm = (pos_ms / 1000) / 60;
            int ps = (pos_ms / 1000) % 60;
            int dm = (dur_ms / 1000) / 60;
            int ds = (dur_ms / 1000) % 60;
            pos_str[0] = '0' + (pm % 10); pos_str[1] = ':'; pos_str[2] = '0' + (ps / 10); pos_str[3] = '0' + (ps % 10); pos_str[4] = 0;
            dur_str[0] = '0' + (dm % 10); dur_str[1] = ':'; dur_str[2] = '0' + (ds / 10); dur_str[3] = '0' + (ds % 10); dur_str[4] = 0;
        }
        
        // Text labels for progress move outward
        int start_p_tx = start_px - 50;
        int target_p_tx = target_px;
        int start_d_tx = start_px + start_p_width + 20;
        int target_d_tx = target_px + target_p_width - 35;
        
        int p_tx = start_p_tx + (int)((target_p_tx - start_p_tx) * t);
        int p_ty = start_py - 5 + (int)((target_py - 5 - (start_py - 5)) * t);
        int d_tx = start_d_tx + (int)((target_d_tx - start_d_tx) * t);
        
        winmgr_draw_text(win, p_tx, p_ty, pos_str, COL_SP_TEXT_MUTED);
        winmgr_draw_text(win, d_tx, p_ty, dur_str, COL_SP_TEXT_MUTED);
        
        // Base bar
        int bar_h = 6 + (int)(2 * t);
        int bar_y = py + (int)(15 * t);
        
        winmgr_draw_rounded_rect_ex(win, px, bar_y, p_width, bar_h, 0xFF3E3E3E, 0, 0, bar_h/2);
        
        if (dur_ms > 0) {
            int w = (int)app->progress_anim.current_val; 
            int cur_w = (w * p_width) / 400; 
            
            static float color_t = 0.0f;
            if (app->is_playing) color_t += 0.25f;
            extern double sin(double);
            int r = (int)((sin(color_t) * 0.5f + 0.5f) * 200 + 55);
            int g = (int)((sin(color_t + 2.094f) * 0.5f + 0.5f) * 200 + 55); 
            int b = (int)((sin(color_t + 4.188f) * 0.5f + 0.5f) * 200 + 55); 
            uint32_t fill_color = 0xFF000000 | (r << 16) | (g << 8) | b;
            
            winmgr_draw_rounded_rect_ex(win, px, bar_y, cur_w, bar_h, fill_color, 0, 0, bar_h/2);
            
            float thumb_scale = app->thumb_scale_anim.current_val;
            int tw = (int)((12 + 4 * t) * thumb_scale);
            int th = (int)((12 + 4 * t) * thumb_scale);
            winmgr_draw_rounded_rect_ex(win, px + cur_w - (tw/2), bar_y + (bar_h/2) - (th/2), tw, th, COL_SP_TEXT_WHT, 0, 0, tw/2);
        }
        
        // Controls
        int prev_x = (start_cx_btn - 60) + (int)(((target_cx_btn - 80) - (start_cx_btn - 60)) * t);
        int play_x = cx_btn;
        int next_x = (start_cx_btn + 50) + (int)(((target_cx_btn + 65) - (start_cx_btn + 50)) * t);
        
        int play_size = 36 + (int)(24 * t); // 36 to 60
        
        winmgr_draw_text(win, prev_x, ctrl_y, "|<", (app->hover_btn == 1) ? COL_SP_TEXT_WHT : COL_SP_TEXT_MUTED);
        
        int play_y_offset = -8 + (int)((-20 - (-8)) * t); 
        winmgr_draw_rounded_rect_ex(win, play_x - play_size/2, ctrl_y + play_y_offset, play_size, play_size, COL_SP_TEXT_WHT, 0, 0, play_size/2);
        
        if (app->is_playing) {
            int bar_w = 4 + (int)(2 * t);
            int bar_h2 = 12 + (int)(18 * t);
            int bar_y_offset = 2 + (int)((-5 - 2) * t);
            int offset_x1 = -6 - (int)(4*t);
            int offset_x2 = 2 + (int)(2*t);
            winmgr_fill_rect(win, play_x + offset_x1, ctrl_y + bar_y_offset, bar_w, bar_h2, COL_SP_SIDEBAR);
            winmgr_fill_rect(win, play_x + offset_x2, ctrl_y + bar_y_offset, bar_w, bar_h2, COL_SP_SIDEBAR);
        } else {
            int text_y_offset = 2 + (int)((0 - 2) * t);
            winmgr_draw_text(win, play_x - 5, ctrl_y + text_y_offset, ">", COL_SP_SIDEBAR);
        }
        winmgr_draw_text(win, next_x, ctrl_y, ">|", (app->hover_btn == 3) ? COL_SP_TEXT_WHT : COL_SP_TEXT_MUTED);
    }
}

void song_draw(window_t *win) {
    song_app_t *app = get_app(win);
    if (!app) return;
    
    anim_tick(&app->now_playing_anim, 0.04f);
    if (app->now_playing_anim.active) {
        win->needs_redraw = 1;
        extern int ui_dirty;
        ui_dirty = 1;
    }
    
    draw_sidebar(win, app);
    draw_main_area(win, app);
    draw_bottom_bar(win, app);
    
    if (app->now_playing_anim.current_val > 0.001f) {
        draw_now_playing(win, app);
    }
}

void song_handle_mouse(window_t *win, int mx, int my, int buttons) {
    song_app_t *app = get_app(win);
    
    int new_hover_idx = -1;
    int new_hover_btn = 0;
    
    float t = app->now_playing_anim.current_val;
    
    if (t > 0.5f) {
        int bw = win->width;
        int offset_y = (int)((1.0f - t) * win->height);
        
        // Back button
        if (mx >= 20 && mx <= 80 && my >= 20 + offset_y && my <= 60 + offset_y) {
            if (buttons & 1) {
                anim_start_spring(&app->now_playing_anim, t, 0.0f, 400.0f, 35.0f);
            }
        }
        
        int target_cv_y = 120;
        int target_cv_size = 320;
        int target_p_width = 460;
        int target_py = target_cv_y + target_cv_size + 110;
        int target_ctrl_y = target_py + 70;
        
        int start_py = win->height - 90 + 70;
        int py = start_py + (int)((target_py - start_py) * t);
        int start_ctrl_y = win->height - 90 + 25;
        int ctrl_y = start_ctrl_y + (int)((target_ctrl_y - start_ctrl_y) * t);
        
        int cx_btn = bw/2;
        if (my >= ctrl_y - 30 && my <= ctrl_y + 40) {
            if (mx >= cx_btn - 100 && mx <= cx_btn - 50) new_hover_btn = 1;
            else if (mx >= cx_btn - 30 && mx <= cx_btn + 30) new_hover_btn = 2;
            else if (mx >= cx_btn + 50 && mx <= cx_btn + 100) new_hover_btn = 3;
        }
        
        int start_px = bw/2 - 200;
        int target_px = bw/2 - target_p_width/2;
        int px = start_px + (int)((target_px - start_px) * t);
        int start_p_width = 400;
        int p_width = start_p_width + (int)((target_p_width - start_p_width) * t);
        if (my >= py && my <= py + 30 && mx >= px && mx <= px + p_width) {
            if (!app->thumb_scale_anim.active && app->thumb_scale_anim.current_val != 1.5f) {
                anim_start_spring(&app->thumb_scale_anim, app->thumb_scale_anim.current_val, 1.5f, 400.0f, 25.0f);
            }
        } else {
            if (!app->thumb_scale_anim.active && app->thumb_scale_anim.current_val != 1.0f) {
                anim_start_spring(&app->thumb_scale_anim, app->thumb_scale_anim.current_val, 1.0f, 400.0f, 25.0f);
            }
        }
    } else {
        // Sidebar list check
        if (mx >= 10 && mx <= 240 && my >= 180 && my <= win->height - 90) {
            new_hover_idx = (my - 180) / 65;
            if (new_hover_idx >= app->song_count) new_hover_idx = -1;
        }
        
        // Sidebar nav
        if (mx >= 10 && mx <= 200 && my >= 20 && my <= 50) {
            if (buttons & 1) { 
                app->is_searching = 0; 
                load_local_songs(app); 
            }
        }
        if (mx >= 10 && mx <= 200 && my >= 55 && my <= 85) {
            if (buttons & 1) { 
                app->is_searching = 1; 
                app->search_focused = 1;
                app->song_count = 0; 
            }
        }
        
        int search_w = 400;
        int search_x = (win->width - search_w) / 2;
        int search_y = 20;
        
        if (mx >= search_x && mx <= search_x + search_w && my >= search_y && my <= search_y + 40) {
            if (buttons & 1) {
                app->is_searching = 1;
                app->search_focused = 1;
            }
        } else if (buttons & 1) {
            app->search_focused = 0;
        }

        if (!app->is_searching) {
            // Main Area Card check
            if (mx >= 280 && mx <= 430 && my >= 400 && my <= 570) {
                if (app->song_count > 0) new_hover_idx = 0;
            }
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
        
        if (my >= by + 60 && my <= by + 80 && mx >= cx - 210 && mx <= cx + 210) {
            if (!app->thumb_scale_anim.active && app->thumb_scale_anim.current_val != 1.5f) {
                anim_start_spring(&app->thumb_scale_anim, app->thumb_scale_anim.current_val, 1.5f, 400.0f, 25.0f);
            }
        } else {
            if (!app->thumb_scale_anim.active && app->thumb_scale_anim.current_val != 1.0f) {
                anim_start_spring(&app->thumb_scale_anim, app->thumb_scale_anim.current_val, 1.0f, 400.0f, 25.0f);
            }
        }
        
        // Open Now Playing
        if (mx >= 30 && mx <= 90 && my >= by + 15 && my <= by + 75) {
            if (buttons & 1 && app->current_song_idx >= 0) {
                anim_start_spring(&app->now_playing_anim, t, 1.0f, 400.0f, 35.0f);
            }
        }
    }

    if (new_hover_idx != app->hover_idx || new_hover_btn != app->hover_btn || app->thumb_scale_anim.active || app->now_playing_anim.active) {
        app->hover_idx = new_hover_idx;
        app->hover_btn = new_hover_btn;
        winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
    }
    
    if (buttons & 1) { // Left click
        if (app->hover_idx != -1) {
            app->current_song_idx = app->hover_idx;
            app->is_playing = 1;
            app->is_searching = 0;
            
            app->lyrics_loaded = 0;
            strcpy(lyrics_fetch_title, app->songs[app->current_song_idx].title);
            extern void* create_task(void (*entry)(), char *name);
            create_task(lyrics_fetch_thread, "YTLyrics");
            
            // Trigger playback
            if (strncmp(app->songs[app->current_song_idx].filename_mp3, "yt:", 3) == 0) {
                strcpy(yt_fetch_vid, app->songs[app->current_song_idx].filename_mp3 + 3);
                extern void* create_task(void (*entry)(), char *name);
                create_task(yt_play_thread, "YTPlay");
            } else {
                int fsize = 0;
                uint8_t *fdata = song_read_file(app->songs[app->current_song_idx].filename_mp3, &fsize);
                if (fdata) {
                    mp3_play(fdata, fsize);
                    kfree(fdata);
                }
            }
            
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        } else if (app->hover_btn == 2) {
            app->is_playing = !app->is_playing;
            if (app->is_playing) {
                if (strncmp(app->songs[app->current_song_idx].filename_mp3, "yt:", 3) == 0) {
                    strcpy(yt_fetch_vid, app->songs[app->current_song_idx].filename_mp3 + 3);
                    extern void* create_task(void (*entry)(), char *name);
                    create_task(yt_play_thread, "YTPlay");
                } else {
                    int fsize = 0;
                    uint8_t *fdata = song_read_file(app->songs[app->current_song_idx].filename_mp3, &fsize);
                    if (fdata) {
                        mp3_play(fdata, fsize);
                        kfree(fdata);
                    }
                }
            } else {
                mp3_stop();
            }
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        } else if (app->hover_btn == 1) {
            if (app->current_song_idx > 0) app->current_song_idx--;
            app->is_playing = 1;
            
            app->lyrics_loaded = 0;
            strcpy(lyrics_fetch_title, app->songs[app->current_song_idx].title);
            extern void* create_task(void (*entry)(), char *name);
            create_task(lyrics_fetch_thread, "YTLyrics");
            
            if (strncmp(app->songs[app->current_song_idx].filename_mp3, "yt:", 3) == 0) {
                strcpy(yt_fetch_vid, app->songs[app->current_song_idx].filename_mp3 + 3);
                extern void* create_task(void (*entry)(), char *name);
                create_task(yt_play_thread, "YTPlay");
            } else {
                int fsize = 0;
                uint8_t *fdata = song_read_file(app->songs[app->current_song_idx].filename_mp3, &fsize);
                if (fdata) {
                    mp3_play(fdata, fsize);
                    kfree(fdata);
                }
            }
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        } else if (app->hover_btn == 3) {
            if (app->current_song_idx < app->song_count - 1) app->current_song_idx++;
            app->is_playing = 1;
            
            app->lyrics_loaded = 0;
            strcpy(lyrics_fetch_title, app->songs[app->current_song_idx].title);
            extern void* create_task(void (*entry)(), char *name);
            create_task(lyrics_fetch_thread, "YTLyrics");
            
            if (strncmp(app->songs[app->current_song_idx].filename_mp3, "yt:", 3) == 0) {
                strcpy(yt_fetch_vid, app->songs[app->current_song_idx].filename_mp3 + 3);
                extern void* create_task(void (*entry)(), char *name);
                create_task(yt_play_thread, "YTPlay");
            } else {
                int fsize = 0;
                uint8_t *fdata = song_read_file(app->songs[app->current_song_idx].filename_mp3, &fsize);
                if (fdata) {
                    mp3_play(fdata, fsize);
                    kfree(fdata);
                }
            }
            winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
        }
    }
}

void song_handle_key(window_t *win, int key, char ascii) {
    song_app_t *app = get_app(win);
    if (!app || !app->search_focused) return;
    
    if (key == 0x0E || ascii == '\b') {
        if (app->search_cursor > 0) {
            app->search_query[--app->search_cursor] = 0;
            win->needs_redraw = 1;
        }
    } else if (key == 0x1C || ascii == '\n' || ascii == '\r') {
        app->search_focused = 0;
        app->yt_fetching = 1;
        strcpy(search_query_global, app->search_query);
        win->needs_redraw = 1;
        extern void* create_task(void (*entry)(), char *name);
        create_task(yt_search_thread, "YTSearch");
    } else if (ascii >= 32 && ascii <= 126 && app->search_cursor < 63) {
        app->search_query[app->search_cursor++] = ascii;
        app->search_query[app->search_cursor] = 0;
        win->needs_redraw = 1;
    }
}

void song_handle_scroll(window_t *win, int direction) {
    song_app_t *app = get_app(win);
    if (!app || app->lyrics_loaded != 2) return;
    app->lyrics_scroll_y += direction * 60.0f;
    if (app->lyrics_scroll_y > 0) app->lyrics_scroll_y = 0;
    winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
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
    win->user_data = 0;
}

void song_app_init() {
    window_t *win = winmgr_create_window(-1, -1, 1024, 768, "Music");
    if (!win) return;
    
    song_app_t *app = (song_app_t *)kmalloc(sizeof(song_app_t));
    memset(app, 0, sizeof(song_app_t));
    
    app->win = win;
    app->current_song_idx = -1;
    app->hover_idx = -1;
    anim_init(&app->progress_anim);
    anim_init_val(&app->thumb_scale_anim, 1.0f);
    anim_init_val(&app->now_playing_anim, 0.0f);
    app->show_now_playing = 0;
    win->user_data = app;
    win->flags |= WINDOW_FLAG_NO_TITLEBAR;
    win->app_type = 17; // APP_MUSIC
    song_player_win = win;
    
    win->draw = (void (*)(void *))song_draw;
    win->on_mouse = (void (*)(void *, int, int, int))song_handle_mouse;
    win->on_key = (void (*)(void *, int, char))song_handle_key;
    win->on_scroll = (void (*)(void *, int))song_handle_scroll;
    win->on_close = song_on_close;
    
    // Scan for MP3s in root directory
    load_local_songs(app);
    
    winmgr_invalidate_rect(win, 0, 0, win->width, win->height);
}
