#include "../kernel/heap.h"
#include "../kernel/simd.h"
#include "../kernel/spinlock.h"
#include "../kernel/string.h"
#include "../kernel/window.h"
#include "../net/net.h"
#include "browser/css.h"
#include "browser/dom.h"
#include "browser/js.h"
#include "browser/layout.h"

extern void print_serial(const char *);
extern int ui_dirty;

static void browser_navigate(const char *input);

// Browser state
static char url_bar[256] = "about:home";
static int url_cursor = 10;
static int url_editing = 0;
static dom_node_t *focused_node = 0;
char input_buffer[256];
int input_cursor = 0;

// BSS Hardening Sentinels
static uint32_t bss_sentinel_1 = 0xDEADBEEF;
dom_node_t *current_document = 0;
css_stylesheet_t *current_stylesheet = 0;
static uint32_t bss_sentinel_2 = 0xCAFEEBEE;
render_node_t *current_render_tree = 0;
static uint32_t bss_sentinel_3 = 0xBAADF00D;

// Page content
#define PAGE_MAX 131072
static char page_content[PAGE_MAX];
static int page_len = 0;
static int scroll_y = 0;
static int layout_dirty = 1;        // 1 = need to recalculate layout
static int cached_layout_width = 0; // last width used for layout
static uint32_t *content_backing_store = NULL;
static int backing_store_dirty = 1; // 1 = need to redraw content to cache

// History
#define HISTORY_MAX 8
static char history[HISTORY_MAX][256];
static int history_pos = -1;
static int history_count = 0;

// Loading state
static int is_loading = 0;
static int browser_updating = 0;
char status_text[128] = "PureBrowser (FrogFind Enabled)";

// Browser window dimensions
#define TOOLBAR_H 30
#define URLBAR_X 80
#define URLBAR_W 390
#define CONTENT_Y (TOOLBAR_H + 2)

// Simple HTML token types
#define TOK_TEXT 0
#define TOK_H1 1
#define TOK_H2 2
#define TOK_P 3
#define TOK_BR 4
#define TOK_HR 5
#define TOK_BOLD 6
#define TOK_LINK 7

// Parsed page element
typedef struct {
  int type;
  char text[128];
  char href[128];
} page_element_t;

#define MAX_ELEMENTS 128
static page_element_t elements[MAX_ELEMENTS];
static int element_count = 0;

// Link hitboxes for click navigation (relative to window)
typedef struct {
  int x, y, w, h;
  char href[128];
} link_hitbox_t;

static link_hitbox_t link_boxes[128];
static int link_count = 0;

typedef struct {
  int x, y, w, h;
  dom_node_t *node;
} form_hitbox_t;

static form_hitbox_t form_boxes[128];
static int form_count = 0;

// Set default home page content
static void browser_set_home(void) {
  strcpy(page_content, "<h1>PureBrowser</h1>"
                       "<p>Welcome to PureBrowser 1.0 for PureOS!</p>"
                       "<hr>"
                       "<h2>Quick Links</h2>"
                       "<p><a href=\"http://example.com\">Example.com</a></p>"
                       "<p><a href=\"about:system\">System Information</a></p>"
                       "<p><a href=\"about:help\">Browser Help</a></p>"
                       "<hr>"
                       "<p>This browser requires a network-enabled emulator</p>"
                       "<p>(QEMU with SLIRP) to access real websites.</p>");
  page_len = strlen(page_content);
}

static void browser_set_sysinfo(void) {
  strcpy(page_content, "<h1>System Information</h1>"
                       "<hr>"
                       "<p>OS: PureOS 1.0</p>"
                       "<p>Resolution: 1024x768 (16bpp)</p>"
                       "<p>Memory: 32MB (14MB Heap)</p>"
                       "<p>CPU: i686 Protected Mode</p>"
                       "<p>Network: NE2000 NIC</p>"
                       "<p>IP: 10.0.2.15</p>"
                       "<p>Gateway: 10.0.2.2</p>"
                       "<hr>"
                       "<p><a href=\"about:home\">Back to Home</a></p>");
  page_len = strlen(page_content);
}

