// css.c - PureBrowser CSS parser and Object Model
#include "css.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"

extern void print_serial(const char *);
extern void kernel_poll_events(void);
extern int screen_width;
extern int screen_height;

// Local case-insensitive string compare
static int strcasecmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    char c1 = *s1, c2 = *s2;
    if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
    if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
    if (c1 != c2) return c1 - c2;
    s1++; s2++;
  }
  return (unsigned char)*s1 - (unsigned char)*s2;
}

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
    if (len == 4) {
      uint32_t r = (val >> 8) & 0xF;
      uint32_t g = (val >> 4) & 0xF;
      uint32_t b = val & 0xF;
      val = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
    }
    return val;
  }

  // rgb(r,g,b) and rgba(r,g,b,a) support
  if (strncmp(temp, "rgb", 3) == 0) {
    const char *s = temp + 3;
    if (*s == 'a') s++;
    if (*s == '(') s++;
    int r = 0, g = 0, b = 0;
    while (*s >= '0' && *s <= '9') { r = r * 10 + (*s - '0'); s++; }
    while (*s == ' ' || *s == ',') s++;
    while (*s >= '0' && *s <= '9') { g = g * 10 + (*s - '0'); s++; }
    while (*s == ' ' || *s == ',') s++;
    while (*s >= '0' && *s <= '9') { b = b * 10 + (*s - '0'); s++; }
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    return (r << 16) | (g << 8) | b;
  }

  // hsl() support
  if (strncmp(temp, "hsl", 3) == 0) {
    const char *s = temp + 3;
    if (*s == 'a') s++;
    if (*s == '(') s++;
    int h = 0, sl = 0, l = 0;
    while (*s >= '0' && *s <= '9') { h = h * 10 + (*s - '0'); s++; }
    while (*s == ' ' || *s == ',') s++;
    while (*s >= '0' && *s <= '9') { sl = sl * 10 + (*s - '0'); s++; }
    while (*s == ' ' || *s == ',') s++;
    while (*s >= '0' && *s <= '9') { l = l * 10 + (*s - '0'); s++; }

    double hue = (h % 360) / 360.0;
    double sat = (sl > 100 ? 100 : sl) / 100.0;
    double lit = (l > 100 ? 100 : l) / 100.0;

    double r2, g2, b2;
    if (sat == 0) {
      r2 = g2 = b2 = lit;
    } else {
      double q2 = lit < 0.5 ? lit * (1 + sat) : lit + sat - lit * sat;
      double p2 = 2 * lit - q2;

      // Inline HSL to RGB conversion
      { double t = hue + 1.0/3; if (t < 0) t += 1; if (t > 1) t -= 1;
        if (t < 1.0/6) r2 = p2 + (q2 - p2) * 6 * t;
        else if (t < 1.0/2) r2 = q2;
        else if (t < 2.0/3) r2 = p2 + (q2 - p2) * (2.0/3 - t) * 6;
        else r2 = p2; }
      { double t = hue; if (t < 0) t += 1; if (t > 1) t -= 1;
        if (t < 1.0/6) g2 = p2 + (q2 - p2) * 6 * t;
        else if (t < 1.0/2) g2 = q2;
        else if (t < 2.0/3) g2 = p2 + (q2 - p2) * (2.0/3 - t) * 6;
        else g2 = p2; }
      { double t = hue - 1.0/3; if (t < 0) t += 1; if (t > 1) t -= 1;
        if (t < 1.0/6) b2 = p2 + (q2 - p2) * 6 * t;
        else if (t < 1.0/2) b2 = q2;
        else if (t < 2.0/3) b2 = p2 + (q2 - p2) * (2.0/3 - t) * 6;
        else b2 = p2; }
    }
    return ((int)(r2 * 255) << 16) | ((int)(g2 * 255) << 8) | (int)(b2 * 255);
  }

  // Named colors (common CSS colors)
  char lc[64];
  for (int i = 0; temp[i] && i < 63; i++) {
    lc[i] = (temp[i] >= 'A' && temp[i] <= 'Z') ? temp[i] + 32 : temp[i];
    lc[i+1] = 0;
  }

  if (strcmp(lc, "black") == 0)       return 0x000000;
  if (strcmp(lc, "white") == 0)       return 0xFFFFFF;
  if (strcmp(lc, "red") == 0)         return 0xFF0000;
  if (strcmp(lc, "green") == 0)       return 0x008000;
  if (strcmp(lc, "blue") == 0)        return 0x0000FF;
  if (strcmp(lc, "yellow") == 0)      return 0xFFFF00;
  if (strcmp(lc, "orange") == 0)      return 0xFFA500;
  if (strcmp(lc, "purple") == 0)      return 0x800080;
  if (strcmp(lc, "pink") == 0)        return 0xFFC0CB;
  if (strcmp(lc, "brown") == 0)       return 0xA52A2A;
  if (strcmp(lc, "gray") == 0)        return 0x808080;
  if (strcmp(lc, "grey") == 0)        return 0x808080;
  if (strcmp(lc, "silver") == 0)      return 0xC0C0C0;
  if (strcmp(lc, "navy") == 0)        return 0x000080;
  if (strcmp(lc, "teal") == 0)        return 0x008080;
  if (strcmp(lc, "aqua") == 0)        return 0x00FFFF;
  if (strcmp(lc, "cyan") == 0)        return 0x00FFFF;
  if (strcmp(lc, "magenta") == 0)     return 0xFF00FF;
  if (strcmp(lc, "fuchsia") == 0)     return 0xFF00FF;
  if (strcmp(lc, "lime") == 0)        return 0x00FF00;
  if (strcmp(lc, "maroon") == 0)      return 0x800000;
  if (strcmp(lc, "olive") == 0)       return 0x808000;
  if (strcmp(lc, "coral") == 0)       return 0xFF7F50;
  if (strcmp(lc, "salmon") == 0)      return 0xFA8072;
  if (strcmp(lc, "tomato") == 0)      return 0xFF6347;
  if (strcmp(lc, "gold") == 0)        return 0xFFD700;
  if (strcmp(lc, "khaki") == 0)       return 0xF0E68C;
  if (strcmp(lc, "violet") == 0)      return 0xEE82EE;
  if (strcmp(lc, "indigo") == 0)      return 0x4B0082;
  if (strcmp(lc, "crimson") == 0)     return 0xDC143C;
  if (strcmp(lc, "lightgray") == 0)   return 0xD3D3D3;
  if (strcmp(lc, "lightgrey") == 0)   return 0xD3D3D3;
  if (strcmp(lc, "darkgray") == 0)    return 0xA9A9A9;
  if (strcmp(lc, "darkgrey") == 0)    return 0xA9A9A9;
  if (strcmp(lc, "lightblue") == 0)   return 0xADD8E6;
  if (strcmp(lc, "darkblue") == 0)    return 0x00008B;
  if (strcmp(lc, "lightgreen") == 0)  return 0x90EE90;
  if (strcmp(lc, "darkgreen") == 0)   return 0x006400;
  if (strcmp(lc, "darkred") == 0)     return 0x8B0000;
  if (strcmp(lc, "skyblue") == 0)     return 0x87CEEB;
  if (strcmp(lc, "steelblue") == 0)   return 0x4682B4;
  if (strcmp(lc, "tan") == 0)         return 0xD2B48C;
  if (strcmp(lc, "wheat") == 0)       return 0xF5DEB3;
  if (strcmp(lc, "beige") == 0)       return 0xF5F5DC;
  if (strcmp(lc, "ivory") == 0)       return 0xFFFFF0;
  if (strcmp(lc, "linen") == 0)       return 0xFAF0E6;
  if (strcmp(lc, "transparent") == 0) return 0x000000;

  return 0;
}

