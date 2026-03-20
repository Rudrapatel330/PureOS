// dom.c - PureBrowser Document Object Model
#include "dom.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"

extern void print_serial(const char *);
extern void kernel_poll_events(void);

// Simple global allocator helper for strings
static char *dom_strdup(const char *src) {
  if (!src)
    return 0;
  int len = strlen(src);
  char *str = (char *)kmalloc(len + 1);
  if (str) {
    strcpy(str, src);
  }
  return str;
}

// Case-insensitive tag matcher
static int tag_match(const char *p, const char *tag, int len) {
  for (int i = 0; i < len; i++) {
    char c1 = p[i];
    char c2 = tag[i];
    if (c1 >= 'A' && c1 <= 'Z')
      c1 += 32;
    if (c2 >= 'A' && c2 <= 'Z')
      c2 += 32;
    if (c1 != c2)
      return 1; // Not match
  }
  return 0; // Match
}

dom_node_t *dom_create_element(const char *tag) {
  dom_node_t *node = (dom_node_t *)kmalloc(sizeof(dom_node_t));
  if (!node)
    return 0;

  memset(node, 0, sizeof(dom_node_t));
  node->type = DOM_NODE_ELEMENT;

  strncpy(node->tag_name, tag, DOM_MAX_TAG_LEN - 1);
  node->tag_name[DOM_MAX_TAG_LEN - 1] = '\0';

  return node;
}

dom_node_t *dom_create_text(const char *text) {
  dom_node_t *node = (dom_node_t *)kmalloc(sizeof(dom_node_t));
  if (!node)
    return 0;

  memset(node, 0, sizeof(dom_node_t));
  node->type = DOM_NODE_TEXT;
  node->text_content = dom_strdup(text);

  return node;
}

void dom_append_child(dom_node_t *parent, dom_node_t *child) {
  if (!parent || !child)
    return;

  child->parent = parent;
  child->next_sibling = 0;

  if (!parent->first_child) {
    parent->first_child = child;
    parent->last_child = child;
  } else {
    parent->last_child->next_sibling = child;
    parent->last_child = child;
  }
  parent->child_count++;
}

void dom_set_attribute(dom_node_t *node, const char *name, const char *value) {
  if (!node || node->type != DOM_NODE_ELEMENT)
    return;

  // Check if replacing
  dom_attr_t *curr = node->attributes;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      strncpy(curr->value, value, DOM_MAX_ATTR_VAL - 1);
      return;
    }
    curr = curr->next;
  }

  // Create new
  dom_attr_t *attr = (dom_attr_t *)kmalloc(sizeof(dom_attr_t));
  if (!attr)
    return;

  strncpy(attr->name, name, DOM_MAX_ATTR_NAME - 1);
  attr->name[DOM_MAX_ATTR_NAME - 1] = '\0';

  strncpy(attr->value, value, DOM_MAX_ATTR_VAL - 1);
  attr->value[DOM_MAX_ATTR_VAL - 1] = '\0';

  attr->next = node->attributes;
  node->attributes = attr;
}

const char *dom_get_attribute(dom_node_t *node, const char *name) {
  if (!node || node->type != DOM_NODE_ELEMENT)
    return 0;

  dom_attr_t *curr = node->attributes;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      return curr->value;
    }
    curr = curr->next;
  }
  return 0; // Not found
}

void dom_free_node(dom_node_t *node) {
  if (!node)
    return;

  if (node->type == DOM_NODE_TEXT && node->text_content) {
    kfree(node->text_content);
  }

  dom_attr_t *attr = node->attributes;
  while (attr) {
    dom_attr_t *next = attr->next;
    kfree(attr);
    attr = next;
  }

  dom_node_t *child = node->first_child;
  while (child) {
    dom_node_t *next = child->next_sibling;
    dom_free_node(child);
    child = next;
  }

  kfree(node);
}

