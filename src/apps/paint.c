#include "paint.h"
#include "../drivers/mouse.h"
#include "../drivers/vga.h"
#include "../fs/fs.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/window.h" // For window_t

// Canvas Settings
#define CANVAS_WIDTH 340
#define CANVAS_HEIGHT 230
#define CANVAS_OFFSET_X 50
#define CANVAS_OFFSET_Y 25

// Tools
#define TOOL_PENCIL 0
#define TOOL_ERASER 1
#define TOOL_LINE 2
#define TOOL_RECT 3
#define TOOL_RECT_FILLED 4

typedef struct {
  uint32_t *pixels;
  uint32_t current_color;
  int current_tool;
  int brush_size;
  int is_drawing;
  int start_x, start_y;
  int last_x, last_y;
  window_t *win;

  // Dialog State
  int dialog_mode; // 0=None, 1=Save, 2=Open
  char filename[32];
  int cursor_timer;
} paint_state_t;

static paint_state_t paint_state;

// Helper: Get pixel color safely
uint32_t get_pixel(int x, int y) {
  if (x < 0 || x >= CANVAS_WIDTH || y < 0 || y >= CANVAS_HEIGHT)
    return 0;
  return paint_state.pixels[y * CANVAS_WIDTH + x];
}

// Helper: Set pixel logic
void paint_put_pixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= CANVAS_WIDTH || y < 0 || y >= CANVAS_HEIGHT)
    return;
  paint_state.pixels[y * CANVAS_WIDTH + x] = color;

  // Immediate feedback if window active
  if (paint_state.win) {
    winmgr_put_pixel(paint_state.win, x + CANVAS_OFFSET_X, y + CANVAS_OFFSET_Y,
                     color);
  }
}

// Bresenham Line
void draw_line(int x1, int y1, int x2, int y2, uint32_t color, int size) {
  int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
  int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  while (1) {
    // Draw Brush Size
    for (int py = -size / 2; py <= size / 2; py++) {
      for (int px = -size / 2; px <= size / 2; px++) {
        paint_put_pixel(x1 + px, y1 + py, color);
      }
    }

    if (x1 == x2 && y1 == y2)
      break;
    int e2 = err * 2;
    if (e2 > -dy) {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }
}

// Draw Rect
void draw_alloc_rect(int x, int y, int w, int h, uint32_t color, int filled) {
  // Clamp
  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;
  if (x + w > CANVAS_WIDTH)
    w = CANVAS_WIDTH - x;
  if (y + h > CANVAS_HEIGHT)
    h = CANVAS_HEIGHT - y;

  for (int cy = 0; cy < h; cy++) {
    for (int cx = 0; cx < w; cx++) {
      if (filled || cy == 0 || cy == h - 1 || cx == 0 || cx == w - 1) {
        paint_put_pixel(x + cx, y + cy, color);
      }
    }
  }
}

void paint_clear() {
  for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++) {
    paint_state.pixels[i] = 0xFFFFFFFF;
  }
  if (paint_state.win)
    paint_state.win->needs_redraw = 1;
}

// Keyboard input for dialogs
void paint_on_key(window_t *win, int key, char ascii) {
  if (paint_state.dialog_mode > 0) {
    int len = strlen(paint_state.filename);
    if (ascii == '\n' || ascii == '\r') {
      if (paint_state.dialog_mode == 1) { // Save
        if (fs_write(paint_state.filename, (uint8_t *)paint_state.pixels,
                     CANVAS_WIDTH * CANVAS_HEIGHT * 4)) {
          strcpy(win->title, "Paint - Saved");
        } else {
          strcpy(win->title, "Paint - Save Error");
        }
      } else if (paint_state.dialog_mode == 2) { // Open
        if (fs_read(paint_state.filename, (uint8_t *)paint_state.pixels)) {
          strcpy(win->title, "Paint - Loaded");
        } else {
          strcpy(win->title, "Paint - Load Error");
        }
      }
      paint_state.dialog_mode = 0;
      win->needs_redraw = 1;
    } else if (ascii == 27) { // ESC
      paint_state.dialog_mode = 0;
      win->needs_redraw = 1;
    } else if (ascii == '\b') {
      if (len > 0)
        paint_state.filename[len - 1] = 0;
      win->needs_redraw = 1;
    } else if (len < 31 && ascii >= 32 && ascii < 127) {
      paint_state.filename[len] = ascii;
      paint_state.filename[len + 1] = 0;
      win->needs_redraw = 1;
    }
  }
}