// Parse CSS gradient (linear-gradient)
void css_parse_gradient(const char *val, css_gradient_t *grad) {
  if (!val || !grad) return;
  memset(grad, 0, sizeof(css_gradient_t));

  char temp[256];
  strncpy(temp, val, 255);
  temp[255] = 0;
  trim(temp);

  if (strncmp(temp, "linear-gradient", 15) == 0) {
    grad->type = CSS_GRADIENT_LINEAR;
    const char *s = temp + 15;
    while (*s && *s != '(') s++;
    if (*s == '(') s++;

    // Parse optional angle
    if (*s >= '0' && *s <= '9') {
      grad->angle = 0;
      while (*s >= '0' && *s <= '9') { grad->angle = grad->angle * 10 + (*s - '0'); s++; }
      if (strncmp(s, "deg", 3) == 0) s += 3;
      while (*s == ' ' || *s == ',') s++;
    } else if (strncmp(s, "to ", 3) == 0) {
      s += 3;
      if (strncmp(s, "top", 3) == 0) { grad->angle = 0; s += 3; }
      else if (strncmp(s, "bottom", 6) == 0) { grad->angle = 180; s += 6; }
      else if (strncmp(s, "left", 4) == 0) { grad->angle = 270; s += 4; }
      else if (strncmp(s, "right", 5) == 0) { grad->angle = 90; s += 5; }
      while (*s == ' ' || *s == ',') s++;
    }

    // Parse color stops
    int stop_idx = 0;
    while (*s && *s != ')' && stop_idx < CSS_MAX_GRADIENT_STOPS) {
      while (*s == ' ' || *s == ',') s++;
      if (*s == ')') break;

      // Read color
      char col_buf[64];
      int ci = 0;
      while (*s && *s != ' ' && *s != ',' && *s != ')' && *s != '%' && ci < 63) {
        col_buf[ci++] = *s++;
      }
      col_buf[ci] = 0;
      if (ci > 0) {
        grad->stops[stop_idx] = parse_color(col_buf);
        while (*s == ' ') s++;
        if (*s >= '0' && *s <= '9') {
          grad->stop_positions[stop_idx] = 0;
          while (*s >= '0' && *s <= '9') { grad->stop_positions[stop_idx] = grad->stop_positions[stop_idx] * 10 + (*s - '0'); s++; }
          if (*s == '%') { s++; }
          grad->stop_positions[stop_idx] *= 10; // 0-1000 scale
        } else {
          grad->stop_positions[stop_idx] = stop_idx * 1000 / (CSS_MAX_GRADIENT_STOPS - 1);
        }
        stop_idx++;
      }
    }
    grad->stop_count = stop_idx;
  } else if (strncmp(temp, "radial-gradient", 15) == 0) {
    grad->type = CSS_GRADIENT_RADIAL;
    const char *s = temp + 15;
    while (*s && *s != '(') s++;
    if (*s == '(') s++;
    // Simplified: just parse color stops like linear
    int stop_idx = 0;
    while (*s && *s != ')' && stop_idx < CSS_MAX_GRADIENT_STOPS) {
      while (*s == ' ' || *s == ',') s++;
      if (*s == ')') break;
      char col_buf[64];
      int ci = 0;
      while (*s && *s != ' ' && *s != ',' && *s != ')' && *s != '%' && ci < 63) {
        col_buf[ci++] = *s++;
      }
      col_buf[ci] = 0;
      if (ci > 0) {
        grad->stops[stop_idx] = parse_color(col_buf);
        while (*s == ' ') s++;
        if (*s >= '0' && *s <= '9') {
          grad->stop_positions[stop_idx] = 0;
          while (*s >= '0' && *s <= '9') { grad->stop_positions[stop_idx] = grad->stop_positions[stop_idx] * 10 + (*s - '0'); s++; }
          if (*s == '%') { s++; }
          grad->stop_positions[stop_idx] *= 10;
        } else {
          grad->stop_positions[stop_idx] = stop_idx * 1000 / (CSS_MAX_GRADIENT_STOPS - 1);
        }
        stop_idx++;
      }
    }
    grad->stop_count = stop_idx;
  }
}

