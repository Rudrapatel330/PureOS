#ifndef EXPLORER_ICONS_H
#define EXPLORER_ICONS_H

#include <stdint.h>
#include "../kernel/window.h"

// Drawn with winmgr_put_pixel alpha blending
void draw_bitmap_argb(window_t *win, int x, int y, int w, int h, const uint32_t *data);

extern const uint32_t icon_folder_48[2304];
void draw_icon_folder_48(window_t *win, int x, int y);

extern const uint32_t icon_folder_16[256];
void draw_icon_folder_16(window_t *win, int x, int y);

extern const uint32_t icon_file_48[2304];
void draw_icon_file_48(window_t *win, int x, int y);

extern const uint32_t icon_file_16[256];
void draw_icon_file_16(window_t *win, int x, int y);

extern const uint32_t icon_pdf_48[2304];
void draw_icon_pdf_48(window_t *win, int x, int y);

extern const uint32_t icon_pdf_16[256];
void draw_icon_pdf_16(window_t *win, int x, int y);

extern const uint32_t icon_image_48[2304];
void draw_icon_image_48(window_t *win, int x, int y);

extern const uint32_t icon_image_16[256];
void draw_icon_image_16(window_t *win, int x, int y);

extern const uint32_t icon_video_48[2304];
void draw_icon_video_48(window_t *win, int x, int y);

extern const uint32_t icon_video_16[256];
void draw_icon_video_16(window_t *win, int x, int y);

extern const uint32_t icon_app_48[2304];
void draw_icon_app_48(window_t *win, int x, int y);

extern const uint32_t icon_app_16[256];
void draw_icon_app_16(window_t *win, int x, int y);

extern const uint32_t icon_drive_48[2304];
void draw_icon_drive_48(window_t *win, int x, int y);

extern const uint32_t icon_drive_16[256];
void draw_icon_drive_16(window_t *win, int x, int y);

#endif
