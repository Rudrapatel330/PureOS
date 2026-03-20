#include "../fs/fs.h"
#include "../kernel/heap.h" // Actually we use malloc/free, need heap.h
#include "../kernel/image.h"
#include "../kernel/string.h" // We use strcpy/strlen/strcmp, need string.h
#include "../kernel/window.h"

#define MAX_PHOTOS 64
#define SIDEBAR_W 120

typedef struct {
  char filenames[MAX_PHOTOS][32];
  int file_sizes[MAX_PHOTOS];
  int count;
  int selected_idx;

  // Loaded image data
  uint32_t *img_data;
  int img_w;
  int img_h;
  int img_channels;

  char status[64];
  int prev_buttons;
} photos_app_t;

extern void print_serial(const char *);

static void photos_load_image(photos_app_t *app, const char *filename,
                              int expected_size) {
  if (app->img_data) {
    stbi_image_free(app->img_data);
    app->img_data = 0;
  }

  print_serial("PHOTOS: Loading ");
  print_serial(filename);
  print_serial("\n");

  // 1. Check size
  if (expected_size <= 0) {
    // Find size if not provided
    FileInfo files[MAX_PHOTOS];
    int count = fs_list_files("/", files, MAX_PHOTOS);
    for (int i = 0; i < count; i++) {
      if (strcmp(files[i].name, filename) == 0) {
        expected_size = files[i].size;
        break;
      }
    }
  }

  if (expected_size <= 0 || expected_size > 1024 * 1024 * 64) { // 64MB limit
    strcpy(app->status, "File too large/invalid.");
    return;
  }

  uint8_t *raw_data = (uint8_t *)malloc(expected_size + 16); // Tiny padding
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

  // 2. Decode
  int w, h, n;
  // Always request 4 channels (RGBA) for consistent blitting
  unsigned char *pixels = stbi_load_from_memory(raw_data, size, &w, &h, &n, 4);
  free(raw_data);

  if (!pixels) {
    strcpy(app->status, "Decode failed.");
    return;
  }

  app->img_data = (uint32_t *)pixels;
  app->img_w = w;
  app->img_h = h;
  app->img_channels = 4;

  strcpy(app->status, filename);
}

static void photos_refresh_list(photos_app_t *app) {
  FileInfo files[MAX_PHOTOS];
  int count = fs_list_files("/", files, MAX_PHOTOS);

  print_serial("PHOTOS: Refreshing list, found total files: ");
  char buf[16];
  extern void k_itoa(int, char *);
  k_itoa(count, buf);
  print_serial(buf);
  print_serial("\n");

  app->count = 0;
  for (int i = 0; i < count; i++) {
    const char *name = files[i].name;
    int len = strlen(name);

    print_serial("  File: ");
    print_serial(name);
    print_serial("\n");

    if (len > 4) {
      const char *ext = name + len - 4;
      int match = 0;
      if (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0)
        match = 1;
      if (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0)
        match = 1;

      if (match) {
        strcpy(app->filenames[app->count], name);
        app->file_sizes[app->count] = files[i].size;
        app->count++;
        if (app->count >= MAX_PHOTOS)
          break;
      }
    }
  }

  if (app->count == 0) {
    strcpy(app->status, "No images found.");
  } else {
    strcpy(app->status, "Select an image.");
  }
}

