#include "../kernel/ui_layout.h"
#include "../fs/fs.h"
#include "../kernel/heap.h"
#include "../kernel/image.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"

#define MAX_PHOTOS 128
#define THUMB_SIZE 180
#define GRID_PADDING 24
#define GRID_TEXT_H 20
#define TITLE_H 32

typedef struct {
  uint32_t *data;
  int w, h;
} thumb_t;

typedef struct {
  char filenames[MAX_PHOTOS][32];
  int file_sizes[MAX_PHOTOS];
  thumb_t thumbs[MAX_PHOTOS];
  int count;
  int selected_idx; // -1 for gallery, 0+ for viewer

  // Loaded full image data
  uint32_t *img_data;
  int img_w;
  int img_h;

  int scroll_y;
  char status[64];
  int prev_buttons;

  // Zoom and pan state
  float zoom;
  int pan_x;
  int pan_y;
  int is_dragging;
  int last_mx;
  int last_my;
} photos_app_t;

extern void print_serial(const char *);
extern void k_itoa(int, char *);

static void photos_load_image(photos_app_t *app, const char *filename, int expected_size) {
  if (app->img_data) {
    stbi_image_free(app->img_data);
    app->img_data = 0;
  }

  print_serial("PHOTOS: Loading ");
  print_serial(filename);
  print_serial("\n");

  if (expected_size <= 0) {
    FileInfo files[MAX_PHOTOS];
    int count = fs_list_files("/", files, MAX_PHOTOS);
    for (int i = 0; i < count; i++) {
      if (strcmp(files[i].name, filename) == 0) {
        expected_size = files[i].size;
        break;
      }
    }
  }

  if (expected_size <= 0 || expected_size > 1024 * 1024 * 64) {
    strcpy(app->status, "File too large/invalid.");
    return;
  }

  uint8_t *raw_data = (uint8_t *)malloc(expected_size + 16);
  if (!raw_data) {
    strcpy(app->status, "Out of memory.");
    return;
  }

  int size = fs_read(filename, raw_data);
  if (size <= 0) {
    free(raw_data);
    strcpy(app->status, "Read failed.");
    return;
  }

  int w, h, n;
  unsigned char *pixels = stbi_load_from_memory(raw_data, size, &w, &h, &n, 4);
  free(raw_data);

  if (!pixels) {
    strcpy(app->status, "Decode failed.");
    return;
  }

  // Convert to ARGB (STB is RGBA)
  uint32_t *p32 = (uint32_t *)pixels;
  for (int i = 0; i < w * h; i++) {
    uint32_t p = p32[i];
    uint32_t r = p & 0xFF;
    uint32_t g = (p >> 8) & 0xFF;
    uint32_t b = (p >> 16) & 0xFF;
    uint32_t a = (p >> 24) & 0xFF;
    
    if (a < 255) {
      int cx = i % w;
      int cy = i / w;
      uint32_t bg = ((cx / 16) + (cy / 16)) % 2 == 0 ? 0x2A : 0x1A;
      uint32_t inv_a = 255 - a;
      r = (r * a + bg * inv_a) >> 8;
      g = (g * a + bg * inv_a) >> 8;
      b = (b * a + bg * inv_a) >> 8;
      a = 255;
    }
    p32[i] = (a << 24) | (r << 16) | (g << 8) | b;
  }

  app->img_data = (uint32_t *)pixels;
  app->img_w = w;
  app->img_h = h;
  app->zoom = 1.0f;
  app->pan_x = 0;
  app->pan_y = 0;

  strcpy(app->status, filename);
}

