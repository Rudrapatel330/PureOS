#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "window.h"
#include <stdint.h>

// Typedef rect_t moved to window.h

// Helper: Intersect two rects
int rect_intersect(rect_t a, rect_t b, rect_t *out);

// Initialize Compositor
void compositor_init();

// Mark a region of the screen as dirty (needs redraw)
void compositor_invalidate_rect(int x, int y, int w, int h);

// Mark a window's current area as dirty
void compositor_invalidate_window(window_t *win);

// Main Render Loop (call instead of winmgr_render_all)
void compositor_render();

// Fast Blur
void compositor_blur_rect(int x, int y, int w, int h, int radius);

// Debug: Enable/Disable visual dirty rects
void compositor_set_debug(int enabled);

// Check if dirty
int compositor_is_dirty();

// Set global overlay rect (e.g. for snap preview)
// x,y,w,h: rect definition
// enabled: 1 to show, 0 to hide
// Clear both VRAM pages and sync with backbuffer
void compositor_clear_vram();

void compositor_set_overlay(int x, int y, int w, int h, int enabled);

// Global toggle for expensive software animations
extern int disable_animations;

extern int magnifier_enabled;
extern float magnifier_scale;

#endif
