// css.h - PureBrowser CSS Object Model (Enhanced)
#ifndef BROWSER_CSS_H
#define BROWSER_CSS_H

#include "dom.h"
#include <stdint.h>

// Position values
#define CSS_POSITION_STATIC    0
#define CSS_POSITION_RELATIVE  1
#define CSS_POSITION_ABSOLUTE  2
#define CSS_POSITION_FIXED     3

// Text alignment values
#define CSS_TEXT_ALIGN_LEFT    0
#define CSS_TEXT_ALIGN_CENTER  1
#define CSS_TEXT_ALIGN_RIGHT   2

// Text decoration values
#define CSS_TEXT_DECOR_NONE         0
#define CSS_TEXT_DECOR_UNDERLINE    1
#define CSS_TEXT_DECOR_LINETHROUGH  2

// List style type values
#define CSS_LIST_NONE    0
#define CSS_LIST_DISC    1
#define CSS_LIST_CIRCLE  2
#define CSS_LIST_SQUARE  3
#define CSS_LIST_DECIMAL 4

// White-space values
#define CSS_WS_NORMAL 0
#define CSS_WS_NOWRAP 1
#define CSS_WS_PRE    2

// Display values (stored in display_type)
#define CSS_DISPLAY_BLOCK        0
#define CSS_DISPLAY_INLINE       1
#define CSS_DISPLAY_INLINE_BLOCK 2
#define CSS_DISPLAY_FLEX         3
#define CSS_DISPLAY_NONE         4
#define CSS_DISPLAY_TABLE        5
#define CSS_DISPLAY_TABLE_ROW    6
#define CSS_DISPLAY_TABLE_CELL   7
#define CSS_DISPLAY_LIST_ITEM    8
#define CSS_DISPLAY_GRID         9

// Border style
#define CSS_BORDER_NONE  0
#define CSS_BORDER_SOLID 1

// CSS Variable storage
typedef struct css_var {
  char name[64];
  char value[256];
  struct css_var *next;
} css_var_t;

// CSS gradient types
#define CSS_GRADIENT_NONE        0
#define CSS_GRADIENT_LINEAR      1
#define CSS_GRADIENT_RADIAL      2

#define CSS_MAX_GRADIENT_STOPS 8

typedef struct css_gradient {
  int type;                     // CSS_GRADIENT_LINEAR or CSS_GRADIENT_RADIAL
  int angle;                    // for linear gradients (degrees, 0=up)
  uint32_t stops[CSS_MAX_GRADIENT_STOPS];
  int stop_positions[CSS_MAX_GRADIENT_STOPS]; // 0-1000 (representing 0%-100%)
  int stop_count;
} css_gradient_t;

// Descendant selector chain
#define CSS_SELECTOR_MAX_CHAIN 8
typedef struct css_selector_part {
  char tag[32];
  char class_name[32];
  char id[32];
  int has_tag;
  int has_class;
  int has_id;
} css_selector_part_t;

typedef struct css_selector {
  css_selector_part_t parts[CSS_SELECTOR_MAX_CHAIN];
  int part_count;
  int combinators[CSS_SELECTOR_MAX_CHAIN]; // 0=descendant, 1=child, 2=adjacent
} css_selector_t;