static void browser_set_help(void) {
  strcpy(page_content, "<h1>Browser Help</h1>"
                       "<hr>"
                       "<h2>Navigation</h2>"
                       "<p>Click the address bar and type a URL.</p>"
                       "<p>Press Enter to navigate.</p>"
                       "<p>Use Back/Forward buttons for history.</p>"
                       "<p>Click blue links to follow them.</p>"
                       "<h2>Supported HTML Tags</h2>"
                       "<p>h1, h2, p, br, hr, b, a href, script</p>"
                       "<hr>"
                       "<p><a href=\"about:home\">Back to Home</a></p>");
  page_len = strlen(page_content);
}

static void browser_set_js_test(void) {
  strcpy(page_content, "<h1>JavaScript Test</h1>"
                       "<hr>"
                       "<p id=\"test\">Waiting for JS to run...</p>"
                       "<script>"
                       "  var el = document.getElementById(\"test\");"
                       "  el.textContent = \"JS Executed Successfully!\";"
                       "</script>"
                       "<hr>"
                       "<p><a href=\"about:home\">Back to Home</a></p>");
  page_len = strlen(page_content);
}

static void browser_set_form_test(void) {
  strcpy(page_content,
         "<h1>Phase 7: Forms Verified</h1>"
         "<hr>"
         "<p>If you see this title, the browser is running the latest code.</p>"
         "<h2>Web Search</h2>"
         "<form action=\"http://frogfind.com/\">"
         "  <p>Search: <input type=\"text\" name=\"q\" value=\"PureOS\"></p>"
         "  <p><button type=\"submit\">Search FrogFind</button></p>"
         "</form>"
         "<hr>"
         "<p><a href=\"about:home\">Back to Home</a></p>");
  page_len = strlen(page_content);
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

// Memory guards removed (consolidated at top)

// Helper to find <style> tags and concat them
static void extract_styles_recursive(dom_node_t *node, char *css_buf,
                                     int max_len) {
  if (!node)
    return;
  if (node->type == DOM_NODE_ELEMENT && strcmp(node->tag_name, "style") == 0) {
    // Collect text children
    dom_node_t *child = node->first_child;
    while (child) {
      if (child->type == DOM_NODE_TEXT && child->text_content) {
        int clen = strlen(css_buf);
        int add_len = strlen(child->text_content);
        if (clen + add_len < max_len - 2) {
          strcat(css_buf, child->text_content);
          strcat(css_buf, "\n");
        }
      }
      child = child->next_sibling;
    }
  }

  dom_node_t *child = node->first_child;
  while (child) {
    extract_styles_recursive(child, css_buf, max_len);
    child = child->next_sibling;
  }
}

// Helper to find and execute <script> tags
static void execute_scripts_recursive(dom_node_t *node) {
  if (!node)
    return;

  if (node->type == DOM_NODE_ELEMENT && strcmp(node->tag_name, "script") == 0) {
    // Basic inline script execution
    dom_node_t *child = node->first_child;
    while (child) {
      if (child->type == DOM_NODE_TEXT && child->text_content) {
        js_execute(child->text_content);
      }
      child = child->next_sibling;
    }
  }

  dom_node_t *child = node->first_child;
  while (child) {
    execute_scripts_recursive(child);
    child = child->next_sibling;
  }
}

// Simple HTML parser (now uses DOM)
static void browser_parse_html(void) {
  print_serial("BROWSER: parse_html: Starting...\n");
  __asm__ volatile("cli");
  browser_updating = 1;

  // Safe cleanup with extra validation
  print_serial("BROWSER: parse_html: Freeing old trees...\n");
  if (current_render_tree &&
      ((uintptr_t)current_render_tree & 0xF0000000) == 0xC0000000) {
    layout_free_tree(current_render_tree);
  }
  current_render_tree = 0;

  print_serial("BROWSER: parse_html: Freeing old document...\n");
  if (current_document &&
      ((uintptr_t)current_document & 0xF0000000) == 0xC0000000) {
    dom_free_node(current_document);
  }
  current_document = 0;

  print_serial("BROWSER: parse_html: Freeing old stylesheet...\n");
  if (current_stylesheet &&
      ((uintptr_t)current_stylesheet & 0xF0000000) == 0xC0000000) {
    css_free_stylesheet(current_stylesheet);
  }
  current_stylesheet = 0;

  static spinlock_irq_t browser_parse_lock = {0};
  spinlock_irq_acquire(&browser_parse_lock);
  browser_updating = 1;
  __asm__ volatile("sti");
  print_serial("BROWSER: parse_html: Cleanup done, parsing new...\n");

  if (page_len > 0 && page_content[0] != '\0') {
    print_serial("BROWSER: parse_html: Parsing DOM...\n");
    current_document = dom_parse_html(page_content);
    if (!current_document) {
      print_serial("BROWSER: ERROR: dom_parse_html returned NULL!\n");
    } else {
      char c_str[16];
      k_itoa(current_document->child_count, c_str);
      print_serial("BROWSER: DOM root has ");
      print_serial(c_str);
      print_serial(" children\n");
    }
    print_serial("BROWSER: parse_html: DOM parsed\n");

    // Extract CSS
    print_serial("BROWSER: parse_html: Extracting CSS...\n");
    char *css_buffer = kmalloc(8192); // 8KB CSS limit for now
    if (css_buffer) {
      css_buffer[0] = '\0';
      extract_styles_recursive(current_document, css_buffer, 8192);
      if (strlen(css_buffer) > 0) {
        current_stylesheet = css_parse(css_buffer);
      }
      kfree(css_buffer);
    }
    print_serial("BROWSER: parse_html: CSS done\n");

    // Apply default styles first
    print_serial("BROWSER: parse_html: Applying default styles...\n");
    css_apply_default_styles(current_document);

    // Apply parsed CSS
    if (current_stylesheet) {
      css_apply_styles(current_document, current_stylesheet);
    }
  }

  __asm__ volatile("cli");
  browser_updating = 0;
  spinlock_irq_release(&browser_parse_lock);
  __asm__ volatile("sti");

  // Execute scripts OUTSIDE the lock to keep UI responsive
  if (current_document) {
    execute_scripts_recursive(current_document);
  }

  print_serial("BROWSER: parse_html: Complete\n");
}

// Helper: Check if input is a search query or a direct URL
static int is_search_query(const char *input) {
  if (strncmp(input, "about:", 6) == 0)
    return 0;
  if (strncmp(input, "http://", 7) == 0 || strncmp(input, "https://", 8) == 0)
    return 0;

  // If it has spaces, it's definitely a search query
  const char *p = input;
  while (*p) {
    if (*p == ' ')
      return 1;
    p++;
  }

  // If it has a dot, check if it's domain-like (dot followed by letters)
  int has_dot = 0;
  p = input;
  while (*p) {
    if (*p == '.') {
      has_dot = 1;
      p++;
      // If we see letters after a dot, it's likely a URL (e.g., .com, .org, or
      // domain.tld/path)
      if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) {
        return 0;
      }
    } else {
      p++;
    }
  }

  if (!has_dot)
    return 1; // No dots at all -> search

  return 0;
}