static void photos_create_thumbnail(photos_app_t *app, int idx) {
  if (app->thumbs[idx].data) return;

  uint8_t *raw_data = (uint8_t *)malloc(app->file_sizes[idx] + 16);
  if (!raw_data) return;
  int size = fs_read(app->filenames[idx], raw_data);
  if (size <= 0) { free(raw_data); return; }

  int w, h, n;
  unsigned char *pixels = stbi_load_from_memory(raw_data, size, &w, &h, &n, 4);
  free(raw_data);
  if (!pixels) return;

  int tw = THUMB_SIZE;
  int th = (h * THUMB_SIZE) / w;
  if (th > THUMB_SIZE) {
    th = THUMB_SIZE;
    tw = (w * THUMB_SIZE) / h;
  }

  uint32_t *thumb_data = (uint32_t *)malloc(tw * th * 4);
  if (!thumb_data) { stbi_image_free(pixels); return; }

  uint32_t *src32 = (uint32_t *)pixels;
  for (int y = 0; y < th; y++) {
    int src_y_start = (y * h) / th;
    int src_y_end = ((y + 1) * h) / th;
    if (src_y_end == src_y_start) src_y_end++;
    if (src_y_end > h) src_y_end = h;

    for (int x = 0; x < tw; x++) {
      int src_x_start = (x * w) / tw;
      int src_x_end = ((x + 1) * w) / tw;
      if (src_x_end == src_x_start) src_x_end++;
      if (src_x_end > w) src_x_end = w;

      int sum_r = 0, sum_g = 0, sum_b = 0, sum_a = 0;
      int count = 0;

      for (int sy = src_y_start; sy < src_y_end; sy++) {
        uint32_t *row = &src32[sy * w];
        for (int sx = src_x_start; sx < src_x_end; sx++) {
          uint32_t p = row[sx];
          sum_r += p & 0xFF;
          sum_g += (p >> 8) & 0xFF;
          sum_b += (p >> 16) & 0xFF;
          sum_a += (p >> 24) & 0xFF;
          count++;
        }
      }

      if (count > 0) {
        sum_r /= count;
        sum_g /= count;
        sum_b /= count;
        sum_a /= count;
      }
      
      if (sum_a < 255) {
        uint32_t bg = ((x / 8) + (y / 8)) % 2 == 0 ? 0x2A : 0x1A;
        uint32_t inv_a = 255 - sum_a;
        sum_r = (sum_r * sum_a + bg * inv_a) >> 8;
        sum_g = (sum_g * sum_a + bg * inv_a) >> 8;
        sum_b = (sum_b * sum_a + bg * inv_a) >> 8;
        sum_a = 255;
      }

      thumb_data[y * tw + x] = (sum_a << 24) | (sum_r << 16) | (sum_g << 8) | sum_b;
    }
  }

  app->thumbs[idx].data = thumb_data;
  app->thumbs[idx].w = tw;
  app->thumbs[idx].h = th;

  stbi_image_free(pixels);
}

static void photos_refresh_list(photos_app_t *app) {
  FileInfo files[MAX_PHOTOS];
  int count = fs_list_files("/", files, MAX_PHOTOS);

  app->count = 0;
  for (int i = 0; i < count; i++) {
    const char *name = files[i].name;
    const char *ext = strrchr(name, '.');

    if (ext) {
      if (strncmp(name, "temp_temp_", 10) == 0) continue;

      int match = 0;
      if (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0) match = 1;
      else if (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0) match = 1;
      else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".JPG") == 0) match = 1;
      else if (strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".JPEG") == 0) match = 1;
      else if (strcmp(ext, ".gif") == 0 || strcmp(ext, ".GIF") == 0) match = 1;
      else if (strcmp(ext, ".tga") == 0 || strcmp(ext, ".TGA") == 0) match = 1;

      if (match) {
        strcpy(app->filenames[app->count], name);
        app->file_sizes[app->count] = files[i].size;
        app->count++;
        if (app->count >= MAX_PHOTOS) break;
      }
    }
  }

  if (app->count == 0) strcpy(app->status, "No images found.");
  else strcpy(app->status, "Gallery View");
}

