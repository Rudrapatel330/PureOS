#include "paint.h"
#include "../fs/fs.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h" // For window_t

// Canvas Settings
#define CANVAS_OFFSET_X 20
#define CANVAS_OFFSET_Y 120

// Tools
#define TOOL_PENCIL 0
#define TOOL_ERASER 1
#define TOOL_FILL 2
#define TOOL_PICKER 3
#define TOOL_TEXT 4
#define TOOL_MAGNIFY 5

// Shapes
#define SHAPE_LINE 6
#define SHAPE_CURVE 7
#define SHAPE_OVAL 8
#define SHAPE_RECT 9
#define SHAPE_TRIANGLE 10
#define SHAPE_DIAMOND 11

typedef struct {
  uint32_t *pixels;
  int canvas_w;
  int canvas_h;
  
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

void draw_line(int x1, int y1, int x2, int y2, uint32_t color, int size);


void paint_flood_fill(int start_x, int start_y, uint32_t target_col) {
   if (start_x < 0 || start_x >= paint_state.canvas_w || start_y < 0 || start_y >= paint_state.canvas_h) return;
   uint32_t bg = paint_state.pixels[start_y * paint_state.canvas_w + start_x];
   if (bg == target_col) return;
   
   int MAX_QUEUE = 65536; 
   int *qx = (int*)kmalloc(MAX_QUEUE * sizeof(int));
   int *qy = (int*)kmalloc(MAX_QUEUE * sizeof(int));
   if(!qx || !qy){ if(qx) kfree(qx); if(qy) kfree(qy); return; }
   
   int head = 0, tail = 0, count = 0;
   
   paint_state.pixels[start_y * paint_state.canvas_w + start_x] = target_col;
   qx[tail] = start_x; qy[tail] = start_y; tail = (tail+1)%MAX_QUEUE; count++;

   int iter = 0;
   while(count > 0 && iter < paint_state.canvas_w * paint_state.canvas_h * 4) {
      iter++;
      int px = qx[head];
      int py = qy[head];
      head = (head+1)%MAX_QUEUE;
      count--;
      
      if (px > 0 && paint_state.pixels[py * paint_state.canvas_w + (px - 1)] == bg) {
         paint_state.pixels[py * paint_state.canvas_w + (px - 1)] = target_col;
         if (count < MAX_QUEUE) { qx[tail] = px - 1; qy[tail] = py; tail = (tail+1)%MAX_QUEUE; count++; }
      }
      if (px < paint_state.canvas_w - 1 && paint_state.pixels[py * paint_state.canvas_w + (px + 1)] == bg) {
         paint_state.pixels[py * paint_state.canvas_w + (px + 1)] = target_col;
         if (count < MAX_QUEUE) { qx[tail] = px + 1; qy[tail] = py; tail = (tail+1)%MAX_QUEUE; count++; }
      }
      if (py > 0 && paint_state.pixels[(py - 1) * paint_state.canvas_w + px] == bg) {
         paint_state.pixels[(py - 1) * paint_state.canvas_w + px] = target_col;
         if (count < MAX_QUEUE) { qx[tail] = px; qy[tail] = py - 1; tail = (tail+1)%MAX_QUEUE; count++; }
      }
      if (py < paint_state.canvas_h - 1 && paint_state.pixels[(py + 1) * paint_state.canvas_w + px] == bg) {
         paint_state.pixels[(py + 1) * paint_state.canvas_w + px] = target_col;
         if (count < MAX_QUEUE) { qx[tail] = px; qy[tail] = py + 1; tail = (tail+1)%MAX_QUEUE; count++; }
      }
   }
   kfree(qx);
   kfree(qy);
}

uint32_t get_pixel(int x, int y) {
  if (x < 0 || x >= paint_state.canvas_w || y < 0 || y >= paint_state.canvas_h)
    return 0;
  return paint_state.pixels[y * paint_state.canvas_w + x];
}

void paint_put_pixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= paint_state.canvas_w || y < 0 || y >= paint_state.canvas_h)
    return;
  paint_state.pixels[y * paint_state.canvas_w + x] = color;

  if (paint_state.win) {
    winmgr_put_pixel(paint_state.win, x + CANVAS_OFFSET_X, y + CANVAS_OFFSET_Y, color);
  }
}