static void clean_html_text(char *dest, const char *src, int max_len) {
  int d = 0;
  int s = 0;
  int in_space = 0;

  // Optional: trim leading whitespace unconditionally. For now, we just
  // collapse.
  int iters = 0;
  while (src[s] && d < max_len - 1) {
    if (++iters > 100) {
      kernel_poll_events();
      iters = 0;
    }
    unsigned char c = (unsigned char)src[s];
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
      if (!in_space) {
        if (d > 0)
          dest[d++] = ' ';
        in_space = 1;
      }
      s++;
    } else if (c == '&') {
      in_space = 0;
      if (strncmp(&src[s], "&nbsp;", 6) == 0) {
        dest[d++] = ' ';
        s += 6;
      } else if (strncmp(&src[s], "&amp;", 5) == 0) {
        dest[d++] = '&';
        s += 5;
      } else if (strncmp(&src[s], "&lt;", 4) == 0) {
        dest[d++] = '<';
        s += 4;
      } else if (strncmp(&src[s], "&gt;", 4) == 0) {
        dest[d++] = '>';
        s += 4;
      } else if (strncmp(&src[s], "&raquo;", 7) == 0) {
        dest[d++] = '>';
        s += 7;
      } else if (strncmp(&src[s], "&copy;", 6) == 0) {
        dest[d++] = 'C';
        s += 6;
      } else if (strncmp(&src[s], "&#", 2) == 0) {
        s += 2;
        while (src[s] >= '0' && src[s] <= '9')
          s++;
        if (src[s] == ';')
          s++;
        dest[d++] = ' '; // Replace unknown numerical entities with space
      } else {
        dest[d++] = '&';
        s++;
      }
    } else if (c >= 0x80) {
      // Non-ASCII detected.
      // PureBrowser's font only handles ASCII.
      // UTF-8 lead bytes and continuation bytes should be skipped/merged.
      // For now, let's just skip all high-bit characters to avoid "???"
      // rendering.
      s++;
      in_space = 0;
    } else {
      in_space = 0;
      dest[d++] = c;
      s++;
    }
  }

  // Trim trailing space if any
  if (d > 0 && dest[d - 1] == ' ')
    d--;
  dest[d] = '\0';
}

