#include "../kernel/ui_layout.h"
#include "../fs/fs.h"
#include "../kernel/heap.h"
#include "../kernel/image.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"

#include "../include/math.h"

extern uint8_t key_state[256];

// Game Structures
typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    float angular_velocity;
    float wheel_dist;
    float tire_radius;
    float tire_angle_f;
    float tire_angle_b;
} RigidCar;

#define MAX_COINS 50
typedef struct {
    float x, y;
    int active;
    int value;
} Coin;

typedef struct {
    window_t *win;
    
    // Images
    uint32_t *img_car;
    int car_w, car_h;
    
    uint32_t *img_tire;
    int tire_w, tire_h;
    
    uint32_t *img_ground;
    int ground_w, ground_h;
    
    uint32_t *img_surf;
    int surf_w, surf_h;
    
    uint32_t *img_coin5; int coin5_w, coin5_h;
    uint32_t *img_coin25; int coin25_w, coin25_h;
    uint32_t *img_coin100; int coin100_w, coin100_h;
    uint32_t *img_coin500; int coin500_w, coin500_h;
    
    Coin coins[MAX_COINS];
    float last_coin_gen_x;
    int coins_collected;
    
    RigidCar car;
    
    float scroll_x;
    int score;
    int game_over;
} hillclimb_app_t;

window_t *hillclimb_win = 0;

static inline hillclimb_app_t *get_hc(void *w) {
    return (hillclimb_app_t *)((window_t *)w)->user_data;
}

