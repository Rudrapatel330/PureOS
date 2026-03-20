#include "workspace.h"
#include "compositor.h"
#include "string.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);

static int current_workspace = 0;

void workspace_init(void) {
  current_workspace = 0;
  // All windows default to workspace 0 (set in winmgr_create_window)
  print_serial("WORKSPACES: Initialized (4 desktops)\n");
}

void workspace_switch(int idx) {
  if (idx < 0 || idx >= MAX_WORKSPACES || idx == current_workspace)
    return;

  char buf[8];
  print_serial("WORKSPACE: Switching to ");
  k_itoa(idx + 1, buf);
  print_serial(buf);
  print_serial("\n");

  // Hide all windows of old workspace, show new workspace's windows
  extern window_t windows[];
  extern int window_count;

  for (int i = 0; i < window_count; i++) {
    if (!windows[i].exists)
      continue;

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

  // Invalidate entire screen to redraw
  extern int screen_width, screen_height;
  compositor_invalidate_rect(0, 0, screen_width, screen_height);
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
