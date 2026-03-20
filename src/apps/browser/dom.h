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
} dom_node_t;

// Core DOM functions
dom_node_t *dom_create_element(const char *tag);
dom_node_t *dom_create_text(const char *text);
void dom_append_child(dom_node_t *parent, dom_node_t *child);
void dom_set_attribute(dom_node_t *node, const char *name, const char *value);
const char *dom_get_attribute(dom_node_t *node, const char *name);
void dom_free_node(dom_node_t *node);

// HTML Parsing
dom_node_t *dom_parse_html(const char *html_str);

// Debugging
void dom_print_tree(dom_node_t *root, int depth);

#endif