// Submit a form by collecting all its inputs
static void browser_submit_form(dom_node_t *node) {
  dom_node_t *form = node;
  while (form && strcmp(form->tag_name, "form") != 0) {
    form = form->parent;
  }
  if (!form)
    return;

  const char *action = dom_get_attribute(form, "action");
  char new_url[512];
  if (action) {
    if (strncmp(action, "http://", 7) == 0 ||
        strncmp(action, "about:", 6) == 0) {
      strcpy(new_url, action);
    } else {
      // Relative URL - very simple join
      if (strncmp(url_bar, "http://", 7) == 0) {
        strcpy(new_url, url_bar);
        char *slash = strchr(new_url + 7, '/');
        if (slash)
          *slash = 0;
      } else {
        strcpy(new_url, "http://frogfind.com");
      }
      if (action[0] != '/') {
        int clen = strlen(new_url);
        strncat(new_url, "/", 511 - clen);
      }
      int clen = strlen(new_url);
      strncat(new_url, action, 511 - clen);
    }
  } else {
    strncpy(new_url, url_bar, 511);
    new_url[511] = 0;
  }

  int cur_len = strlen(new_url);
  if (!strchr(new_url, '?')) {
    strncat(new_url, "?", 511 - cur_len);
  } else {
    strncat(new_url, "&", 511 - cur_len);
  }

  for (int i = 0; i < form_count; i++) {
    dom_node_t *fnode = form_boxes[i].node;
    if (strcmp(fnode->tag_name, "input") == 0) {
      // Basic descendant check
      dom_node_t *temp = fnode;
      int is_descendant = 0;
      while (temp) {
        if (temp == form) {
          is_descendant = 1;
          break;
        }
        temp = temp->parent;
      }

      if (is_descendant) {
        const char *name = dom_get_attribute(fnode, "name");
        const char *val = dom_get_attribute(fnode, "value");
        if (name && val) {
          int clen = strlen(new_url);
          strncat(new_url, name, 511 - clen);
          clen = strlen(new_url);
          strncat(new_url, "=", 511 - clen);
          int start_idx = strlen(new_url);
          strncat(new_url, val, 511 - clen - 1);
          for (int j = start_idx; new_url[j]; j++) {
            if (new_url[j] == ' ')
              new_url[j] = '+';
          }
          clen = strlen(new_url);
          strncat(new_url, "&", 511 - clen);
        }
      }
    }
  }

  int flen = strlen(new_url);
  if (flen > 0 && (new_url[flen - 1] == '&' || new_url[flen - 1] == '?'))
    new_url[flen - 1] = 0;

  browser_navigate(new_url);
}