static void paint_on_scroll(void *win, int direction) {
  (void)win;
  // Scroll UP (direction > 0) -> Increase brush size
  // Scroll DOWN (direction < 0) -> Decrease brush size
  if (direction > 0) {
    if (paint_state.brush_size < 15)
      paint_state.brush_size += 2;
  } else {
    if (paint_state.brush_size > 1)
      paint_state.brush_size -= 2;
  }

  if (paint_state.win)
    paint_state.win->needs_redraw = 1;
  extern int ui_dirty;
  ui_dirty = 1;
}

void paint_open(const char *filename) {
  if (!paint_state.win || paint_state.win->id == 0) {
    paint_init();
  }

  if (fs_read(filename, (uint8_t *)paint_state.pixels)) {
    strcpy(paint_state.win->title, "Paint - Loaded");
    strcpy(paint_state.filename, filename);
  } else {
    strcpy(paint_state.win->title, "Paint - Load Error");
  }

  paint_state.win->needs_redraw = 1;
  paint_state.win->is_minimized = 0;

  extern void winmgr_bring_to_front(window_t * win);
  winmgr_bring_to_front(paint_state.win);
}

// UI Drawing
void paint_draw_ui() {
  window_t *w = paint_state.win;
  if (!w)
    return;

  // 1. Toolbar (Left)
  winmgr_fill_rect(w, 5, 25, 40, 240, 0xFFCCCCCC); // Extends lower

  // Tools
  const char *tools[] = {"P", "E", "L", "R", "F", "C", "S", "O"};
  // P=Pencil, E=Eraser, L=Line, R=Rect, F=FillRect, C=Clear, S=Save, O=Open

  for (int i = 0; i < 8; i++) {
    int ty = 30 + (i * 26);
    uint32_t bg = 0xDDDDDD;

    // Active Tool Highlight
    if (i <= 4 && paint_state.current_tool == i)
      bg = 0xFFFFFF;

    winmgr_fill_rect(w, 10, ty, 30, 24, bg);
    winmgr_draw_rect(w, 10, ty, 30, 24, 0x000000);
    winmgr_draw_text(w, 20, ty + 8, tools[i], 0x000000);
  }

  // Brush Size
  winmgr_draw_text(w, 12, 240, "Size", 0xFF000000);
  winmgr_fill_rect(w, 10, 250, 30, 15, 0xFFEEEEEE); // Current
  // Draw current size dot
  int s = paint_state.brush_size;
  winmgr_fill_rect(w, 25 - (s / 2), 257 - (s / 2), s, s, 0xFF000000);

  // Palette (Bottom)
  uint32_t colors[] = {0xFF000000, 0xFFFFFFFF, 0xFFFF0000, 0xFF00FF00,
                       0xFF0000FF, 0xFFFFFF00, 0xFF00FFFF, 0xFFFF00FF};
  for (int i = 0; i < 8; i++) {
    int cx = CANVAS_OFFSET_X + (i * 42);
    int cy = 265;
    winmgr_fill_rect(w, cx, cy, 32, 20, colors[i]);
    if (paint_state.current_color == colors[i]) {
      winmgr_draw_rect(w, cx - 1, cy - 1, 34, 22, 0xFFFFFFFF); // Selected
    }
  }

  // Canvas Frame
  winmgr_draw_rect(w, CANVAS_OFFSET_X - 1, CANVAS_OFFSET_Y - 1,
                   CANVAS_WIDTH + 2, CANVAS_HEIGHT + 2, 0xFF000000);

  // Canvas Content
  for (int y = 0; y < CANVAS_HEIGHT; y++) {
    for (int x = 0; x < CANVAS_WIDTH; x++) {
      uint32_t c = paint_state.pixels[y * CANVAS_WIDTH + x];
      winmgr_put_pixel(w, CANVAS_OFFSET_X + x, CANVAS_OFFSET_Y + y, c);
    }
  }

  // 4. DIALOG OVERLAY
  if (paint_state.dialog_mode > 0) {
    int dw = 300;
    int dh = 120;
    int dx = (w->width - dw) / 2;
    int dy = (w->height - dh) / 2;

    // Shadow & Backdrop
    winmgr_fill_rect(w, dx + 5, dy + 5, dw, dh, 0xFF111111);
    winmgr_fill_rect(w, dx, dy, dw, dh, 0xFFCCCCCC);
    winmgr_draw_rect(w, dx, dy, dw, dh, 0xFF000000);

    const char *dtitle =
        (paint_state.dialog_mode == 1) ? "SAVE IMAGE AS:" : "OPEN IMAGE:";
    winmgr_draw_text(w, dx + 10, dy + 10, dtitle, 0xFFFF0000);

    // Input Field
    winmgr_fill_rect(w, dx + 10, dy + 40, dw - 20, 25, 0xFFFFFFFF);
    winmgr_draw_rect(w, dx + 10, dy + 40, dw - 20, 25, 0xFF000000);
    winmgr_draw_text(w, dx + 15, dy + 47, paint_state.filename, 0xFF000000);

    // Cursor
    int flen = strlen(paint_state.filename);
    if ((paint_state.cursor_timer++ / 10) % 2 == 0) {
      winmgr_fill_rect(w, dx + 15 + (flen * 8), dy + 45, 2, 15, 0xFF000000);
    }

    winmgr_draw_text(w, dx + 10, dy + 80, "ENTER to confirm", 0xFF000000);
    winmgr_draw_text(w, dx + 10, dy + 100, "ESC to cancel", 0xFF000000);
  }
}

