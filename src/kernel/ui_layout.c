#include "ui_layout.h"
#include "string.h"

int ui_get_font_scale(void) {
    if (global_config.font_size == 1) return FONT_SCALE_LARGE;
    if (global_config.font_size == 2) return FONT_SCALE_XL;
    return FONT_SCALE_NORMAL;
}

extern int font_get_width_16(unsigned char c);
int ui_measure_text_width(const char *text, int font_scale) {
    if (!text) return 0;
    int w = 0;
    while (*text) {
        if (font_scale >= 10) w += (font_get_width_16((unsigned char)*text) * font_scale) / 16;
        else w += font_scale;
        text++;
    }
    return w;
}

int ui_measure_text_height(const char *text, int max_width, int font_scale) {
    if (!text || max_width <= 0) return 0;
    
    int line_height = font_scale + 4;
    int lines = 1;
    int current_w = 0;
    
    const char *p = text;
    const char *last_space = 0;
    
    while (*p) {
        if (*p == '\n') {
            lines++;
            current_w = 0;
            last_space = 0;
        } else {
            if (*p == ' ') last_space = p;
            
            if (font_scale >= 10) current_w += (font_get_width_16((unsigned char)*p) * font_scale) / 16; else current_w += font_scale;
            if (current_w > max_width) {
                lines++;
                if (last_space) {
                    // Wrap at last space
                    p = last_space;
                    last_space = 0;
                }
                current_w = 0;
            }
        }
        p++;
    }
    
    return lines * line_height;
}

extern void winmgr_draw_char_scaled(window_t *win, int x, int y, char c, uint32_t color, int scale);

void ui_draw_text_scaled(window_t *win, int x, int y, const char *text, uint32_t color, int font_scale) {
    if (!win || !text) return;
    int cur_x = x;
    extern int font_get_width_16(unsigned char c);
    while (*text) {
        if (*text == '\n') break;
        unsigned char c = *text++;
        winmgr_draw_char_scaled(win, cur_x, y, c, color, font_scale);
        if (font_scale >= 10) cur_x += (font_get_width_16(c) * font_scale) / 16;
        else cur_x += font_scale;
    }
}

void ui_draw_text_wrapped(window_t *win, int x, int y, int max_width, const char *text, uint32_t color, int font_scale) {
    if (!win || !text || max_width <= 0) return;
    
    int line_height = font_scale + 4;
    int cur_x = x;
    int cur_y = y;
    
    const char *p = text;
    const char *last_space = 0;
    const char *line_start = text;
    
    int current_w = 0;
    
    while (*p) {
        if (*p == '\n') {
            // Draw current line
            char temp[256];
            int len = p - line_start;
            if (len > 255) len = 255;
            strncpy(temp, line_start, len);
            temp[len] = 0;
            ui_draw_text_scaled(win, x, cur_y, temp, color, font_scale);
            
            cur_y += line_height;
            line_start = p + 1;
            current_w = 0;
            last_space = 0;
        } else {
            if (*p == ' ') last_space = p;
            
            if (font_scale >= 10) current_w += (font_get_width_16((unsigned char)*p) * font_scale) / 16; else current_w += font_scale;
            if (current_w > max_width) {
                // Wrap
                const char *wrap_point = last_space ? last_space : p;
                
                char temp[256];
                int len = wrap_point - line_start;
                if (len > 255) len = 255;
                strncpy(temp, line_start, len);
                temp[len] = 0;
                ui_draw_text_scaled(win, x, cur_y, temp, color, font_scale);
                
                cur_y += line_height;
                p = wrap_point;
                if (*p == ' ') p++; // Skip space at wrap
                line_start = p;
                current_w = 0;
                last_space = 0;
                continue; // Re-process p
            }
        }
        p++;
    }
    
    // Draw last line
    if (*line_start) {
        ui_draw_text_scaled(win, x, cur_y, line_start, color, font_scale);
    }
}
