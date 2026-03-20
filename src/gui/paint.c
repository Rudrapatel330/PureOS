#include "../drivers/vga.h"
#include "../kernel/mem.h"
#include "../kernel/string.h"
#include "../kernel/window.h"

// Constants
#define PAINT_TOOLBAR_H 40
#define PAINT_CANVAS_W 590 // window width - borders
#define PAINT_CANVAS_H 350 // window height - toolbar - borders

// Helpers
int abs(int n) { return n < 0 ? -n : n; }

// No direct VGA access in windowed apps

void paint_init(window_t *win) {
  // Allocate canvas buffer (approx 600x400 * 4 bytes = 960KB)
  // Canvas size shvoid paint_init(window_t *win) {
  win->paint_canvas = (uint32_t *)malloc(win->width * win->height * 4);
  win->paint_color = 0xFF000000; // Black
  win->paint_brush_size = 2;
  win->paint_prev_x = -1;
  win->paint_prev_y = -1;

  // Clear canvas
  if (win->paint_canvas) {
    for (int i = 0; i < win->width * win->height; i++) {
      win->paint_canvas[i] = 0xFFFFFFFF; // White
    }
  }
}

void paint_draw(window_t *win) {
  if (!win->paint_canvas)
    return;

  // Draw Canvas (Centered or Offset)
  // For simplicity blit directly to surface
  if (win->paint_canvas) {
    // We should ideally use a winmgr_blit function, but let's just copy
    // The canvas content starts at y=0 in paint_canvas and is copied to y+22 on
    // the window surface. The x-offset on the surface is 2. The height of the
    // canvas area is win->height - 22 (toolbar height) - 4 (bottom border). The
    // width of the canvas area is win->width - 4 (left/right borders).
    for (int y = 0; y < win->height - 22 - 4;
         y++) { // Adjusted loop limit for canvas height
      memcpy(&win->surface[(y + 22) * win->surface_w +
                           2], // Destination: surface, below toolbar, with left
                               // border offset
             &win->paint_canvas[y *
                                win->width], // Source: paint_canvas, full width
             (win->width - 4) * 4); // Number of bytes to copy: width minus
                                    // borders * 4 bytes/pixel
    }
  }

  // Draw Toolbar
  winmgr_draw_rect(win, 0, 0, win->width, 22,
                   0xFFD0D0D0); // Toolbar at top, height 22
  winmgr_draw_text(win, 5, 5, "Brush:", 0xFF000000);

  // Color Preview
  winmgr_draw_rect(win, 60, 4, 14, 14, win->paint_color);

  // Palette
  uint32_t colors[] = {0xFF000000, 0xFFFFFFFF, 0xFFFF0000,
                       0xFF00FF00, 0xFF0000FF, 0xFFFFFF00,
                       0xFF00FFFF, 0xFFFF00FF, 0xFF808080};

  for (int i = 0; i < 9; i++) {
    int cx = 10 + (i * 30);
    int cy = 22 + 5; // Defined cy
    winmgr_draw_rect(win, cx, cy, 25, 25, colors[i]);

    // Highlight selected
    if (win->paint_color == colors[i]) {
      winmgr_draw_rect(win, cx - 2, cy - 2, 29, 29, 0xFF0000FF);
    }
  }

  // Brush Size Indicator
  winmgr_draw_text(win, 300, 22 + 10, "Size:", 0xFF000000);
  // Draw current size circle/rect
  winmgr_draw_rect(win, 350, 22 + 10, win->paint_brush_size * 3,
                   win->paint_brush_size * 3, 0xFF000000);

  // 2. Copy Canvas to Surface
  int canvas_y_rel = 22 + PAINT_TOOLBAR_H;
  int canvas_h = win->height - 24 - PAINT_TOOLBAR_H;

  if (win->surface) {
    for (int j = 0; j < canvas_h; j++) {
      if (canvas_y_rel + j >= win->surface_h)
        break;
      uint32_t *src = &win->paint_canvas[j * (win->width - 4)];
      uint32_t *dst = &win->surface[(canvas_y_rel + j) * win->surface_w + 2];

      for (int i = 0; i < (win->width - 4); i++) {
        if (2 + i >= win->surface_w)
          break;
        dst[i] = src[i];
      }
    }
  }
}

void paint_click(window_t *win, int mx, int my) {
  int wx = win->x + 2;
  int wy = win->y + 32;

  // Toolbar Click?
  if (my < wy + PAINT_TOOLBAR_H) {
    // Palette
    for (int i = 0; i < 9; i++) {
      int cx = wx + 10 + (i * 30);
      int cy = wy + 5;
      if (mx >= cx && mx < cx + 25 && my >= cy && my < cy + 25) {
        uint32_t colors[] = {0xFF000000, 0xFFFFFFFF, 0xFFFF0000,
                             0xFF00FF00, 0xFF0000FF, 0xFFFFFF00,
                             0xFF00FFFF, 0xFFFF00FF, 0xFF808080};
        win->paint_color = colors[i];
        // Force redraw
        return;
      }
    }

    // Brush Size (Simple toggle for now)
    if (mx > wx + 300) {
      win->paint_brush_size++;
      if (win->paint_brush_size > 5)
        win->paint_brush_size = 1;
    }
  } else {
    // Canvas Click (Start Line)
    win->paint_prev_x = mx - wx;
    win->paint_prev_y = my - (wy + PAINT_TOOLBAR_H);

    // Draw single dot
    // paint_drag will handle it usually, but initial dot is good
  }
}

// Bresenham line on canvas
void paint_line(window_t *win, int x0, int y0, int x1, int y1) {
  int w = win->width - 4;
  int h = win->height - 30 - 4 - PAINT_TOOLBAR_H;

  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (1) {
    // Draw Brush
    int size = win->paint_brush_size;
    for (int by = 0; by < size; by++) {
      for (int bx = 0; bx < size; bx++) {
        int px = x0 + bx;
        int py = y0 + by;
        if (px >= 0 && px < w && py >= 0 && py < h) {
          win->paint_canvas[py * w + px] = win->paint_color;
        }
      }
    }

    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void paint_drag(window_t *win, int mx, int my) {
  int wx = win->x + 2;
  int wy = win->y + 32 + PAINT_TOOLBAR_H;

  int cx = mx - wx;
  int cy = my - wy;

  if (win->paint_prev_x != -1) {
    paint_line(win, win->paint_prev_x, win->paint_prev_y, cx, cy);
  }

  win->paint_prev_x = cx;
  win->paint_prev_y = cy;
}