static void photos_draw_gallery(window_t *win, photos_app_t *app) {
  const theme_t *theme = theme_get();
  int w = win->width;
  int h = win->height;

  int cell_w = THUMB_SIZE + GRID_PADDING;
  int cell_h = THUMB_SIZE + GRID_PADDING + GRID_TEXT_H;
  int cols = (w - GRID_PADDING) / cell_w;
  if (cols < 1) cols = 1;

  int start_x = (w - (cols * cell_w - GRID_PADDING)) / 2;
  int start_y = TITLE_H + GRID_PADDING - app->scroll_y;

  for (int i = 0; i < app->count; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = start_x + col * cell_w;
    int y = start_y + row * cell_h;

    // Viewport check
    if (y + cell_h < 0 || y > h) continue;

    // Load thumbnail if needed (Lazy Loading)
    if (!app->thumbs[i].data) {
      photos_create_thumbnail(app, i);
    }

    if (app->thumbs[i].data) {
      // Center thumbnail in its cell
      int tx = x + (THUMB_SIZE - app->thumbs[i].w) / 2;
      int ty = y + (THUMB_SIZE - app->thumbs[i].h) / 2;
      
      // Draw shadow/border
      winmgr_draw_rect(win, tx - 1, ty - 1, app->thumbs[i].w + 2, app->thumbs[i].h + 2, theme->border);
      
      // Draw image
      winmgr_blit(win, tx, ty, app->thumbs[i].data, app->thumbs[i].w, app->thumbs[i].h, 0, 0, app->thumbs[i].w, app->thumbs[i].h);
    } else {
      // Loading placeholder
      winmgr_fill_rect(win, x, y, THUMB_SIZE, THUMB_SIZE, theme->menu_bg);
    }

    // Filename
    char name_buf[16];
    strncpy(name_buf, app->filenames[i], 12);
    name_buf[12] = 0;
    if (strlen(app->filenames[i]) > 12) strcat(name_buf, "..");
    
    int tw = strlen(name_buf) * 8;
    winmgr_draw_text(win, x + (THUMB_SIZE - tw) / 2, y + THUMB_SIZE + 5, name_buf, theme->fg);
  }

  // Update max scroll
  int total_rows = (app->count + cols - 1) / cols;
  win->max_scroll = (total_rows * cell_h) - h + GRID_PADDING * 2;
  if (win->max_scroll < 0) win->max_scroll = 0;
}

static inline uint32_t bilinear_pixel(uint32_t *data, int w, int h, int fx, int fy) {
  int sx = fx >> 16;
  int sy = fy >> 16;
  int x_diff = (fx >> 8) & 0xFF;
  int y_diff = (fy >> 8) & 0xFF;
  
  if (sx < 0) sx = 0; if (sx >= w - 1) sx = w - 2;
  if (sy < 0) sy = 0; if (sy >= h - 1) sy = h - 2;
  if (sx < 0 || sy < 0) return 0;

  uint32_t p00 = data[sy * w + sx];
  uint32_t p01 = data[sy * w + sx + 1];
  uint32_t p10 = data[(sy + 1) * w + sx];
  uint32_t p11 = data[(sy + 1) * w + sx + 1];

  int x_inv = 256 - x_diff;
  int y_inv = 256 - y_diff;

  int w00 = (x_inv * y_inv) >> 8;
  int w01 = (x_diff * y_inv) >> 8;
  int w10 = (x_inv * y_diff) >> 8;
  int w11 = (x_diff * y_diff) >> 8;

  uint32_t r = (((p00 >> 16) & 0xFF) * w00 + ((p01 >> 16) & 0xFF) * w01 + ((p10 >> 16) & 0xFF) * w10 + ((p11 >> 16) & 0xFF) * w11) >> 8;
  uint32_t g = (((p00 >> 8) & 0xFF) * w00 + ((p01 >> 8) & 0xFF) * w01 + ((p10 >> 8) & 0xFF) * w10 + ((p11 >> 8) & 0xFF) * w11) >> 8;
  uint32_t b = ((p00 & 0xFF) * w00 + (p01 & 0xFF) * w01 + (p10 & 0xFF) * w10 + (p11 & 0xFF) * w11) >> 8;
  uint32_t a = (((p00 >> 24) & 0xFF) * w00 + ((p01 >> 24) & 0xFF) * w01 + ((p10 >> 24) & 0xFF) * w10 + ((p11 >> 24) & 0xFF) * w11) >> 8;

  return (a << 24) | (r << 16) | (g << 8) | b;
}

static inline uint32_t sample_pixel(uint32_t *data, int w, int h, int fx, int fy, int dx_step, int dy_step) {
  if (dx_step <= 196608 && dy_step <= 196608) {
    return bilinear_pixel(data, w, h, fx, fy);
  }

  int sx_start = fx >> 16;
  int sx_end = (fx + dx_step) >> 16;
  if (sx_end == sx_start) sx_end++;
  
  int sy_start = fy >> 16;
  int sy_end = (fy + dy_step) >> 16;
  if (sy_end == sy_start) sy_end++;

  if (sx_start < 0) sx_start = 0;
  if (sy_start < 0) sy_start = 0;
  if (sx_end > w) sx_end = w;
  if (sy_end > h) sy_end = h;

  int sum_r = 0, sum_g = 0, sum_b = 0, sum_a = 0;
  int count = 0;

  for (int sy = sy_start; sy < sy_end; sy++) {
    uint32_t *row = &data[sy * w];
    for (int sx = sx_start; sx < sx_end; sx++) {
      uint32_t p = row[sx];
      sum_r += (p >> 16) & 0xFF;
      sum_g += (p >> 8) & 0xFF;
      sum_b += p & 0xFF;
      sum_a += (p >> 24) & 0xFF;
      count++;
    }
  }

  if (count == 0) return 0xFF000000;

  sum_r /= count;
  sum_g /= count;
  sum_b /= count;
  sum_a /= count;

  return (sum_a << 24) | (sum_r << 16) | (sum_g << 8) | sum_b;
}