static void photos_draw(void *w) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;

  // Background
  winmgr_fill_rect(win, 0, 0, win->width, win->height, 0xFFF0F0F0);

  // Sidebar
  winmgr_fill_rect(win, 0, 0, SIDEBAR_W, win->height, 0xFFE0E0E0);
  winmgr_draw_line(win, SIDEBAR_W, 0, SIDEBAR_W, win->height, 0xFFCCCCCC);

  winmgr_draw_text(win, 5, 5, "Photos", 0xFF333333);

  // File List
  for (int i = 0; i < app->count; i++) {
    uint32_t text_col = (i == app->selected_idx) ? 0xFF0078D4 : 0xFF333333;
    if (i == app->selected_idx) {
      winmgr_fill_rect(win, 5, 30 + i * 20, SIDEBAR_W - 10, 18, 0xFFD0E0FF);
    }
    winmgr_draw_text(win, 10, 32 + i * 20, app->filenames[i], text_col);
  }

  // Viewer Area
  int view_x = SIDEBAR_W + 10;
  int view_y = 10;
  int view_w = win->width - SIDEBAR_W - 20;
  int view_h = win->height - 40;

  winmgr_draw_rect(win, view_x - 1, view_y - 1, view_w + 2, view_h + 2,
                   0xFFCCCCCC);

  if (app->img_data) {
    // Determine draw coordinates (Fit to viewer area while preserving aspect
    // ratio)
    int dw = app->img_w;
    int dh = app->img_h;

    int target_w = dw;
    int target_h = dh;

    // Scale down if image is larger than the view area
    if (dw > view_w || dh > view_h) {
      if (dw * view_h > dh * view_w) {
        // Width constrained
        target_w = view_w;
        target_h = (dh * view_w) / dw;
      } else {
        // Height constrained
        target_h = view_h;
        target_w = (dw * view_h) / dh;
      }
    }

    int dx = view_x + (view_w - target_w) / 2;
    int dy = view_y + (view_h - target_h) / 2;

    // Manual blit with scaling (nearest neighbor) and color correction
    for (int y = 0; y < target_h; y++) {
      int screen_y = dy + y;
      if (screen_y < view_y || screen_y >= view_y + view_h)
        continue;

      // Source Y mapping
      int src_y = (y * app->img_h) / target_h;
      if (src_y >= app->img_h)
        src_y = app->img_h - 1;

      for (int x = 0; x < target_w; x++) {
        int screen_x = dx + x;
        if (screen_x < view_x || screen_x >= view_x + view_w)
          continue;

        // Source X mapping
        int src_x = (x * app->img_w) / target_w;
        if (src_x >= app->img_w)
          src_x = app->img_w - 1;

        uint32_t p = app->img_data[src_y * app->img_w + src_x];
        // STB RGBA (byte order): R at 0, G at 1, B at 2, A at 3
        uint8_t r = p & 0xFF;
        uint8_t g = (p >> 8) & 0xFF;
        uint8_t b = (p >> 16) & 0xFF;
        uint8_t a = (p >> 24) & 0xFF;

        uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;
        if (a == 0)
          color |= 0xFF000000;

        winmgr_put_pixel(win, screen_x, screen_y, color);
      }
    }
  } else {
    winmgr_draw_text(win, view_x + 20, view_y + 50, "No image selected",
                     0xFF888888);
  }

  // Status bar at bottom
  winmgr_draw_text(win, SIDEBAR_W + 10, win->height - 20, app->status,
                   0xFF666666);
}

static void photos_on_mouse(void *w, int mx, int my, int buttons) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;

  if (buttons && !app->prev_buttons) {
    // Check sidebar click
    if (mx < SIDEBAR_W && my > 30) {
      int idx = (my - 30) / 20;
      if (idx >= 0 && idx < app->count) {
        app->selected_idx = idx;
        photos_load_image(app, app->filenames[idx], app->file_sizes[idx]);
        win->needs_redraw = 1;
      }
    }
  }
  app->prev_buttons = buttons;
}

static void photos_on_scroll(void *w, int direction) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  if (!app || app->count <= 0)
    return;

  // Scroll DOWN (direction < 0) -> Next image
  // Scroll UP (direction > 0) -> Previous image
  if (direction < 0) {
    app->selected_idx++;
    if (app->selected_idx >= app->count)
      app->selected_idx = 0;
  } else {
    app->selected_idx--;
    if (app->selected_idx < 0)
      app->selected_idx = app->count - 1;
  }

  photos_load_image(app, app->filenames[app->selected_idx],
                    app->file_sizes[app->selected_idx]);
  win->needs_redraw = 1;
}

static void photos_on_close(void *w) {
  window_t *win = (window_t *)w;
  photos_app_t *app = (photos_app_t *)win->user_data;
  if (app) {
    if (app->img_data) {
      stbi_image_free(app->img_data);
      app->img_data = 0;
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
    win = winmgr_create_window(100, 100, 500, 400, "Photos");
    if (!win)
      return;

    app = (photos_app_t *)malloc(sizeof(photos_app_t));
    memset(app, 0, sizeof(photos_app_t));
    app->selected_idx = -1;

    win->user_data = app;
    win->draw = photos_draw;
    win->on_mouse = photos_on_mouse;
    win->on_scroll = (void (*)(void *, int))photos_on_scroll;
    win->on_close = photos_on_close;
    win->app_type = 12;

    photos_refresh_list(app);
  }

  if (path && path[0] != 0) {
    // Determine filename from path for the status bar
    const char *filename = path;
    for (int i = 0; path[i]; i++) {
      if (path[i] == '/')
        filename = &path[i + 1];
    }

    // Update selected index if found in list
    for (int i = 0; i < app->count; i++) {
      if (strcmp(app->filenames[i], filename) == 0) {
        app->selected_idx = i;
        break;
      }
    }
    photos_load_image(app, path, 0); // Load by full path
  }

  win->needs_redraw = 1;
}

void photos_init() { photos_open(0); }
