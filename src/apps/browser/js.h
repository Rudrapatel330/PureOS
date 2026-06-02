#ifndef JS_H
#define JS_H

// Forward declaration for DOM node type
typedef struct dom_node dom_node_t;

void js_init(void);
void js_execute(const char *source);
extern int layout_dirty; // Set to 1 to schedule re-layout
void js_request_rerender(void); // Call after DOM mutations to trigger full re-layout
void js_dispatch_event(void *node, const char *event_type, int mx, int my);
void js_tick_timers(void);

#endif