static void photos_draw_viewer(window_t *win, photos_app_t *app) {
  const theme_t *theme = theme_get();
  if (!app->img_data) {
    winmgr_draw_text(win, 20, 20, "Loading...", theme->fg);
    return;
  }

  int vw = win->width;
  int vh = win->height - TITLE_H;
  int dw = app->img_w;
  int dh = app->img_h;

  int tw = dw;
  int th = dh;

  if (dw > vw || dh > vh) {
    float sw = (float)vw / dw;
    float sh = (float)vh / dh;
    float scale = (sw < sh) ? sw : sh;
    tw = (int)(dw * scale);
    th = (int)(dh * scale);
  }

  tw = (int)(tw * app->zoom);
  th = (int)(th * app->zoom);

  int dx = (vw - tw) / 2 + app->pan_x;
  int dy = TITLE_H + (vh - th) / 2 + app->pan_y;

  // Background for viewer (darker)
  winmgr_fill_rect(win, 0, TITLE_H, vw, vh, 0xFF000000);

  int start_y = 0;
  int end_y = th;
  if (dy < TITLE_H) start_y = TITLE_H - dy;
  if (dy + end_y > win->height) end_y = win->height - dy;

  int start_x = 0;
  int end_x = tw;
  if (dx < 0) start_x = -dx;
  if (dx + end_x > win->width) end_x = win->width - dx;

  int dx_step = (int)(((long long)dw << 16) / tw);
  int dy_step = (int)(((long long)dh << 16) / th);

  int fy = start_y * dy_step;
  for (int y = start_y; y < end_y; y++) {
    int fx = start_x * dx_step;
    for (int x = start_x; x < end_x; x++) {
      winmgr_put_pixel(win, dx + x, dy + y, sample_pixel(app->img_data, dw, dh, fx, fy, dx_step, dy_step));
      fx += dx_step;
    }
    fy += dy_step;
  }

  // Back button (top left of content area)
  int bx = 10;
  int by = TITLE_H + 10;
  winmgr_fill_rect(win, bx, by, 80, 30, 0x80000000);
  winmgr_draw_rect(win, bx, by, 80, 30, 0xFFFFFFFF);
  winmgr_draw_text(win, bx + 15, by + 8, "< Back", 0xFFFFFFFF);

  // Overlay file info
  char info[64];
  snprintf(info, sizeof(info), "%s (%dx%d) - %.1fx", app->filenames[app->selected_idx], app->img_w, app->img_h, app->zoom);
  int iw = winmgr_measure_text(info);
  winmgr_draw_text(win, (win->width - iw) / 2, win->height - 25, info, 0xFFFFFFFF);
}

static void photos_draw(void *w) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  const theme_t *theme = theme_get();

  winmgr_fill_rect(win, 0, TITLE_H, win->width, win->height - TITLE_H, theme->bg);

  if (app->selected_idx == -1) {
    photos_draw_gallery(win, app);
  } else {
    photos_draw_viewer(win, app);
  }
}

