// css.c - PureBrowser CSS parser and Object Model
#include "css.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"

extern void print_serial(const char *);
extern void kernel_poll_events(void);

// Helper: Trim leading/trailing whitespace
static void trim(char *str) {
  if (!str || !*str)
    return;

  // Trim leading
  char *p = str;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    p++;
  if (p != str) {
    int i = 0;
    while (p[i]) {
      str[i] = p[i];
      i++;
    }
    str[i] = '\0';
  }

  // Trim trailing
  int len = strlen(str);
  while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' ||
                     str[len - 1] == '\n' || str[len - 1] == '\r')) {
    str[len - 1] = '\0';
    len--;
  }
}

// Very basic RGB color parser (handles standard #RRGGBB, #RGB and basic color
// names)
static uint32_t parse_color(const char *c_str) {
  if (!c_str)
    return 0;
  char temp[64];
  strncpy(temp, c_str, 63);
  temp[63] = 0;
  trim(temp);

  if (temp[0] == '#') {
    // Hex color - rudimentary parse
    uint32_t val = 0;
    int len = strlen(temp);
    for (int i = 1; i < len && i < 7; i++) {
      char c = temp[i];
      uint32_t v = 0;
      if (c >= '0' && c <= '9')
        v = c - '0';
      else if (c >= 'a' && c <= 'f')
        v = c - 'a' + 10;
      else if (c >= 'A' && c <= 'F')
        v = c - 'A' + 10;
      val = (val << 4) | v;
    }
    if (len == 4) { // #RGB
      uint32_t r = (val >> 8) & 0xF;
      uint32_t g = (val >> 4) & 0xF;
      uint32_t b = val & 0xF;
      val = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
    }
    return val;
  }

  // Basic names
  if (strcmp(temp, "red") == 0)
    return 0xFF0000;
  if (strcmp(temp, "green") == 0)
    return 0x00FF00;
  if (strcmp(temp, "blue") == 0)
    return 0x0000FF;
  if (strcmp(temp, "black") == 0)
    return 0x000000;
  if (strcmp(temp, "white") == 0)
    return 0xFFFFFF;
  if (strcmp(temp, "yellow") == 0)
    return 0xFFFF00;

  return 0; // Default transparent/black
}

static int parse_px(const char *val) {
  int v = 0;
  while (*val >= '0' && *val <= '9') {
    v = v * 10 + (*val - '0');
    val++;
  }
  return v;
}

static void parse_declaration(css_style_t *style, const char *prop,
                              const char *val) {
  char p[64];
  char v[128];
  strncpy(p, prop, 63);
  p[63] = 0;
  strncpy(v, val, 127);
  v[127] = 0;

  trim(p);
  trim(v);

  if (strcmp(p, "color") == 0) {
    style->color = parse_color(v);
    style->has_color = 1;
  } else if (strcmp(p, "background-color") == 0 ||
             strcmp(p, "background") == 0) {
    style->background_color = parse_color(v);
    style->has_bg_color = 1;
  } else if (strcmp(p, "margin") == 0) {
    int m = parse_px(v);
    style->margin_top = m;
    style->margin_bottom = m;
    style->margin_left = m;
    style->margin_right = m;
    style->has_margin = 1;
  } else if (strcmp(p, "margin-top") == 0) {
    style->margin_top = parse_px(v);
    style->has_margin = 1;
  } else if (strcmp(p, "margin-bottom") == 0) {
    style->margin_bottom = parse_px(v);
    style->has_margin = 1;
  } else if (strcmp(p, "padding") == 0) {
    int pad = parse_px(v);
    style->padding_top = pad;
    style->padding_bottom = pad;
    style->padding_left = pad;
    style->padding_right = pad;
    style->has_padding = 1;
  } else if (strcmp(p, "width") == 0) {
    style->width = parse_px(v);
    style->has_width = 1;
  } else if (strcmp(p, "height") == 0) {
    style->height = parse_px(v);
    style->has_height = 1;
  } else if (strcmp(p, "display") == 0) {
    if (strcmp(v, "none") == 0) {
      style->display_none = 1;
    } else if (strcmp(v, "flex") == 0) {
      style->display_flex = 1;
    }
  } else if (strcmp(p, "flex-direction") == 0) {
    if (strcmp(v, "row") == 0) {
      style->flex_direction = 0;
      style->has_flex_direction = 1;
    } else if (strcmp(v, "column") == 0) {
      style->flex_direction = 1;
      style->has_flex_direction = 1;
    }
  } else if (strcmp(p, "justify-content") == 0) {
    if (strcmp(v, "flex-start") == 0) {
      style->justify_content = 0;
      style->has_justify_content = 1;
    } else if (strcmp(v, "flex-end") == 0) {
      style->justify_content = 1;
      style->has_justify_content = 1;
    } else if (strcmp(v, "center") == 0) {
      style->justify_content = 2;
      style->has_justify_content = 1;
    } else if (strcmp(v, "space-between") == 0) {
      style->justify_content = 3;
      style->has_justify_content = 1;
    }
  } else if (strcmp(p, "align-items") == 0) {
    if (strcmp(v, "flex-start") == 0) {
      style->align_items = 0;
      style->has_align_items = 1;
    } else if (strcmp(v, "flex-end") == 0) {
      style->align_items = 1;
      style->has_align_items = 1;
    } else if (strcmp(v, "center") == 0) {
      style->align_items = 2;
      style->has_align_items = 1;
    }
  }
}