void paint_handle_mouse(window_t *w, int mx, int my, int buttons) {
  // mx, my are already window-relative
  int rel_x = mx;
  int rel_y = my;

  // 1. Toolbar Click
  if (rel_x >= 10 && rel_x <= 40 && (buttons & 1)) {
    if (paint_state.dialog_mode > 0)
      return; // Ignore toolbar if dialog open

    for (int i = 0; i < 8; i++) {
      int ty = 30 + (i * 26);
      if (rel_y >= ty && rel_y < ty + 24) {
        if (i <= 4)
          paint_state.current_tool = i;
        else if (i == 5)
          paint_clear();
        else if (i == 6) {
          paint_state.dialog_mode = 1;
          paint_state.cursor_timer = 0;
        } else if (i == 7) {
          paint_state.dialog_mode = 2;
          paint_state.cursor_timer = 0;
        }

        w->needs_redraw = 1;
        return;
      }
    }

    // Brush Size Change (Simple toggle 1->3->5->1)
    if (rel_y >= 250 && rel_y < 265) {
      paint_state.brush_size += 2;
      if (paint_state.brush_size > 5)
        paint_state.brush_size = 1;
      w->needs_redraw = 1;
      return; // Debounce needed?
    }
  }

  // 2. Palette Click
  if (rel_y >= 265 && rel_y <= 285 && (buttons & 1)) {
    int idx = (rel_x - CANVAS_OFFSET_X) / 42;
    if (idx >= 0 && idx < 8) {
      uint32_t colors[] = {0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00,
                           0x0000FF, 0xFFFF00, 0x00FFFF, 0xFF00FF};
      paint_state.current_color = colors[idx];
      w->needs_redraw = 1;
    }
  }

  // 3. Canvas Interaction
  int cx = rel_x - CANVAS_OFFSET_X;
  int cy = rel_y - CANVAS_OFFSET_Y;

  if (paint_state.dialog_mode > 0)
    return; // Disable drawing when dialog is open

  if (cx >= 0 && cx < CANVAS_WIDTH && cy >= 0 && cy < CANVAS_HEIGHT) {
    if (buttons & 1) {
      uint32_t col = (paint_state.current_tool == TOOL_ERASER)
                         ? 0xFFFFFF
                         : paint_state.current_color;

      if (!paint_state.is_drawing) {
        paint_state.is_drawing = 1;
        paint_state.start_x = cx;
        paint_state.start_y = cy;
        paint_state.last_x = cx;
        paint_state.last_y = cy;
        if (paint_state.current_tool == TOOL_PENCIL ||
            paint_state.current_tool == TOOL_ERASER) {
          draw_line(cx, cy, cx, cy, col, paint_state.brush_size);
        }
      } else {
        if (paint_state.current_tool == TOOL_PENCIL ||
            paint_state.current_tool == TOOL_ERASER) {
          draw_line(paint_state.last_x, paint_state.last_y, cx, cy, col,
                    paint_state.brush_size);
          paint_state.last_x = cx;
          paint_state.last_y = cy;
        }
        // For Shapes (Line/Rect), we only finalize on release?
        // Real-time preview is hard without Layers.
        // We will just draw on release for simplicity in this OS.
      }
    } else {
      // Mouse Released
      if (paint_state.is_drawing) {
        uint32_t col = paint_state.current_color;
        if (paint_state.current_tool == TOOL_LINE) {
          draw_line(paint_state.start_x, paint_state.start_y, cx, cy, col,
                    paint_state.brush_size);
        } else if (paint_state.current_tool == TOOL_RECT) {
          int x = (paint_state.start_x < cx) ? paint_state.start_x : cx;
          int y = (paint_state.start_y < cy) ? paint_state.start_y : cy;
          int w = (cx > paint_state.start_x) ? cx - paint_state.start_x
                                             : paint_state.start_x - cx;
          int h = (cy > paint_state.start_y) ? cy - paint_state.start_y
                                             : paint_state.start_y - cy;
          draw_alloc_rect(x, y, w, h, col, 0);
        } else if (paint_state.current_tool == TOOL_RECT_FILLED) {
          int x = (paint_state.start_x < cx) ? paint_state.start_x : cx;
          int y = (paint_state.start_y < cy) ? paint_state.start_y : cy;
          int w = (cx > paint_state.start_x) ? cx - paint_state.start_x
                                             : paint_state.start_x - cx;
          int h = (cy > paint_state.start_y) ? cy - paint_state.start_y
                                             : paint_state.start_y - cy;
          draw_alloc_rect(x, y, w, h, col, 1);
        }
        paint_state.is_drawing = 0;
      }
    }
  }
}