static void photos_on_mouse(void *w, int mx, int my, int buttons) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;

  if (buttons && !app->prev_buttons) {
    if (app->selected_idx == -1) {
      // Gallery interaction
      int cell_w = THUMB_SIZE + GRID_PADDING;
      int cell_h = THUMB_SIZE + GRID_PADDING + GRID_TEXT_H;
      int cols = (win->width - GRID_PADDING) / cell_w;
      if (cols < 1) cols = 1;

      int start_x = (win->width - (cols * cell_w - GRID_PADDING)) / 2;
      int start_y = TITLE_H + GRID_PADDING - app->scroll_y;

      for (int i = 0; i < app->count; i++) {
        int col = i % cols;
        int row = i / cols;
        int x = start_x + col * cell_w;
        int y = start_y + row * cell_h;

        if (mx >= x && mx < x + THUMB_SIZE && my >= y && my < y + THUMB_SIZE) {
          app->selected_idx = i;
          photos_load_image(app, app->filenames[i], app->file_sizes[i]);
          win->needs_redraw = 1;
          break;
        }
      }
    } else {
      // Back button click (10, TITLE_H + 10, 80, 30)
      if (mx >= 10 && mx <= 90 && my >= TITLE_H + 10 && my <= TITLE_H + 40) {
        app->selected_idx = -1;
        if (app->img_data) {
          stbi_image_free(app->img_data);
          app->img_data = 0;
        }
        win->needs_redraw = 1;
      } else {
        app->is_dragging = 1;
        app->last_mx = mx;
        app->last_my = my;
      }
    }
  } else if (buttons && app->prev_buttons) {
    if (app->selected_idx != -1 && app->is_dragging) {
      app->pan_x += (mx - app->last_mx);
      app->pan_y += (my - app->last_my);
      app->last_mx = mx;
      app->last_my = my;
      win->needs_redraw = 1;
    }
  } else if (!buttons) {
    app->is_dragging = 0;
  }

  app->prev_buttons = buttons;
}

static void photos_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  if (!app) return;

  if (app->selected_idx == -1) {
    app->scroll_y -= direction * 40;
    if (app->scroll_y < 0) app->scroll_y = 0;
    if (app->scroll_y > win->max_scroll) app->scroll_y = win->max_scroll;
    win->needs_redraw = 1;
  } else {
    // Zoom in viewer
    if (direction > 0) app->zoom *= 1.2f;
    else if (direction < 0) app->zoom /= 1.2f;
    if (app->zoom < 0.1f) app->zoom = 0.1f;
    if (app->zoom > 10.0f) app->zoom = 10.0f;
    win->needs_redraw = 1;
  }
}

static void photos_on_key(void *w, int key, char ascii) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  if (!app) return;

  if (app->selected_idx != -1) {
    if (key == 0x4D) { // Right Arrow (typically 0x4D in some scancode mappings, wait, we might not know scancodes)
      // We will handle 'n' for next, 'p' for prev as a fallback.
    }
    if (ascii == 'n' || ascii == 'N' || key == 0x4D) { // 0x4D might be right arrow depending on keymap
      if (app->selected_idx < app->count - 1) {
        app->selected_idx++;
        photos_load_image(app, app->filenames[app->selected_idx], app->file_sizes[app->selected_idx]);
        win->needs_redraw = 1;
      }
    } else if (ascii == 'p' || ascii == 'P' || key == 0x4B) { // 0x4B might be left arrow
      if (app->selected_idx > 0) {
        app->selected_idx--;
        photos_load_image(app, app->filenames[app->selected_idx], app->file_sizes[app->selected_idx]);
        win->needs_redraw = 1;
      }
    } else if (key == 0x01) { // ESC maybe? Let's just use 'q' or Backspace
       app->selected_idx = -1;
       if (app->img_data) {
         stbi_image_free(app->img_data);
         app->img_data = 0;
       }
       win->needs_redraw = 1;
    }
  }
}

static void photos_on_close(void *w) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  if (app) {
    if (app->img_data) stbi_image_free(app->img_data);
    for (int i = 0; i < MAX_PHOTOS; i++) {
      if (app->thumbs[i].data) free(app->thumbs[i].data);
    }
    kfree(app);
    win->user_data = 0;
  }
}

void photos_open(const char *path) {
  window_t *win = winmgr_get_window_by_app_type(12);
  photos_app_t *app;

  if (win) {
    app = (photos_app_t *)win->user_data;
    winmgr_bring_to_front(win);
  } else {
    win = winmgr_create_window(-1, -1, 800, 600, "Photos");
    if (!win) return;

    app = (photos_app_t *)malloc(sizeof(photos_app_t));
    memset(app, 0, sizeof(photos_app_t));
    app->selected_idx = -1;

    win->user_data = app;
    win->draw = photos_draw;
    win->on_mouse = photos_on_mouse;
    win->on_scroll = (void (*)(void *, int))photos_on_scroll;
    win->on_key = photos_on_key;
    win->on_close = photos_on_close;
    win->app_type = 12;

    photos_refresh_list(app);
  }

  if (path && path[0] != 0) {
    // Open specific file
    for (int i = 0; i < app->count; i++) {
      if (strstr(path, app->filenames[i])) {
        app->selected_idx = i;
        photos_load_image(app, path, 0);
        break;
      }
    }
  }

  win->needs_redraw = 1;
}

void photos_init() { photos_open(0); }
