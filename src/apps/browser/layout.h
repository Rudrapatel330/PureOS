// layout.h - PureBrowser Layout Engine & Render Tree
#ifndef BROWSER_LAYOUT_H
#define BROWSER_LAYOUT_H

#include "css.h"
#include "dom.h"

// Box Model Values
typedef struct layout_box {
  int x, y;
  int width, height;
  int content_w, content_h;

  // Extents
  int margin_top, margin_bottom, margin_left, margin_right;
  int padding_top, padding_bottom, padding_left, padding_right;
} layout_box_t;

// Render Tree Node
typedef struct render_node {
  dom_node_t *dom_node; // Associated DOM node
  css_style_t *style;   // Computed styles (from DOM node)

  layout_box_t box; // Calculated geometry

  int is_block; // 1 if block, 0 if inline
  int is_text;  // 1 if text-only node
  int is_flex;  // 1 if flex container

  struct render_node *parent;
  struct render_node *first_child;
  struct render_node *last_child;
  struct render_node *next_sibling;
  int child_count;
} render_node_t;

// Build the render tree from a styled DOM
render_node_t *layout_build_tree(dom_node_t *dom_root);

// Calculate dimensions and positions recursively
void layout_calculate(render_node_t *root, int container_w, int start_x,
                      int start_y);

// Free the render tree
void layout_free_tree(render_node_t *root);

#endif