css_stylesheet_t *css_parse(const char *css_text) {
  if (!css_text)
    return 0;

  css_stylesheet_t *sheet =
      (css_stylesheet_t *)kmalloc(sizeof(css_stylesheet_t));
  if (!sheet)
    return 0;
  sheet->rules = 0;

  const char *p = css_text;

  while (*p) {
    // Skip whitespace
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
      p++;
    if (!*p)
      break;

    // Parse Selector
    char selector[64] = {0};
    int s_idx = 0;
    while (*p && *p != '{' && s_idx < 63) {
      selector[s_idx++] = *p++;
    }
    selector[s_idx] = '\0';
    trim(selector);

    if (*p == '{')
      p++;

    css_rule_t *rule = (css_rule_t *)kmalloc(sizeof(css_rule_t));
    if (!rule)
      break; // Out of memory
    memset(rule, 0, sizeof(css_rule_t));
    strncpy(rule->selector, selector, 63);

    // Parse Declarations
    while (*p && *p != '}') {
      while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
        p++;
      if (*p == '}')
        break;

      char prop[64] = {0};
      int p_idx = 0;
      while (*p && *p != ':' && *p != '}' && p_idx < 63) {
        prop[p_idx++] = *p++;
      }
      prop[p_idx] = 0;

      if (*p == ':')
        p++;

      char val[128] = {0};
      int v_idx = 0;
      while (*p && *p != ';' && *p != '}' && v_idx < 127) {
        val[v_idx++] = *p++;
      }
      val[v_idx] = 0;

      if (*p == ';')
        p++;

      parse_declaration(&rule->style, prop, val);
    }

    if (*p == '}')
      p++;

    // Append rule to sheet
    rule->next = sheet->rules;
    sheet->rules = rule;
  }

  return sheet;
}

void css_free_stylesheet(css_stylesheet_t *sheet) {
  if (!sheet)
    return;
  css_rule_t *r = sheet->rules;
  while (r) {
    css_rule_t *next = r->next;
    kfree(r);
    r = next;
  }
  kfree(sheet);
}

// Selector matching (very rudimentary)
static int match_selector(dom_node_t *node, const char *selector) {
  if (!node || !selector)
    return 0;

  // Tag match
  if (strcmp(node->tag_name, selector) == 0)
    return 1;

  // Class match
  if (selector[0] == '.') {
    const char *classes = dom_get_attribute(node, "class");
    if (classes) {
      // Need a proper tokenized check in real CSS. Simple substring for now.
      // A robust way would be to check whole words
      int len = strlen(selector) - 1;
      char *found = (char *)strstr(classes, selector + 1);
      if (found) {
        // Check if it's an exact word match
        char after = found[len];
        if (after == ' ' || after == '\0' || after == '\t')
          return 1;
      }
    }
  }

  // ID match
  if (selector[0] == '#') {
    const char *id = dom_get_attribute(node, "id");
    if (id && strcmp(id, selector + 1) == 0)
      return 1;
  }

  return 0;
}

