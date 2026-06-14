#include "browser_container.h"
#include "../lib/litehtml/include/litehtml/render_item.h"
extern "C" {
    #include "../kernel/window.h"
    #include <stdio.h>

    int http_get(const char* url, char* response, int max_len);
    int https_get(const char* url, char* response, int max_len);
    void* kmalloc(size_t size);
    void kfree(void* ptr);
    void print_serial(const char*);
    
    unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
    void stbi_image_free(void *retval_from_stbi_load);
    
    void* __dso_handle = nullptr;
    int __cxa_atexit(void (*)(void*), void*, void*) {
        return 0;
    }
    
    int __cxa_guard_acquire(long* g) {
        return !*(char*)(g);
    }
    void __cxa_guard_release(long* g) {
        *(char*)g = 1;
    }
    void __cxa_guard_abort(long* g) {
        (void)g;
    }

    double round(double x) {
        return (double)(int)(x + 0.5);
    }
}

extern "C" int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

namespace EA {
namespace StdC {
    int Vsnprintf(char* pDestination, size_t n, const char* pFormat, va_list arguments) {
        return vsnprintf(pDestination, n, pFormat, arguments);
    }
}
}

browser_container::browser_container() {
    m_viewport.width = 800;
    m_viewport.height = 600;
}

browser_container::~browser_container() {
    for (auto& pair : m_image_cache) {
        if (pair.second.data) {
            stbi_image_free(pair.second.data);
        }
    }
}

static std::string resolve_url(const std::string& url, const std::string& base) {
    if (url.find("http://") == 0 || url.find("https://") == 0) {
        return url;
    }
    if (base.empty()) return url;
    
    if (url[0] == '/') {
        size_t slash_pos = base.find('/', 8);
        if (slash_pos != std::string::npos) {
            return base.substr(0, slash_pos) + url;
        } else {
            return base + url;
        }
    } else {
        size_t slash_pos = base.rfind('/');
        if (slash_pos != std::string::npos && slash_pos > 7) {
            return base.substr(0, slash_pos + 1) + url;
        } else {
            std::string res = base;
            if (res.back() != '/') res += '/';
            return res + url;
        }
    }
}

litehtml::uint_ptr browser_container::create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) {
    (void)doc;
    if (fm) {
        int size = descr.size;
        fm->ascent = size;
        fm->descent = size / 4;
        fm->height = size + size / 4;
        fm->x_height = size / 2;
        fm->draw_spaces = false;
    }
    return (litehtml::uint_ptr)1;
}

void browser_container::delete_font(litehtml::uint_ptr hFont) {
    (void)hFont;
}

litehtml::pixel_t browser_container::text_width(const char* text, litehtml::uint_ptr hFont) {
    (void)hFont;
    return strlen(text) * 8;
}

void browser_container::draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) {
    (void)hFont;
    window_t* ctx = (window_t*)hdc;
    if (!ctx) return;
    
    uint32_t col = (color.alpha << 24) | (color.red << 16) | (color.green << 8) | color.blue;
    winmgr_draw_text(ctx, pos.x, pos.y, text, col);
}

litehtml::pixel_t browser_container::pt_to_px(float pt) const {
    return (litehtml::pixel_t)(pt * 96 / 72.0);
}

litehtml::pixel_t browser_container::get_default_font_size() const {
    return 16;
}

const char* browser_container::get_default_font_name() const {
    return "Arial";
}

void browser_container::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) {
    window_t* ctx = (window_t*)hdc;
    if (!ctx) return;
    uint32_t col = (marker.color.alpha << 24) | (marker.color.red << 16) | (marker.color.green << 8) | marker.color.blue;
    winmgr_fill_rect(ctx, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height, col);
}

