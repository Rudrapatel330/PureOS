#include "libpure.h"

// A simple user-space program that tests the window server IPC
int main(void) {
  pure_printf("Window Server Test starting...\n");
  pure_printf("My PID: %d\n", pure_getpid());
  pure_printf("My UID: %d\n", pure_getuid());

  // Connect to window server
  int client_id = pure_wm_connect();
  pure_printf("WM Client ID: %d\n", client_id);

  if (client_id < 0) {
    pure_printf("Failed to connect to window server!\n");
    return 1;
  }

  // Create a window
  int win_id = pure_wm_create_window(640, 480);
  pure_printf("WM Window ID: %d\n", win_id);

  if (win_id < 0) {
    pure_printf("Failed to create window!\n");
    return 1;
  }

  // Listen for events
  wm_event_t ev;
  int running = 1;
  int frame_count = 0;

  while (running) {
    if (pure_wm_get_event(&ev) > 0) {
      switch (ev.type) {
      case 0: // Key event
        pure_printf("Key: %d\n", ev.d1);
        break;
      case 1: // Mouse move
        break;
      case 2: // Mouse click
        pure_printf("Click at %d,%d button=%d\n", ev.d1, ev.d2, ev.d3);
        break;
      case 3: // Close
        pure_printf("Close requested!\n");
        running = 0;
        break;
      }
    }

    frame_count++;
    if (frame_count % 60 == 0) {
      int mouse[3];
      pure_wm_get_mouse(mouse);
      pure_printf("Frame %d, mouse at %d,%d buttons=%d\n",
                  frame_count, mouse[0], mouse[1], mouse[2]);
    }
  }

  pure_printf("Window server test complete.\n");
  return 0;
}