void paint_draw_line_ui(window_t *w, int x0, int y0, int x1, int y1, uint32_t col, int sz) {
  int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
  int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
  int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;
  int px=x0, py=y0;
  
  while (1) {
    for (int bx = -sz/2; bx <= sz/2; bx++) {
      for (int by = -sz/2; by <= sz/2; by++) {
          winmgr_put_pixel(w, px + bx, py + by, col);
      }
    }
    if (px == x1 && py == y1) break;
    int e2 = err * 2;
    if (e2 > -dy) { err -= dy; px += sx; }
    if (e2 < dx)  { err += dx; py += sy; }
  }
}

void paint_draw_ellipse(int x0, int y0, int x1, int y1, uint32_t col, int bold, int preview, window_t *w) {
    long a = (x1 > x0 ? x1 - x0 : x0 - x1) / 2;
    long b = (y1 > y0 ? y1 - y0 : y0 - y1) / 2;
    if (a == 0 && b == 0) return;
    int cx = (x0 + x1) / 2;
    int cy = (y0 + y1) / 2;

    long x = 0, y = b;
    long a2 = a*a, b2 = b*b;
    long d1 = b2 - a2*b + a2/4;
    
    #define PLOT_ELLIPSE_PTS \
        for(int b_x=-bold/2; b_x<=bold/2; b_x++) { \
            for(int b_y=-bold/2; b_y<=bold/2; b_y++) { \
                if (preview && w) { \
                    winmgr_put_pixel(w, CANVAS_OFFSET_X + cx+x+b_x, CANVAS_OFFSET_Y + cy+y+b_y, col); \
                    winmgr_put_pixel(w, CANVAS_OFFSET_X + cx-x+b_x, CANVAS_OFFSET_Y + cy+y+b_y, col); \
                    winmgr_put_pixel(w, CANVAS_OFFSET_X + cx+x+b_x, CANVAS_OFFSET_Y + cy-y+b_y, col); \
                    winmgr_put_pixel(w, CANVAS_OFFSET_X + cx-x+b_x, CANVAS_OFFSET_Y + cy-y+b_y, col); \
                } else { \
                    paint_put_pixel(cx+x+b_x, cy+y+b_y, col); \
                    paint_put_pixel(cx-x+b_x, cy+y+b_y, col); \
                    paint_put_pixel(cx+x+b_x, cy-y+b_y, col); \
                    paint_put_pixel(cx-x+b_x, cy-y+b_y, col); \
                } \
            } \
        }

    while(2*b2*x <= 2*a2*y) {
       PLOT_ELLIPSE_PTS;
       x++;
       if (d1 < 0) d1 += b2*(2*x + 3);
       else { y--; d1 += b2*(2*x + 3) - a2*(2*y); }
    }
    long d2 = b2*(x+0.5)*(x+0.5) + a2*(y-1)*(y-1) - a2*b2;
    while(y >= 0) {
       PLOT_ELLIPSE_PTS;
       y--;
       if (d2 > 0) d2 += a2*(3 - 2*y);
       else { x++; d2 += b2*(2*x) + a2*(3 - 2*y); }
    }
}

void paint_draw_triangle(int x0, int y0, int x1, int y1, uint32_t col, int bold, int preview, window_t *w) {
    int cx = (x0 + x1) / 2;
    int top = (y0 < y1) ? y0 : y1;
    int btm = (y0 > y1) ? y0 : y1;
    if (preview && w) {
        paint_draw_line_ui(w, CANVAS_OFFSET_X+cx, CANVAS_OFFSET_Y+top, CANVAS_OFFSET_X+x0, CANVAS_OFFSET_Y+btm, col, bold);
        paint_draw_line_ui(w, CANVAS_OFFSET_X+cx, CANVAS_OFFSET_Y+top, CANVAS_OFFSET_X+x1, CANVAS_OFFSET_Y+btm, col, bold);
        paint_draw_line_ui(w, CANVAS_OFFSET_X+x0, CANVAS_OFFSET_Y+btm, CANVAS_OFFSET_X+x1, CANVAS_OFFSET_Y+btm, col, bold);
    } else {
        draw_line(cx, top, x0, btm, col, bold);
        draw_line(cx, top, x1, btm, col, bold);
        draw_line(x0, btm, x1, btm, col, bold);
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

void draw_alloc_rect(int x, int y, int w, int h, uint32_t color, int filled) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x + w > paint_state.canvas_w) w = paint_state.canvas_w - x;
  if (y + h > paint_state.canvas_h) h = paint_state.canvas_h - y;

  for (int cy = 0; cy < h; cy++) {
    for (int cx = 0; cx < w; cx++) {
      if (filled || cy == 0 || cy == h - 1 || cx == 0 || cx == w - 1) {
        paint_put_pixel(x + cx, y + cy, color);
      }
    }
  }
}