void browser_container::load_image(const char* src, const char* baseurl, bool redraw_on_ready) {
    if (!src) return;
    std::string base = baseurl ? baseurl : m_base_url.c_str();
    std::string full_url = resolve_url(src, base);
    
    if (m_image_cache.find(full_url) != m_image_cache.end()) return;
    
    print_serial("BROWSER: Fetching image: ");
    print_serial(full_url.c_str());
    print_serial("\n");
    
    int max_len = 2 * 1024 * 1024; // 2MB max
    char* buf = (char*)kmalloc(max_len);
    if (!buf) return;
    
    int len = 0;
    if (full_url.find("https://") == 0) len = https_get(full_url.c_str(), buf, max_len);
    else if (full_url.find("http://") == 0) len = http_get(full_url.c_str(), buf, max_len);
    
    if (len > 0) {
        int w, h, c;
        unsigned char* pixels = stbi_load_from_memory((unsigned char*)buf, len, &w, &h, &c, 4);
        if (pixels) {
            cached_image img = {w, h, 4, pixels};
            m_image_cache[full_url] = img;
            print_serial("BROWSER: Decoded image successfully\n");
        } else {
            print_serial("BROWSER: Image decode failed\n");
        }
    }
    kfree(buf);
}

void browser_container::get_image_size(const char* src, const char* baseurl, litehtml::size& sz) {
    sz.width = 0;
    sz.height = 0;
    if (!src) return;
    std::string base = baseurl ? baseurl : m_base_url.c_str();
    std::string full_url = resolve_url(src, base);
    
    auto it = m_image_cache.find(full_url);
    if (it != m_image_cache.end()) {
        sz.width = it->second.width;
        sz.height = it->second.height;
    }
}

void browser_container::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) {
    window_t* ctx = (window_t*)hdc;
    if (!ctx) return;
    
    std::string base = base_url.empty() ? m_base_url.c_str() : base_url;
    std::string full_url = resolve_url(url, base);
    
    auto it = m_image_cache.find(full_url);
    if (it != m_image_cache.end()) {
        cached_image& img = it->second;
        uint32_t* buf32 = (uint32_t*)kmalloc(img.width * img.height * 4);
        if (!buf32) return;
        
        for (int i = 0; i < img.width * img.height; ++i) {
            uint8_t r = img.data[i*4 + 0];
            uint8_t g = img.data[i*4 + 1];
            uint8_t b = img.data[i*4 + 2];
            uint8_t a = img.data[i*4 + 3];
            buf32[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        
        winmgr_blend_blit(ctx, layer.clip_box.x, layer.clip_box.y, buf32, img.width, img.height, 0, 0, layer.clip_box.width, layer.clip_box.height, 255);
        
        kfree(buf32);
    }
}

void browser_container::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color) {
    window_t* ctx = (window_t*)hdc;
    if (!ctx) return;
    if (color.alpha > 0) {
        uint32_t col = (color.alpha << 24) | (color.red << 16) | (color.green << 8) | color.blue;
        winmgr_fill_rect(ctx, layer.clip_box.x, layer.clip_box.y, layer.clip_box.width, layer.clip_box.height, col);
    }
}

void browser_container::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient) {
    (void)hdc;
    (void)layer;
    (void)gradient;
}

void browser_container::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient) {
    (void)hdc;
    (void)layer;
    (void)gradient;
}

void browser_container::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient) {
    (void)hdc;
    (void)layer;
    (void)gradient;
}

void browser_container::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) {
    (void)root;
    window_t* ctx = (window_t*)hdc;
    if (!ctx) return;

    if (borders.top.width > 0 && borders.top.color.alpha > 0) {
        uint32_t col = (borders.top.color.alpha << 24) | (borders.top.color.red << 16) | (borders.top.color.green << 8) | borders.top.color.blue;
        winmgr_fill_rect(ctx, draw_pos.x, draw_pos.y, draw_pos.width, borders.top.width, col);
    }
    if (borders.bottom.width > 0 && borders.bottom.color.alpha > 0) {
        uint32_t col = (borders.bottom.color.alpha << 24) | (borders.bottom.color.red << 16) | (borders.bottom.color.green << 8) | borders.bottom.color.blue;
        winmgr_fill_rect(ctx, draw_pos.x, draw_pos.bottom() - borders.bottom.width, draw_pos.width, borders.bottom.width, col);
    }
    if (borders.left.width > 0 && borders.left.color.alpha > 0) {
        uint32_t col = (borders.left.color.alpha << 24) | (borders.left.color.red << 16) | (borders.left.color.green << 8) | borders.left.color.blue;
        winmgr_fill_rect(ctx, draw_pos.x, draw_pos.y, borders.left.width, draw_pos.height, col);
    }
    if (borders.right.width > 0 && borders.right.color.alpha > 0) {
        uint32_t col = (borders.right.color.alpha << 24) | (borders.right.color.red << 16) | (borders.right.color.green << 8) | borders.right.color.blue;
        winmgr_fill_rect(ctx, draw_pos.right() - borders.right.width, draw_pos.y, borders.right.width, draw_pos.height, col);
    }
}