static uint32_t* load_png(const char *filename, int *w, int *h) {
    uint8_t *raw_data = (uint8_t *)kmalloc(1024*1024);
    if (!raw_data) return 0;
    int size = fs_read(filename, raw_data);
    if (size <= 0) { kfree(raw_data); return 0; }
    
    int n;
    unsigned char *pixels = stbi_load_from_memory(raw_data, size, w, h, &n, 4);
    kfree(raw_data);
    
    if (pixels) {
        uint32_t *p32 = (uint32_t *)pixels;
        for (int i=0; i<(*w)*(*h); i++) {
            uint32_t p = p32[i];
            uint32_t r = p & 0xFF;
            uint32_t g = (p >> 8) & 0xFF;
            uint32_t b = (p >> 16) & 0xFF;
            uint32_t a = (p >> 24) & 0xFF;
            p32[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    return (uint32_t*)pixels;
}

static void draw_rotated(window_t *win, uint32_t *src, int w, int h, int dx, int dy, float angle, float scale, int mirror_x) {
    if (!src) return;
    float s = sinf(angle);
    float c = cosf(angle);
    
    int max_radius = (int)(sqrtf(w*w + h*h) / 2 * scale) + 1;
    int cx = w / 2;
    int cy = h / 2;
    
    for (int y = -max_radius; y <= max_radius; y++) {
        for (int x = -max_radius; x <= max_radius; x++) {
            float sx = (x * c + y * s) / scale;
            float sy = (-x * s + y * c) / scale;
            
            int src_x = (int)(cx + (mirror_x ? -sx : sx));
            int src_y = (int)(cy + sy);
            
            if (src_x >= 0 && src_x < w && src_y >= 0 && src_y < h) {
                uint32_t p = src[src_y * w + src_x];
                if ((p >> 24) > 128) {
                    winmgr_put_pixel(win, dx + x, dy + y, p);
                }
            }
        }
    }
}

// Procedural Terrain Function (Countryside map)
static float get_terrain_y(float x) {
    // Large, smooth rolling hills
    float h1 = sinf(x * 0.002f) * 150.0f;
    // Medium variations
    float h2 = sinf(x * 0.005f) * 50.0f;
    // Small bumps
    float h3 = sinf(x * 0.015f) * 15.0f;
    return 400.0f - (h1 + h2 + h3);
}

static void reset_game(hillclimb_app_t *app) {
    app->car.x = 200.0f;
    app->car.y = get_terrain_y(200.0f) - 50.0f;
    app->car.vx = 0; app->car.vy = 0;
    app->car.angle = 0;
    app->car.angular_velocity = 0;
    app->car.wheel_dist = 45.0f;
    app->car.tire_radius = 20.0f;
    app->car.tire_angle_f = 0;
    app->car.tire_angle_b = 0;
    
    app->scroll_x = 0;
    app->score = 0;
    app->game_over = 0;
    app->last_coin_gen_x = 400.0f;
    app->coins_collected = 0;
    for (int i=0; i<MAX_COINS; i++) app->coins[i].active = 0;
}

static void spawn_coin_group(hillclimb_app_t *app, float start_x, int value, int shape) {
    int spawned = 0;
    int max_to_spawn = (shape == 2) ? 10 : 5;
    for (int i = 0; i < MAX_COINS; i++) {
        if (!app->coins[i].active) {
            float cx = 0, cy = 0;
            
            if (shape == 0) { // Small Line
                cx = start_x + spawned * 70.0f;
                cy = get_terrain_y(cx) - 45.0f;
            } else if (shape == 1) { // High Arc
                cx = start_x + spawned * 100.0f;
                cy = get_terrain_y(cx) - 45.0f - sinf(spawned * M_PI / 4.0f) * 150.0f;
            } else if (shape == 2) { // Long Line
                cx = start_x + spawned * 60.0f;
                cy = get_terrain_y(cx) - 45.0f;
            } else if (shape == 3) { // Triangle
                if (spawned == 0) { cx = start_x; cy = get_terrain_y(cx) - 45.0f; }
                else if (spawned == 1) { cx = start_x - 30; cy = get_terrain_y(cx) - 85.0f; }
                else if (spawned == 2) { cx = start_x + 30; cy = get_terrain_y(cx) - 85.0f; }
                else if (spawned == 3) { cx = start_x - 60; cy = get_terrain_y(cx) - 125.0f; }
                else if (spawned == 4) { cx = start_x + 60; cy = get_terrain_y(cx) - 125.0f; }
            }
            
            app->coins[i].x = cx;
            app->coins[i].y = cy;
            app->coins[i].value = value;
            app->coins[i].active = 1;
            spawned++;
            if (spawned >= max_to_spawn) break;
        }
    }
}

void hillclimb_update(window_t *win) {
    hillclimb_app_t *app = get_hc(win);
    if (!app) return;
    
    if (app->game_over) {
        if (key_state[0x39]) { // Spacebar
            reset_game(app);
        }
        return;
    }
    
    float engine_power = 0;
    if (key_state[0x4D]) engine_power = 0.25f;
    if (key_state[0x4B]) engine_power = -0.2f;
    
    RigidCar *car = &app->car;
    
    // Gravity & Air friction
    car->vy += 0.4f;
    car->vx *= 0.99f;
    car->vy *= 0.99f;
    car->angular_velocity *= 0.95f;
    
    car->x += car->vx;
    car->y += car->vy;
    car->angle += car->angular_velocity;
    
    // Normalize angle between -PI and PI
    while (car->angle > M_PI) car->angle -= 2.0f * M_PI;
    while (car->angle < -M_PI) car->angle += 2.0f * M_PI;
    
    float cos_a = cosf(car->angle);
    float sin_a = sinf(car->angle);
    
    float fx = car->x + car->wheel_dist * cos_a;
    float fy = car->y + car->wheel_dist * sin_a;
    float bx = car->x - car->wheel_dist * cos_a;
    float by = car->y - car->wheel_dist * sin_a;
    
    int front_hit = 0, back_hit = 0;
    
    float tf = get_terrain_y(fx);
    if (fy + car->tire_radius > tf) {
        float depth = (fy + car->tire_radius) - tf;
        car->y -= depth * 0.5f;
        car->vy -= depth * 0.1f;
        car->angular_velocity -= depth * 0.003f;
        
        float dy = get_terrain_y(fx + 5.0f) - tf;
        float len = sqrtf(25.0f + dy*dy);
        if ((engine_power > 0 && car->vx < 12.0f) || (engine_power < 0 && car->vx > -6.0f)) {
            car->vx += engine_power * (5.0f / len);
            car->vy += engine_power * (dy / len);
        }
        car->tire_angle_f += car->vx * 0.1f;
        front_hit = 1;
    }
    
    float tb = get_terrain_y(bx);
    if (by + car->tire_radius > tb) {
        float depth = (by + car->tire_radius) - tb;
        car->y -= depth * 0.5f;
        car->vy -= depth * 0.1f;
        car->angular_velocity += depth * 0.003f;
        
        float dy = get_terrain_y(bx + 5.0f) - tb;
        float len = sqrtf(25.0f + dy*dy);
        if ((engine_power > 0 && car->vx < 12.0f) || (engine_power < 0 && car->vx > -6.0f)) {
            car->vx += engine_power * (5.0f / len);
            car->vy += engine_power * (dy / len);
        }
        car->tire_angle_b += car->vx * 0.1f;
        back_hit = 1;
    }
    
    if (front_hit || back_hit) {
        float dy = get_terrain_y(car->x + 5.0f) - get_terrain_y(car->x);
        float len2 = 25.0f + dy*dy;
        // Gravity pulls down the slope (increased to make hills harder)
        car->vx += 0.8f * dy * 5.0f / len2;
    } else {
        if (key_state[0x4D]) car->angular_velocity -= 0.02f;
        if (key_state[0x4B]) car->angular_velocity += 0.02f;
    }
    
    // Only die if upside down AND hitting the ground (roof collision check)
    if (car->angle > 2.0f || car->angle < -2.0f) {
        if (car->y > get_terrain_y(car->x) - 40.0f) {
            app->game_over = 1;
        }
    }
    
    // Generate Coins
    if (car->x > app->last_coin_gen_x) {
        int r = ((int)car->x / 1000) % 10;
        int value = 5;
        if (r == 3 || r == 7) value = 25;
        else if (r == 5) value = 100;
        else if (r == 9) value = 500;
        
        int shape = ((int)car->x / 1337) % 4; // Pseudo-random shape based on X
        spawn_coin_group(app, app->last_coin_gen_x + 600.0f, value, shape);
        app->last_coin_gen_x += (shape == 2) ? 1400.0f : 1000.0f;
    }
    
    // Check Coin Collisions
    for (int i = 0; i < MAX_COINS; i++) {
        if (app->coins[i].active) {
            float dx = car->x - app->coins[i].x;
            float dy = car->y - app->coins[i].y;
            if (dx*dx + dy*dy < 6400.0f) { // 80px radius
                app->coins[i].active = 0;
                app->coins_collected += app->coins[i].value;
            } else if (app->coins[i].x < car->x - 1000.0f) {
                // Despawn far behind coins
                app->coins[i].active = 0;
            }
        }
    }
    
    app->scroll_x = car->x - win->width / 2.0f;
    if (car->x > app->score) app->score = (int)car->x;
    
    win->needs_redraw = 1;
    extern int ui_dirty;
    ui_dirty = 1;
}

void hillclimb_draw(window_t *win) {
    hillclimb_app_t *app = get_hc(win);
    if (!app) return;
    
    // Draw Sky
    winmgr_fill_rect(win, 0, 32, win->width, win->height - 32, 0xFF87CEEB);
    
    // Draw Terrain with textures
    for (int x = 0; x < win->width; x++) {
        float world_x = app->scroll_x + x;
        int u = (int)world_x % app->ground_w;
        if (u < 0) u += app->ground_w;
        
        float h = get_terrain_y(world_x);
        int top_y = (int)h;
        if (top_y < 0) top_y = 0;
        
        for (int y = top_y; y < win->height; y++) {
            int depth = y - (int)h;
            uint32_t color = 0;
            if (depth >= 0 && depth < app->surf_h && app->img_surf) {
                color = app->img_surf[depth * app->surf_w + u];
            } else if (app->img_ground) {
                int g_v = y % app->ground_h;
                if (g_v < 0) g_v += app->ground_h;
                color = app->img_ground[g_v * app->ground_w + u];
            }
            
            if ((color >> 24) > 128) {
                winmgr_put_pixel(win, x, y, color);
            } else {
                winmgr_put_pixel(win, x, y, 0xFF8B4513); // fallback dirt
            }
        }
    }
    
    if (app->game_over) {
        char *msg1 = "VEHICLE DESTROYED";
        char *msg2 = "SPACE to Restart";
        winmgr_draw_text(win, win->width/2 - strlen(msg1)*4, win->height/2 - 20, msg1, 0xFFFF0000);
        winmgr_draw_text(win, win->width/2 - strlen(msg2)*4, win->height/2, msg2, 0xFF000000);
    }
    
    // Draw Vehicle
    RigidCar *car = &app->car;
    float car_scale = 1.0f;
    if (app->car_w > 0) {
        car_scale = (car->wheel_dist * 2.0f) / (app->car_w * 0.65f);
    }
    
    float offset_y = app->car_h * car_scale * 0.2f; 
    float draw_x = car->x + offset_y * sinf(car->angle);
    float draw_y = car->y - offset_y * cosf(car->angle);
    
    draw_rotated(win, app->img_car, app->car_w, app->car_h, 
                 (int)(draw_x - app->scroll_x), (int)draw_y, 
                 car->angle, car_scale, 0);
                 
    float fx = car->x + car->wheel_dist * cosf(car->angle);
    float fy = car->y + car->wheel_dist * sinf(car->angle);
    float bx = car->x - car->wheel_dist * cosf(car->angle);
    float by = car->y - car->wheel_dist * sinf(car->angle);
    
    float tire_scale = 0.5f;
    draw_rotated(win, app->img_tire, app->tire_w, app->tire_h, 
                 (int)(bx - app->scroll_x), (int)by, 
                 car->tire_angle_b, tire_scale, 0);
                 
    draw_rotated(win, app->img_tire, app->tire_w, app->tire_h, 
                 (int)(fx - app->scroll_x), (int)fy, 
                 car->tire_angle_f, tire_scale, 0);
                 
    // Draw Coins
    for (int i = 0; i < MAX_COINS; i++) {
        if (app->coins[i].active) {
            int cx = (int)(app->coins[i].x - app->scroll_x);
            int cy = (int)app->coins[i].y;
            if (cx > -50 && cx < win->width + 50) {
                uint32_t *img = 0; int cw = 0, ch = 0;
                if (app->coins[i].value == 5) { img = app->img_coin5; cw = app->coin5_w; ch = app->coin5_h; }
                else if (app->coins[i].value == 25) { img = app->img_coin25; cw = app->coin25_w; ch = app->coin25_h; }
                else if (app->coins[i].value == 100) { img = app->img_coin100; cw = app->coin100_w; ch = app->coin100_h; }
                else if (app->coins[i].value == 500) { img = app->img_coin500; cw = app->coin500_w; ch = app->coin500_h; }
                
                if (img) draw_rotated(win, img, cw, ch, cx, cy, 0.0f, 0.6f, 0);
            }
        }
    }
    
    // UI
    char score_str[64] = "Distance: ";
    k_itoa(app->score / 10, score_str + 10);
    int len = 0; while (score_str[len]) len++;
    score_str[len++] = ' '; score_str[len++] = 'C'; score_str[len++] = 'o'; 
    score_str[len++] = 'i'; score_str[len++] = 'n'; score_str[len++] = 's'; 
    score_str[len++] = ':'; score_str[len++] = ' '; score_str[len] = '\0';
    k_itoa(app->coins_collected, score_str + len);
    winmgr_draw_text(win, 10, 40, score_str, 0xFF000000);
}

void hillclimb_close(window_t *win) {
    hillclimb_app_t *app = get_hc(win);
    if (app) {
        if (app->img_car) kfree(app->img_car);
        if (app->img_tire) kfree(app->img_tire);
        if (app->img_ground) kfree(app->img_ground);
        if (app->img_surf) kfree(app->img_surf);
        if (app->img_coin5) kfree(app->img_coin5);
        if (app->img_coin25) kfree(app->img_coin25);
        if (app->img_coin100) kfree(app->img_coin100);
        if (app->img_coin500) kfree(app->img_coin500);
        kfree(app);
    }
    hillclimb_win = 0;
}

void hillclimb_init() {
    if (hillclimb_win) return;
    
    hillclimb_app_t *app = (hillclimb_app_t *)kmalloc(sizeof(hillclimb_app_t));
    app->img_car = load_png("CAR.PNG", &app->car_w, &app->car_h);
    app->img_tire = load_png("TIRE.PNG", &app->tire_w, &app->tire_h);
    app->img_ground = load_png("GROUND.PNG", &app->ground_w, &app->ground_h);
    app->img_surf = load_png("SURF.PNG", &app->surf_w, &app->surf_h);
    app->img_coin5 = load_png("COIN5.PNG", &app->coin5_w, &app->coin5_h);
    app->img_coin25 = load_png("COIN25.PNG", &app->coin25_w, &app->coin25_h);
    app->img_coin100 = load_png("COIN100.PNG", &app->coin100_w, &app->coin100_h);
    app->img_coin500 = load_png("COIN500.PNG", &app->coin500_w, &app->coin500_h);
    
    reset_game(app);
    
    window_t *win = winmgr_create_window(100, 100, 640, 480, "Hill Climb Racing");
        
    win->user_data = app;
    app->win = win;
    
    win->draw = hillclimb_draw;
    win->on_close = hillclimb_close;
    
    hillclimb_win = win;
}