void paint_clear() {
  for (int i = 0; i < paint_state.canvas_w * paint_state.canvas_h; i++) {
    paint_state.pixels[i] = 0xFFFFFFFF;
  }
  if (paint_state.win) paint_state.win->needs_redraw = 1;
}


// Keyboard input for dialogs
void paint_on_key(window_t *win, int key, char ascii) {
  if (paint_state.dialog_mode > 0) {
    int len = strlen(paint_state.filename);
    if (ascii == '\n' || ascii == '\r') {
      if (paint_state.dialog_mode == 1) { // Save
        if (fs_write(paint_state.filename, (uint8_t *)paint_state.pixels,
                     paint_state.canvas_w * paint_state.canvas_h * 4)) {
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
  if (!w) return;

  // DYNAMIC CANVAS RESIZE CHECK
  int new_w = w->width - 40;
  if (new_w < 10) new_w = 10;
  int new_h = w->height - 180;
  if (new_h < 10) new_h = 10;

  if (paint_state.canvas_w != new_w || paint_state.canvas_h != new_h) {
    uint32_t *new_pixels = (uint32_t *)kmalloc(new_w * new_h * 4);
    if (new_pixels) {
       for (int i=0; i < new_w * new_h; i++) new_pixels[i] = 0xFFFFFFFF;
       for (int y=0; y < paint_state.canvas_h && y < new_h; y++) {
         for (int x=0; x < paint_state.canvas_w && x < new_w; x++) {
            new_pixels[y * new_w + x] = paint_state.pixels[y * paint_state.canvas_w + x];
         }
       }
       kfree(paint_state.pixels);
       paint_state.pixels = new_pixels;
       paint_state.canvas_w = new_w;
       paint_state.canvas_h = new_h;
    }
  }

  const theme_t *theme = theme_get();
 
  // Background
  winmgr_fill_rect(w, 0, 0, w->width, w->height, theme->bg);

  // Top Menu Bar
  winmgr_fill_rect(w, 0, 24, w->width, 30, theme->titlebar);
  winmgr_draw_text(w, 15, 34, "File", theme->titlebar_text);
  winmgr_draw_text(w, 60, 34, "Edit", theme->titlebar_text);
  winmgr_draw_text(w, 105, 34, "View", theme->titlebar_text);
  winmgr_draw_rect(w, 8, 28, 42, 22, theme->border);

  // Main Ribbon Toolbar
  winmgr_fill_rect(w, 0, 54, w->width, 60, theme->menu_bg);
  winmgr_fill_rect(w, 0, 114, w->width, 1, theme->border);

  // Section 1: Tools (3x2 Grid)
  winmgr_draw_text(w, 40, 100, "Tools", 0xFFAAAAAA);
  for (int i = 0; i < 6; i++) {
    int col = i % 3;
    int row = i / 3;
    int tx = 20 + (col * 30);
    int ty = 56 + (row * 22);
    uint32_t bg = (paint_state.current_tool == i) ? theme->accent : theme->button;
    winmgr_fill_rect(w, tx, ty, 26, 20, bg);
    winmgr_draw_rect(w, tx, ty, 26, 20, theme->border);
    
    // Pixel Art Icons!
    int ix = tx + 6;
    int iy = ty + 3;
    if (i == TOOL_PENCIL) { 
        paint_draw_line_ui(w, ix+2, iy+12, ix+10, iy+4, 0xFFEFE4B0, 1);
        paint_draw_line_ui(w, ix+3, iy+13, ix+11, iy+5, 0xFFEFE4B0, 1);
        paint_draw_line_ui(w, ix+1, iy+13, ix+2,  iy+12, 0xFF111111, 1); 
        paint_draw_line_ui(w, ix+11, iy+4, ix+12, iy+5, 0xFFFFB6C1, 1); 
    } else if (i == TOOL_ERASER) { 
        winmgr_fill_rect(w, ix+2, iy+2, 8, 8, 0xFFFFB6C1);
        winmgr_fill_rect(w, ix+6, iy+6, 4, 6, 0xFFFFFFFF);
    } else if (i == TOOL_FILL) { 
        winmgr_fill_rect(w, ix+4, iy+4, 6, 6, 0xFFAAAAAA);
        winmgr_put_pixel(w, ix+5, iy+6, 0xFF00A2E8); 
        winmgr_fill_rect(w, ix+2, iy+7, 2, 2, 0xFF00A2E8); 
    } else if (i == TOOL_PICKER) { 
        paint_draw_line_ui(w, ix+3, iy+11, ix+9, iy+5, 0xFFAAAAAA, 1); 
        winmgr_fill_rect(w, ix+9, iy+2, 4, 3, 0xFF444444); 
    } else if (i == TOOL_TEXT) { 
        winmgr_draw_text(w, ix+4, iy+4, "A", 0xFFFFFFFF);
    } else if (i == TOOL_MAGNIFY) { 
        winmgr_draw_rect(w, ix+5, iy+2, 6, 6, 0xFFFFFFFF);
        paint_draw_line_ui(w, ix+2, iy+11, ix+6, iy+7, 0xFFFFFFFF, 1);
    }
  }

  winmgr_fill_rect(w, 115, 60, 1, 40, 0xFF444444); // Divider

  // Section 2: Shapes (3x2 Grid)
  winmgr_draw_text(w, 150, 100, "Shapes", 0xFFAAAAAA);
  for (int i = 0; i < 6; i++) {
    int col = i % 3;
    int row = i / 3;
    int tx = 130 + (col * 30);
    int ty = 56 + (row * 22);
    int tool_id = SHAPE_LINE + i;
    uint32_t bg = (paint_state.current_tool == tool_id) ? 0xFF404040 : 0xFF252526;
    winmgr_fill_rect(w, tx, ty, 26, 20, bg);
    winmgr_draw_rect(w, tx, ty, 26, 20, 0xFF111111);
    
    int ix = tx + 3;
    int iy = ty + 2;
    if (tool_id == SHAPE_LINE) {
        paint_draw_line_ui(w, ix+2, iy+14, ix+14, iy+2, 0xFFFFFFFF, 1);
    } else if (tool_id == SHAPE_CURVE) {
        for (int t = 0; t < 12; t++) {
            int x_val = t - 6; 
            int y_val = (x_val * x_val * x_val) / 36; 
            winmgr_put_pixel(w, ix + 8 + x_val, iy + 7 + y_val, 0xFFFFFFFF);
            winmgr_put_pixel(w, ix + 8 + x_val, iy + 8 + y_val, 0xFFFFFFFF);
            winmgr_put_pixel(w, ix + 9 + x_val, iy + 7 + y_val, 0xFFFFFFFF);
        }
    } else if (tool_id == SHAPE_OVAL) {
        for (int dy = -5; dy <= 5; dy++) {
           for (int dx = -6; dx <= 6; dx++) {
              int d = dx*dx + dy*dy;
              if (d >= 15 && d <= 23) winmgr_put_pixel(w, ix + 8 + dx, iy + 8 + dy, 0xFFFFFFFF);
           }
        }
    } else if (tool_id == SHAPE_RECT) {
        winmgr_draw_rect(w, ix+3, iy+4, 12, 8, 0xFFFFFFFF);
        winmgr_draw_rect(w, ix+4, iy+5, 10, 6, 0xFFFFFFFF); // Bold border
    } else if (tool_id == SHAPE_TRIANGLE) {
        paint_draw_line_ui(w, ix+10, iy+2, ix+3, iy+14, 0xFFFFFFFF, 1);
        paint_draw_line_ui(w, ix+10, iy+2, ix+17, iy+14, 0xFFFFFFFF, 1);
        paint_draw_line_ui(w, ix+3, iy+14, ix+17, iy+14, 0xFFFFFFFF, 1);
    } else if (tool_id == SHAPE_DIAMOND) {
        paint_draw_line_ui(w, ix+10, iy+2, ix+3, iy+8, 0xFFFFFFFF, 1);
        paint_draw_line_ui(w, ix+10, iy+2, ix+17, iy+8, 0xFFFFFFFF, 1);
        paint_draw_line_ui(w, ix+3, iy+8, ix+10, iy+14, 0xFFFFFFFF, 1);
        paint_draw_line_ui(w, ix+17, iy+8, ix+10, iy+14, 0xFFFFFFFF, 1);
    }
  }

  // Clear Button
  winmgr_fill_rect(w, 230, 80, 50, 16, 0xFF252526);
  winmgr_draw_rect(w, 230, 80, 50, 16, 0xFF111111);
  winmgr_draw_text(w, 230 + 4, 80 + 4, "Clear", 0xFFFF6666);

  winmgr_fill_rect(w, 290, 60, 1, 40, 0xFF444444); 

  // Section 2: Size
  winmgr_draw_text(w, 305, 100, "Size", theme->fg_secondary);
  winmgr_fill_rect(w, 295, 65, 45, 20, theme->input_bg);
  winmgr_draw_rect(w, 295, 65, 45, 20, theme->input_border);
  int s = paint_state.brush_size;
  winmgr_fill_rect(w, 317 - (s / 2), 75 - (s / 2), s, s, 0xFFFFFFFF);

  winmgr_fill_rect(w, 355, 60, 1, 40, 0xFF444444); 

  // Section 3: Color Palette
  winmgr_draw_text(w, 465, 100, "Colors", 0xFFAAAAAA);
  winmgr_fill_rect(w, 370, 60, 30, 30, paint_state.current_color);
  winmgr_draw_rect(w, 370, 60, 30, 30, 0xFFFFFFFF); 

  uint32_t colors[] = {
      0xFF000000, 0xFF7F7F7F, 0xFF880015, 0xFFED1C24, 0xFFFF7F27, 0xFFFFF200, 0xFF22B14C, 0xFF00A2E8, 0xFF3F48CC, 0xFFA349A4,
      0xFFFFFFFF, 0xFFC3C3C3, 0xFFB97A57, 0xFFFFAEC9, 0xFFFFC90E, 0xFFEFE4B0, 0xFFB5E61D, 0xFF99D9EA, 0xFF7092BE, 0xFFC8BFE7
  };
  
  for (int i = 0; i < 20; i++) {
     int row = i / 10;
     int col = i % 10;
     int cx = 415 + col * 20;
     int cy = 60 + row * 20;
     winmgr_fill_rect(w, cx, cy, 16, 16, colors[i]);
     winmgr_draw_rect(w, cx, cy, 16, 16, 0xFF111111);
     if (paint_state.current_color == colors[i]) {
       winmgr_draw_rect(w, cx - 1, cy - 1, 18, 18, 0xFFFFFFFF);
     }
  }

  winmgr_fill_rect(w, 625, 60, 1, 40, 0xFF444444); // Divider

  // Save/Open Actions
  winmgr_fill_rect(w, 635, 60, 35, 18, 0xFF18181A);
  winmgr_draw_rect(w, 635, 60, 35, 18, 0xFF444444);
  winmgr_draw_text(w, 638, 65, "Save", 0xFFFFFFFF);

  winmgr_fill_rect(w, 635, 82, 35, 18, 0xFF18181A);
  winmgr_draw_rect(w, 635, 82, 35, 18, 0xFF444444);
  winmgr_draw_text(w, 638, 87, "Open", 0xFFFFFFFF);

  // Canvas Frame
  winmgr_draw_rect(w, CANVAS_OFFSET_X - 1, CANVAS_OFFSET_Y - 1, paint_state.canvas_w + 2, paint_state.canvas_h + 2, 0xFF000000);

  // Canvas Content
  for (int y = 0; y < paint_state.canvas_h; y++) {
    for (int x = 0; x < paint_state.canvas_w; x++) {
      winmgr_put_pixel(w, CANVAS_OFFSET_X + x, CANVAS_OFFSET_Y + y, paint_state.pixels[y * paint_state.canvas_w + x]);
    }
  }

  // PREVIEW GHOST SHAPE (Draw over window surface directly)
  if (paint_state.is_drawing && paint_state.current_tool >= SHAPE_LINE) {
     uint32_t col = paint_state.current_color;
     int x0 = paint_state.start_x, y0 = paint_state.start_y;
     int x1 = paint_state.last_x,  y1 = paint_state.last_y;
     int sz = paint_state.brush_size;
     
     if (paint_state.current_tool == SHAPE_LINE) {
        paint_draw_line_ui(w, CANVAS_OFFSET_X + x0, CANVAS_OFFSET_Y + y0, CANVAS_OFFSET_X + x1, CANVAS_OFFSET_Y + y1, col, sz);
     } else if (paint_state.current_tool == SHAPE_CURVE) {
        paint_draw_line_ui(w, CANVAS_OFFSET_X + x0, CANVAS_OFFSET_Y + y0, CANVAS_OFFSET_X + x1, CANVAS_OFFSET_Y + y1, col, sz); // Simplistic preview for curve block
     } else if (paint_state.current_tool == SHAPE_RECT || paint_state.current_tool == SHAPE_DIAMOND) {
        int left = (x0 < x1) ? x0 : x1;
        int top = (y0 < y1) ? y0 : y1;
        int right = (x1 > x0) ? x1 : x0;
        int bottom = (y1 > y0) ? y1 : y0;
        
        if (paint_state.current_tool == SHAPE_RECT) {
            paint_draw_line_ui(w, CANVAS_OFFSET_X+left, CANVAS_OFFSET_Y+top, CANVAS_OFFSET_X+right, CANVAS_OFFSET_Y+top, col, sz);
            paint_draw_line_ui(w, CANVAS_OFFSET_X+right, CANVAS_OFFSET_Y+top, CANVAS_OFFSET_X+right, CANVAS_OFFSET_Y+bottom, col, sz);
            paint_draw_line_ui(w, CANVAS_OFFSET_X+right, CANVAS_OFFSET_Y+bottom, CANVAS_OFFSET_X+left, CANVAS_OFFSET_Y+bottom, col, sz);
            paint_draw_line_ui(w, CANVAS_OFFSET_X+left, CANVAS_OFFSET_Y+bottom, CANVAS_OFFSET_X+left, CANVAS_OFFSET_Y+top, col, sz);
        } else {
            int cx = (left + right)/2, cy = (top + bottom)/2;
            paint_draw_line_ui(w, CANVAS_OFFSET_X+cx, CANVAS_OFFSET_Y+top, CANVAS_OFFSET_X+right, CANVAS_OFFSET_Y+cy, col, sz);
            paint_draw_line_ui(w, CANVAS_OFFSET_X+right, CANVAS_OFFSET_Y+cy, CANVAS_OFFSET_X+cx, CANVAS_OFFSET_Y+bottom, col, sz);
            paint_draw_line_ui(w, CANVAS_OFFSET_X+cx, CANVAS_OFFSET_Y+bottom, CANVAS_OFFSET_X+left, CANVAS_OFFSET_Y+cy, col, sz);
            paint_draw_line_ui(w, CANVAS_OFFSET_X+left, CANVAS_OFFSET_Y+cy, CANVAS_OFFSET_X+cx, CANVAS_OFFSET_Y+top, col, sz);
        }
     } else if (paint_state.current_tool == SHAPE_OVAL) {
        paint_draw_ellipse(x0, y0, x1, y1, col, sz, 1, w);
     } else if (paint_state.current_tool == SHAPE_TRIANGLE) {
        paint_draw_triangle(x0, y0, x1, y1, col, sz, 1, w);
     }
  }

  // --- BOTTOM STATUS BAR ---
  winmgr_fill_rect(w, 0, w->height - 24, w->width, 24, theme->titlebar_inactive);
  winmgr_fill_rect(w, 0, w->height - 24, w->width, 1, theme->border);
  
  char dims[64];
  k_itoa(paint_state.canvas_w, dims);
  strcat(dims, " x ");
  char dhs[16];
  k_itoa(paint_state.canvas_h, dhs);
  strcat(dims, dhs);
  strcat(dims, " px");
  winmgr_draw_text(w, 30, w->height - 16, dims, 0xFFAAAAAA);
  winmgr_draw_text(w, w->width - 80, w->height - 16, "[    100%    ]", 0xFFAAAAAA);

  // Dialog Overlay
  if (paint_state.dialog_mode > 0) {
    int dw = 300, dh = 120;
    int dx = (w->width - dw) / 2;
    int dy = (w->height - dh) / 2;

    winmgr_fill_rect(w, dx + 5, dy + 5, dw, dh, 0xFF111111); // Shadow
    winmgr_fill_rect(w, dx, dy, dw, dh, 0xFF2C313A); // Dark Dialog BG
    winmgr_draw_rect(w, dx, dy, dw, dh, 0xFF00A2E8); // Accent Border

    const char *dtitle = (paint_state.dialog_mode == 1) ? "Save Canvas As:" : "Open Image Path:";
    winmgr_draw_text(w, dx + 15, dy + 15, dtitle, 0xFF00A2E8);

    winmgr_fill_rect(w, dx + 15, dy + 50, dw - 30, 25, 0xFFFFFFFF);
    winmgr_draw_text(w, dx + 20, dy + 58, paint_state.filename, 0xFF000000);

    int flen = strlen(paint_state.filename);
    if ((paint_state.cursor_timer++ / 10) % 2 == 0) {
      winmgr_fill_rect(w, dx + 20 + (flen * 8), dy + 55, 2, 15, 0xFF000000);
    }

    winmgr_draw_text(w, dx + 15, dy + 90, "ENTER: Confirm   |   ESC: Cancel", 0xFFAAAAAA);
  }
}

void paint_handle_mouse(window_t *w, int mx, int my, int buttons) {
  int rel_x = mx;
  int rel_y = my;

  if (paint_state.dialog_mode > 0) return; // Prevent interaction when dialog active

  // Top Ribbon Selection
  if (rel_y >= 54 && rel_y <= 114 && (buttons & 1)) {
    // Tools (0-5)
    for (int i = 0; i < 6; i++) {
      int col = i % 3;
      int row = i / 3;
      int tx = 20 + (col * 30);
      int ty = 56 + (row * 22);
      if (rel_x >= tx && rel_x <= tx + 26 && rel_y >= ty && rel_y <= ty + 20) {
        paint_state.current_tool = i;
        w->needs_redraw = 1;
        return;
      }
    }
    
    // Shapes (6-11)
    for (int i = 0; i < 6; i++) {
      int col = i % 3;
      int row = i / 3;
      int tx = 130 + (col * 30);
      int ty = 56 + (row * 22);
      if (rel_x >= tx && rel_x <= tx + 26 && rel_y >= ty && rel_y <= ty + 20) {
        paint_state.current_tool = SHAPE_LINE + i;
        w->needs_redraw = 1;
        return;
      }
    }
    
    // Clear All
    if (rel_x >= 230 && rel_x <= 280 && rel_y >= 80 && rel_y <= 96) {
      paint_clear();
      w->needs_redraw = 1;
      return;
    }

    // Size Menu (Toggle 1->3->5->7->9)
    if (rel_x >= 295 && rel_x <= 340 && rel_y >= 65 && rel_y <= 85) {
      paint_state.brush_size += 2;
      if (paint_state.brush_size > 9) paint_state.brush_size = 1;
      w->needs_redraw = 1;
      // Soft debounce wait would belong here, but redraw 1 usually limits spamming
      return;
    }

    // Colors
    uint32_t colors[] = {
      0xFF000000, 0xFF7F7F7F, 0xFF880015, 0xFFED1C24, 0xFFFF7F27, 0xFFFFF200, 0xFF22B14C, 0xFF00A2E8, 0xFF3F48CC, 0xFFA349A4,
      0xFFFFFFFF, 0xFFC3C3C3, 0xFFB97A57, 0xFFFFAEC9, 0xFFFFC90E, 0xFFEFE4B0, 0xFFB5E61D, 0xFF99D9EA, 0xFF7092BE, 0xFFC8BFE7
    };
    for (int i = 0; i < 20; i++) {
       int row = i / 10;
       int col = i % 10;
       int cx = 415 + col * 20;
       int cy = 60 + row * 20;
       if (rel_x >= cx && rel_x <= cx + 16 && rel_y >= cy && rel_y <= cy + 16) {
          paint_state.current_color = colors[i];
          w->needs_redraw = 1;
          return;
       }
    }

    // Save
    if (rel_x >= 635 && rel_x <= 670 && rel_y >= 60 && rel_y <= 78) {
       paint_state.dialog_mode = 1;
       paint_state.cursor_timer = 0;
       w->needs_redraw = 1;
       return;
    }
    // Open
    if (rel_x >= 635 && rel_x <= 670 && rel_y >= 82 && rel_y <= 100) {
       paint_state.dialog_mode = 2;
       paint_state.cursor_timer = 0;
       w->needs_redraw = 1;
       return;
    }
  }

  // 3. Canvas Interaction
  int cx = rel_x - CANVAS_OFFSET_X;
  int cy = rel_y - CANVAS_OFFSET_Y;

  if (cx >= 0 && cx < paint_state.canvas_w && cy >= 0 && cy < paint_state.canvas_h) {
    if (buttons & 1) {
      uint32_t col = (paint_state.current_tool == TOOL_ERASER) ? 0xFFFFFFFF : paint_state.current_color;

      if (!paint_state.is_drawing) {
        if (paint_state.current_tool == TOOL_FILL) {
            paint_flood_fill(cx, cy, paint_state.current_color);
            w->needs_redraw = 1;
            return;
        }

        paint_state.is_drawing = 1;
        paint_state.start_x = cx;
        paint_state.start_y = cy;
        paint_state.last_x = cx;
        paint_state.last_y = cy;
        if (paint_state.current_tool == TOOL_PENCIL || paint_state.current_tool == TOOL_ERASER) {
          draw_line(cx, cy, cx, cy, col, paint_state.brush_size);
        }
      } else {
        if (paint_state.current_tool == TOOL_PENCIL || paint_state.current_tool == TOOL_ERASER) {
          draw_line(paint_state.last_x, paint_state.last_y, cx, cy, col, paint_state.brush_size);
          paint_state.last_x = cx;
          paint_state.last_y = cy;
        } else {
          // Previews are drawn in paint_draw_ui, just track coordinates here
          paint_state.last_x = cx;
          paint_state.last_y = cy;
          w->needs_redraw = 1;
        }
      }
    } else {
      // Mouse Released
      if (paint_state.is_drawing) {
        uint32_t col = paint_state.current_color;
        if (paint_state.current_tool == SHAPE_LINE || paint_state.current_tool == SHAPE_CURVE) {
          draw_line(paint_state.start_x, paint_state.start_y, cx, cy, col, paint_state.brush_size);
        } else if (paint_state.current_tool == SHAPE_RECT) {
          int x = (paint_state.start_x < cx) ? paint_state.start_x : cx;
          int y = (paint_state.start_y < cy) ? paint_state.start_y : cy;
          int bw = (cx > paint_state.start_x) ? cx - paint_state.start_x : paint_state.start_x - cx;
          int bh = (cy > paint_state.start_y) ? cy - paint_state.start_y : paint_state.start_y - cy;
          draw_alloc_rect(x, y, bw, bh, col, 0);
        } else if (paint_state.current_tool == SHAPE_DIAMOND) {
          int left = (paint_state.start_x < cx) ? paint_state.start_x : cx;
          int top = (paint_state.start_y < cy) ? paint_state.start_y : cy;
          int right = (cx > paint_state.start_x) ? cx : paint_state.start_x;
          int bottom = (cy > paint_state.start_y) ? cy : paint_state.start_y;
          int sx = (left + right)/2, sy = (top + bottom)/2;
          draw_line(sx, top, right, sy, col, paint_state.brush_size);
          draw_line(right, sy, sx, bottom, col, paint_state.brush_size);
          draw_line(sx, bottom, left, sy, col, paint_state.brush_size);
          draw_line(left, sy, sx, top, col, paint_state.brush_size);
        } else if (paint_state.current_tool == SHAPE_OVAL) {
          paint_draw_ellipse(paint_state.start_x, paint_state.start_y, cx, cy, col, paint_state.brush_size, 0, w);
        } else if (paint_state.current_tool == SHAPE_TRIANGLE) {
          paint_draw_triangle(paint_state.start_x, paint_state.start_y, cx, cy, col, paint_state.brush_size, 0, w);
        }
        paint_state.is_drawing = 0;
        w->needs_redraw = 1;
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
  paint_state.win = winmgr_create_window(-1, -1, 900, 700, "Untitled - Paint");
  if (!paint_state.win)
    return;
  paint_state.win->app_type = 4; // Paint
  paint_state.current_color = 0xFF000000;
  paint_state.current_tool = TOOL_PENCIL;
  paint_state.brush_size = 3;
  paint_state.is_drawing = 0;
  paint_state.dialog_mode = 0;
  
  paint_state.canvas_w = 640;
  paint_state.canvas_h = 360;

  paint_state.pixels = (uint32_t *)kmalloc(paint_state.canvas_w * paint_state.canvas_h * 4);
  if (!paint_state.pixels) {
    winmgr_close_window(paint_state.win);
    paint_state.win = 0;
    return;
  }
  paint_clear();

  paint_state.win->draw = (void (*)(void *))paint_draw_ui;
  paint_state.win->on_mouse = (void (*)(void *, int, int, int))paint_handle_mouse;
  paint_state.win->on_key = (void (*)(void *, int, char))paint_on_key;
  paint_state.win->on_scroll = (void (*)(void *, int))paint_on_scroll;
  paint_state.win->on_close = paint_on_close;
}