// Navigate to a URL or search query
static void browser_navigate(const char *input) {
  if (is_loading) {
    return;
  }
  print_serial("BROWSER: Navigating to: ");
  print_serial(input);
  print_serial("\n");
  char url[256];

  if (is_search_query(input)) {
    strcpy(url, "http://www.google.com/search?udm=14&tbs=li:1&q=");
  } else {
    if (strncmp(input, "about:", 6) == 0 || strncmp(input, "http://", 7) == 0) {
      strncpy(url, input, 255);
      url[255] = 0;
    } else if (strncmp(input, "https://", 8) == 0) {
      strncpy(url, input, 255);
      url[255] = 0;
    } else {
      strcpy(url, "https://");
      strncat(url, input, 255 - 8);
      url[255] = 0;
    }
  }

  scroll_y = 0;
  link_count = 0;
  strcpy(status_text, "Loading...");
  is_loading = 1;
  backing_store_dirty = 1;

  __asm__ volatile("cli");
  browser_updating = 1;
  // Invalidate old trees immediately to prevent draw racer
  if (current_render_tree) {
    layout_free_tree(current_render_tree);
    current_render_tree = 0;
  }
  if (current_document) {
    dom_free_node(current_document);
    current_document = 0;
  }
  __asm__ volatile("sti");

  if (history_pos < 0 || strcmp(history[history_pos], url) != 0) {
    history_pos++;
    if (history_pos >= HISTORY_MAX)
      history_pos = HISTORY_MAX - 1;
    strncpy(history[history_pos], url, 255);
    history[history_pos][255] = 0;
    history_count = history_pos + 1;
  }

  strncpy(url_bar, url, 255);
  url_bar[255] = 0;
  url_cursor = strlen(url_bar);

  if (strncmp(url, "about:home", 10) == 0) {
    browser_set_home();
  } else if (strncmp(url, "about:system", 12) == 0) {
    browser_set_sysinfo();
  } else if (strncmp(url, "about:help", 10) == 0) {
    browser_set_help();
  } else if (strncmp(url, "about:js", 8) == 0) {
    browser_set_js_test();
  } else if (strncmp(url, "about:forms", 11) == 0) {
    browser_set_form_test();
  } else {
    int redirects = 0;
    char fetch_url[256];
    strncpy(fetch_url, url, 255);
    fetch_url[255] = 0;

    while (redirects < 5) {
      int result;
      if (strncmp(fetch_url, "https://", 8) == 0) {
        result = https_get(fetch_url, page_content, PAGE_MAX - 1);
      } else {
        result = http_get(fetch_url, page_content, PAGE_MAX - 1);
      }

      if (result == -301) {
        if (strncmp(page_content, "REDIRECT:", 9) == 0) {
          char *redir_url = page_content + 9;

          // Handle relative redirects
          if (redir_url[0] == '/') {
            char base[256];
            // Extract protocol and host from current fetch_url
            const char *p = fetch_url;
            int len = 0;
            if (strncmp(p, "https://", 8) == 0) {
              len = 8;
            } else if (strncmp(p, "http://", 7) == 0) {
              len = 7;
            }
            while (p[len] && p[len] != '/') {
              len++;
            }
            memcpy(base, fetch_url, len);
            base[len] = 0;

            char full_url[256];
            strcpy(full_url, base);
            strncat(full_url, redir_url, 255 - strlen(base));
            strncpy(fetch_url, full_url, 255);
          } else {
            strncpy(fetch_url, redir_url, 255);
          }

          fetch_url[255] = 0;
          strncpy(url_bar, fetch_url, 255);
          url_bar[255] = 0;
          url_cursor = strlen(url_bar);
          redirects++;
          continue;
        }
      }
      if (result > 0) {
        page_len = result;
        strcpy(status_text, "Done");
        print_serial("BROWSER: Received ");
        char p_str[16];
        k_itoa(page_len, p_str);
        print_serial(p_str);
        print_serial(" bytes. First 40: ");
        char head[41];
        memcpy(head, page_content, 40);
        head[40] = 0;
        print_serial(head);
        print_serial("\n");
      } else {
        strcpy(page_content, "<h1>Error Loading Page (");
        char res_str[16];
        k_itoa(result, res_str);
        strcat(page_content, res_str);
        strcat(page_content, ")</h1>");
        page_len = strlen(page_content);
        strcpy(status_text, "Error");
      }
      break;
    }
  }

  is_loading = 0;
  strcpy(status_text, "Ready");
  browser_parse_html();

  __asm__ volatile("cli");
  browser_updating = 0;
  __asm__ volatile("sti");

  ui_dirty = 1;
}