void browser_container::set_caption(const char* caption) {
    (void)caption;
    // printf("Title: %s\n", caption);
}

void browser_container::set_base_url(const char* base_url) {
    if (base_url) m_base_url = base_url;
    else m_base_url = "";
}

void browser_container::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) {
    (void)doc;
    (void)el;
}

void browser_container::on_anchor_click(const char* url, const litehtml::element::ptr& el) {
    (void)url;
    (void)el;
    // printf("Click on: %s\n", url);
}

void browser_container::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) {
    (void)el;
    (void)event;
}

void browser_container::set_cursor(const char* cursor) {
    (void)cursor;
}

void browser_container::transform_text(litehtml::string& text, litehtml::text_transform tt) {
    if (text.empty()) return;
    if (tt == litehtml::text_transform_capitalize) {
        text[0] = toupper(text[0]);
    } else if (tt == litehtml::text_transform_uppercase) {
        for (size_t i = 0; i < text.length(); ++i) text[i] = toupper(text[i]);
    } else if (tt == litehtml::text_transform_lowercase) {
        for (size_t i = 0; i < text.length(); ++i) text[i] = tolower(text[i]);
    }
}

void browser_container::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) {
    litehtml::string full_url;
    
    // 1. Resolve URL against base URL
    if (url.find("http://") == 0 || url.find("https://") == 0) {
        full_url = url;
    } else {
        litehtml::string base = baseurl.empty() ? m_base_url : baseurl;
        if (!base.empty()) {
            if (url[0] == '/') {
                // Absolute path relative to host
                size_t slash_pos = base.find('/', 8); // Skip http:// or https://
                if (slash_pos != litehtml::string::npos) {
                    full_url = base.substr(0, slash_pos) + url;
                } else {
                    full_url = base + url;
                }
            } else {
                // Relative path
                size_t slash_pos = base.rfind('/');
                if (slash_pos != litehtml::string::npos && slash_pos > 7) {
                    full_url = base.substr(0, slash_pos + 1) + url;
                } else {
                    if (base[base.length() - 1] != '/') base += '/';
                    full_url = base + url;
                }
            }
        } else {
            full_url = url;
        }
    }
    
    print_serial("BROWSER: import_css fetching: ");
    print_serial(full_url.c_str());
    print_serial("\n");
    
    // 2. Fetch over network
    int max_len = 128 * 1024; // 128KB buffer
    char* buf = (char*)kmalloc(max_len);
    if (!buf) {
        print_serial("BROWSER: import_css kmalloc failed\n");
        return;
    }
    
    int len = 0;
    if (full_url.find("https://") == 0) {
        len = https_get(full_url.c_str(), buf, max_len);
    } else if (full_url.find("http://") == 0) {
        len = http_get(full_url.c_str(), buf, max_len);
    } else {
        print_serial("BROWSER: Unsupported protocol in import_css\n");
    }
    
    // 3. Store in litehtml::string
    if (len > 0) {
        text.assign(buf, len);
    } else if (len == -301 && strncmp(buf, "REDIRECT:", 9) == 0) {
        litehtml::string redir_url = buf + 9;
        kfree(buf);
        import_css(text, redir_url, baseurl);
        return;
    } else {
        print_serial("BROWSER: CSS fetch failed or empty\n");
    }
    
    kfree(buf);
}

void browser_container::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) {
    (void)pos;
    (void)bdr_radius;
}

void browser_container::del_clip() {
}

