// layout.c - PureBrowser Layout Engine
#include "layout.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"

extern void kernel_poll_events(void);

// Helper to determine if a tag is inherently block-level
static int is_block_element(const char *tag) {
  if (!tag)
    return 0;
  if (strcmp(tag, "div") == 0 || strcmp(tag, "p") == 0 ||
      strcmp(tag, "h1") == 0 || strcmp(tag, "h2") == 0 ||
      strcmp(tag, "ul") == 0 || strcmp(tag, "li") == 0 ||
      strcmp(tag, "hr") == 0 || strcmp(tag, "body") == 0 ||
      strcmp(tag, "html") == 0 || strcmp(tag, "document") == 0 ||
      strcmp(tag, "form") == 0) {
    return 1;
  }
  return 0;
}

// Recursively build tree, skipping display:none elements
render_node_t *layout_build_tree(dom_node_t *dom_node) {
  if (!dom_node)
    return 0;

  static int build_poll = 0;
  if (++build_poll > 50) {
    kernel_poll_events();
    build_poll = 0;
  }

  // Skip invisible elements like script, style, head, meta, etc.
  if (dom_node->type == DOM_NODE_ELEMENT) {
    if (strcmp(dom_node->tag_name, "script") == 0 ||
        strcmp(dom_node->tag_name, "style") == 0 ||
        strcmp(dom_node->tag_name, "head") == 0 ||
        strcmp(dom_node->tag_name, "meta") == 0 ||
        strcmp(dom_node->tag_name, "link") == 0 ||
        strcmp(dom_node->tag_name, "title") == 0) {
      return 0;
    }

    if (strcmp(dom_node->tag_name, "input") == 0) {
      const char *type = dom_get_attribute(dom_node, "type");
      if (type && strcmp(type, "hidden") == 0) {
        return 0;
      }
    }
  }

  css_style_t *style = (css_style_t *)dom_node->style;
  if (style && style->display_none) {
    return 0; // Skip
  }

  render_node_t *rnode = (render_node_t *)kmalloc(sizeof(render_node_t));
  if (!rnode)
    return 0;
  memset(rnode, 0, sizeof(render_node_t));

  rnode->dom_node = dom_node;
  rnode->style = style;

  if (dom_node->type == DOM_NODE_TEXT) {
    rnode->is_text = 1;
    rnode->is_block = 0; // Text is inline
  } else {
    rnode->is_text = 0;
    rnode->is_block = is_block_element(dom_node->tag_name);
    if (style && style->display_flex) {
      rnode->is_flex = 1;
      rnode->is_block = 1;
    }
  }

  // Copy margins/padding from style if present
  if (style) {
    if (style->has_margin) {
      rnode->box.margin_top = style->margin_top;
      rnode->box.margin_bottom = style->margin_bottom;
      rnode->box.margin_left = style->margin_left;
      rnode->box.margin_right = style->margin_right;
    }
    if (style->has_padding) {
      rnode->box.padding_top = style->padding_top;
      rnode->box.padding_bottom = style->padding_bottom;
      rnode->box.padding_left = style->padding_left;
      rnode->box.padding_right = style->padding_right;
    }
  }

  dom_node_t *dom_child = dom_node->first_child;
  while (dom_child) {
    render_node_t *child_rnode = layout_build_tree(dom_child);
    if (child_rnode) {
      child_rnode->parent = rnode;
      child_rnode->next_sibling = 0;
      if (!rnode->first_child) {
        rnode->first_child = child_rnode;
        rnode->last_child = child_rnode;
      } else {
        rnode->last_child->next_sibling = child_rnode;
        rnode->last_child = child_rnode;
      }
      rnode->child_count++;
    }
    dom_child = dom_child->next_sibling;
  }

  return rnode;
}

void layout_free_tree(render_node_t *root) {
  if (!root)
    return;
  render_node_t *child = root->first_child;
  while (child) {
    render_node_t *next = child->next_sibling;
    layout_free_tree(child);
    child = next;
  }
  kfree(root);
}