// ====== Window Callbacks ======

static void browser_draw_render_node(window_t *win, render_node_t *node,
                                     int offset_x, int offset_y, int max_y,
                                     int min_y, int cw) {
  if (!node)
    return;

  int render_x = offset_x + node->box.x;
  int render_y = offset_y + node->box.y;

  if (node->style && node->style->has_bg_color && node->box.width > 0 &&
      node->box.height > 0) {
    int bg_y = render_y;
    int bg_h = node->box.height;
    if (bg_y < min_y) {
      bg_h -= (min_y - bg_y);
      bg_y = min_y;
    }
    if (bg_y + bg_h > max_y) {
      bg_h = max_y - bg_y;
    }
    if (bg_h > 0) {
      winmgr_fill_rect(win, render_x, bg_y, node->box.width, bg_h,
                       node->style->background_color);
    }
  }

  if (node->is_text && node->dom_node && node->dom_node->text_content) {
    int text_h = 8;
    int is_h1 = 0, is_h2 = 0, is_link = 0;

    if (node->parent && node->parent->dom_node) {
      if (strcmp(node->parent->dom_node->tag_name, "h1") == 0)
        is_h1 = 1;
      if (strcmp(node->parent->dom_node->tag_name, "h2") == 0)
        is_h2 = 1;
      if (strcmp(node->parent->dom_node->tag_name, "a") == 0)
        is_link = 1;
    }

    if (is_h1 || is_h2)
      text_h = 16;

    if (render_y + text_h > min_y && render_y < max_y) {
      int color =
          node->style && node->style->has_color ? node->style->color : 0xFF000000;
      if (is_link)
        color = 0xFF0000FF;

      if (is_h1) {
        winmgr_draw_text(win, render_x, render_y, node->dom_node->text_content,
                         color);
        winmgr_draw_text(win, render_x + 1, render_y,
                         node->dom_node->text_content, color);
      } else {
        winmgr_draw_text(win, render_x, render_y, node->dom_node->text_content,
                         color);
      }

      if (is_link) {
        int tlen = strlen(node->dom_node->text_content);
        if (render_y + text_h + 2 < max_y) {
          winmgr_draw_rect(win, render_x, render_y + text_h + 2, tlen * 8, 1,
                           color);
        }
      }
    }
  } else if (node->dom_node && node->dom_node->type == DOM_NODE_ELEMENT) {
    if (strcmp(node->dom_node->tag_name, "hr") == 0) {
      if (render_y + 4 > min_y && render_y < max_y) {
        winmgr_draw_rect(win, render_x, render_y + 4, cw - 16, 1, 0x888888);
      }
    } else if (strcmp(node->dom_node->tag_name, "input") == 0) {
      if (render_y + 20 > min_y && render_y < max_y) {
        uint32_t border_col =
            (focused_node == node->dom_node) ? 0xFF00FF00 : 0xFF888888;
        winmgr_fill_rect(win, render_x, render_y, node->box.width,
                         node->box.height, 0xFFFFFFFF);
        winmgr_draw_rect(win, render_x, render_y, node->box.width,
                         node->box.height, border_col);
        const char *val = dom_get_attribute(node->dom_node, "value");
        if (val)
          winmgr_draw_text(win, render_x + 4, render_y + 6, val, 0x000000);
        if (focused_node == node->dom_node) {
          int vlen = val ? strlen(val) : 0;
          winmgr_draw_rect(win, render_x + 4 + vlen * 8, render_y + 4, 1, 12,
                           0x000000);
        }
      }
    } else if (strcmp(node->dom_node->tag_name, "button") == 0) {
      if (render_y + 24 > min_y && render_y < max_y) {
        winmgr_fill_rect(win, render_x, render_y, node->box.width,
                         node->box.height, 0xFFDDDDDD);
        winmgr_draw_rect(win, render_x, render_y, node->box.width,
                         node->box.height, 0xFF444444);
        const char *label = "Submit";
        if (node->dom_node && node->dom_node->first_child) {
          dom_node_t *tc = node->dom_node->first_child;
          if (tc->type == DOM_NODE_TEXT && tc->text_content)
            label = tc->text_content;
        }
        winmgr_draw_text(win, render_x + 4, render_y + 8, label, 0x000000);
      }
    } else if (strcmp(node->dom_node->tag_name, "a") == 0) {
      const char *href = dom_get_attribute(node->dom_node, "href");
      if (link_count < 128 && href) {
        if (render_y + 14 > min_y && render_y < max_y) {
          link_boxes[link_count].x = render_x;
          link_boxes[link_count].y = render_y;
          link_boxes[link_count].w = node->box.width > 0 ? node->box.width : 50;
          link_boxes[link_count].h = 14;
          strncpy(link_boxes[link_count].href, href, 127);
          link_boxes[link_count].href[127] = 0;
          link_count++;
        }
      }
    }

    if (strcmp(node->dom_node->tag_name, "input") == 0 ||
        strcmp(node->dom_node->tag_name, "button") == 0) {
      if (form_count < 128) {
        if (render_y + node->box.height > min_y && render_y < max_y) {
          form_boxes[form_count].x = render_x;
          form_boxes[form_count].y = render_y;
          form_boxes[form_count].w = node->box.width;
          form_boxes[form_count].h = node->box.height;
          form_boxes[form_count].node = node->dom_node;
          form_count++;
        }
      }
    }
  }

  render_node_t *child = node->first_child;
  while (child) {
    browser_draw_render_node(win, child, offset_x, offset_y, max_y, min_y, cw);
    child = child->next_sibling;
  }
}

