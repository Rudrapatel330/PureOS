#include "../kernel/heap.h"
#include "../kernel/simd.h"
#include "../kernel/spinlock.h"
#include "../kernel/string.h"
#include "../kernel/theme.h"
#include "../kernel/window.h"
#include "../net/net.h"
#include "../gui/ttf_font.h"
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
static window_t *browser_win = 0;
char input_buffer[256];
int input_cursor = 0;

// BSS Hardening Sentinels
static uint32_t bss_sentinel_1 = 0xDEADBEEF;
dom_node_t *current_document = 0;
css_stylesheet_t *current_stylesheet = 0;
static uint32_t bss_sentinel_2 = 0xCAFEEBEE;
render_node_t *current_render_tree = 0;
static uint32_t bss_sentinel_3 = 0xBAADF00D;

extern void* browser_engine_create(int width, int height);
extern void browser_engine_destroy(void* engine);
extern void browser_engine_load_html(void* engine, const char* html, const char* master_css);
extern void browser_engine_render(void* engine, int width);
extern void browser_engine_draw(void* engine, void* window, int x, int y, int clip_x, int clip_y, int clip_w, int clip_h);
extern int browser_engine_get_height(void* engine);
extern int browser_engine_get_width(void* engine);
extern int browser_engine_on_mouse(void* engine, int doc_x, int doc_y);
extern void browser_engine_on_click(void* engine, int doc_x, int doc_y);

static void* litehtml_engine = 0;

// Page content
#define PAGE_MAX 131072
static char page_content[PAGE_MAX];
static int page_len = 0;
static int scroll_y = 0;
static animation_t scroll_anim;
int layout_dirty = 1;        // 1 = need to recalculate layout (non-static for JS bridge)
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
extern void kernel_poll_events(void);

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
    static int ext_poll = 0;
    if (++ext_poll > 50) {
      kernel_poll_events();
      ext_poll = 0;
    }
    extract_styles_recursive(child, css_buf, max_len);
    child = child->next_sibling;
  }
}



// Recursively find <link rel="stylesheet"> and add to resource list
static void collect_external_css(dom_node_t *node, dom_resource_t **list) {
    if (!node) return;
    if (node->type == DOM_NODE_ELEMENT && strcmp(node->tag_name, "link") == 0) {
        const char *rel = dom_get_attribute(node, "rel");
        const char *href = dom_get_attribute(node, "href");
        if (rel && href && (strcmp(rel, "stylesheet") == 0 || strstr(rel, "stylesheet") != 0)) {
            char resolved[512];
            dom_resolve_url(url_bar, href, resolved, 511);
            // Avoid duplicates
            dom_resource_t *exist = *list;
            int found = 0;
            while (exist) {
                if (strcmp(exist->url, resolved) == 0) { found = 1; break; }
                exist = exist->next;
            }
            if (!found) {
                dom_add_resource(list, resolved, 0);
                print_serial("BROWSER: Found external CSS: ");
                print_serial(resolved);
                print_serial("\n");
            }
        }
    }
    // Recurse into children
    dom_node_t *child = node->first_child;
    while (child) {
        collect_external_css(child, list);
        child = child->next_sibling;
    }
}