void browser_container::get_viewport(litehtml::position& viewport) const {
    viewport = m_viewport;
}

litehtml::element::ptr browser_container::create_element(const char* tag_name, const litehtml::string_map& attributes, const std::shared_ptr<litehtml::document>& doc) {
    return 0;
}

void browser_container::get_media_features(litehtml::media_features& media) const {
    litehtml::position client;
    get_viewport(client);
    media.type = litehtml::media_type_screen;
    media.width = client.width;
    media.height = client.height;
    media.device_width = 1024;
    media.device_height = 768;
    media.color = 8;
    media.monochrome = 0;
    media.color_index = 256;
    media.resolution = 96;
}

void browser_container::get_language(litehtml::string& language, litehtml::string& culture) const {
    language = "en";
    culture = "US";
}

void browser_container::set_client_rect(int width, int height) {
    m_viewport.width = width;
    m_viewport.height = height;
}

// ============================================================================
// C Bridge for browser.c
// ============================================================================

struct browser_engine_state {
    browser_container* container;
    std::shared_ptr<litehtml::document> document;
};

static browser_engine_state* current_browser_state = nullptr;

extern "C" {

void* browser_engine_create(int width, int height) {
    browser_engine_state* state = new browser_engine_state();
    state->container = new browser_container();
    state->container->set_client_rect(width, height);
    return state;
}

void browser_engine_destroy(void* engine) {
    if (!engine) return;
    browser_engine_state* state = (browser_engine_state*)engine;
    if (current_browser_state == state) current_browser_state = nullptr;
    state->document = nullptr;
    delete state->container;
    delete state;
}

void browser_engine_load_html(void* engine, const char* html, const char* master_css) {
    if (!engine) return;
    browser_engine_state* state = (browser_engine_state*)engine;
    current_browser_state = state;
    
    if (!master_css) {
        master_css = "html { display: block; } head { display: none; } body { display: block; margin: 8px; } "
                     "script, style, noscript, meta, title, link, iframe { display: none !important; } "
                     "h1 { display: block; font-size: 2em; margin-top: 0.67em; margin-bottom: 0.67em; font-weight: bold; } "
                     "h2 { display: block; font-size: 1.5em; margin-top: 0.83em; margin-bottom: 0.83em; font-weight: bold; } "
                     "p, div { display: block; margin-top: 1em; margin-bottom: 1em; } "
                     "a { text-decoration: underline; color: blue; cursor: pointer; } "
                     "b, strong { font-weight: bold; } i, em { font-style: italic; } "
                     "hr { display: block; border-style: inset; border-width: 1px; margin: 0.5em auto; }";
    }
    
    extern void print_serial(const char*);
    print_serial("BROWSER_CPP: Calling createFromString...\n");
    litehtml::string_id_enable_debug();
    state->document = litehtml::document::createFromString(html, state->container, master_css);
    print_serial("BROWSER_CPP: createFromString SUCCESS.\n");
}

void browser_engine_render(void* engine, int width) {
    if (!engine) return;
    browser_engine_state* state = (browser_engine_state*)engine;
    if (state->document) {
        state->document->render(width);
    }
}

void browser_engine_draw(void* engine, void* window_ptr, int x, int y, int clip_x, int clip_y, int clip_w, int clip_h) {
    if (!engine) return;
    browser_engine_state* state = (browser_engine_state*)engine;
    if (state->document) {
        litehtml::position clip(clip_x, clip_y, clip_w, clip_h);
        state->document->draw((litehtml::uint_ptr)window_ptr, x, y, &clip);
    }
}

int browser_engine_get_height(void* engine) {
    if (!engine) return 0;
    browser_engine_state* state = (browser_engine_state*)engine;
    if (state->document) {
        return state->document->height();
    }
    return 0;
}

int browser_engine_get_width(void* engine) {
    if (!engine) return 0;
    browser_engine_state* state = (browser_engine_state*)engine;
    if (state->document) {
        return state->document->width();
    }
    return 0;
}

}

extern "C" {
    void js_execute(const char* source);
    extern int layout_dirty;
}

