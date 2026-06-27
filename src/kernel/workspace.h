#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "window.h"

#define MAX_WORKSPACES 16

extern int num_workspaces;

typedef struct {
  int focused_window_id; // ID of the window that has focus on this workspace (-1 = none)
} workspace_state_t;

// Initialize workspace system
void workspace_init(void);

// Pre-set the focused window for a workspace (used before switching)
void workspace_set_focus_on(int ws_idx, window_t *win);

// Switch to a workspace (0-3)
void workspace_switch(int idx);

// Move a window to a specific workspace
void workspace_move_window(window_t *win, int workspace_idx);

// Get current active workspace index
int workspace_get_current(void);

// Check if a window should be visible on current workspace
int workspace_is_visible(window_t *win);

// Add a new workspace
void workspace_add(void);

// Get number of workspaces
int workspace_get_count(void);

#endif
