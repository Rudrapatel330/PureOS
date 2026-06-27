#ifndef DRAGDROP_H
#define DRAGDROP_H

#include "window.h"

typedef struct {
    int active;
    char source_path[128];
    char filename[32];
    int is_dir;
    int is_cut; // 1 = move (cut), 0 = copy
    window_t *source_win;
    int last_mx; // screen coords for ghost
    int last_my;
} file_drag_state_t;

extern file_drag_state_t g_file_drag;

void file_drag_start(window_t *win, const char *path, const char *filename, int is_dir, int is_cut, int mx, int my);
void file_drag_end(void);
int file_drag_is_active(void);
void file_draw_drag_ghost(void);

#endif