// Simple HTML parser (now uses DOM)
static void browser_parse_html(void) {
  print_serial("BROWSER: parse_html: Starting Litehtml rendering...\n");
  __asm__ volatile("cli");
  browser_updating = 1;
  __asm__ volatile("sti");

  if (!litehtml_engine) {
      int cw = browser_win ? browser_win->width - 4 : 800;
      litehtml_engine = browser_engine_create(cw, 2000);
  }

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

  // No lock used here to avoid deadlocks with kmalloc
  // static spinlock_irq_t browser_parse_lock = {0};
  // spinlock_irq_acquire(&browser_parse_lock);
  __asm__ volatile("cli");
  browser_updating = 1;
  __asm__ volatile("sti");

  if (page_len > 0 && page_content[0] != '\0') {
      print_serial("BROWSER: About to call browser_engine_load_html...\n");
      char buf[20];
      k_itoa_hex((uint32_t)(uintptr_t)litehtml_engine, buf);
      print_serial("litehtml_engine = 0x");
      print_serial(buf);
      print_serial("\n");
      
      // Use Litehtml to parse and load the page!
      browser_engine_load_html(litehtml_engine, page_content, 0);
      print_serial("BROWSER: browser_engine_load_html returned.\n");
      
      extern void jsbridge_execute_scripts(void);
      jsbridge_execute_scripts();
      
      // Perform initial render
      int cw = browser_win ? browser_win->width - 4 : 800;
      browser_engine_render(litehtml_engine, cw - 12);
      print_serial("BROWSER: browser_engine_render returned.\n");
  }

  __asm__ volatile("cli");
  browser_updating = 0;
  __asm__ volatile("sti");
  
  // lock is disabled to avoid deadlock
  // spinlock_irq_release(&browser_parse_lock);

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

static char fetch_url_global[256];

static void browser_fetch_thread(void) {
  int redirects = 0;
  char fetch_url[256];
  strncpy(fetch_url, fetch_url_global, 255);
  fetch_url[255] = 0;

  while (redirects < 5) {
    int result;
    if (strncmp(fetch_url, "https://", 8) == 0) {
      result = https_get(fetch_url, page_content, PAGE_MAX - 1);
    } else if (strncmp(fetch_url, "file://", 7) == 0) {
      extern int vfs_open(const char *path, int flags);
      extern int vfs_read(int fd, uint8_t *buf, uint32_t size);
      extern void vfs_close(int fd);
      
      const char *raw_path = fetch_url + 7;
      char vfs_path[256];
      if (strncmp(raw_path, "/disk/", 6) == 0) {
          strncpy(vfs_path, raw_path, 255);
      } else {
          strcpy(vfs_path, "/disk/");
          if (raw_path[0] == '/') raw_path++;
          strncat(vfs_path, raw_path, 249);
      }
      vfs_path[255] = 0;
      
      int fd = vfs_open(vfs_path, 0); // O_RDONLY
      if (fd >= 0) {
          result = vfs_read(fd, (uint8_t*)page_content, PAGE_MAX - 1);
          if (result >= 0) page_content[result] = 0;
          vfs_close(fd);
      } else {
          result = -404;
      }
    } else {
      result = http_get(fetch_url, page_content, PAGE_MAX - 1);
    }

    if (result == -301) {
      if (strncmp(page_content, "REDIRECT:", 9) == 0) {
        char *redir_url = page_content + 9;
        if (redir_url[0] == '/') {
          char base[256];
          const char *p = fetch_url;
          int len = 0;
          if (strncmp(p, "https://", 8) == 0) len = 8;
          else if (strncmp(p, "http://", 7) == 0) len = 7;
          while (p[len] && p[len] != '/') len++;
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

  extern volatile int http_download_progress;
  http_download_progress = 131072;
  
  strcpy(status_text, "Rendering...");
  browser_parse_html();

  is_loading = 0;
  strcpy(status_text, "Ready");

  __asm__ volatile("cli");
  browser_updating = 0;
  __asm__ volatile("sti");

  backing_store_dirty = 1;
  if (browser_win)
    browser_win->needs_redraw = 1;
  ui_dirty = 1;

  extern void exit(int status);
  exit(0);
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
    int j = strlen(url);
    for (int i = 0; input[i] && j < 255; i++) {
        if (input[i] == ' ') {
            url[j++] = '+';
        } else {
            url[j++] = input[i];
        }
    }
    url[j] = '\0';
  } else {
    if (strncmp(input, "about:", 6) == 0 || strncmp(input, "http://", 7) == 0 || strncmp(input, "file://", 7) == 0) {
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
  if (browser_win)
    browser_win->needs_redraw = 1;
  ui_dirty = 1;

  extern void winmgr_flush_updates(void);
  extern void compositor_render(void);
  winmgr_flush_updates();
  compositor_render();

  __asm__ volatile("cli");
  browser_updating = 1;
  __asm__ volatile("sti");
  // Invalidate old trees immediately to prevent draw racer
  if (current_render_tree) {
    layout_free_tree(current_render_tree);
    current_render_tree = 0;
  }
  if (current_document) {
    dom_free_node(current_document);
    current_document = 0;
  }
  js_init();
  {
    // Fonts are now initialized globally in kernel.c
  }

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
    strncpy(fetch_url_global, url, 255);
    fetch_url_global[255] = 0;
    extern void* create_task(void (*entry)(), char *name);
    create_task(browser_fetch_thread, "BrowserFetch");
    return;
  }

  extern volatile int http_download_progress;
  http_download_progress = 131072;
  
  strcpy(status_text, "Rendering...");
  browser_parse_html();

  is_loading = 0;
  strcpy(status_text, "Ready");

  __asm__ volatile("cli");
  browser_updating = 0;
  __asm__ volatile("sti");

  backing_store_dirty = 1;
  if (browser_win)
    browser_win->needs_redraw = 1;
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

  if (node->style && node->box.width > 0 && node->box.height > 0) {
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
      if (node->style->has_gradient && node->style->gradient.type == CSS_GRADIENT_LINEAR) {
        css_gradient_t *g = &node->style->gradient;
        if (g->stop_count >= 2) {
          int w = node->box.width;
          int h = bg_h;
          for (int py = 0; py < h; py++) {
            float t = (g->angle == 90) ? 0.0f : (float)py / h;
            // Find which two stops to interpolate between
            int pos = (int)(t * 1000);
            int si = 0;
            for (int s = 0; s < g->stop_count - 1; s++) {
              if (pos >= g->stop_positions[s] && pos <= g->stop_positions[s+1]) {
                si = s; break;
              }
            }
            int range = g->stop_positions[si+1] - g->stop_positions[si];
            float frac = (range > 0) ? (float)(pos - g->stop_positions[si]) / range : 0.5f;
            uint32_t c1 = g->stops[si], c2 = g->stops[si+1];
            int r = (int)(((c1 >> 16) & 0xFF) * (1-frac) + ((c2 >> 16) & 0xFF) * frac);
            int g2 = (int)(((c1 >> 8) & 0xFF) * (1-frac) + ((c2 >> 8) & 0xFF) * frac);
            int b = (int)((c1 & 0xFF) * (1-frac) + (c2 & 0xFF) * frac);
            uint32_t col = 0xFF000000 | (r << 16) | (g2 << 8) | b;
            winmgr_fill_rect(win, render_x, bg_y + py, w, 1, col);
          }
        } else {
          winmgr_fill_rect(win, render_x, bg_y, node->box.width, bg_h, node->style->background_color);
        }
      } else if (node->style->has_bg_color) {
        winmgr_fill_rect(win, render_x, bg_y, node->box.width, bg_h,
                         node->style->background_color);
      }
    }
  }

  if (node->is_text && node->dom_node && node->dom_node->text_content) {
    int text_h = 12;
    int is_h1 = 0, is_h2 = 0, is_link = 0, is_bold = 0;
    int draw_font_slot = ttf_get_default_font();

    if (node->parent && node->parent->dom_node) {
      const char *ptag = node->parent->dom_node->tag_name;
      if (strcmp(ptag, "h1") == 0) is_h1 = 1;
      if (strcmp(ptag, "h2") == 0) is_h2 = 1;
      if (strcmp(ptag, "a") == 0) is_link = 1;
      if (strcmp(ptag, "b") == 0 || strcmp(ptag, "strong") == 0) is_bold = 1;
    }
    if (node->parent && node->parent->style && node->parent->style->has_font_weight
        && node->parent->style->font_weight) is_bold = 1;

    if (draw_font_slot >= 0) {
      int sz = is_h1 ? 24 : (is_h2 ? 18 : 14);
      ttf_set_size(draw_font_slot, (float)sz);
      text_h = ttf_font_height(draw_font_slot);
    }
    if (is_h1) text_h = (text_h < 20) ? 20 : text_h;
    if (is_h2) text_h = (text_h < 16) ? 16 : text_h;

    if (render_y + node->box.height > min_y && render_y < max_y) {
      const theme_t *theme = theme_get();
      int color = node->style && node->style->has_color ? node->style->color : theme->fg;
      if (!node->style && node->parent && node->parent->style && node->parent->style->has_color)
        color = node->parent->style->color;
      if (is_link) color = 0xFF0000FF;

      const char *text = node->dom_node->text_content;
      int text_len = strlen(text);
      int avail_w = node->box.content_w > 0 ? node->box.content_w : cw - 20;
      int line_h = text_h + 4;
      int draw_x = render_x;
      int draw_line_y = render_y;

      int total_w;
      if (draw_font_slot >= 0) {
        total_w = ttf_text_width(draw_font_slot, text);
      } else {
        total_w = text_len * 8;
      }

      if (total_w <= avail_w) {
        if (draw_font_slot >= 0) {
          ttf_draw_text(win, draw_font_slot, draw_x, draw_line_y, text, color);
          if (is_bold || is_h1)
            ttf_draw_text(win, draw_font_slot, draw_x + 1, draw_line_y, text, color);
        } else {
          winmgr_draw_text(win, draw_x, draw_line_y, text, color);
          if (is_bold || is_h1)
            winmgr_draw_text(win, draw_x + 1, draw_line_y, text, color);
        }
      } else {
        char line_buf[256];
        int line_w = 0;
        int buf_pos = 0;
        int i = 0;
        while (i < text_len && i < 4096) {
          int word_start = i;
          while (i < text_len && text[i] != ' ') i++;
          int word_len = i - word_start;
          int word_w;
          if (draw_font_slot >= 0 && word_len > 0) {
            char saved = text[i];
            ((char*)text)[i] = 0;
            word_w = ttf_text_width(draw_font_slot, text + word_start);
            ((char*)text)[i] = saved;
          } else {
            word_w = word_len * 8;
          }

          if (line_w + word_w > avail_w && line_w > 0) {
            line_buf[buf_pos] = 0;
            if (draw_line_y + line_h > min_y && draw_line_y < max_y) {
              if (draw_font_slot >= 0)
                ttf_draw_text(win, draw_font_slot, draw_x, draw_line_y, line_buf, color);
              else
                winmgr_draw_text(win, draw_x, draw_line_y, line_buf, color);
            }
            draw_line_y += line_h;
            buf_pos = 0; line_w = 0;
          }
          if (buf_pos + word_len < 255) {
            memcpy(line_buf + buf_pos, text + word_start, word_len);
            buf_pos += word_len;
            line_w += word_w;
          }
          while (i < text_len && text[i] == ' ') {
            if (buf_pos < 255) {
              line_buf[buf_pos++] = ' ';
              if (draw_font_slot >= 0)
                line_w += ttf_text_width(draw_font_slot, " ");
              else
                line_w += 8;
            }
            i++;
          }
        }
        if (buf_pos > 0) {
          line_buf[buf_pos] = 0;
          if (draw_line_y + line_h > min_y && draw_line_y < max_y) {
            if (draw_font_slot >= 0)
              ttf_draw_text(win, draw_font_slot, draw_x, draw_line_y, line_buf, color);
            else
              winmgr_draw_text(win, draw_x, draw_line_y, line_buf, color);
          }
        }
      }

      if (is_link) {
        int ul_w = (total_w <= avail_w) ? total_w : avail_w;
        int ul_y = render_y + text_h + 2;
        if (ul_y < max_y) {
          winmgr_draw_rect(win, render_x, ul_y, ul_w, 1, color);
        }
      }
    }
  } else if (node->dom_node && node->dom_node->type == DOM_NODE_ELEMENT) {
    if (strcmp(node->dom_node->tag_name, "hr") == 0) {
      if (render_y + 4 > min_y && render_y < max_y) {
        winmgr_draw_rect(win, render_x, render_y + 4, cw - 16, 1, theme_get()->border);
      }
    } else if (strcmp(node->dom_node->tag_name, "input") == 0) {
      if (render_y + 20 > min_y && render_y < max_y) {
        uint32_t border_col =
            (focused_node == node->dom_node) ? 0xFF00FF00 : 0xFF888888;
        winmgr_fill_rect(win, render_x, render_y, node->box.width,
                         node->box.height, theme_get()->bg);
        winmgr_draw_rect(win, render_x, render_y, node->box.width,
                         node->box.height, border_col);
        const char *val = dom_get_attribute(node->dom_node, "value");
        if (val) {
          int fs = ttf_get_default_font();
          if (fs >= 0) ttf_draw_text(win, fs, render_x + 4, render_y + 6, val, theme_get()->fg);
          else winmgr_draw_text(win, render_x + 4, render_y + 6, val, theme_get()->fg);
        }
        if (focused_node == node->dom_node) {
          int vlen = val ? strlen(val) : 0;
          int cursor_x;
          int fs = ttf_get_default_font();
          if (fs >= 0) {
            ttf_set_size(fs, 14.0f);
            char tmp[256]; memcpy(tmp, val, vlen); tmp[vlen] = 0;
            cursor_x = render_x + 4 + ttf_text_width(fs, tmp);
          } else {
            cursor_x = render_x + 4 + vlen * 8;
          }
          winmgr_draw_rect(win, cursor_x, render_y + 4, 1, 12, 0x000000);
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
        int fs = ttf_get_default_font();
        if (fs >= 0) ttf_draw_text(win, fs, render_x + 4, render_y + 8, label, theme_get()->fg);
        else winmgr_draw_text(win, render_x + 4, render_y + 8, label, theme_get()->fg);
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

    if (strcmp(node->dom_node->tag_name, "td") == 0 || strcmp(node->dom_node->tag_name, "th") == 0) {
      if (render_y + node->box.height > min_y && render_y < max_y) {
        winmgr_draw_rect(win, render_x, render_y, node->box.width,
                         node->box.height, 0xFF999999);
      }
    }

    if (strcmp(node->dom_node->tag_name, "li") == 0) {
      if (render_y + 14 > min_y && render_y < max_y) {
        css_style_t *ls = (css_style_t *)node->style;
        if (ls && ls->list_style_type == CSS_LIST_DISC) {
          winmgr_fill_rect(win, render_x - 12, render_y + 4, 4, 4, theme_get()->fg);
        } else if (ls && ls->list_style_type == CSS_LIST_DECIMAL) {
          int idx = 1;
          if (node->dom_node) {
            dom_node_t *sib = node->dom_node->parent ? node->dom_node->parent->first_child : 0;
            while (sib && sib != node->dom_node) {
              if (sib->type == DOM_NODE_ELEMENT && strcmp(sib->tag_name, "li") == 0) idx++;
              sib = sib->next_sibling;
            }
          }
          char num[8];
          num[0] = '0' + (idx % 10); num[1] = '.'; num[2] = 0;
          if (idx >= 10) { num[0] = '0' + (idx / 10); num[1] = '0' + (idx % 10); num[2] = '.'; num[3] = 0; }
          int fs = ttf_get_default_font();
          if (fs >= 0) ttf_draw_text(win, fs, render_x - 16, render_y, num, theme_get()->fg);
          else winmgr_draw_text(win, render_x - 16, render_y, num, theme_get()->fg);
        } else if (ls && ls->list_style_type == CSS_LIST_CIRCLE) {
          winmgr_draw_rect(win, render_x - 12, render_y + 4, 4, 4, theme_get()->fg);
        } else if (ls && ls->list_style_type == CSS_LIST_SQUARE) {
          winmgr_fill_rect(win, render_x - 13, render_y + 3, 6, 6, theme_get()->fg);
        }
      }
    }

    // Generic CSS border rendering
    if (node->style && node->style->has_border && node->style->border_width > 0 &&
        node->style->border_style != CSS_BORDER_NONE) {
      if (render_y + node->box.height > min_y && render_y < max_y) {
        uint32_t bc = 0xFF000000 | node->style->border_color;
        for (int b = 0; b < node->style->border_width; b++) {
          winmgr_draw_rect(win, render_x + b, render_y + b,
                           node->box.width - b * 2, node->box.height - b * 2, bc);
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

  const theme_t *theme = theme_get();
 
  winmgr_fill_rect(win, cx, cy, cw, TOOLBAR_H, theme->titlebar);
  winmgr_draw_text(win, cx + 7, cy + 10, "<", theme->fg);
  winmgr_draw_text(win, cx + 33, cy + 10, ">", theme->fg);
  winmgr_draw_text(win, cx + 55, cy + 10, "R", theme->fg);

  int ux = cx + URLBAR_X;
  int uy = cy + 4;
  winmgr_fill_rect(win, ux, uy, URLBAR_W, 22, theme->input_bg);
  winmgr_draw_rect(win, ux, uy, URLBAR_W, 22,
                   url_editing ? theme->accent : theme->border);
  winmgr_draw_text(win, ux + 4, uy + 7, url_bar, theme->fg);

  if (is_loading) {
    extern volatile int http_download_progress;
    int progress_bytes = http_download_progress;
    
    int progress_w = (progress_bytes * (URLBAR_W - 2)) / 131072;
    if (progress_w > URLBAR_W - 2) progress_w = URLBAR_W - 2;
    if (progress_w < 5 && progress_bytes > 0) progress_w = 5;
    if (progress_bytes == 0) progress_w = 2;

    winmgr_fill_rect(win, ux + 1, uy + 22 - 3, progress_w, 2, theme->accent);

    win->needs_redraw = 1;
    ui_dirty = 1;
  }

  int content_top = cy + CONTENT_Y;
  int content_h = win->height - 26 - TOOLBAR_H - 20;
  winmgr_fill_rect(win, cx, content_top, cw, content_h, theme->bg);

  // Smooth scrolling tick
  anim_tick(&scroll_anim, 0.04f);
  if (scroll_anim.active) {
      scroll_y = (int)scroll_anim.current_val;
      win->needs_redraw = 1;
      extern int ui_dirty;
      ui_dirty = 1;
  }

  int draw_y = content_top + 4 - scroll_y;
  int content_x = cx + 6;
  link_count = 0;
  form_count = 0;

  js_tick_timers();
  __asm__ volatile("cli");
  int is_updating = browser_updating;
  __asm__ volatile("sti");

  int sy = win->height - 18;
  winmgr_draw_rect(win, cx, sy, cw, 16, 0xFFDDDDDD);
  winmgr_draw_text(win, cx + 4, sy + 4, status_text, 0xFF444444);
  
  if (is_updating) {
    return;
  }
  
  // Render Litehtml Engine
  if (litehtml_engine) {
      if (layout_dirty || cw - 12 != cached_layout_width) {
          browser_engine_render(litehtml_engine, cw - 12);
          cached_layout_width = cw - 12;
          layout_dirty = 0;
      }
      
      int content_y_scroll = scroll_y;
      int doc_height = browser_engine_get_height(litehtml_engine);
      
      // Setup temporary surface for hardware backbuffer caching if preferred,
      // but for simplicity we draw directly with our litehtml container.
      
      // Define clipping rect for the content area
      int clip_x = content_x;
      int clip_y = content_top;
      int clip_w = cw - 12;
      int clip_h = content_h;
      
      // Draw litehtml document directly to the window surface using our graphics hooks
      // Notice we draw at Y = content_top - scroll_y
      browser_engine_draw(litehtml_engine, win, content_x, content_top - scroll_y, clip_x, clip_y, clip_w, clip_h);
  }
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
  // Translate screen coords to litehtml document coords
  // content_x = cx(2) + 6 = 8,  content_top = cy(24) + CONTENT_Y(32) = 56
  int content_x_off = 2 + 6;
  int content_top_off = 24 + CONTENT_Y;
  int doc_x = mx - content_x_off;
  int doc_y = my - content_top_off + scroll_y;

  // Hover state update (CSS :hover, :focus)
  if (litehtml_engine && doc_y >= 0) {
      if (browser_engine_on_mouse(litehtml_engine, doc_x, doc_y)) {
          // If hover state changed, we just need a redraw (not a full relayout)
          // Since litehtml on_mouse_over does not recompute layout, we must trigger layout to apply CSS pseudo-classes
          layout_dirty = 1; 
          ui_dirty = 1;
          win->needs_redraw = 1;
      }
  }

  // Only process clicks below the toolbar
  if (my < content_top_off) return;

  // Link navigation via our hitbox list (fast path)
  for (int i = 0; i < link_count; i++) {
    if (mx >= link_boxes[i].x && mx < link_boxes[i].x + link_boxes[i].w &&
        my >= link_boxes[i].y && my < link_boxes[i].y + link_boxes[i].h) {
      browser_navigate(link_boxes[i].href);
      return;
    }
  }

  // Forward click into litehtml (JS event dispatch + :active + <a> handling)
  if (litehtml_engine && doc_y >= 0) {
      browser_engine_on_click(litehtml_engine, doc_x, doc_y);
      ui_dirty = 1;
      win->needs_redraw = 1;
  }
}

static void browser_scroll_cb(void *w, int direction) {
  window_t *win = (window_t *)w;
  int content_h = win->height - CONTENT_Y - 18;

  float target = scroll_anim.end_val - direction * 60.0f;

  int max_scroll = 2000 - content_h;
  if (max_scroll < 0)
    max_scroll = 0;
  if (target < 0.0f)
    target = 0.0f;
  if (target > max_scroll)
    target = max_scroll;

  anim_start_spring(&scroll_anim, scroll_y, target, 400.0f, 30.0f);

  ui_dirty = 1;
  win->needs_redraw = 1;
}

static void browser_on_close(void *w) {
  (void)w;
  print_serial("BROWSER: on_close: Freeing resources...\n");
  __asm__ volatile("cli");
  browser_updating = 1;
  __asm__ volatile("sti");

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

  __asm__ volatile("cli");
  browser_updating = 0;
  __asm__ volatile("sti");
  print_serial("BROWSER: on_close: Cleanup complete.\n");
}

void js_request_rerender(void) {
  if (current_render_tree) {
    layout_free_tree(current_render_tree);
    current_render_tree = 0;
  }
  
  __asm__ volatile("cli");
  layout_dirty = 1;
  ui_dirty = 1;
  if (browser_win)
    browser_win->needs_redraw = 1;
  __asm__ volatile("sti");
}

void browser_init(void) {
  print_serial("BROWSER: Starting init...\n");
  is_loading = 0;
  js_init();
  anim_init_val(&scroll_anim, 0.0f);
  print_serial("BROWSER: JS init done\n");
  browser_win = winmgr_create_window(-1, -1, 850, 650, "PureBrowser");
  print_serial("BROWSER: Window created\n");
  if (!browser_win)
    return;
  window_t *win = browser_win;
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