static void apply_styles_recursive(dom_node_t *node, css_stylesheet_t *sheet) {
  kernel_poll_events();
  if (!node)
    return;

  if (node->type == DOM_NODE_ELEMENT) {
    if (!node->style) {
      node->style = kmalloc(sizeof(css_style_t));
      if (node->style) {
        memset(node->style, 0, sizeof(css_style_t));
      }
    }

    if (node->style && sheet) {
      css_style_t *computed = (css_style_t *)node->style;
      css_rule_t *r = sheet->rules;
      while (r) {
        if (match_selector(node, r->selector)) {
          // Apply properties
          if (r->style.has_color) {
            computed->color = r->style.color;
            computed->has_color = 1;
          }
          if (r->style.has_bg_color) {
            computed->background_color = r->style.background_color;
            computed->has_bg_color = 1;
          }
          if (r->style.has_margin) {
            computed->margin_top = r->style.margin_top;
            computed->margin_bottom = r->style.margin_bottom;
            computed->margin_left = r->style.margin_left;
            computed->margin_right = r->style.margin_right;
            computed->has_margin = 1;
          }
          if (r->style.has_padding) {
            computed->padding_top = r->style.padding_top;
            computed->padding_bottom = r->style.padding_bottom;
            computed->padding_left = r->style.padding_left;
            computed->padding_right = r->style.padding_right;
            computed->has_padding = 1;
          }
          if (r->style.has_width) {
            computed->width = r->style.width;
            computed->has_width = 1;
          }
          if (r->style.has_height) {
            computed->height = r->style.height;
            computed->has_height = 1;
          }
          if (r->style.display_none) {
            computed->display_none = 1;
          }
          if (r->style.display_flex) {
            computed->display_flex = 1;
          }
          if (r->style.has_flex_direction) {
            computed->flex_direction = r->style.flex_direction;
            computed->has_flex_direction = 1;
          }
          if (r->style.has_justify_content) {
            computed->justify_content = r->style.justify_content;
            computed->has_justify_content = 1;
          }
          if (r->style.has_align_items) {
            computed->align_items = r->style.align_items;
            computed->has_align_items = 1;
          }
        }
        r = r->next;
      }
    }

    dom_node_t *child = node->first_child;
    while (child) {
      apply_styles_recursive(child, sheet);
      child = child->next_sibling;
    }
  }
}

void css_apply_styles(dom_node_t *root, css_stylesheet_t *sheet) {
  apply_styles_recursive(root, sheet);
}

// User Agent Default Stylesheet essentially
void css_apply_default_styles(dom_node_t *node) {
  kernel_poll_events();
  if (!node)
    return;
  if (node->type == DOM_NODE_ELEMENT) {
    if (!node->style) {
      node->style = kmalloc(sizeof(css_style_t));
      if (node->style)
        memset(node->style, 0, sizeof(css_style_t));
    }

    css_style_t *computed = (css_style_t *)node->style;
    if (computed) {
      if (strcmp(node->tag_name, "h1") == 0) {
        computed->margin_top = 8;
        computed->margin_bottom = 8;
        computed->has_margin = 1;
      } else if (strcmp(node->tag_name, "h2") == 0) {
        computed->margin_top = 6;
        computed->margin_bottom = 6;
        computed->has_margin = 1;
      } else if (strcmp(node->tag_name, "p") == 0) {
        computed->margin_top = 8;
        computed->margin_bottom = 8;
        computed->has_margin = 1;
      }
    }

    dom_node_t *child = node->first_child;
    while (child) {
      css_apply_default_styles(child);
      child = child->next_sibling;
    }
  }
}
