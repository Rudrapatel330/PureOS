// layout.c - PureBrowser Layout Engine
#include "layout.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"
#include "../../gui/ttf_font.h"

extern int screen_height;
extern void kernel_poll_events(void);

// Font size helper: get the effective font size for a text node
static int get_font_size(render_node_t *node) {
  if (!node) return 14;
  // Check parent element's style
  if (node->parent && node->parent->style) {
    css_style_t *ps = node->parent->style;
    if (ps->has_font_size && ps->font_size > 0) return ps->font_size;
    // Check for h1/h2
    if (node->parent->dom_node) {
      if (strcmp(node->parent->dom_node->tag_name, "h1") == 0) return 24;
      if (strcmp(node->parent->dom_node->tag_name, "h2") == 0) return 18;
    }
  }
  return 14;
}

// Helper to determine if a tag is inherently block-level
static int is_block_element(const char *tag) {
  if (!tag)
    return 0;
  if (strcmp(tag, "div") == 0 || strcmp(tag, "p") == 0 ||
      strcmp(tag, "h1") == 0 || strcmp(tag, "h2") == 0 ||
      strcmp(tag, "h3") == 0 || strcmp(tag, "h4") == 0 ||
      strcmp(tag, "h5") == 0 || strcmp(tag, "h6") == 0 ||
      strcmp(tag, "ul") == 0 || strcmp(tag, "ol") == 0 ||
      strcmp(tag, "li") == 0 ||
      strcmp(tag, "hr") == 0 || strcmp(tag, "body") == 0 ||
      strcmp(tag, "html") == 0 || strcmp(tag, "document") == 0 ||
      strcmp(tag, "form") == 0 || strcmp(tag, "table") == 0 ||
      strcmp(tag, "tr") == 0 || strcmp(tag, "thead") == 0 ||
      strcmp(tag, "tbody") == 0 || strcmp(tag, "tfoot") == 0 ||
      strcmp(tag, "pre") == 0 || strcmp(tag, "blockquote") == 0 ||
      strcmp(tag, "center") == 0 || strcmp(tag, "section") == 0 ||
      strcmp(tag, "article") == 0 || strcmp(tag, "nav") == 0 ||
      strcmp(tag, "header") == 0 || strcmp(tag, "footer") == 0 ||
      strcmp(tag, "main") == 0 || strcmp(tag, "aside") == 0 ||
      strcmp(tag, "dl") == 0 || strcmp(tag, "dt") == 0 ||
      strcmp(tag, "dd") == 0 || strcmp(tag, "figcaption") == 0 ||
      strcmp(tag, "figure") == 0) {
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
    if (style && style->display_type == CSS_DISPLAY_GRID) {
      rnode->is_grid = 1;
      rnode->is_block = 1;
    }
    // Table flags
    if (strcmp(dom_node->tag_name, "table") == 0) {
      rnode->is_table = 1; rnode->is_block = 1;
    } else if (strcmp(dom_node->tag_name, "tr") == 0) {
      rnode->is_table_row = 1; rnode->is_block = 1;
    } else if (strcmp(dom_node->tag_name, "td") == 0 || strcmp(dom_node->tag_name, "th") == 0) {
      rnode->is_table_cell = 1;
    }
    // List item flag
    if (strcmp(dom_node->tag_name, "li") == 0) {
      rnode->is_list_item = 1; rnode->is_block = 1;
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
    // Copy positioning
    rnode->z_index = style->z_index;
    if (style->has_position && style->position_type != CSS_POSITION_STATIC) {
      rnode->is_positioned = 1;
      rnode->position_type = style->position_type;
      rnode->pos_left = style->left;
      rnode->pos_top = style->top;
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

  // Sort children by z-index for proper stacking
  if (rnode->child_count > 1) {
    // Bubble sort by z-index (lower z-index drawn first, i.e., behind)
    int swapped = 1;
    while (swapped) {
      swapped = 0;
      render_node_t *prev = 0;
      render_node_t *curr = rnode->first_child;
      while (curr && curr->next_sibling) {
        render_node_t *next = curr->next_sibling;
        if (curr->z_index > next->z_index) {
          // Swap
          if (prev)
            prev->next_sibling = next;
          else
            rnode->first_child = next;
          curr->next_sibling = next->next_sibling;
          next->next_sibling = curr;
          if (!curr->next_sibling)
            rnode->last_child = curr;
          swapped = 1;
          curr = next;
        }
        prev = curr;
        curr = curr->next_sibling;
      }
    }
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

// Helper: get text width for a node considering content
static int measure_text_width(render_node_t *node) {
  if (!node || !node->dom_node || !node->dom_node->text_content)
    return 0;
  const char *text = node->dom_node->text_content;
  int font_slot = ttf_get_default_font();
  if (font_slot >= 0) {
    int font_size = get_font_size(node);
    ttf_set_size(font_slot, (float)font_size);
    return ttf_text_width(font_slot, text);
  }
  return strlen(text) * 8;
}

// Helper: measure the minimum content width of a subtree
static int measure_subtree_width(render_node_t *node) {
  if (!node) return 0;
  if (node->is_text) return measure_text_width(node);
  int max_w = 0;
  render_node_t *child = node->first_child;
  while (child) {
    int w = measure_subtree_width(child);
    if (w > max_w) max_w = w;
    child = child->next_sibling;
  }
  return max_w;
}

// Table layout: calculate column widths and position cells
static void layout_table(render_node_t *table, int container_w, int start_x, int start_y) {
  if (!table) return;
  
  // Count columns by finding max cells in any row
  int max_cols = 0;
  render_node_t *row = table->first_child;
  while (row) {
    if (row->is_table_row || (row->dom_node && (strcmp(row->dom_node->tag_name, "thead") == 0 ||
        strcmp(row->dom_node->tag_name, "tbody") == 0 || strcmp(row->dom_node->tag_name, "tfoot") == 0))) {
      // For thead/tbody/tfoot, look inside for tr
      render_node_t *actual_row = row;
      if (!row->is_table_row && row->first_child) {
        actual_row = row->first_child;
      }
      while (actual_row) {
        int cols = 0;
        render_node_t *cell = actual_row->first_child;
        while (cell) { cols++; cell = cell->next_sibling; }
        if (cols > max_cols) max_cols = cols;
        if (row->is_table_row) break;
        actual_row = actual_row->next_sibling;
      }
    }
    row = row->next_sibling;
  }
  if (max_cols == 0) max_cols = 1;
  if (max_cols > 32) max_cols = 32;

  // Calculate column widths (equal distribution for now)
  int table_w = container_w - table->box.margin_left - table->box.margin_right;
  if (table->style && table->style->has_width) table_w = table->style->width;
  int col_w = table_w / max_cols;

  // Position rows and cells
  int cur_y = start_y + table->box.margin_top + table->box.padding_top;
  table->box.x = start_x + table->box.margin_left;
  table->box.y = start_y + table->box.margin_top;

  row = table->first_child;
  while (row) {
    // Handle thead/tbody/tfoot wrappers
    render_node_t *first_row = row;
    render_node_t *last_row = row;
    if (!row->is_table_row && row->first_child) {
      first_row = row->first_child;
      last_row = NULL; // iterate all children
    }

    render_node_t *cur_row = first_row;
    while (cur_row) {
      if (cur_row->is_table_row || cur_row->is_table_cell) {
        cur_row->box.x = table->box.x + table->box.padding_left;
        cur_row->box.y = cur_y;
        int cur_x = table->box.x + table->box.padding_left;
        int row_h = 14; // min row height

        render_node_t *cell = cur_row->first_child;
        int col_idx = 0;
        while (cell && col_idx < max_cols) {
          cell->box.x = cur_x + cell->box.padding_left;
          cell->box.y = cur_y + cell->box.padding_top;
          cell->box.width = col_w - cell->box.padding_left - cell->box.padding_right;
          
          // Layout cell contents
          layout_calculate(cell, col_w, cur_x, cur_y);
          
          int cell_h = cell->box.height + cell->box.padding_top + cell->box.padding_bottom;
          if (cell_h < 14) cell_h = 14;
          if (cell_h > row_h) row_h = cell_h;

          cur_x += col_w;
          col_idx++;
          cell = cell->next_sibling;
        }

        cur_row->box.width = max_cols * col_w;
        cur_row->box.height = row_h;
        cur_y += row_h;
      }
      if (last_row == first_row) break; // single row, not wrapper
      cur_row = cur_row->next_sibling;
    }
    
    if (!row->is_table_row) {
      // wrapper node (thead/tbody)
      row->box.x = table->box.x;
      row->box.y = table->box.y;
      row->box.width = max_cols * col_w;
    }
    row = row->next_sibling;
  }

  table->box.content_w = max_cols * col_w;
  table->box.content_h = cur_y - (start_y + table->box.margin_top + table->box.padding_top);
  table->box.width = table->box.content_w + table->box.padding_left + table->box.padding_right;
  table->box.height = table->box.content_h + table->box.padding_top + table->box.padding_bottom;
}

// Calculate dimensions and positions recursively
void layout_calculate(render_node_t *node, int container_w, int current_x,
                      int current_y) {
  if (!node)
    return;
  if (!node->dom_node)
    return;

  if (node->style && (node->style->display_none || node->style->display_type == CSS_DISPLAY_NONE)) {
    node->box.width = 0;
    node->box.height = 0;
    node->box.content_w = 0;
    node->box.content_h = 0;
    return;
  }

  static int calc_poll = 0;
  if (++calc_poll > 50) {
    kernel_poll_events();
    calc_poll = 0;
  }

  // Default line height for text-based elements
  int line_h = 14;
  int fslot = ttf_get_default_font();
  if (fslot >= 0) {
    ttf_set_size(fslot, 14.0f);
    line_h = ttf_font_height(fslot) + 2;
  }
  if (node->style && node->style->has_line_height && node->style->line_height > 0) {
    line_h = node->style->line_height;
  } else if (node->style && node->style->has_font_size && node->style->font_size > 0) {
    line_h = node->style->font_size + 4;
  }
  if (node->dom_node && !node->is_text) {
    if (strcmp(node->dom_node->tag_name, "h1") == 0) {
      if (fslot >= 0) { ttf_set_size(fslot, 24.0f); line_h = ttf_font_height(fslot) + 8; }
      else line_h = 28;
    } else if (strcmp(node->dom_node->tag_name, "h2") == 0) {
      if (fslot >= 0) { ttf_set_size(fslot, 18.0f); line_h = ttf_font_height(fslot) + 4; }
      else line_h = 20;
    }
  }

  // Node bounds start coordinates (including margin wrapper)
  node->box.x = current_x + node->box.margin_left;
  node->box.y = current_y + node->box.margin_top;

  int content_start_x = node->box.x + node->box.padding_left;
  int content_start_y = node->box.y + node->box.padding_top;

  if (node->is_text) {
    // Text node dimensions with word wrapping
    const char *text = node->dom_node->text_content;
    int text_len = text ? strlen(text) : 0;
    int font_slot = ttf_get_default_font();
    if (font_slot >= 0) {
      int font_size = get_font_size(node);
      ttf_set_size(font_slot, (float)font_size);
      line_h = ttf_font_height(font_slot) + 2;
    }
    int total_w;
    if (font_slot >= 0) {
      total_w = ttf_text_width(font_slot, text);
    } else {
      total_w = text_len * 8;
    }
    int avail_w = container_w > 16 ? container_w - 8 : container_w;
    if (avail_w < 40) avail_w = 40;

    if (total_w <= avail_w || avail_w <= 0) {
      node->box.content_w = total_w;
      node->box.content_h = line_h;
    } else {
      // Word wrap: count lines needed
      int lines = 1;
      int line_w = 0;
      int i = 0;
      while (i < text_len) {
        int word_start = i;
        while (i < text_len && text[i] != ' ') i++;
        int word_len = i - word_start;
        int word_w;
        if (font_slot >= 0 && word_len > 0) {
          char saved = text[i];
          ((char*)text)[i] = 0;
          word_w = ttf_text_width(font_slot, text + word_start);
          ((char*)text)[i] = saved;
        } else {
          word_w = word_len * 8;
        }

        if (line_w + word_w > avail_w && line_w > 0) {
          lines++;
          line_w = word_w;
        } else {
          line_w += word_w;
        }
        // Skip spaces
        while (i < text_len && text[i] == ' ') {
          int space_w;
          if (font_slot >= 0) {
            space_w = ttf_text_width(font_slot, " ");
          } else {
            space_w = 8;
          }
          i++;
          line_w += space_w;
        }
      }
      node->box.content_w = avail_w;
      node->box.content_h = lines * line_h;
    }

    // Apply text alignment if parent has it
    if (node->parent && node->parent->style && node->parent->style->has_text_align) {
      if (node->parent->style->text_align == CSS_TEXT_ALIGN_CENTER) {
        int diff = container_w - node->box.content_w;
        if (diff > 0) node->box.x += diff / 2;
      } else if (node->parent->style->text_align == CSS_TEXT_ALIGN_RIGHT) {
        int diff = container_w - node->box.content_w;
        if (diff > 0) node->box.x += diff;
      }
    }

    node->box.width = node->box.content_w;
    node->box.height = node->box.content_h;
    return;
  }

  // Table layout dispatch
  if (node->is_table) {
    layout_table(node, container_w, current_x, current_y);
    return;
  }

  int available_w =
      container_w - (node->box.margin_left + node->box.margin_right +
                     node->box.padding_left + node->box.padding_right);
  if (available_w < 0)
    available_w = 0;

  if (node->style && node->style->has_width) {
    available_w = node->style->width;
  } else if (node->style && node->style->has_width_pct) {
    available_w = container_w * node->style->width_pct / 100;
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

  // First pass: absolute children are skipped from normal flow, then positioned
  // at the end
  render_node_t *absolute_children[128];
  int absolute_count = 0;

  if (node->is_flex) {
    int is_column = (node->style && node->style->has_flex_direction &&
                     node->style->flex_direction == 1);
    int align_items = (node->style && node->style->has_align_items)
                          ? node->style->align_items
                          : 0;
    int justify_content = (node->style && node->style->has_justify_content)
                              ? node->style->justify_content
                              : 0;
    int flex_wrap = (node->style && node->style->has_flex_wrap)
                        ? node->style->flex_wrap
                        : 0;

    int flex_w = available_w;
    if (is_column && (node->style && node->style->has_width)) {
      flex_w = available_w;
    }

    // Pass 1: measure children into arrays
    int child_mains[128];
    int child_crosses[128];
    int child_count = 0;

    render_node_t *child = node->first_child;
    while (child) {
      if (child->position_type == CSS_POSITION_ABSOLUTE || child->position_type == CSS_POSITION_FIXED) {
        if (absolute_count < 128) absolute_children[absolute_count++] = child;
        child = child->next_sibling;
        continue;
      }
      layout_calculate(child, available_w, 0, 0);

      int cm = is_column
          ? child->box.height + child->box.margin_top + child->box.margin_bottom
          : child->box.width + child->box.margin_left + child->box.margin_right;
      int cc = is_column
          ? child->box.width + child->box.margin_left + child->box.margin_right
          : child->box.height + child->box.margin_top + child->box.margin_bottom;

      child_mains[child_count] = cm;
      child_crosses[child_count] = cc;
      child_count++;
      child = child->next_sibling;
    }

    // Determine flex container cross size
    int max_cross = 0;
    int line_acc = 0;
    for (int i = 0; i < child_count; i++) {
      if (flex_wrap && !is_column && line_acc + child_mains[i] > flex_w && line_acc > 0) {
        line_acc = 0;
      }
      line_acc += child_mains[i];
      if (child_crosses[i] > max_cross) max_cross = child_crosses[i];
    }
    int flex_h = max_cross;

    if (is_column) {
      int total_main = 0;
      int max_cross2 = 0;
      for (int i = 0; i < child_count; i++) {
        total_main += child_mains[i];
        if (child_crosses[i] > max_cross2) max_cross2 = child_crosses[i];
      }
      flex_h = total_main;
      flex_w = (node->style && node->style->has_width) ? available_w : max_cross2;
    }
    if (node->style && node->style->has_height) {
      flex_h = node->style->height;
    }

    // Pass 2: position children in lines with wrapping
    child = node->first_child;
    int child_i = 0;
    int line_idx = 0;

    while (child_i < child_count) {
      // Skip absolute children in linked list
      while (child && (child->position_type == CSS_POSITION_ABSOLUTE || child->position_type == CSS_POSITION_FIXED))
        child = child->next_sibling;

      // Find line boundaries
      int line_end = child_i;
      int line_total = 0;
      int line_max_cross = 0;
      while (line_end < child_count) {
        if (flex_wrap && !is_column && line_total + child_mains[line_end] > flex_w && line_total > 0)
          break;
        line_total += child_mains[line_end];
        if (child_crosses[line_end] > line_max_cross)
          line_max_cross = child_crosses[line_end];
        line_end++;
      }

      // Justify content within this line
      int main_axis = is_column ? flex_h : flex_w;
      int free_space = main_axis - line_total;
      int line_start_off = 0;
      int spacing = 0;
      int line_n = line_end - child_i;
      if (free_space > 0) {
        if (justify_content == 1)
          line_start_off = free_space;
        else if (justify_content == 2)
          line_start_off = free_space / 2;
        else if (justify_content == 3 && line_n > 1)
          spacing = free_space / (line_n - 1);
      }

      // Position children in this line
      int current_main = (is_column ? content_start_y : content_start_x) + line_start_off;
      int cross_base = is_column ? content_start_x : content_start_y;
      int cross_offset = flex_wrap ? line_idx * line_max_cross : 0;

      for (int j = child_i; j < line_end; j++) {
        while (child && (child->position_type == CSS_POSITION_ABSOLUTE || child->position_type == CSS_POSITION_FIXED))
          child = child->next_sibling;
        if (!child) break;

        int cm = child_mains[j];
        int cc = child_crosses[j];

        int cx, cy;
        if (is_column) {
          cy = current_main;
          current_main += cm + spacing;
          cx = cross_base;
          if (align_items == 1)
            cx += (line_max_cross - cc);
          else if (align_items == 2)
            cx += (line_max_cross - cc) / 2;
        } else {
          cx = current_main;
          current_main += cm + spacing;
          cy = cross_base + cross_offset;
          if (align_items == 1)
            cy += (line_max_cross - cc);
          else if (align_items == 2)
            cy += (line_max_cross - cc) / 2;
        }

        layout_calculate(child, is_column ? available_w : child->box.content_w, cx, cy);
        child = child->next_sibling;
      }

      child_i = line_end;
      line_idx++;
    }

    node->box.content_w = flex_w;
    node->box.content_h = flex_h;
  } else if (node->is_grid) {
    int col_count = (node->style && node->style->has_grid_template) ? node->style->grid_col_count : 1;
    int gap = (node->style && node->style->has_grid_gap) ? node->style->grid_gap : 0;

    // Solve fr units: compute actual track widths
    int actual_track_widths[8];
    memset(actual_track_widths, 0, sizeof(actual_track_widths));
    int total_fixed_px = 0;
    int total_fr = 0;

    for (int ci = 0; ci < col_count; ci++) {
      if (node->style->grid_col_is_fr[ci]) {
        total_fr += node->style->grid_cols[ci];
      } else {
        total_fixed_px += node->style->grid_cols[ci];
      }
    }

    int grid_avail_w = available_w;
    if (node->style && node->style->has_width) {
      grid_avail_w = node->style->width;
    }

    int space_left = grid_avail_w - total_fixed_px - (gap * (col_count - 1));
    if (space_left < 0) space_left = 0;
    int one_fr_px = total_fr > 0 ? (space_left / total_fr) : 0;

    int total_col_w = 0;
    for (int ci = 0; ci < col_count; ci++) {
      if (node->style->grid_col_is_fr[ci]) {
        actual_track_widths[ci] = node->style->grid_cols[ci] * one_fr_px;
      } else {
        actual_track_widths[ci] = node->style->grid_cols[ci];
      }
      total_col_w += actual_track_widths[ci] + (ci < col_count - 1 ? gap : 0);
    }
    if (total_col_w < 1) total_col_w = grid_avail_w;

    int col = 0, cur_row_start = content_start_y;
    int row_max_h = 0;
    int grid_content_w = total_col_w;
    int grid_content_h = 0;
    int child_idx = 0;

    render_node_t *child = node->first_child;
    while (child) {
      if (child->position_type == CSS_POSITION_ABSOLUTE || child->position_type == CSS_POSITION_FIXED) {
        if (absolute_count < 128) absolute_children[absolute_count++] = child;
        child = child->next_sibling;
        continue;
      }

      int span = (child->style && child->style->grid_column_span > 0) ? child->style->grid_column_span : 1;
      
      // Auto-placement wrap if it doesn't fit on this line
      if (col_count > 0 && col + span > col_count && col > 0) {
        cur_row_start += row_max_h + gap;
        grid_content_h += row_max_h + gap;
        row_max_h = 0;
        col = 0;
      }

      int cell_x = content_start_x;
      for (int ci = 0; ci < col && ci < col_count; ci++) {
        cell_x += actual_track_widths[ci] + gap;
      }

      int col_w = 0;
      if (col_count > 0) {
        for (int s = 0; s < span && (col + s) < col_count; s++) {
          col_w += actual_track_widths[col + s];
          if (s > 0) col_w += gap;
        }
      } else {
        col_w = grid_avail_w;
      }

      int child_avail_w = col_w - child->box.margin_left - child->box.margin_right;
      if (child_avail_w < 0) child_avail_w = 0;

      layout_calculate(child, child_avail_w, 0, 0);

      int ch = child->box.height + child->box.margin_top + child->box.margin_bottom;
      if (ch > row_max_h) row_max_h = ch;

      child->box.x = cell_x + child->box.margin_left;
      child->box.y = cur_row_start + child->box.margin_top;

      col += span;
      if (col_count > 0 && col >= col_count) {
        cur_row_start += row_max_h + gap;
        grid_content_h += row_max_h + gap;
        row_max_h = 0;
        col = 0;
      }
      child_idx++;
      child = child->next_sibling;
    }

    if (row_max_h > 0) {
      grid_content_h += row_max_h;
      cur_row_start += row_max_h;
    } else if (grid_content_h > 0 && gap > 0) {
      grid_content_h -= gap;
    }

    node->box.content_w = grid_content_w < grid_avail_w ? grid_content_w : grid_avail_w;
    node->box.content_h = grid_content_h;
  } else {
    render_node_t *child = node->first_child;
    while (child) {
      // Skip absolute/fixed children from normal flow
      if (child->position_type == CSS_POSITION_ABSOLUTE || child->position_type == CSS_POSITION_FIXED) {
        if (absolute_count < 128) absolute_children[absolute_count++] = child;
        child = child->next_sibling;
        continue;
      }

      // Calculate child layout using current coordinates
      int pass_w = available_w;
      if (!child->is_block && child_x > content_start_x) {
        pass_w = available_w - (child_x - content_start_x);
        if (pass_w < 40) pass_w = available_w;
      }
      layout_calculate(child, pass_w, child_x, child_y);

      if (child->is_block) {
        if (max_inline_h > 0) {
          child_y += max_inline_h;
          max_inline_h = 0;
          layout_calculate(child, available_w, content_start_x, child_y);
        }

        child_y += child->box.height + child->box.margin_top +
                   child->box.margin_bottom;
        child_x = content_start_x;
      } else {
        int total_child_w =
            child->box.width + child->box.margin_left + child->box.margin_right;
        int total_child_h = child->box.height + child->box.margin_top +
                            child->box.margin_bottom;

        if (child_x + total_child_w > content_start_x + available_w &&
            child_x > content_start_x) {
          child_y += max_inline_h > line_h ? max_inline_h : line_h;
          child_x = content_start_x;
          max_inline_h = 0;

          layout_calculate(child, available_w, child_x, child_y);
          total_child_w = child->box.width + child->box.margin_left + child->box.margin_right;
          total_child_h = child->box.height + child->box.margin_top + child->box.margin_bottom;
        }

        child_x +=
            child->box.width + child->box.margin_left + child->box.margin_right;
        if (total_child_h > max_inline_h)
          max_inline_h = total_child_h;

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

  if (node->is_flex || node->is_grid) {
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

  // Percentage height override (resolve against container width as proxy)
  if (node->style && node->style->has_height_pct) {
    node->box.content_h = container_w * node->style->height_pct / 100;
    node->box.height = node->box.content_h + node->box.padding_top +
                       node->box.padding_bottom;
  } else if (node->style && node->style->has_height) {
    node->box.height = node->style->height;
  }

  // Position absolutely positioned children
  for (int i = 0; i < absolute_count; i++) {
    render_node_t *ac = absolute_children[i];
    // Calculate size first
    int abs_w = ac->box.width > 0 ? ac->box.width : available_w;
    int abs_h = ac->box.height > 0 ? ac->box.height : 100;
    if (ac->style && ac->style->has_width) abs_w = ac->style->width;
    if (ac->style && ac->style->has_height) abs_h = ac->style->height;

    // Position relative to positioned ancestor or initial containing block
    int abs_x = node->box.x + node->box.padding_left;
    int abs_y = node->box.y + node->box.padding_top;
    
    if (ac->style && ac->style->has_left) {
      abs_x += ac->style->left;
    } else if (ac->style && ac->style->has_right) {
      abs_x = node->box.x + node->box.width - node->box.padding_right - abs_w - ac->style->right;
    }
    
    if (ac->style && ac->style->has_top) {
      abs_y += ac->style->top;
    } else if (ac->style && ac->style->has_bottom) {
      // Use container's actual height or screen height if it's the root
      int container_h = node->box.height > 0 ? node->box.height : screen_height;
      abs_y = node->box.y + container_h - node->box.padding_bottom - abs_h - ac->style->bottom;
    }

    ac->box.x = abs_x;
    ac->box.y = abs_y;
    ac->box.width = abs_w;
    ac->box.height = abs_h;
    ac->box.content_w = abs_w;
    ac->box.content_h = abs_h;

    // Layout children within the absolutely positioned element
    if (ac->first_child) {
      layout_calculate(ac, abs_w, abs_x, abs_y);
    }
  }
}

dom_node_t *layout_hit_test(render_node_t *node, int mx, int my) {
  if (!node) return 0;
  // Check if point is within this node's box
  if (mx >= node->box.x && mx < node->box.x + node->box.width &&
      my >= node->box.y && my < node->box.y + node->box.height) {
    // Check children first (innermost node)
    render_node_t *child = node->last_child;
    while (child) {
      dom_node_t *found = layout_hit_test(child, mx, my);
      if (found) return found;
      child = child->next_sibling;
    }
    // No child matched, return this node if it has a DOM node
    return node->dom_node;
  }
  return 0;
}