// Resolve CSS variable from stylesheet
const char *css_resolve_var(css_stylesheet_t *sheet, const char *var_name) {
  if (!sheet || !var_name) return NULL;
  if (strncmp(var_name, "--", 2) != 0) return NULL;

  css_var_t *v = sheet->variables;
  while (v) {
    if (strcmp(v->name, var_name) == 0) return v->value;
    v = v->next;
  }
  return NULL;
}

// Parse a full CSS selector into parts (supports descendant, child combinators)
static void parse_selector(const char *sel_str, css_selector_t *out_sel) {
  memset(out_sel, 0, sizeof(css_selector_t));
  char temp[64];
  strncpy(temp, sel_str, 63);
  temp[63] = 0;
  trim(temp);

  int part_idx = 0;
  int comb_idx = 0;
  const char *p = temp;
  while (*p && part_idx < CSS_SELECTOR_MAX_CHAIN) {
    while (*p == ' ' || *p == '\t') p++;

    // Check for combinators
    if (*p == '>') { p++; comb_idx++; while (*p == ' ' || *p == '\t') p++; }
    else if (*p == '+') { p++; comb_idx++; while (*p == ' ' || *p == '\t') p++; }
    else if (*p == '~') { p++; comb_idx++; while (*p == ' ' || *p == '\t') p++; }

    css_selector_part_t *part = &out_sel->parts[part_idx];

    // Parse tag name
    if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '*' || *p == '_') {
      int ti = 0;
      while (*p && *p != '.' && *p != '#' && *p != ':' && *p != '[' && *p != ' ' && *p != '>' && *p != '+' && *p != '~' && ti < 31) {
        part->tag[ti++] = *p++;
      }
      part->tag[ti] = 0;
      part->has_tag = 1;
    }

    // Parse classes and id
    while (*p == '.' || *p == '#') {
      if (*p == '.') {
        p++;
        int ci = 0;
        while (*p && *p != '.' && *p != '#' && *p != ':' && *p != '[' && *p != ' ' && *p != '>' && *p != '+' && *p != '~' && ci < 31) {
          part->class_name[ci++] = *p++;
        }
        part->class_name[ci] = 0;
        part->has_class = 1;
      } else if (*p == '#') {
        p++;
        int ii = 0;
        while (*p && *p != '.' && *p != '#' && *p != ':' && *p != '[' && *p != ' ' && *p != '>' && *p != '+' && *p != '~' && ii < 31) {
          part->id[ii++] = *p++;
        }
        part->id[ii] = 0;
        part->has_id = 1;
      }
    }

    // Skip pseudo-classes and attributes (simplified)
    if (*p == ':') { while (*p && *p != ' ' && *p != '>' && *p != '+' && *p != '~') p++; }
    if (*p == '[') { while (*p && *p != ']') p++; if (*p == ']') p++; }

    part_idx++;

    // Check for descendant combinator (space between parts)
    while (*p == ' ' || *p == '\t') {
      const char *next = p + 1;
      while (*next == ' ' || *next == '\t') next++;
      if (*next && *next != '>' && *next != '+' && *next != '~' && *next != ',' && *next != '{') {
        if (part_idx < CSS_SELECTOR_MAX_CHAIN) {
          out_sel->combinators[comb_idx++] = 0; // descendant
          p = next;
          break;
        }
      }
      p++;
    }
  }

  out_sel->part_count = part_idx;
  if (comb_idx < part_idx - 1) {
    // Fill remaining combinators as descendant
    for (int i = comb_idx; i < part_idx - 1; i++) {
      out_sel->combinators[i] = 0;
    }
  }
}

// Match a selector against a DOM node (deep matching with combinators)
int css_match_selector_deep(dom_node_t *node, css_selector_t *sel) {
  if (!node || !sel || sel->part_count == 0) return 0;

  // Start from the last part (the actual target)
  int pi = sel->part_count - 1;

  // Match the last (target) part against this node
  dom_node_t *current = node;

  // Check each part
  while (pi >= 0 && current) {
    css_selector_part_t *part = &sel->parts[pi];
    int matched = 1;

    if (part->has_tag && part->tag[0] != '*' && strcmp(part->tag, "*") != 0) {
      if (strcmp(current->tag_name, part->tag) != 0 &&
          strcmp(current->tag_name, part->tag) != 0) {
        // Try case-insensitive
        if (strcasecmp(current->tag_name, part->tag) != 0)
          matched = 0;
      }
    }

    if (matched && part->has_class) {
      const char *classes = dom_get_attribute(current, "class");
      if (!classes) { matched = 0; }
      else {
        // Check for the class as a whole word
        char class_copy[256];
        strncpy(class_copy, classes, 255);
        class_copy[255] = 0;
        char *token = strtok(class_copy, " ");
        int found = 0;
        while (token) {
          if (strcmp(token, part->class_name) == 0) { found = 1; break; }
          token = strtok(NULL, " ");
        }
        if (!found) matched = 0;
      }
    }

    if (matched && part->has_id) {
      const char *id = dom_get_attribute(current, "id");
      if (!id || strcmp(id, part->id) != 0) matched = 0;
    }

    if (matched) {
      pi--;
      if (pi >= 0) {
        int combinator = (pi < sel->part_count - 1) ? sel->combinators[pi] : 0;
        if (combinator == 1) {
          // Child combinator: move to parent only
          current = current->parent;
        } else {
          // Descendant combinator: move to parent and continue
          // Try siblings first, then parent
          current = current->parent;
        }
      }
    } else if (pi < sel->part_count - 1) {
      // If we're matching ancestors and this one doesn't match, move up
      current = current->parent;
    } else {
      break;
    }
  }

  return (pi < 0) ? 1 : 0;
}

