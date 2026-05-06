// dom.c - PureBrowser Document Object Model
#include "dom.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"

extern void print_serial(const char *);
extern void kernel_poll_events(void);

#define MAX_TAG_STACK 128

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

static int dom_strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

static const char* void_tags[] = {
    "area", "base", "br", "col", "embed", "hr", "img", "input",
    "link", "meta", "param", "source", "track", "wbr", "!doctype", 0
};

static int is_void_tag(const char* tag) {
    for (int i = 0; void_tags[i]; i++) {
        if (dom_strcasecmp(tag, void_tags[i]) == 0) return 1;
    }
    return 0;
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
    
    static int free_poll = 0;
    if (++free_poll > 100) {
      kernel_poll_events();
      free_poll = 0;
    }
  }

  kfree(node);
}

static void clean_html_text(char *dest, const char *src, int max_len) {
  int d = 0;
  int s = 0;
  int in_space = 0;
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
        dest[d++] = ' ';
      } else {
        dest[d++] = '&';
        s++;
      }
    } else if (c >= 0x80) {
      s++;
      in_space = 0;
    } else {
      in_space = 0;
      dest[d++] = c;
      s++;
    }
  }
  if (d > 0 && dest[d - 1] == ' ') d--;
  dest[d] = '\0';
}

dom_node_t *dom_parse_html(const char *html) {
    if (!html) return NULL;
    
    dom_node_t *root = dom_create_element("root");
    dom_node_t *current_parent = root;
    dom_node_t *stack[MAX_TAG_STACK];
    int stack_ptr = 0;
    stack[stack_ptr++] = root;

    const char *p = html;
    int iters = 0;

    while (*p) {
        if (++iters > 100) {
            kernel_poll_events();
            iters = 0;
        }

        if (*p == '<') {
            if (p[1] == '!') {
                p += 2;
                if (strncmp(p, "DOCTYPE", 7) == 0 || strncmp(p, "doctype", 7) == 0) {
                    while (*p && *p != '>') p++;
                    if (*p) p++;
                    continue;
                }
                while (*p && !(*p == '-' && *(p+1) == '-' && *(p+2) == '>')) p++;
                if (*p) p += 3;
                continue;
            }

            if (p[1] == '/') {
                p += 2;
                char tag_name[64];
                int i = 0;
                while (*p && *p != '>' && *p != ' ' && i < 63) tag_name[i++] = *p++;
                tag_name[i] = 0;
                while (*p && *p != '>') p++;
                if (*p) p++;

                if (stack_ptr > 1) {
                    stack_ptr--;
                    current_parent = stack[stack_ptr-1];
                }
                continue;
            }

            p++;
            char tag_name[64];
            int i = 0;
            while (*p && *p != '>' && *p != ' ' && *p != '/' && i < 63) tag_name[i++] = *p++;
            tag_name[i] = 0;

            dom_node_t *node = dom_create_element(tag_name);
            
            while (*p && *p != '>' && *p != '/') {
                while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
                if (*p == '>' || *p == '/') break;
                
                char attr_name[64];
                int ai = 0;
                while (*p && *p != '=' && *p != ' ' && *p != '>' && ai < 63) attr_name[ai++] = *p++;
                attr_name[ai] = 0;
                
                if (*p == '=') {
                    p++;
                    char quote = 0;
                    if (*p == '"' || *p == '\'') quote = *p++;
                    char attr_val[512];
                    int vi = 0;
                    if (quote) {
                        while (*p && *p != quote && vi < 511) attr_val[vi++] = *p++;
                        if (*p) p++;
                    } else {
                        while (*p && *p != ' ' && *p != '>' && vi < 511) attr_val[vi++] = *p++;
                    }
                    attr_val[vi] = 0;
                    dom_set_attribute(node, attr_name, attr_val);
                }
            }

            int self_closing = (*p == '/');
            if (self_closing) p++;
            while (*p && *p != '>') p++;
            if (*p) p++;

            dom_append_child(current_parent, node);

            // Special handling for scripts and styles: don't parse children as HTML
            if (strcmp(tag_name, "script") == 0 || strcmp(tag_name, "style") == 0) {
                const char *content_start = p;
                int content_len = 0;
                while (*p) {
                    if (*p == '<' && *(p + 1) == '/') {
                        const char *tp = p + 2;
                        if (strncmp(tp, tag_name, strlen(tag_name)) == 0) break;
                    }
                    content_len++;
                    p++;
                }

                if (content_len > 0) {
                    char *buf = kmalloc(content_len + 1);
                    strncpy(buf, content_start, content_len);
                    buf[content_len] = 0;
                    
                    if (strcmp(tag_name, "script") == 0) {
                        extern void js_execute(const char *);
                        js_execute(buf);
                    } else {
                        // Styles handled separately or stored in node
                    }
                    
                    // Also add as a text node so it's in the DOM
                    dom_node_t *tnode = dom_create_text(buf);
                    dom_append_child(node, tnode);
                    kfree(buf);
                }

                if (*p) {
                    while (*p && *p != '>') p++;
                    if (*p) p++;
                }
                self_closing = 1;
            }

            if (!self_closing && !is_void_tag(tag_name)) {
                if (stack_ptr < MAX_TAG_STACK) {
                    stack[stack_ptr++] = node;
                    current_parent = node;
                }
            }
        } else {
            const char *start = p;
            while (*p && *p != '<') p++;
            int len = p - start;
            if (len > 0) {
                char *raw_text = kmalloc(len + 1);
                memcpy(raw_text, start, len);
                raw_text[len] = 0;
                
                char *clean_text = kmalloc(len + 1);
                clean_html_text(clean_text, raw_text, len + 1);
                kfree(raw_text);
                
                if (strlen(clean_text) > 0) {
                    dom_node_t *tnode = dom_create_text(clean_text);
                    dom_append_child(current_parent, tnode);
                }
                kfree(clean_text);
            }
        }
    }
    return root;
}

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
    int len = strlen(root->text_content);
    if (len > 40)
      len = 40;
    char tmp[48];
    strncpy(tmp, root->text_content, len);
    tmp[len] = '\0';
    if (strlen(root->text_content) > 40) {
      tmp[40] = '.';
      tmp[41] = '.';
      tmp[42] = '.';
      tmp[43] = '\0';
    }
    print_serial(tmp);
    print_serial("\"\n");
  }
}