// Basic CSS Properties supported
typedef struct css_style {
  uint32_t color;            // 0xAARRGGBB format
  uint32_t background_color; // 0xAARRGGBB format

  int margin_top, margin_bottom, margin_left, margin_right;
  int padding_top, padding_bottom, padding_left, padding_right;
  int width, height;
  int width_pct, height_pct; // 0-100 for percentage values

  // Typography
  int font_size;           // in pixels (0 = default/inherit)
  uint8_t font_weight;     // 0=normal, 1=bold
  uint8_t text_align;      // CSS_TEXT_ALIGN_*
  uint8_t text_decoration; // CSS_TEXT_DECOR_*
  int line_height;         // in pixels (0 = auto)
  int text_indent;         // in pixels

  // Border (simplified: uniform border)
  int border_width;
  uint32_t border_color;
  uint8_t border_style;    // CSS_BORDER_*

  // Layout
  uint8_t display_type;    // CSS_DISPLAY_*
  uint8_t white_space;     // CSS_WS_*
  uint8_t list_style_type; // CSS_LIST_*
  uint8_t overflow_hidden; // 1 if overflow: hidden

  // Flex properties
  uint8_t flex_direction;    // 0=row, 1=column
  uint8_t justify_content;   // 0=flex-start, 1=flex-end, 2=center, 3=space-between
  uint8_t align_items;       // 0=flex-start, 1=flex-end, 2=center
  uint8_t flex_wrap;         // 0=nowrap, 1=wrap

  // Grid properties
  int grid_cols[8];          // column widths in pixels (or numeric value for fr)
  int grid_col_count;
  int grid_col_is_fr[8];     // 1 if column is 'fr', 0 if 'px'
  int grid_gap;
  int grid_column_span;      // number of columns to span

  // Position properties
  uint8_t position_type;     // CSS_POSITION_*
  int left, right, top, bottom;
  int z_index;

  // Gradient background
  css_gradient_t gradient;

  // Flags for which properties are actually set (vs inherited/default)
  uint32_t has_color : 1;
  uint32_t has_bg_color : 1;
  uint32_t has_margin : 1;
  uint32_t has_padding : 1;
  uint32_t has_width : 1;
  uint32_t has_height : 1;
  uint32_t has_font_size : 1;
  uint32_t has_font_weight : 1;
  uint32_t has_text_align : 1;
  uint32_t has_text_decoration : 1;
  uint32_t has_line_height : 1;
  uint32_t has_text_indent : 1;
  uint32_t has_border : 1;
  uint32_t has_display : 1;
  uint32_t has_white_space : 1;
  uint32_t has_list_style : 1;
  uint32_t has_overflow : 1;
  uint32_t has_flex_direction : 1;
  uint32_t has_justify_content : 1;
  uint32_t has_align_items : 1;
  uint32_t has_flex_wrap : 1;
  uint32_t has_position : 1;
  uint32_t has_z_index : 1;
  uint32_t has_gradient : 1;
  uint32_t has_width_pct : 1;
  uint32_t has_height_pct : 1;
  uint32_t has_left : 1;
  uint32_t has_right : 1;
  uint32_t has_top : 1;
  uint32_t has_bottom : 1;
  uint32_t has_grid_template : 1;
  uint32_t has_grid_gap : 1;
  uint32_t has_grid_column_span : 1;

  // Legacy compat flags (kept for existing code)
  uint32_t display_none : 1;
  uint32_t display_flex : 1;
} css_style_t;

// A parsed CSS rule (selector + styles)
typedef struct css_rule {
  char selector[64]; // e.g. "h1", ".class", "#id"
  css_selector_t parsed_selector;
  int parsed_selector_valid;
  css_style_t style;
  struct css_rule *next; // Linked list of rules
} css_rule_t;

// The CSSOM
typedef struct css_stylesheet {
  css_rule_t *rules;
  css_var_t *variables; // Custom properties (--*)
} css_stylesheet_t;

css_stylesheet_t *css_parse(const char *css_text);
void css_free_stylesheet(css_stylesheet_t *sheet);
void css_apply_styles(dom_node_t *root, css_stylesheet_t *sheet);

// Default element styling (e.g. h1 has large text, bold)
void css_apply_default_styles(dom_node_t *root);

// CSS inheritance pass (color, font-size cascade to children)
void css_inherit_styles(dom_node_t *root);

// CSS variable resolution
const char *css_resolve_var(css_stylesheet_t *sheet, const char *var_name);
void css_parse_gradient(const char *val, css_gradient_t *grad);
int css_match_selector_deep(dom_node_t *node, css_selector_t *sel);

#endif