static void execute_scripts_recursive(litehtml::element::ptr node) {
    if (!node) return;
    if (strcmp(node->get_tagName(), "script") == 0) {
        print_serial("SCRIPTS: Found <script> tag\n");
        const char* src = node->get_attr("src", nullptr);
        if (src && src[0]) {
            print_serial("SCRIPTS: External src=");
            print_serial(src);
            print_serial("\n");
            std::string base = current_browser_state->container->get_base_url();
            std::string full_url = resolve_url(src, base);
            
            int max_len = 256 * 1024;
            char* buf = (char*)kmalloc(max_len);
            if (buf) {
                int len = 0;
                if (full_url.find("https://") == 0) len = https_get(full_url.c_str(), buf, max_len);
                else if (full_url.find("http://") == 0) len = http_get(full_url.c_str(), buf, max_len);
                
                if (len > 0) {
                    buf[len] = 0;
                    js_execute(buf);
                }
                kfree(buf);
            }
        } else {
            litehtml::string text;
            node->get_text(text);
            print_serial("SCRIPTS: Inline script length=");
            {
                char lbuf[16]; int len = (int)text.length(); int li = 0;
                if (len == 0) { lbuf[li++] = '0'; } else { char r[16]; int ri = 0; while(len) { r[ri++] = '0'+(len%10); len/=10; } while(ri) lbuf[li++] = r[--ri]; }
                lbuf[li] = 0;
                print_serial(lbuf);
            }
            print_serial("\n");
            if (!text.empty()) {
                // Print first 200 chars of script for debugging
                print_serial("SCRIPTS: Content='");
                if (text.length() > 200) {
                    print_serial(text.substr(0, 200).c_str());
                    print_serial("...");
                } else {
                    print_serial(text.c_str());
                }
                print_serial("'\n");
                js_execute(text.c_str());
                print_serial("SCRIPTS: js_execute returned\n");
            } else {
                print_serial("SCRIPTS: WARNING - script tag has empty content!\n");
            }
        }
    }
    for (auto& child : node->children()) {
        execute_scripts_recursive(child);
    }
}

