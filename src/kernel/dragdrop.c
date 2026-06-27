#include "dragdrop.h"
#include "screen.h"
#include "../gui/explorer_icons.h"
#include <string.h>

file_drag_state_t g_file_drag;

void file_drag_start(window_t *win, const char *path, const char *filename, int is_dir, int is_cut, int mx, int my) {
    g_file_drag.active = 1;
    strcpy(g_file_drag.source_path, path);
    strcpy(g_file_drag.filename, filename);
    g_file_drag.is_dir = is_dir;
    g_file_drag.is_cut = is_cut;
    g_file_drag.source_win = win;
    g_file_drag.last_mx = mx;
    g_file_drag.last_my = my;
    extern void compositor_invalidate_rect(int x, int y, int w, int h);
    compositor_invalidate_rect(mx - 20, my - 20, 64, 64);
}

void file_drag_end(void) {
    if (g_file_drag.active) {
        extern int screen_width, screen_height;
        extern void compositor_invalidate_rect(int x, int y, int w, int h);
        compositor_invalidate_rect(g_file_drag.last_mx - 20, g_file_drag.last_my - 20, 64, 64);
    }
    g_file_drag.active = 0;
    g_file_drag.source_path[0] = 0;
    g_file_drag.filename[0] = 0;
    g_file_drag.source_win = 0;
}

int file_drag_is_active(void) {
    return g_file_drag.active;
}

void file_draw_drag_ghost(void) {
    if (!g_file_drag.active) return;

    extern int screen_width, screen_height;
    extern uint32_t *backbuffer;
    extern int mouse_x, mouse_y;

    g_file_drag.last_mx = mouse_x;
    g_file_drag.last_my = mouse_y;

    int icon_size = 32;
    int ghost_x = mouse_x - icon_size / 2;
    int ghost_y = mouse_y - icon_size / 2;

    const uint32_t *icon_data = g_file_drag.is_dir ? (const uint32_t *)icon_folder_48 : (const uint32_t *)icon_file_48;
    int src_size = 48;

    for (int j = 0; j < icon_size; j++) {
        int py = ghost_y + j;
        if (py < 0 || py >= screen_height) continue;

        int src_y = (j * src_size) / icon_size;
        for (int i = 0; i < icon_size; i++) {
            int px = ghost_x + i;
            if (px < 0 || px >= screen_width) continue;

            int src_x = (i * src_size) / icon_size;
            uint32_t pixel = icon_data[src_y * src_size + src_x];
            uint8_t a = (pixel >> 24) & 0xFF;
            if (a == 0) continue;

            uint32_t bg = backbuffer[py * screen_width + px];

            uint8_t fa = (uint8_t)((unsigned int)a * 180 / 255); // ~70% opacity ghost effect
            if (fa == 0) continue;

            uint8_t fr = (pixel >> 16) & 0xFF;
            uint8_t fg = (pixel >> 8) & 0xFF;
            uint8_t fb = pixel & 0xFF;

            uint8_t br = (bg >> 16) & 0xFF;
            uint8_t bg_g = (bg >> 8) & 0xFF;
            uint8_t bb = bg & 0xFF;

            uint8_t r = (fr * fa + br * (255 - fa)) / 255;
            uint8_t g = (fg * fa + bg_g * (255 - fa)) / 255;
            uint8_t b = (fb * fa + bb * (255 - fa)) / 255;

            backbuffer[py * screen_width + px] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }

    extern void compositor_invalidate_rect(int x, int y, int w, int h);
    compositor_invalidate_rect(ghost_x - 4, ghost_y - 4, icon_size + 8, icon_size + 8);
}
