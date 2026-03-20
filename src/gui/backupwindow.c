#include "window.h"
#include "../drivers/vga.h"
#include "../drivers/font.h" 
#include "../drivers/rtc.h"
#include "../drivers/mouse.h"
#include "icons_data.h"

// Colors
#define COLOR_DESKTOP 0x03 
#define COLOR_WIN_BG  0x07 
#define COLOR_WIN_TITLE_BG 0x01
#define COLOR_WIN_TITLE_TEXT 0x0F
#define COLOR_WIN_BORDER 0x08 

// ================= STRUCTS =================
typedef struct {
    int x, y;           // Window position
    int width, height;  // Window size
    char title[32];     // Window title
    int is_dragging;    // Is window being dragged?
    int drag_offset_x;  // Mouse offset within window
    int drag_offset_y;
    int is_visible;     // Is window visible?
} Window;

// GLOBALS
Window my_window;
Window* active_window = NULL;

#define MAX_ICONS 5
static Icon icons[MAX_ICONS];
static int num_icons = 0;

// ================= HELPERS =================

int my_strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// ================= DRAWING =================

void draw_window_struct(Window* win) {
    if (!win->is_visible) return;

    // Draw window background
    vga_draw_rect(win->x, win->y, win->width, win->height, COLOR_WIN_BG);
    
    // Draw title bar (Red if dragging, else Blue)
    uint8_t title_color = win->is_dragging ? 0x28 : COLOR_WIN_TITLE_BG;
    vga_draw_rect(win->x + 1, win->y + 1, win->width - 2, 18, title_color);
    
    // Draw Title Text
    int text_x = win->x + 6;
    int text_y = win->y + 5; 
    const char* p = win->title;
    while (*p) {
        vga_draw_char(text_x, text_y, *p, COLOR_WIN_TITLE_TEXT);
        text_x += 4;
        p++;
    }
    
    // Draw Close Button (Red X)
    int btn_size = 12;
    int btn_x = win->x + win->width - btn_size - 4;
    int btn_y = win->y + 4;
    vga_draw_rect(btn_x, btn_y, btn_size, btn_size, 0x04); // Red
    vga_draw_char(btn_x + 4, btn_y + 3, 'x', 0x0F);
}

void draw_icon(Icon* icon) {
    for(int y=0; y<16; y++) {
        for(int x=0; x<16; x++) {
            uint8_t pixel = icon->icon_data[y * 16 + x];
            if (pixel != 0) {
                uint8_t color = pixel;
                if (color == 1) color = 0x00;
                else if (color == 2) color = 0x0F; 
                else if (color == 3) color = 0x01; 
                else if (color == 4) color = 0x07; 
                vga_put_pixel(icon->x + x, icon->y + y, color);
            } else if (icon->selected) {
                 vga_put_pixel(icon->x + x, icon->y + y, 0x01);
            } else {
                 vga_put_pixel(icon->x + x, icon->y + y, COLOR_DESKTOP);
            }
        }
    }
    
    // Draw Label
    int label_len = my_strlen(icon->label);
    int text_width = label_len * 4;
    int icon_center = icon->x + 8; 
    int label_x = icon_center - (text_width / 2);
    int label_y = icon->y + 18;
    const char* p = icon->label;
    int lx = label_x;
    while(*p) {
        vga_draw_char(lx, label_y, *p, 0x0F);
        lx += 4;
        p++;
    }
}

void redraw_icons() {
    for(int i=0; i<num_icons; i++) {
        draw_icon(&icons[i]);
    }
}

void refresh_screen() {
    // Ideally we would redraw everything back-to-front
    // For now, assume cleared background
    // We already do partial redraws in the logic, but let's be safe
    // vga_clear_screen(COLOR_DESKTOP); // Too slow to do full clear every frame
    // redraw_icons();
    // draw_window_struct(&my_window);
}

// ================= LOGIC =================

void init_icons() {
    num_icons = 0;
    
    icons[num_icons].x = 10;
    icons[num_icons].y = 20;
    icons[num_icons].w = 16;
    icons[num_icons].h = 16;
    icons[num_icons].label = "Computer";
    icons[num_icons].icon_data = (const uint8_t*)icon_computer;
    icons[num_icons].selected = 0;
    num_icons++;
    
    icons[num_icons].x = 10;
    icons[num_icons].y = 60; 
    icons[num_icons].w = 16;
    icons[num_icons].h = 16;
    icons[num_icons].label = "Terminal";
    icons[num_icons].icon_data = (const uint8_t*)icon_terminal;
    icons[num_icons].selected = 0;
    num_icons++;
}

void gui_init() {
    // Init Window
    my_window.x = 100;
    my_window.y = 50;
    my_window.width = 200;
    my_window.height = 100;
    my_window.is_visible = 1;
    my_window.is_dragging = 0;
    // Copy title manually or use strcpy if available (we don't have it)
    const char* t = "Welcome to PureOS";
    int i = 0;
    while(t[i] && i < 31) { my_window.title[i] = t[i]; i++; }
    my_window.title[i] = 0;
    
    vga_clear_screen(COLOR_DESKTOP);
    vga_draw_rect(0, 190, 320, 10, 0x17); // Taskbar
    vga_draw_rect(0, 190, 320, 1, 0x0F);
    
    init_icons();
    redraw_icons();
    draw_window_struct(&my_window);
}

