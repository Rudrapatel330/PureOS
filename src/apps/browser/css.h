// css.h - PureBrowser CSS Object Model
#ifndef BROWSER_CSS_H
#define BROWSER_CSS_H

#include "dom.h"
#include <stdint.h>

// Basic CSS Properties supported
typedef struct css_style {
  uint32_t color;            // 0xRRGGBB format
  uint32_t background_color; // 0xRRGGBB format
  int margin_top, margin_bottom, margin_left, margin_right;
  int padding_top, padding_bottom, padding_left, padding_right;
  int width, height;

  // Flags for which properties are actually set
  uint32_t has_color : 1;
  uint32_t has_bg_color : 1;
  uint32_t has_margin : 1;
  uint32_t has_padding : 1;
  uint32_t has_width : 1;
  uint32_t has_height : 1;
  uint32_t display_none : 1; // 1 if display: none

  uint32_t display_flex : 1;
  uint32_t has_flex_direction : 1;
  uint32_t has_justify_content : 1;
  uint32_t has_align_items : 1;

  uint8_t flex_direction; // 0=row, 1=column
  uint8_t
      justify_content; // 0=flex-start, 1=flex-end, 2=center, 3=space-between
  uint8_t align_items; // 0=flex-start, 1=flex-end, 2=center
} css_style_t;

// A parsed CSS rule (selector + styles)
typedef struct css_rule {
  char selector[64]; // e.g. "h1", ".class", "#id"
  css_style_t style;
  struct css_rule *next; // Linked list of rules
} css_rule_t;

// The CSSOM
typedef struct css_stylesheet {
  css_rule_t *rules;
} css_stylesheet_t;

css_stylesheet_t *css_parse(const char *css_text);
void css_free_stylesheet(css_stylesheet_t *sheet);
void css_apply_styles(dom_node_t *root, css_stylesheet_t *sheet);

// Default element styling (e.g. h1 has large text, bold)
void css_apply_default_styles(dom_node_t *root);

#endif
