// dom.h - PureBrowser Document Object Model
#ifndef BROWSER_DOM_H
#define BROWSER_DOM_H

#include <stdint.h>

// DOM Node Types
#define DOM_NODE_ELEMENT 1
#define DOM_NODE_TEXT 3
#define DOM_NODE_COMMENT 8

// Maximum lengths for basic strings to avoid dynamic allocation overhead
// initially
#define DOM_MAX_TAG_LEN 32
#define DOM_MAX_ATTR_NAME 32
#define DOM_MAX_ATTR_VAL 256

typedef struct dom_attr {
  char name[DOM_MAX_ATTR_NAME];
  char value[DOM_MAX_ATTR_VAL];
  struct dom_attr *next;
} dom_attr_t;

// Forward declaration for JS event callbacks
typedef struct js_event_callback js_event_callback_t;

typedef struct dom_node {
  int type; // DOM_NODE_ELEMENT, etc.

  // Element specific
  char tag_name[DOM_MAX_TAG_LEN];
  dom_attr_t *attributes;

  // Text specific (dynamic allocation for text content)
  char *text_content;

  // Tree relationships
  struct dom_node *parent;
  struct dom_node *first_child;
  struct dom_node *last_child;
  struct dom_node *next_sibling;
  int child_count;

  // Computed Style (points to css_style_t)
  void *style;
  // JS event listeners
  js_event_callback_t *event_listeners;
} dom_node_t;

// JS event listener (linked list on DOM nodes)
struct js_event_callback {
  char type[24];                    // "click", "keydown", etc.
  int callback_stash_idx;           // Duktape stash index for the JS function
  struct js_event_callback *next;
};

// External resource tracking
typedef struct dom_resource {
  char url[512];
  int type; // 0=CSS, 1=script, 2=image, 3=font
  struct dom_resource *next;
} dom_resource_t;

// Core DOM functions
dom_node_t *dom_create_element(const char *tag);
dom_node_t *dom_create_text(const char *text);
void dom_append_child(dom_node_t *parent, dom_node_t *child);
void dom_set_attribute(dom_node_t *node, const char *name, const char *value);
const char *dom_get_attribute(dom_node_t *node, const char *name);
void dom_free_node(dom_node_t *node);

// HTML Parsing
dom_node_t *dom_parse_html(const char *html_str);

// Resource management
void dom_add_resource(dom_resource_t **list, const char *url, int type);
void dom_free_resources(dom_resource_t *list);
char *dom_resolve_url(const char *base_url, const char *relative_url, char *out, int out_max);

// Debugging
void dom_print_tree(dom_node_t *root, int depth);

#endif