extern "C" {

void jsbridge_execute_scripts() {
    if (current_browser_state && current_browser_state->document) {
        execute_scripts_recursive(current_browser_state->document->root());
    }
}

void* jsbridge_get_element_by_id(const char* id) {
    if (!current_browser_state || !current_browser_state->document || !id) return nullptr;
    litehtml::string selector = litehtml::string("#") + id;
    litehtml::element::ptr el = current_browser_state->document->root()->select_one(selector);
    return el.get();
}

void* jsbridge_query_selector(void* node, const char* selector) {
    if (!selector || !current_browser_state || !current_browser_state->document) return nullptr;
    litehtml::element::ptr root_node = node ? ((litehtml::element*)node)->shared_from_this() : current_browser_state->document->root();
    if (!root_node) return nullptr;
    litehtml::element::ptr el = root_node->select_one(selector);
    return el.get();
}

// Global buffer for storing array of pointers returned by querySelectorAll
static void* g_qsa_results[256];
int jsbridge_query_selector_all(void* node, const char* selector, void*** out_results) {
    if (!selector || !current_browser_state || !current_browser_state->document) return 0;
    litehtml::element::ptr root_node = node ? ((litehtml::element*)node)->shared_from_this() : current_browser_state->document->root();
    if (!root_node) return 0;
    
    litehtml::elements_list list = root_node->select_all(litehtml::string(selector));
    
    int count = 0;
    for (auto& el : list) {
        if (count >= 256) break;
        g_qsa_results[count++] = el.get();
    }
    *out_results = g_qsa_results;
    return count;
}

const char* jsbridge_get_tag_name(void* node) {
    if (!node) return "";
    return ((litehtml::element*)node)->get_tagName();
}

static litehtml::string g_text_content_buf;
const char* jsbridge_get_text_content(void* node) {
    if (!node) return "";
    g_text_content_buf.clear();
    ((litehtml::element*)node)->get_text(g_text_content_buf);
    return g_text_content_buf.c_str();
}

void jsbridge_set_text_content(void* node, const char* text) {
    if (!node || !current_browser_state || !current_browser_state->document) return;
    litehtml::element* el = (litehtml::element*)node;
    el->clearRecursive();
    if (text && text[0]) {
        current_browser_state->document->append_children_from_string(*el, text, true);
    }
    layout_dirty = 1;
}

void jsbridge_set_inner_html(void* node, const char* html) {
    // In litehtml append_children_from_string parses HTML.
    jsbridge_set_text_content(node, html);
}

void* jsbridge_retain_node(void* raw_ptr) {
    if (!raw_ptr) return nullptr;
    litehtml::element* el = (litehtml::element*)raw_ptr;
    auto* sp = new std::shared_ptr<litehtml::element>(el->shared_from_this());
    return sp;
}

void jsbridge_release_node(void* handle) {
    if (!handle) return;
    auto* sp = (std::shared_ptr<litehtml::element>*)handle;
    delete sp;
}

void jsbridge_add_class(void* node, const char* cls) {
    if (!node || !cls) return;
    ((litehtml::element*)node)->set_class(cls, true);
    if (current_browser_state && current_browser_state->document) current_browser_state->document->force_styles_update();
    layout_dirty = 1;
}

void jsbridge_remove_class(void* node, const char* cls) {
    if (!node || !cls) return;
    ((litehtml::element*)node)->set_class(cls, false);
    if (current_browser_state && current_browser_state->document) current_browser_state->document->force_styles_update();
    layout_dirty = 1;
}

int jsbridge_has_class(void* node, const char* cls) {
    if (!node || !cls) return 0;
    litehtml::string selector = litehtml::string(".") + cls;
    return ((litehtml::element*)node)->select(selector) ? 1 : 0;
}

void jsbridge_toggle_class(void* node, const char* cls) {
    if (!node || !cls) return;
    int has = jsbridge_has_class(node, cls);
    ((litehtml::element*)node)->set_class(cls, !has);
    if (current_browser_state && current_browser_state->document) current_browser_state->document->force_styles_update();
    layout_dirty = 1;
}

// ============================================================================
// Phase 4: Click-to-JS Event Routing
// ============================================================================
// Phase 4: Click-to-JS Event Routing
// ============================================================================

extern "C" void js_dispatch_event(void* node, const char* event_type, int mx, int my);

int browser_engine_on_mouse(void* engine, int doc_x, int doc_y) {
    if (!engine) return 0;
    browser_engine_state* state = (browser_engine_state*)engine;
    if (!state->document) return 0;
    litehtml::position::vector redraw_boxes;
    bool changed = state->document->on_mouse_over(doc_x, doc_y, doc_x, doc_y, redraw_boxes);
    return changed ? 1 : 0;
}

void browser_engine_on_click(void* engine, int doc_x, int doc_y) {
    if (!engine) return;
    browser_engine_state* state = (browser_engine_state*)engine;
    if (!state->document) return;

    // 1. Let litehtml update :hover / :active / handle <a> clicks internally
    litehtml::position::vector redraw_boxes;
    state->document->on_lbutton_down(doc_x, doc_y, doc_x, doc_y, redraw_boxes);
    state->document->on_lbutton_up(doc_x, doc_y, doc_x, doc_y, redraw_boxes);

    // 2. Hit-test to find the deepest element under the cursor
    auto root_ri = state->document->root_render();
    if (!root_ri) return;

    auto check = [](const std::shared_ptr<litehtml::render_item>&) { return true; };
    auto el = root_ri->get_element_by_point(doc_x, doc_y, doc_x, doc_y, check);
    if (!el) return;

    print_serial("BROWSER: Click on <");
    print_serial(el->get_tagName());
    print_serial(">\n");

    // 3. Fire "click" and bubble up through ancestors
    auto cur = el;
    int depth = 0;
    while (cur && depth < 16) {
        js_dispatch_event(cur.get(), "click", doc_x, doc_y);
        const char* tag = cur->get_tagName();
        if (strcmp(tag, "body") == 0 || strcmp(tag, "html") == 0) break;
        cur = cur->parent();
        depth++;
    }

    layout_dirty = 1;
}

} // extern "C"