// Calculate dimensions and positions recursively
void layout_calculate(render_node_t *node, int container_w, int current_x,
                      int current_y) {
  if (!node)
    return;
  if (!node->dom_node)
    return;

  static int calc_poll = 0;
  if (++calc_poll > 50) {
    kernel_poll_events();
    calc_poll = 0;
  }

  // Default line height for text-based elements
  int line_h = 14;
  if (!node->is_text) {
    if (strcmp(node->dom_node->tag_name, "h1") == 0)
      line_h = 24;
    else if (strcmp(node->dom_node->tag_name, "h2") == 0)
      line_h = 20;
  }

  // Node bounds start coordinates (including margin wrapper)
  node->box.x = current_x + node->box.margin_left;
  node->box.y = current_y + node->box.margin_top;

  int content_start_x = node->box.x + node->box.padding_left;
  int content_start_y = node->box.y + node->box.padding_top;

  if (node->is_text) {
    // Text node dimensions
    int text_len =
        node->dom_node->text_content ? strlen(node->dom_node->text_content) : 0;
    node->box.content_w = text_len * 8; // simplified fixed-width font measuring
    node->box.content_h = line_h;

    node->box.width = node->box.content_w;
    node->box.height = node->box.content_h;
    return;
  }

  // Explicit width?
  int available_w =
      container_w - (node->box.margin_left + node->box.margin_right +
                     node->box.padding_left + node->box.padding_right);
  if (available_w < 0)
    available_w = 0;

  if (node->style && node->style->has_width) {
    available_w = node->style->width;
  }

  // Handle elements with intrinsic sizes (input, button, hr)
  if (!node->is_text && node->dom_node) {
    if (strcmp(node->dom_node->tag_name, "input") == 0) {
      node->box.content_w = 160;
      node->box.content_h = 20;
      node->box.width = node->box.content_w + node->box.padding_left +
                        node->box.padding_right;
      node->box.height = node->box.content_h + node->box.padding_top +
                         node->box.padding_bottom;
      return;
    } else if (strcmp(node->dom_node->tag_name, "button") == 0) {
      node->box.content_w = 100;
      node->box.content_h = 24;
      node->box.width = node->box.content_w + node->box.padding_left +
                        node->box.padding_right;
      node->box.height = node->box.content_h + node->box.padding_top +
                         node->box.padding_bottom;
      return;
    } else if (strcmp(node->dom_node->tag_name, "hr") == 0) {
      node->box.content_w = available_w;
      node->box.content_h = 8;
      node->box.width = node->box.content_w;
      node->box.height = node->box.content_h;
      return;
    }
  }

  // Element Node: Layout children
  int child_x = content_start_x;
  int child_y = content_start_y;
  int max_inline_h = 0;

  if (node->is_flex) {
    int is_column = (node->style && node->style->has_flex_direction &&
                     node->style->flex_direction == 1);
    int align_items = (node->style && node->style->has_align_items)
                          ? node->style->align_items
                          : 0;
    int justify_content = (node->style && node->style->has_justify_content)
                              ? node->style->justify_content
                              : 0;

    int total_main = 0;
    int max_cross = 0;

    render_node_t *child = node->first_child;
    while (child) {
      layout_calculate(child, available_w, 0, 0);

      int cx =
          child->box.width + child->box.margin_left + child->box.margin_right;
      int cy =
          child->box.height + child->box.margin_top + child->box.margin_bottom;

      if (is_column) {
        total_main += cy;
        if (cx > max_cross)
          max_cross = cx;
      } else {
        total_main += cx;
        if (cy > max_cross)
          max_cross = cy;
      }
      child = child->next_sibling;
    }

    int flex_w = available_w;
    int flex_h = max_cross;
    if (is_column) {
      flex_h = total_main;
      flex_w =
          (node->style && node->style->has_width) ? available_w : max_cross;
    }
    if (node->style && node->style->has_height) {
      flex_h = node->style->height;
    }

    int main_start = 0;
    int spacing = 0;
    int free_space = (is_column ? flex_h : flex_w) - total_main;

    if (free_space > 0) {
      if (justify_content == 1)
        main_start = free_space;
      else if (justify_content == 2)
        main_start = free_space / 2;
      else if (justify_content == 3 && node->child_count > 1)
        spacing = free_space / (node->child_count - 1);
    }

    int current_main =
        (is_column ? content_start_y : content_start_x) + main_start;
    int cross_start = is_column ? content_start_x : content_start_y;
    int cross_size = is_column ? flex_w : flex_h;

    child = node->first_child;
    while (child) {
      int child_w =
          child->box.width + child->box.margin_left + child->box.margin_right;
      int child_h =
          child->box.height + child->box.margin_top + child->box.margin_bottom;

      int cx, cy;
      if (is_column) {
        cy = current_main;
        current_main += child_h + spacing;
        cx = cross_start;
        if (align_items == 1)
          cx += cross_size - child_w;
        else if (align_items == 2)
          cx += (cross_size - child_w) / 2;
      } else {
        cx = current_main;
        current_main += child_w + spacing;
        cy = cross_start;
        if (align_items == 1)
          cy += cross_size - child_h;
        else if (align_items == 2)
          cy += (cross_size - child_h) / 2;
      }

      layout_calculate(child, is_column ? available_w : child->box.content_w,
                       cx, cy);
      child = child->next_sibling;
    }

    node->box.content_w = flex_w;
    node->box.content_h = flex_h;
  } else {
    render_node_t *child = node->first_child;
    while (child) {

      // Calculate child layout using current coordinates
      layout_calculate(child, available_w, child_x, child_y);

      if (child->is_block) {
        // Block wraps to a new line, takes full width mostly, and pushes cursor
        // down
        if (max_inline_h > 0) {
          child_y += max_inline_h;
          max_inline_h = 0;

          // Recalculate block child now that Y has been pushed down
          layout_calculate(child, available_w, content_start_x, child_y);
        }

        // Advance cursor
        child_y += child->box.height + child->box.margin_top +
                   child->box.margin_bottom;
        child_x = content_start_x; // Reset X
      } else {
        // Inline flow
        int total_child_w =
            child->box.width + child->box.margin_left + child->box.margin_right;
        int total_child_h = child->box.height + child->box.margin_top +
                            child->box.margin_bottom;

        // Very rudimentary line wrapping
        if (child_x + total_child_w > content_start_x + available_w &&
            child_x > content_start_x) {
          child_y += max_inline_h > line_h ? max_inline_h : line_h;
          child_x = content_start_x;
          max_inline_h = 0;

          // Recalculate
          layout_calculate(child, available_w, child_x, child_y);
        }

        child_x +=
            child->box.width + child->box.margin_left + child->box.margin_right;
        if (total_child_h > max_inline_h)
          max_inline_h = total_child_h;

        // Check for explicit breaks
        if (child->dom_node && strcmp(child->dom_node->tag_name, "br") == 0) {
          child_y += max_inline_h > line_h ? max_inline_h : line_h;
          child_x = content_start_x;
          max_inline_h = 0;
        }
      }
      child = child->next_sibling;
    }

    if (max_inline_h > 0) {
      child_y += max_inline_h;
    }
  }

  if (node->is_flex) {
    // content sizes already assigned
  } else if (node->is_block) {
    node->box.content_w = available_w;
  } else {
    int max_w = 0;
    render_node_t *child = node->first_child;
    while (child) {
      int cw = (child->box.x - content_start_x) + child->box.width;
      if (cw > max_w)
        max_w = cw;
      child = child->next_sibling;
    }
    node->box.content_w = max_w;
  }

  node->box.content_h = child_y - content_start_y;

  node->box.width =
      node->box.content_w + node->box.padding_left + node->box.padding_right;
  node->box.height =
      node->box.content_h + node->box.padding_top + node->box.padding_bottom;

  // Explicit height override
  if (node->style && node->style->has_height) {
    node->box.height = node->style->height;
    // Adjust content_h? Or ignore. (Simplified)
  }
}