// Cleanup callback - free canvas memory when Paint window is closed
static void paint_on_close(void *w) {
  (void)w;
  if (paint_state.pixels) {
    kfree(paint_state.pixels);
    paint_state.pixels = 0;
  }
  paint_state.win = 0;
}

void paint_init() {
  paint_state.win = winmgr_create_window(150, 50, 420, 310, "Paint Pro");
  if (!paint_state.win)
    return;
  paint_state.win->app_type = 4; // Paint
  paint_state.current_color = 0xFF000000;
  paint_state.current_tool = TOOL_PENCIL;
  paint_state.brush_size = 3;
  paint_state.is_drawing = 0;

  paint_state.pixels = (uint32_t *)kmalloc(CANVAS_WIDTH * CANVAS_HEIGHT * 4);
  if (!paint_state.pixels) {
    // OOM: abort window creation
    winmgr_close_window(paint_state.win);
    paint_state.win = 0;
    return;
  }
  paint_clear();

  paint_state.win->draw = (void (*)(void *))paint_draw_ui;
  paint_state.win->on_mouse =
      (void (*)(void *, int, int, int))paint_handle_mouse;
  paint_state.win->on_key = (void (*)(void *, int, char))paint_on_key;
  paint_state.win->on_scroll = (void (*)(void *, int))paint_on_scroll;
  paint_state.win->on_close = paint_on_close;
}