// Helper to draw number (for debug)
void draw_num(int x, int y, int num) {
    char buf[16];
    int i = 0;
    if (num == 0) { buf[i++] = '0'; }
    else {
        int temp = num;
        if (temp < 0) {
             vga_draw_char(x, y, '-', 0x0F);
             x += 4;
             temp = -temp;
        }
        int div = 1;
        while (temp / div >= 10) div *= 10;
        while (div > 0) {
            buf[i++] = (temp / div) + '0';
            temp %= div;
            div /= 10;
        }
    }
    buf[i] = 0;
    for(int k=0; k<i; k++) {
        vga_draw_char(x + (k*4), y, buf[k], 0x0F);
    }
}

// User-supplied Logic Structure
void handle_mouse_click(int mouse_x, int mouse_y, int buttons) {
    // Check Close Button
    if (my_window.is_visible) {
        int btn_size = 12;
        int win_x = my_window.x;
        int win_y = my_window.y;
        int btn_x = win_x + my_window.width - btn_size - 4;
        int btn_y = win_y + 4;
        
        if (mouse_x >= btn_x && mouse_x <= btn_x + btn_size &&
            mouse_y >= btn_y && mouse_y <= btn_y + btn_size) {
            
            my_window.is_visible = 0;
            // Clear area
            vga_draw_rect(win_x, win_y, my_window.width, my_window.height, COLOR_DESKTOP);
            redraw_icons();
            return;
        }
    }

    // Check Window Drag (Title Bar)
    if (my_window.is_visible) {
        if (mouse_x >= my_window.x && 
            mouse_x <= my_window.x + my_window.width &&
            mouse_y >= my_window.y && 
            mouse_y <= my_window.y + 20) { // Title bar height
            
            my_window.is_dragging = 1;
            my_window.drag_offset_x = mouse_x - my_window.x;
            my_window.drag_offset_y = mouse_y - my_window.y;
            active_window = &my_window;
            
            draw_window_struct(&my_window); // Redraw to update title color
        }
    }
    
    // Check Icons
    if (!active_window || !active_window->is_dragging) {
         for(int i=0; i<num_icons; i++) {
             if (mouse_x >= icons[i].x && mouse_x < icons[i].x + 16 &&
                 mouse_y >= icons[i].y && mouse_y < icons[i].y + 16) {
                 icons[i].selected = 1;
             } else {
                 icons[i].selected = 0;
             }
             draw_icon(&icons[i]);
         }
    }
}

void handle_mouse_move(int mouse_x, int mouse_y) {
    if (active_window && active_window->is_dragging) {
        // Erase old position
        vga_draw_rect(active_window->x, active_window->y, active_window->width, active_window->height, COLOR_DESKTOP);
        redraw_icons();
        
        active_window->x = mouse_x - active_window->drag_offset_x;
        active_window->y = mouse_y - active_window->drag_offset_y;
        
        // Redraw
        draw_window_struct(active_window);
    }
}

void handle_mouse_release() {
    if (active_window) {
        active_window->is_dragging = 0;
        draw_window_struct(active_window); // Redraw normal title color
        active_window = NULL; // Clear active
    }
}

void gui_update() {
    // 1. Debug Overlay
    vga_draw_rect(240, 0, 80, 20, 0x00); 
    int mx = get_mouse_x();
    int my = get_mouse_y();
    int btns = get_mouse_buttons();
    
    draw_num(245, 2, get_mouse_byte0()); // RAW B0
    draw_num(265, 2, get_mouse_byte1()); // RAW B1
    draw_num(285, 2, get_mouse_byte2()); // RAW B2
    draw_num(305, 2, btns);
    
    // RAW CLICK DEBUG (Green CLK)
    if (btns & 1) {
        vga_draw_char(290, 12, 'C', 0x02); 
        vga_draw_char(294, 12, 'L', 0x02);
        vga_draw_char(298, 12, 'K', 0x02);
    }

    // 2. Input Logic
    static int prev_btns = 0;
    int left_click = (btns & 1) && !(prev_btns & 1); // Click (Rising)
    int left_release = !(btns & 1) && (prev_btns & 1); // Release (Falling)
    static int last_x = -1, last_y = -1;
    
    // A. Handle Click
    if (left_click) {
        handle_mouse_click(mx, my, btns);
    }
    
    // B. Handle Release
    if (left_release) {
        handle_mouse_release();
    }
    
    // C. Handle Move (or Drag loop)
    if (mx != last_x || my != last_y) {
        handle_mouse_move(mx, my);
        last_x = mx;
        last_y = my;
    }
    
    prev_btns = btns;
}