static void browser_draw_cb(void *w) {
  window_t *win = (window_t *)w;
  int cx = 2, cy = 24;
  int cw = win->width - 4;

  winmgr_fill_rect(win, cx, cy, cw, TOOLBAR_H, 0xFFDDDDDD);
  winmgr_draw_text(win, cx + 7, cy + 10, "<", 0xFF000000);
  winmgr_draw_text(win, cx + 33, cy + 10, ">", 0xFF000000);
  winmgr_draw_text(win, cx + 55, cy + 10, "R", 0xFF000000);

  int ux = cx + URLBAR_X;
  int uy = cy + 4;
  winmgr_fill_rect(win, ux, uy, URLBAR_W, 22, 0xFFFFFFFF);
  winmgr_draw_rect(win, ux, uy, URLBAR_W, 22,
                   url_editing ? 0x00FF00 : 0x888888);
  winmgr_draw_text(win, ux + 4, uy + 7, url_bar, 0x000000);

  int content_top = cy + CONTENT_Y;
  int content_h = win->height - 26 - TOOLBAR_H - 20;
  winmgr_fill_rect(win, cx, content_top, cw, content_h, 0xFFFFFFFF);

  int draw_y = content_top + 4 - scroll_y;
  int content_x = cx + 6;
  link_count = 0;
  form_count = 0;

  __asm__ volatile("cli");
  if (bss_sentinel_1 != 0xDEADBEEF || bss_sentinel_2 != 0xCAFEEBEE ||
      bss_sentinel_3 != 0xBAADF00D) {
    print_serial("BROWSER: BSS Corruption detected via sentinels!\n");
    char s1[16], s2[16], s3[16], rt[16];
    k_itoa_hex(bss_sentinel_1, s1);
    k_itoa_hex(bss_sentinel_2, s2);
    k_itoa_hex(bss_sentinel_3, s3);
    k_itoa_hex((uint32_t)(uintptr_t)current_render_tree, rt);
    print_serial("S1: 0x");
    print_serial(s1);
    print_serial(" S2: 0x");
    print_serial(s2);
    print_serial(" S3: 0x");
    print_serial(s3);
    print_serial(" RT: 0x");
    print_serial(rt);
    print_serial("\n");

    bss_sentinel_1 = 0xDEADBEEF;
    bss_sentinel_2 = 0xCAFEEBEE;
    bss_sentinel_3 = 0xBAADF00D;
    current_render_tree = 0;
  }

  if (browser_updating) {
    __asm__ volatile("sti");
    return;
  }
  if (current_document && !current_render_tree) {
    current_render_tree = layout_build_tree(current_document);
    layout_dirty = 1; // Force layout calc on new tree
  }
  if (current_render_tree && (layout_dirty || cw - 12 != cached_layout_width)) {
    layout_calculate(current_render_tree, cw - 12, 0, 0);
    cached_layout_width = cw - 12;
    layout_dirty = 0;
  }
  if (current_render_tree) {
    if (((uintptr_t)current_render_tree & 0xF0000000) == 0x60000000 ||
        ((uintptr_t)current_render_tree & 0xF0000000) == 0xF0000000) {
      print_serial(
          "CRITICAL: current_render_tree CORRUPTED detected in draw_cb!\n");
      char buf[32];
      k_itoa_hex((uint32_t)(uintptr_t)current_render_tree, buf);
      print_serial("Value: 0x");
      print_serial(buf);
      print_serial("\n");
      current_render_tree = 0; // Emergency recover
    } else {
      // Use a temporary window wrapper to redirect drawing to our backing store
      if (!content_backing_store) {
        content_backing_store = (uint32_t *)kmalloc(win->width * 2000 * 4);
      }

      if (content_backing_store && backing_store_dirty) {
        // Create a temporary surface to draw into
        window_t temp_win = *win;
        temp_win.surface = content_backing_store;
        temp_win.surface_w = win->width;
        temp_win.surface_h = 2000; // Large vertical canvas

        // Clear backing store
        for (int i = 0; i < win->width * 2000; i++)
          content_backing_store[i] = 0xFFFFFFFF;

        browser_draw_render_node(&temp_win, current_render_tree, 6, 4, 2000, 0,
                                 cw);
        backing_store_dirty = 0;
      }

      if (content_backing_store) {
        // Blit from backing store to window surface using SIMD
        // Calculate which part of the backing store to copy based on scroll_y
        for (int i = 0; i < content_h; i++) {
          int src_y = scroll_y + i;
          if (src_y >= 2000)
            break;
          uint32_t *src_ptr = content_backing_store + src_y * win->width;
          uint32_t *dst_ptr =
              win->surface + (content_top + i) * win->width + cx;
          simd_memcpy_32(dst_ptr, src_ptr, cw);
        }
      } else {
        // Fallback to slow path if allocation failed
        browser_draw_render_node(win, current_render_tree, content_x, draw_y,
                                 content_top + content_h, content_top, cw);
      }
    }
  }
  __asm__ volatile("sti");

  int sy = win->height - 18;
  winmgr_draw_rect(win, cx, sy, cw, 16, 0xFFDDDDDD);
  winmgr_draw_text(win, cx + 4, sy + 4, status_text, 0xFF444444);
}

