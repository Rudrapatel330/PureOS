#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "window.h"

#define MAX_WORKSPACES 4

// Initialize workspace system
void workspace_init(void);

// Switch to a workspace (0-3)
void workspace_switch(int idx);

// Move a window to a specific workspace
void workspace_move_window(window_t *win, int workspace_idx);

// Get current active workspace index
int workspace_get_current(void);

// Check if a window should be visible on current workspace
int workspace_is_visible(window_t *win);

#endif
