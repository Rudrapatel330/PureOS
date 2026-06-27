#include "workspace.h"
#include "compositor.h"
#include "string.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);

static int current_workspace = 0;
int num_workspaces = 1;
static workspace_state_t workspaces[MAX_WORKSPACES];

void workspace_init(void) {
  current_workspace = 0;
  num_workspaces = 1;
  for (int i = 0; i < MAX_WORKSPACES; i++) {
    workspaces[i].focused_window_id = -1;
  }
  // All windows default to workspace 0 (set in winmgr_create_window)
  print_serial("WORKSPACES: Initialized (1 desktop initially)\n");
}

// Helper: find a window by its ID, returns NULL if not found
static window_t *find_window_by_id(int id) {
  extern window_t windows[];
  extern int window_count;
  for (int i = 0; i < window_count; i++) {
    if (windows[i].exists && windows[i].id == id)
      return &windows[i];
  }
  return 0;
}

// Helper: find the topmost visible (non-hidden, non-minimized) window on a given workspace
static window_t *find_topmost_visible_on_workspace(int ws_idx) {
  extern window_t windows[];
  extern int window_z_order[];
  extern int window_count;
  // Walk Z-order from top (end) to bottom (start)
  for (int i = window_count - 1; i >= 0; i--) {
    int win_idx = window_z_order[i];
    if (win_idx < 0) continue;
    window_t *win = &windows[win_idx];
    if (win->exists && win->id != 0 && win->workspace == ws_idx &&
        !win->ws_hidden && !win->is_minimized && win->fading_mode != 2) {
      return win;
    }
  }
  return 0;
}

void workspace_switch(int idx) {
  if (idx < 0 || idx >= MAX_WORKSPACES || idx == current_workspace)
    return;

  char buf[8];
  print_serial("WORKSPACE: Switching to ");
  k_itoa(idx + 1, buf);
  print_serial(buf);
  print_serial("\n");

  // === SAVE focus state of the current workspace ===
  extern window_t *active_window;
  if (active_window && active_window->id != 0) {
    workspaces[current_workspace].focused_window_id = active_window->id;
    // Blur: invalidate the old active window so it repaints with inactive title bar
    compositor_invalidate_window(active_window);
  } else {
    workspaces[current_workspace].focused_window_id = -1;
  }

  // Hide all windows of old workspace, show new workspace's windows
  // Sticky windows are never hidden — they span all workspaces
  extern window_t windows[];
  extern int window_count;

  for (int i = 0; i < window_count; i++) {
    if (!windows[i].exists)
      continue;

    if (windows[i].is_sticky) {
      // Sticky windows always remain visible
      windows[i].ws_hidden = 0;
      continue;
    }

    if (windows[i].workspace == current_workspace) {
      // Window on old workspace — mark hidden
      windows[i].ws_hidden = 1;
    }
    if (windows[i].workspace == idx) {
      // Window on new workspace — make visible
      windows[i].ws_hidden = 0;
    }
  }

  current_workspace = idx;

  // === RESTORE focus state of the new workspace ===
  // Clear global active_window first
  active_window = 0;
  int saved_id = workspaces[idx].focused_window_id;
  window_t *target = 0;

  if (saved_id >= 0) {
    target = find_window_by_id(saved_id);
  }

  if (!target) {
    // Fall back to topmost visible window on the new workspace
    target = find_topmost_visible_on_workspace(idx);
  }

  if (target) {
    extern void winmgr_bring_to_front(window_t *);
    winmgr_bring_to_front(target);
    compositor_invalidate_window(target);
  }

  // Invalidate entire screen to redraw
  extern int screen_width, screen_height;
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
}

void workspace_set_focus_on(int ws_idx, window_t *win) {
  if (ws_idx < 0 || ws_idx >= MAX_WORKSPACES || !win)
    return;
  workspaces[ws_idx].focused_window_id = win->id;
}

void workspace_move_window(window_t *win, int workspace_idx) {
  if (!win || workspace_idx < 0 || workspace_idx >= MAX_WORKSPACES)
    return;
  win->workspace = workspace_idx;

  // If moving to a different workspace, hide it
  if (workspace_idx != current_workspace) {
    win->ws_hidden = 1;
  }
}

int workspace_get_current(void) { return current_workspace; }

int workspace_is_visible(window_t *win) {
  if (!win)
    return 0;
  return (win->workspace == current_workspace) && !win->ws_hidden;
}

void workspace_add(void) {
  if (num_workspaces < MAX_WORKSPACES) {
    num_workspaces++;
  }
}

int workspace_get_count(void) {
  return num_workspaces;
}