static void browser_key_cb(void *w, int key, char c) {
  window_t *win = (window_t *)w;
  if (focused_node && strcmp(focused_node->tag_name, "input") == 0) {
    char val_buf[256];
    const char *current_val = dom_get_attribute(focused_node, "value");
    if (current_val)
      strcpy(val_buf, current_val);
    else
      val_buf[0] = 0;
    if (key == 0x0E) {
      if (strlen(val_buf) > 0)
        val_buf[strlen(val_buf) - 1] = 0;
    } else if (key == 0x1C) {
      browser_submit_form(focused_node);
      focused_node = 0;
    } else if (c >= 32 && c < 127) {
      int len = strlen(val_buf);
      if (len < 255) {
        val_buf[len] = c;
        val_buf[len + 1] = 0;
      }
    }
    dom_set_attribute(focused_node, "value", val_buf);
    ui_dirty = 1;
    win->needs_redraw = 1;
    return;
  }
  url_editing = 1;
  focused_node = 0;
  if (key == 0x0E) {
    if (url_cursor > 0) {
      url_cursor--;
      url_bar[url_cursor] = 0;
    }
    ui_dirty = 1;
    win->needs_redraw = 1;
  } else if (key == 0x1C) {
    if (!is_loading) {
      url_editing = 0;
      browser_navigate(url_bar);
    }
  } else if (c >= 32 && c < 127) {
    int len = strlen(url_bar);
    if (len < 254) {
      url_bar[len] = c;
      url_bar[len + 1] = 0;
      url_cursor++;
      ui_dirty = 1;
      win->needs_redraw = 1;
    }
  }
}