// Returns the advancing pointer. Null on error.
static const char *parse_node_recursive(const char *p, dom_node_t *stack_parent,
                                        int depth) {
  if (depth > 32)
    return p; // Recursion limit

  int node_iters = 0;
  while (*p) {
    if (++node_iters > 50) {
      kernel_poll_events();
      node_iters = 0;
    }
    // Skip whitespace if not in a text node logically
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
      p++;
    if (!*p)
      break;

    if (*p == '<') {
      p++; // Skip '<'

      // Check closing tag
      if (*p == '/') {
        p++; // Skip '/'
        // We assume well-formed for now. Advance past tag name and closing '>'
        while (*p && *p != '>')
          p++;
        if (*p == '>')
          p++;
        return p; // Returning drops us up a level
      }

      // Handle Comments (rudimentary)
      if (strncmp(p, "!--", 3) == 0) {
        while (*p && strncmp(p, "-->", 3) != 0)
          p++;
        if (*p)
          p += 3;
        continue;
      }

      // Read Tag Name
      char tag[DOM_MAX_TAG_LEN] = {0};
      int t_idx = 0;
      while (*p && *p != ' ' && *p != '>' && *p != '/' &&
             t_idx < DOM_MAX_TAG_LEN - 1) {
        char c = *p++;
        if (c >= 'A' && c <= 'Z')
          c += 32; // Lowercase normalize
        tag[t_idx++] = c;
      }
      tag[t_idx] = '\0';

      dom_node_t *element = dom_create_element(tag);
      if (stack_parent) {
        dom_append_child(stack_parent, element);
      }

      // Read attributes (very simplified)
      while (*p && *p != '>' && *p != '/') {
        while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
          p++;

        if (*p == '>' || *p == '/')
          break;

        char attr_name[DOM_MAX_ATTR_NAME] = {0};
        char attr_val[DOM_MAX_ATTR_VAL] = {0};
        int a_idx = 0;

        while (*p && *p != '=' && *p != ' ' && *p != '>' &&
               a_idx < DOM_MAX_ATTR_NAME - 1) {
          attr_name[a_idx++] = *p++;
        }
        attr_name[a_idx] = '\0';

        if (*p == '=') {
          p++; // Skip '='
          char quote = 0;
          if (*p == '"' || *p == '\'') {
            quote = *p++;
          }

          int v_idx = 0;
          while (*p && *p != '>' && v_idx < DOM_MAX_ATTR_VAL - 1) {
            if (quote && *p == quote) {
              p++; // Skip closing quote
              break;
            } else if (!quote && (*p == ' ' || *p == '>')) {
              break;
            }
            attr_val[v_idx++] = *p++;
          }
          attr_val[v_idx] = '\0';

          if (attr_name[0] != '\0') {
            dom_set_attribute(element, attr_name, attr_val);
          }
        }
      }

      // Check self-closing things like <img />
      int self_closing = 0;
      if (*p == '/') {
        self_closing = 1;
        p++;
      }
      // Void elements in HTML5 don't need closing tags
      if (strcmp(tag, "br") == 0 || strcmp(tag, "img") == 0 ||
          strcmp(tag, "hr") == 0 || strcmp(tag, "meta") == 0 ||
          strcmp(tag, "link") == 0 || strcmp(tag, "input") == 0) {
        self_closing = 1;
      }

      if (*p == '>')
        p++;

      // If script or style, capture content into a text node but don't parse it
      // as HTML.
      if (strcmp(tag, "script") == 0 || strcmp(tag, "style") == 0) {
        const char *content_start = p;
        int content_len = 0;

        // Skip until closing tag
        while (*p) {
          if (*p == '<' && *(p + 1) == '/') {
            const char *cp = p + 2;
            int cl_idx = 0;
            char cl_tag[32] = {0};
            while (*cp && *cp != '>' && cl_idx < 31) {
              char c = *cp++;
              if (c >= 'A' && c <= 'Z')
                c += 32;
              cl_tag[cl_idx++] = c;
            }
            if (strcmp(cl_tag, tag) == 0) {
              // Found closing tag. Create a text node
              if (content_len > 0) {
                char *buf = (char *)kmalloc(content_len + 1);
                if (buf) {
                  strncpy(buf, content_start, content_len);
                  buf[content_len] = '\0';
                  dom_node_t *text_el = dom_create_text(buf);
                  dom_append_child(element, text_el);
                  kfree(buf);
                }
              }

              p = cp;
              if (*p == '>')
                p++;
              break;
            }
          }
          content_len++;
          p++;
        }
        self_closing = 1; // don't parse children recursively.
      }

      if (!self_closing) {
        // Parse children recursively
        p = parse_node_recursive(p, element, depth + 1);
      }

    } else {
      // It's a Text Node
      char *buf =
          (char *)kmalloc(4096); // Increased for larger chunks, but capped
      if (!buf)
        return p;
      memset(buf, 0, 4096);

      int idx = 0;
      while (*p && *p != '<' && idx < 4095) {
        buf[idx++] = *p++;
      }
      buf[idx] = '\0';

      char *clean_buf = (char *)kmalloc(4096);
      if (clean_buf) {
        memset(clean_buf, 0, 4096);
        clean_html_text(clean_buf, buf, 4096);

        if (clean_buf[0] != '\0' && stack_parent) {
          dom_node_t *text_el = dom_create_text(clean_buf);
          dom_append_child(stack_parent, text_el);
        }
        kfree(clean_buf);
      }
      kfree(buf);
    }
  }
  return p;
}

dom_node_t *dom_parse_html(const char *html_str) {
  if (!html_str)
    return 0;

  // Creates a pseudo-document root
  dom_node_t *document = dom_create_element("document");

  parse_node_recursive(html_str, document, 0);

  return document;
}

// Debug Print
void dom_print_tree(dom_node_t *root, int depth) {
  if (!root)
    return;

  for (int i = 0; i < depth; i++)
    print_serial("  ");

  if (root->type == DOM_NODE_ELEMENT) {
    print_serial("<");
    print_serial(root->tag_name);

    dom_attr_t *a = root->attributes;
    while (a) {
      print_serial(" ");
      print_serial(a->name);
      print_serial("='");
      print_serial(a->value);
      print_serial("'");
      a = a->next;
    }
    print_serial(">\n");

    dom_node_t *child = root->first_child;
    while (child) {
      dom_print_tree(child, depth + 1);
      child = child->next_sibling;
    }

  } else if (root->type == DOM_NODE_TEXT) {
    print_serial("\"");
    // Limit print size
    int len = strlen(root->text_content);
    if (len > 40)
      len = 40;
    char tmp[48]; // Increased size from 42 to 48 for safety
    strncpy(tmp, root->text_content, len);
    tmp[len] = '\0';
    if (strlen(root->text_content) > 40) {
      // tmp has len(40) + \0 = 41 bytes.
      // index 40 is \0. index 41, 42, 43 can take "..."
      tmp[40] = '.';
      tmp[41] = '.';
      tmp[42] = '.';
      tmp[43] = '\0';
    }
    print_serial(tmp);
    print_serial("\"\n");
  }
}