static int parse_px(const char *val) {
  float v = 0;
  int is_negative = 0;

  if (*val == '-') { is_negative = 1; val++; }

  while ((*val >= '0' && *val <= '9') || *val == '.') {
    if (*val == '.') { val++; continue; }
    v = v * 10 + (*val - '0');
    val++;
  }

  if (is_negative) v = -v;

  while (*val == ' ') val++;

  if (strncmp(val, "vw", 2) == 0) {
    return (int)((v / 100.0f) * screen_width);
  } else if (strncmp(val, "vh", 2) == 0) {
    return (int)((v / 100.0f) * screen_height);
  } else if (strncmp(val, "rem", 3) == 0 || strncmp(val, "em", 2) == 0) {
    return (int)(v * 16.0f);
  }

  return (int)v;
}

static int parse_pct(const char *val) {
  int v = 0;
  while (*val >= '0' && *val <= '9') {
    v = v * 10 + (*val - '0');
    val++;
  }
  if (v < 0) v = 0;
  if (v > 100) v = 100;
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
    int vlen = strlen(v);
    if (vlen > 0 && v[vlen - 1] == '%') {
      char tmp[16]; strncpy(tmp, v, vlen - 1); tmp[vlen - 1] = 0;
      style->width_pct = parse_pct(tmp);
      style->has_width_pct = 1;
    } else {
      style->width = parse_px(v);
      style->has_width = 1;
    }
  } else if (strcmp(p, "height") == 0) {
    int vlen = strlen(v);
    if (vlen > 0 && v[vlen - 1] == '%') {
      char tmp[16]; strncpy(tmp, v, vlen - 1); tmp[vlen - 1] = 0;
      style->height_pct = parse_pct(tmp);
      style->has_height_pct = 1;
    } else {
      style->height = parse_px(v);
      style->has_height = 1;
    }
  } else if (strcmp(p, "display") == 0) {
    if (strcmp(v, "none") == 0) {
      style->display_none = 1;
      style->display_type = CSS_DISPLAY_NONE;
      style->has_display = 1;
    } else if (strcmp(v, "flex") == 0) {
      style->display_flex = 1;
      style->display_type = CSS_DISPLAY_FLEX;
      style->has_display = 1;
    } else if (strcmp(v, "block") == 0) {
      style->display_type = CSS_DISPLAY_BLOCK;
      style->has_display = 1;
    } else if (strcmp(v, "inline") == 0) {
      style->display_type = CSS_DISPLAY_INLINE;
      style->has_display = 1;
    } else if (strcmp(v, "inline-block") == 0) {
      style->display_type = CSS_DISPLAY_INLINE_BLOCK;
      style->has_display = 1;
    } else if (strcmp(v, "table") == 0) {
      style->display_type = CSS_DISPLAY_TABLE;
      style->has_display = 1;
    } else if (strcmp(v, "table-row") == 0) {
      style->display_type = CSS_DISPLAY_TABLE_ROW;
      style->has_display = 1;
    } else if (strcmp(v, "table-cell") == 0) {
      style->display_type = CSS_DISPLAY_TABLE_CELL;
      style->has_display = 1;
    } else if (strcmp(v, "list-item") == 0) {
      style->display_type = CSS_DISPLAY_LIST_ITEM;
      style->has_display = 1;
    } else if (strcmp(v, "grid") == 0) {
      style->display_type = CSS_DISPLAY_GRID;
      style->has_display = 1;
    }
  } else if (strcmp(p, "font-size") == 0) {
    style->font_size = parse_px(v);
    style->has_font_size = 1;
  } else if (strcmp(p, "font-weight") == 0) {
    if (strcmp(v, "bold") == 0 || strcmp(v, "bolder") == 0) {
      style->font_weight = 1;
      style->has_font_weight = 1;
    } else if (strcmp(v, "normal") == 0) {
      style->font_weight = 0;
      style->has_font_weight = 1;
    } else {
      int w = parse_px(v);
      style->font_weight = (w >= 600) ? 1 : 0;
      style->has_font_weight = 1;
    }
  } else if (strcmp(p, "text-align") == 0) {
    if (strcmp(v, "left") == 0) {
      style->text_align = CSS_TEXT_ALIGN_LEFT;
      style->has_text_align = 1;
    } else if (strcmp(v, "center") == 0) {
      style->text_align = CSS_TEXT_ALIGN_CENTER;
      style->has_text_align = 1;
    } else if (strcmp(v, "right") == 0) {
      style->text_align = CSS_TEXT_ALIGN_RIGHT;
      style->has_text_align = 1;
    }
  } else if (strcmp(p, "text-decoration") == 0) {
    if (strstr(v, "underline")) {
      style->text_decoration = CSS_TEXT_DECOR_UNDERLINE;
      style->has_text_decoration = 1;
    } else if (strstr(v, "line-through")) {
      style->text_decoration = CSS_TEXT_DECOR_LINETHROUGH;
      style->has_text_decoration = 1;
    } else if (strcmp(v, "none") == 0) {
      style->text_decoration = CSS_TEXT_DECOR_NONE;
      style->has_text_decoration = 1;
    }
  } else if (strcmp(p, "line-height") == 0) {
    style->line_height = parse_px(v);
    style->has_line_height = 1;
  } else if (strcmp(p, "text-indent") == 0) {
    style->text_indent = parse_px(v);
    style->has_text_indent = 1;
  } else if (strcmp(p, "border") == 0) {
    // Simple: "1px solid red" or just "1px solid"
    style->border_width = parse_px(v);
    if (style->border_width < 1) style->border_width = 1;
    style->border_style = CSS_BORDER_SOLID;
    // Try to find color after "solid"
    char *sol = (char *)strstr(v, "solid");
    if (sol) {
      sol += 5;
      while (*sol == ' ') sol++;
      if (*sol) {
        style->border_color = parse_color(sol);
      } else {
        style->border_color = 0x000000;
      }
    } else {
      style->border_color = 0x000000;
    }
    style->has_border = 1;
  } else if (strcmp(p, "border-width") == 0) {
    style->border_width = parse_px(v);
    style->has_border = 1;
  } else if (strcmp(p, "border-color") == 0) {
    style->border_color = parse_color(v);
    style->has_border = 1;
  } else if (strcmp(p, "white-space") == 0) {
    if (strcmp(v, "nowrap") == 0) {
      style->white_space = CSS_WS_NOWRAP;
      style->has_white_space = 1;
    } else if (strcmp(v, "pre") == 0) {
      style->white_space = CSS_WS_PRE;
      style->has_white_space = 1;
    } else if (strcmp(v, "normal") == 0) {
      style->white_space = CSS_WS_NORMAL;
      style->has_white_space = 1;
    }
  } else if (strcmp(p, "list-style-type") == 0 || strcmp(p, "list-style") == 0) {
    if (strcmp(v, "none") == 0) {
      style->list_style_type = CSS_LIST_NONE;
      style->has_list_style = 1;
    } else if (strcmp(v, "disc") == 0) {
      style->list_style_type = CSS_LIST_DISC;
      style->has_list_style = 1;
    } else if (strcmp(v, "circle") == 0) {
      style->list_style_type = CSS_LIST_CIRCLE;
      style->has_list_style = 1;
    } else if (strcmp(v, "square") == 0) {
      style->list_style_type = CSS_LIST_SQUARE;
      style->has_list_style = 1;
    } else if (strcmp(v, "decimal") == 0) {
      style->list_style_type = CSS_LIST_DECIMAL;
      style->has_list_style = 1;
    }
  } else if (strcmp(p, "overflow") == 0) {
    if (strcmp(v, "hidden") == 0) {
      style->overflow_hidden = 1;
      style->has_overflow = 1;
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
  } else if (strcmp(p, "flex-wrap") == 0) {
    if (strcmp(v, "wrap") == 0) {
      style->flex_wrap = 1;
      style->has_flex_wrap = 1;
    } else if (strcmp(v, "nowrap") == 0) {
      style->flex_wrap = 0;
      style->has_flex_wrap = 1;
    }
  } else if (strcmp(p, "grid-template-columns") == 0) {
    style->grid_col_count = 0;
    const char *gv = v;

    if (strncmp(gv, "repeat(", 7) == 0) {
      gv += 7;
      int count = parse_px(gv);
      while (*gv != ',' && *gv != ')') gv++;
      if (*gv == ',') gv++;
      while (*gv == ' ') gv++;

      int size = parse_px(gv);
      int is_fr = (strstr(gv, "fr") != NULL);

      for (int i = 0; i < count && style->grid_col_count < 8; i++) {
        style->grid_cols[style->grid_col_count] = size;
        style->grid_col_is_fr[style->grid_col_count] = is_fr;
        style->grid_col_count++;
      }
    } else {
      while (*gv && style->grid_col_count < 8) {
        while (*gv == ' ') gv++;
        if (*gv >= '0' && *gv <= '9') {
          style->grid_cols[style->grid_col_count] = parse_px(gv);
          style->grid_col_is_fr[style->grid_col_count] = (strstr(gv, "fr") != NULL);
          style->grid_col_count++;
          while (*gv != ' ' && *gv != 0) gv++;
        } else break;
      }
    }
    if (style->grid_col_count > 0) style->has_grid_template = 1;
  } else if (strcmp(p, "gap") == 0 || strcmp(p, "grid-gap") == 0) {
    style->grid_gap = parse_px(v);
    style->has_grid_gap = 1;
  } else if (strcmp(p, "grid-column") == 0) {
    if (strncmp(v, "span ", 5) == 0) {
      style->grid_column_span = parse_px(v + 5);
    } else {
      style->grid_column_span = 1;
    }
    style->has_grid_column_span = 1;
  } else if (strcmp(p, "position") == 0) {
    if (strcmp(v, "relative") == 0) { style->position_type = CSS_POSITION_RELATIVE; style->has_position = 1; }
    else if (strcmp(v, "absolute") == 0) { style->position_type = CSS_POSITION_ABSOLUTE; style->has_position = 1; }
    else if (strcmp(v, "fixed") == 0) { style->position_type = CSS_POSITION_FIXED; style->has_position = 1; }
    else if (strcmp(v, "static") == 0) { style->position_type = CSS_POSITION_STATIC; style->has_position = 1; }
  } else if (strcmp(p, "z-index") == 0) {
    style->z_index = atoi(v);
    style->has_z_index = 1;
  } else if (strcmp(p, "left") == 0) {
    if (strchr(v, '%')) style->left = (parse_px(v) * screen_width) / 100;
    else style->left = parse_px(v);
    style->has_position = 1;
    style->has_left = 1;
  } else if (strcmp(p, "right") == 0) {
    if (strchr(v, '%')) style->right = (parse_px(v) * screen_width) / 100;
    else style->right = parse_px(v);
    style->has_position = 1;
    style->has_right = 1;
  } else if (strcmp(p, "top") == 0) {
    if (strchr(v, '%')) style->top = (parse_px(v) * screen_height) / 100;
    else style->top = parse_px(v);
    style->has_position = 1;
    style->has_top = 1;
  } else if (strcmp(p, "bottom") == 0) {
    if (strchr(v, '%')) style->bottom = (parse_px(v) * screen_height) / 100;
    else style->bottom = parse_px(v);
    style->has_position = 1;
    style->has_bottom = 1;
  } else if (strcmp(p, "background") == 0 && (strstr(v, "gradient") != 0)) {
    css_parse_gradient(v, &style->gradient);
    style->has_gradient = 1;
  } else if (strcmp(p, "background-image") == 0) {
    if (strstr(v, "gradient") != 0) {
      css_parse_gradient(v, &style->gradient);
      style->has_gradient = 1;
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
  memset(sheet, 0, sizeof(css_stylesheet_t));

  const char *p = css_text;

  while (*p) {
    // Skip whitespace and comments
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
      p++;
    if (!*p)
      break;

    // Skip CSS comments /* ... */
    if (*p == '/' && *(p+1) == '*') {
      p += 2;
      while (*p && !(*p == '*' && *(p+1) == '/')) p++;
      if (*p) p += 2;
      continue;
    }

    // Skip dangling } from nested media queries we don't support
    if (*p == '}') {
      p++;
      continue;
    }

    // Parse Selector
    char selector[256] = {0};
    int s_idx = 0;
    while (*p && *p != '{' && s_idx < 255) {
      selector[s_idx++] = *p++;
    }
    selector[s_idx] = '\0';
    trim(selector);

    if (*p == '{')
      p++;

    // Check if this is a CSS variables block (:root { --var: val; })
    if (strncmp(selector, ":root", 5) == 0) {
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

        char val[1024] = {0};
        int v_idx = 0;
        while (*p && *p != ';' && *p != '}') {
          if (v_idx < 1023) val[v_idx++] = *p;
          p++;
        }
        val[v_idx] = 0;

        if (*p == ';')
          p++;

        trim(prop);
        trim(val);

        // Store as variable if it starts with --
        if (strncmp(prop, "--", 2) == 0) {
          css_var_t *var = (css_var_t *)kmalloc(sizeof(css_var_t));
          if (var) {
            memset(var, 0, sizeof(css_var_t));
            strncpy(var->name, prop, 63);
            strncpy(var->value, val, 255);
            var->next = sheet->variables;
            sheet->variables = var;
          }
        }
      }
      if (*p == '}')
        p++;
      continue;
    }

    css_rule_t *rule = (css_rule_t *)kmalloc(sizeof(css_rule_t));
    if (!rule)
      break;
    memset(rule, 0, sizeof(css_rule_t));
    strncpy(rule->selector, selector, 63);

    // Parse selector into structured form
    parse_selector(selector, &rule->parsed_selector);
    rule->parsed_selector_valid = rule->parsed_selector.part_count > 0;

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

      char val[1024] = {0};
      int v_idx = 0;
      while (*p && *p != ';' && *p != '}') {
        if (v_idx < 1023) val[v_idx++] = *p;
        p++;
      }
      val[v_idx] = 0;

      if (*p == ';')
        p++;

      // Resolve CSS variables in value
      trim(prop);
      trim(val);

      // Check for var() references
      char resolved_val[256];
      strncpy(resolved_val, val, 255);
      resolved_val[255] = 0;

      char *var_pos = strstr(resolved_val, "var(--");
      if (var_pos && sheet->variables) {
        // Extract variable name
        char var_name[64];
        const char *vs = var_pos + 4; // skip "var("
        int vni = 0;
        while (*vs && *vs != ')' && *vs != ',' && vni < 63) {
          var_name[vni++] = *vs++;
        }
        var_name[vni] = 0;
        trim(var_name);

        const char *var_val = css_resolve_var(sheet, var_name);
        if (var_val) {
          // Replace var() with value
          char before[256];
          strncpy(before, resolved_val, var_pos - resolved_val);
          before[var_pos - resolved_val] = 0;
          char after[64];
          const char *close_paren = strchr(var_pos + 4, ')');
          if (close_paren) {
            strncpy(after, close_paren + 1, 63);
            after[63] = 0;
          } else {
            after[0] = 0;
          }
          strncpy(resolved_val, before, 255);
          strncat(resolved_val, var_val, 255 - strlen(resolved_val));
          strncat(resolved_val, after, 255 - strlen(resolved_val));
        }
      }

      parse_declaration(&rule->style, prop, resolved_val);
    }

    if (*p == '}')
      p++;

    // Handle comma-separated selectors
    if (strchr(selector, ',')) {
      char sel_copy[256];
      strncpy(sel_copy, selector, 255);
      sel_copy[255] = 0;
      char *token = strtok(sel_copy, ",");
      while (token) {
        trim(token);
        if (token[0] != '\0') {
          css_rule_t *new_rule = (css_rule_t *)kmalloc(sizeof(css_rule_t));
          memcpy(new_rule, rule, sizeof(css_rule_t));
          strncpy(new_rule->selector, token, 63);
          new_rule->selector[63] = '\0';
          parse_selector(token, &new_rule->parsed_selector);
          new_rule->parsed_selector_valid = new_rule->parsed_selector.part_count > 0;
          new_rule->next = sheet->rules;
          sheet->rules = new_rule;
        }
        token = strtok(NULL, ",");
      }
      kfree(rule);
    } else {
      // Append rule to sheet
      rule->next = sheet->rules;
      sheet->rules = rule;
    }
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
  css_var_t *v = sheet->variables;
  while (v) {
    css_var_t *next = v->next;
    kfree(v);
    v = next;
  }
  kfree(sheet);
}

// Selector matching (supports basic and deep selectors)
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
      // Tokenized word match
      char class_copy[256];
      strncpy(class_copy, classes, 255);
      class_copy[255] = 0;
      char *token = strtok(class_copy, " ");
      while (token) {
        if (strcmp(token, selector + 1) == 0) return 1;
        token = strtok(NULL, " ");
      }
    }
  }

  // ID match
  if (selector[0] == '#') {
    const char *id = dom_get_attribute(node, "id");
    if (id && strcmp(id, selector + 1) == 0)
      return 1;
  }

  // Combined selector: tag.class or tag#id
  char tag[32];
  int ti = 0;
  const char *p = selector;
  while (*p && *p != '.' && *p != '#' && ti < 31) {
    tag[ti++] = *p++;
  }
  tag[ti] = 0;

  if (ti > 0 && strcmp(node->tag_name, tag) == 0) {
    if (*p == '.') {
      const char *classes = dom_get_attribute(node, "class");
      if (classes) {
        char class_copy[256];
        strncpy(class_copy, classes, 255);
        class_copy[255] = 0;
        char *token = strtok(class_copy, " ");
        while (token) {
          if (strcmp(token, p + 1) == 0) return 1;
          token = strtok(NULL, " ");
        }
      }
    } else if (*p == '#') {
      const char *id = dom_get_attribute(node, "id");
      if (id && strcmp(id, p + 1) == 0) return 1;
    }
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
        int matched = 0;
        if (r->parsed_selector_valid && r->parsed_selector.part_count > 0) {
          matched = css_match_selector_deep(node, &r->parsed_selector);
        } else {
          matched = match_selector(node, r->selector);
        }

        if (matched) {
          if (r->style.has_color) {
            computed->color = r->style.color;
            computed->has_color = 1;
          }
          if (r->style.has_bg_color) {
            computed->background_color = r->style.background_color;
            computed->has_bg_color = 1;
          }
          if (r->style.has_gradient) {
            computed->gradient = r->style.gradient;
            computed->has_gradient = 1;
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
          if (r->style.has_font_size) {
            computed->font_size = r->style.font_size;
            computed->has_font_size = 1;
          }
          if (r->style.has_font_weight) {
            computed->font_weight = r->style.font_weight;
            computed->has_font_weight = 1;
          }
          if (r->style.has_text_align) {
            computed->text_align = r->style.text_align;
            computed->has_text_align = 1;
          }
          if (r->style.display_none) {
            computed->display_none = 1;
            computed->display_type = CSS_DISPLAY_NONE;
            computed->has_display = 1;
          }
          if (r->style.display_flex || r->style.display_type == CSS_DISPLAY_FLEX) {
            computed->display_flex = 1;
            computed->display_type = CSS_DISPLAY_FLEX;
            computed->has_display = 1;
          }
          if (r->style.has_display && r->style.display_type != CSS_DISPLAY_FLEX) {
            computed->display_type = r->style.display_type;
            computed->has_display = 1;
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
          if (r->style.has_flex_wrap) {
            computed->flex_wrap = r->style.flex_wrap;
            computed->has_flex_wrap = 1;
          }
          if (r->style.has_position) {
            computed->position_type = r->style.position_type;
            computed->left = r->style.left;
            computed->right = r->style.right;
            computed->top = r->style.top;
            computed->bottom = r->style.bottom;
            computed->has_position = 1;
            computed->has_left = r->style.has_left;
            computed->has_right = r->style.has_right;
            computed->has_top = r->style.has_top;
            computed->has_bottom = r->style.has_bottom;
          }
          if (r->style.has_z_index) {
            computed->z_index = r->style.z_index;
            computed->has_z_index = 1;
          }
          if (r->style.has_border) {
            computed->border_width = r->style.border_width;
            computed->border_color = r->style.border_color;
            computed->border_style = r->style.border_style;
            computed->has_border = 1;
          }
          if (r->style.has_grid_template) {
            computed->grid_col_count = r->style.grid_col_count;
            for (int ci = 0; ci < 8; ci++) {
              computed->grid_cols[ci] = r->style.grid_cols[ci];
              computed->grid_col_is_fr[ci] = r->style.grid_col_is_fr[ci];
            }
            computed->has_grid_template = 1;
          }
          if (r->style.has_grid_gap) {
            computed->grid_gap = r->style.grid_gap;
            computed->has_grid_gap = 1;
          }
          if (r->style.has_grid_column_span) {
            computed->grid_column_span = r->style.grid_column_span;
            computed->has_grid_column_span = 1;
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
      const char *tag = node->tag_name;
      if (strcmp(tag, "h1") == 0) {
        computed->margin_top = 10; computed->margin_bottom = 10;
        computed->has_margin = 1;
        computed->font_size = 24; computed->has_font_size = 1;
        computed->font_weight = 1; computed->has_font_weight = 1;
      } else if (strcmp(tag, "h2") == 0) {
        computed->margin_top = 8; computed->margin_bottom = 8;
        computed->has_margin = 1;
        computed->font_size = 18; computed->has_font_size = 1;
        computed->font_weight = 1; computed->has_font_weight = 1;
      } else if (strcmp(tag, "h3") == 0) {
        computed->margin_top = 6; computed->margin_bottom = 6;
        computed->has_margin = 1;
        computed->font_size = 14; computed->has_font_size = 1;
        computed->font_weight = 1; computed->has_font_weight = 1;
      } else if (strcmp(tag, "h4") == 0 || strcmp(tag, "h5") == 0 || strcmp(tag, "h6") == 0) {
        computed->margin_top = 4; computed->margin_bottom = 4;
        computed->has_margin = 1;
        computed->font_weight = 1; computed->has_font_weight = 1;
      } else if (strcmp(tag, "p") == 0) {
        computed->margin_top = 8; computed->margin_bottom = 8;
        computed->has_margin = 1;
      } else if (strcmp(tag, "ul") == 0 || strcmp(tag, "ol") == 0) {
        computed->margin_top = 4; computed->margin_bottom = 4;
        computed->padding_left = 20;
        computed->has_margin = 1; computed->has_padding = 1;
      } else if (strcmp(tag, "li") == 0) {
        computed->margin_top = 2; computed->margin_bottom = 2;
        computed->has_margin = 1;
        computed->display_type = CSS_DISPLAY_LIST_ITEM;
        computed->has_display = 1;
        // Set list style based on parent
        if (node->parent && strcmp(node->parent->tag_name, "ol") == 0) {
          computed->list_style_type = CSS_LIST_DECIMAL;
        } else {
          computed->list_style_type = CSS_LIST_DISC;
        }
        computed->has_list_style = 1;
      } else if (strcmp(tag, "table") == 0) {
        computed->display_type = CSS_DISPLAY_TABLE;
        computed->has_display = 1;
        computed->margin_top = 4; computed->margin_bottom = 4;
        computed->has_margin = 1;
      } else if (strcmp(tag, "tr") == 0) {
        computed->display_type = CSS_DISPLAY_TABLE_ROW;
        computed->has_display = 1;
      } else if (strcmp(tag, "td") == 0 || strcmp(tag, "th") == 0) {
        computed->display_type = CSS_DISPLAY_TABLE_CELL;
        computed->has_display = 1;
        computed->padding_top = 2; computed->padding_bottom = 2;
        computed->padding_left = 4; computed->padding_right = 4;
        computed->has_padding = 1;
        if (strcmp(tag, "th") == 0) {
          computed->font_weight = 1; computed->has_font_weight = 1;
        }
      } else if (strcmp(tag, "b") == 0 || strcmp(tag, "strong") == 0) {
        computed->font_weight = 1; computed->has_font_weight = 1;
      } else if (strcmp(tag, "em") == 0 || strcmp(tag, "i") == 0) {
        // Italic not supported in font, but we note it
      } else if (strcmp(tag, "pre") == 0 || strcmp(tag, "code") == 0) {
        computed->white_space = CSS_WS_PRE;
        computed->has_white_space = 1;
        if (strcmp(tag, "pre") == 0) {
          computed->margin_top = 4; computed->margin_bottom = 4;
          computed->has_margin = 1;
        }
      } else if (strcmp(tag, "blockquote") == 0) {
        computed->margin_top = 6; computed->margin_bottom = 6;
        computed->margin_left = 20;
        computed->has_margin = 1;
        computed->border_width = 3; computed->border_color = 0xCCCCCC;
        computed->border_style = CSS_BORDER_SOLID;
        computed->padding_left = 8; computed->has_padding = 1;
        computed->has_border = 1;
      } else if (strcmp(tag, "a") == 0) {
        computed->color = 0x0000FF;
        computed->has_color = 1;
        computed->text_decoration = CSS_TEXT_DECOR_UNDERLINE;
        computed->has_text_decoration = 1;
      } else if (strcmp(tag, "center") == 0) {
        computed->text_align = CSS_TEXT_ALIGN_CENTER;
        computed->has_text_align = 1;
      }
    }

    dom_node_t *child = node->first_child;
    while (child) {
      css_apply_default_styles(child);
      child = child->next_sibling;
    }
  }
}

// CSS Inheritance: propagate inheritable properties from parent to child
static void inherit_recursive(dom_node_t *node, css_style_t *parent_style) {
  if (!node) return;
  kernel_poll_events();

  if (node->type == DOM_NODE_ELEMENT && node->style) {
    css_style_t *s = (css_style_t *)node->style;
    if (parent_style) {
      // Inherit color if not explicitly set
      if (!s->has_color && parent_style->has_color) {
        s->color = parent_style->color;
        s->has_color = 1;
      }
      // Inherit font-size
      if (!s->has_font_size && parent_style->has_font_size) {
        s->font_size = parent_style->font_size;
        s->has_font_size = 1;
      }
      // Inherit font-weight
      if (!s->has_font_weight && parent_style->has_font_weight) {
        s->font_weight = parent_style->font_weight;
        s->has_font_weight = 1;
      }
      // Inherit text-align
      if (!s->has_text_align && parent_style->has_text_align) {
        s->text_align = parent_style->text_align;
        s->has_text_align = 1;
      }
      // Inherit line-height
      if (!s->has_line_height && parent_style->has_line_height) {
        s->line_height = parent_style->line_height;
        s->has_line_height = 1;
      }
      // Inherit white-space
      if (!s->has_white_space && parent_style->has_white_space) {
        s->white_space = parent_style->white_space;
        s->has_white_space = 1;
      }
      // Inherit list-style
      if (!s->has_list_style && parent_style->has_list_style) {
        s->list_style_type = parent_style->list_style_type;
        s->has_list_style = 1;
      }
    }
    dom_node_t *child = node->first_child;
    while (child) {
      inherit_recursive(child, s);
      child = child->next_sibling;
    }
  } else {
    // Non-element nodes: recurse with same parent style
    dom_node_t *child = node->first_child;
    while (child) {
      inherit_recursive(child, parent_style);
      child = child->next_sibling;
    }
  }
}

void css_inherit_styles(dom_node_t *root) {
  inherit_recursive(root, NULL);
}