static void browser_mouse_cb(void *w, int mx, int my, int buttons) {
  window_t *win = (window_t *)w;
  if (!(buttons & 1))
    return;
  if (my >= 24 && my < 54) {
    if (mx >= URLBAR_X && mx < URLBAR_X + URLBAR_W) {
      url_editing = 1;
      ui_dirty = 1;
      win->needs_redraw = 1;
      return;
    }
    if (mx > URLBAR_X + URLBAR_W) {
      if (!is_loading) {
        url_editing = 0;
        browser_navigate(url_bar);
      }
      return;
    }
  }
  // Link/Form handling... (Simplified for restoration)
  for (int i = 0; i < link_count; i++) {
    if (mx >= link_boxes[i].x && mx < link_boxes[i].x + link_boxes[i].w &&
        my >= link_boxes[i].y && my < link_boxes[i].y + link_boxes[i].h) {
      browser_navigate(link_boxes[i].href);
      return;
    }
  }
}

static void browser_scroll_cb(void *w, int direction) {
  window_t *win = (window_t *)w;
  int content_h = win->height - CONTENT_Y - 18;

  // Wheel UP (direction > 0) -> scroll viewport UP -> decrease scroll_y
  scroll_y -= direction * 20;

  // Bounds checking
  if (scroll_y < 0)
    scroll_y = 0;
  int max_scroll = 2000 - content_h;
  if (max_scroll < 0)
    max_scroll = 0;
  if (scroll_y > max_scroll)
    scroll_y = max_scroll;

  ui_dirty = 1;
  win->needs_redraw = 1;
}

static void browser_on_close(void *w) {
  (void)w;
  print_serial("BROWSER: on_close: Freeing resources...\n");
  __asm__ volatile("cli");
  browser_updating = 1;

  if (current_render_tree &&
      ((uintptr_t)current_render_tree & 0xF0000000) == 0xC0000000) {
    layout_free_tree(current_render_tree);
  }
  current_render_tree = 0;

  if (current_document &&
      ((uintptr_t)current_document & 0xF0000000) == 0xC0000000) {
    dom_free_node(current_document);
  }
  current_document = 0;

  if (current_stylesheet &&
      ((uintptr_t)current_stylesheet & 0xF0000000) == 0xC0000000) {
    css_free_stylesheet(current_stylesheet);
  }
  current_stylesheet = 0;

  if (content_backing_store) {
    kfree(content_backing_store);
    content_backing_store = NULL;
  }

  browser_updating = 0;
  __asm__ volatile("sti");
  print_serial("BROWSER: on_close: Cleanup complete.\n");
}

void browser_init(void) {
  print_serial("BROWSER: Starting init...\n");
  is_loading = 0;
  js_init();
  print_serial("BROWSER: JS init done\n");
  window_t *win = winmgr_create_window(50, 30, 560, 440, "PureBrowser");
  print_serial("BROWSER: Window created\n");
  if (!win)
    return;
  win->draw = browser_draw_cb;
  win->on_key = browser_key_cb;
  win->on_mouse = browser_mouse_cb;
  win->on_scroll = browser_scroll_cb;
  win->on_close = browser_on_close;
  win->app_type = 7;
  win->bg_color = 0xFFFFFF;
  win->needs_redraw = 1;
  browser_set_home();
  print_serial("BROWSER: Home set, parsing...\n");
  browser_parse_html();
  print_serial("BROWSER: Parse done, ready\n");
  ui_dirty = 1;
}
