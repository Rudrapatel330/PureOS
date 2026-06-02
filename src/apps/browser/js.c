// js.c - Duktape Bridge for PureBrowser (Enhanced DOM Support)
#include "js.h"
#include "duktape.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"
#include "dom.h"
#include <time.h>
#include "../../lib/tinyexpr.h"

static duk_context *ctx = NULL;

extern void print_serial(const char *);
extern void kernel_poll_events(void);
extern dom_node_t *current_document;
extern int ui_dirty;

// Forward declarations
static duk_ret_t native_get_element_by_id(duk_context *ctx);
static duk_ret_t native_add_event_listener(duk_context *ctx);

// Compatibility stubs for Duktape
void abort(void) {
    print_serial("FATAL: JS engine called abort()!\n");
    while(1) kernel_poll_events();
}

time_t mktime(struct tm *tp) {
    (void)tp;
    return 0;
}

double difftime(time_t time1, time_t time0) {
    return (double)(time1 - time0);
}

// Native log function for JS
static duk_ret_t native_print(duk_context *ctx) {
    if (duk_get_top(ctx) > 0) {
        const char *msg = duk_to_string(ctx, 0);
        print_serial("JS LOG: ");
        print_serial(msg);
        print_serial("\n");
    }
    return 0;
}

// Math.evaluate binding using TinyExpr
static duk_ret_t native_math_evaluate(duk_context *ctx) {
    if (duk_is_string(ctx, 0)) {
        const char *expr = duk_get_string(ctx, 0);
        int error = 0;
        double result = te_interp(expr, &error);
        
        if (error != 0) {
            duk_push_nan(ctx);
        } else {
            duk_push_number(ctx, result);
        }
    } else {
        duk_push_nan(ctx);
    }
    return 1;
}

extern void* jsbridge_get_element_by_id(const char* id);
extern const char* jsbridge_get_tag_name(void* node);
extern const char* jsbridge_get_text_content(void* node);
extern void jsbridge_set_text_content(void* node, const char* text);
extern void jsbridge_add_class(void* node, const char* cls);
extern void jsbridge_remove_class(void* node, const char* cls);
extern int jsbridge_has_class(void* node, const char* cls);
extern void jsbridge_toggle_class(void* node, const char* cls);
extern void jsbridge_set_inner_html(void* node, const char* html);
extern void* jsbridge_query_selector(void* node, const char* selector);
extern int jsbridge_query_selector_all(void* node, const char* selector, void*** out_results);
extern void* jsbridge_retain_node(void* raw_ptr);
extern void jsbridge_release_node(void* handle);
extern void k_itoa_hex(uint32_t val, char *str);

static uint32_t parse_hex(const char *s) {
    uint32_t v = 0;
    while (*s) {
        v <<= 4;
        if (*s >= '0' && *s <= '9') v |= (*s - '0');
        else if (*s >= 'a' && *s <= 'f') v |= (*s - 'a' + 10);
        else if (*s >= 'A' && *s <= 'F') v |= (*s - 'A' + 10);
        s++;
    }
    return v;
}

static duk_ret_t dom_text_getter(duk_context *ctx) {
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__ptr");
    void *node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
    duk_pop_2(ctx);
    if (!node) return 0;
    duk_push_string(ctx, jsbridge_get_text_content(node));
    return 1;
}

static duk_ret_t dom_text_setter(duk_context *ctx) {
    const char *text = duk_to_string(ctx, 0);
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__ptr");
    void *node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
    duk_pop_2(ctx);
    if (node && text) {
        jsbridge_set_text_content(node, text);
    }
    return 0;
}

static duk_ret_t dom_html_getter(duk_context *ctx) {
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__ptr");
    void *node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
    duk_pop_2(ctx);
    if (!node) return 0;
    // We just return textContent for now as HTML serialization isn't built into litehtml
    duk_push_string(ctx, jsbridge_get_text_content(node));
    return 1;
}

static duk_ret_t dom_html_setter(duk_context *ctx) {
    const char *html = duk_to_string(ctx, 0);
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__ptr");
    void *node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
    duk_pop_2(ctx);
    if (node && html) {
        jsbridge_set_inner_html(node, html);
    }
    return 0;
}

static duk_ret_t native_classlist_contains(duk_context *ctx) {
    const char *cls = duk_to_string(ctx, 0);
    if (!cls || !*cls) { duk_push_false(ctx); return 1; }
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__ptr");
    void *node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
    duk_pop_2(ctx);
    if (!node) { duk_push_false(ctx); return 1; }
    duk_push_boolean(ctx, jsbridge_has_class(node, cls));
    return 1;
}

static duk_ret_t native_classlist_add(duk_context *ctx) {
    const char *cls = duk_to_string(ctx, 0);
    if (!cls || !*cls) return 0;
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__ptr");
    void *node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
    duk_pop_2(ctx);
    if (node) jsbridge_add_class(node, cls);
    return 0;
}

static duk_ret_t native_classlist_remove(duk_context *ctx) {
    const char *cls = duk_to_string(ctx, 0);
    if (!cls || !*cls) return 0;
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__ptr");
    void *node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
    duk_pop_2(ctx);
    if (node) jsbridge_remove_class(node, cls);
    return 0;
}

static duk_ret_t native_classlist_toggle(duk_context *ctx) {
    const char *cls = duk_to_string(ctx, 0);
    if (!cls || !*cls) return 0;
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "__ptr");
    void *node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
    duk_pop_2(ctx);
    if (node) jsbridge_toggle_class(node, cls);
    return 0;
}

#define MAX_EVENT_LISTENERS 128
typedef struct {
    void* node;
    char type[24];
    int stash_idx;
} event_listener_t;
static event_listener_t event_listeners[MAX_EVENT_LISTENERS];
static int event_listener_count = 0;

static duk_ret_t native_add_event_listener(duk_context *ctx) {
    const char *event_type = duk_to_string(ctx, 0);
    if (!event_type || !duk_is_function(ctx, 1)) {
        print_serial("JS addEventListener: missing type or function\n");
        return 0;
    }

    duk_push_this(ctx);
    if (!duk_has_prop_string(ctx, -1, "__ptr")) {
        print_serial("JS addEventListener: 'this' has no __ptr (not a DOM node)\n");
        duk_pop(ctx);
        return 0;
    }
    duk_get_prop_string(ctx, -1, "__ptr");
    const char *ptr_str = duk_to_string(ctx, -1);
    print_serial("JS addEventListener: type='");
    print_serial(event_type);
    print_serial("' ptr_str='");
    print_serial(ptr_str);
    print_serial("'\n");
    void *node = (void *)(uintptr_t)parse_hex(ptr_str);
    duk_pop_2(ctx);
    if (!node) {
        print_serial("JS addEventListener: parsed node is NULL!\n");
        return 0;
    }

    if (event_listener_count >= MAX_EVENT_LISTENERS) return 0;

    duk_push_global_stash(ctx);
    int stash_idx = 0;
    while (1) {
        duk_push_int(ctx, stash_idx);
        duk_get_prop(ctx, -2);
        if (duk_is_undefined(ctx, -1)) { duk_pop(ctx); break; }
        duk_pop(ctx);
        stash_idx++;
        if (stash_idx > 256) { duk_pop(ctx); return 0; }
    }
    duk_dup(ctx, 1);
    duk_push_int(ctx, stash_idx);
    duk_dup(ctx, -2);
    duk_put_prop(ctx, -4);
    duk_pop_2(ctx);

    event_listeners[event_listener_count].node = node;
    strncpy(event_listeners[event_listener_count].type, event_type, 23);
    event_listeners[event_listener_count].type[23] = 0;
    event_listeners[event_listener_count].stash_idx = stash_idx;
    event_listener_count++;

    {
        char buf[32];
        k_itoa_hex((uint32_t)(uintptr_t)node, buf);
        print_serial("JS addEventListener: registered node=0x");
        print_serial(buf);
        print_serial(" type='");
        print_serial(event_type);
        print_serial("' count=");
        char cbuf[8]; int c = event_listener_count;
        int ci = 0;
        if (c == 0) { cbuf[ci++] = '0'; } else { char r[8]; int ri = 0; while(c) { r[ri++] = '0'+(c%10); c/=10; } while(ri) cbuf[ci++] = r[--ri]; }
        cbuf[ci] = 0;
        print_serial(cbuf);
        print_serial("\n");
    }

    return 0;
}

void js_dispatch_event(void *node, const char *event_type, int mx, int my) {
    if (!node || !ctx) {
        print_serial("js_dispatch_event: SKIP (node or ctx is NULL)\n");
        return;
    }
    {
        char buf[32];
        k_itoa_hex((uint32_t)(uintptr_t)node, buf);
        print_serial("js_dispatch_event: node=0x");
        print_serial(buf);
        print_serial(" type='");
        print_serial(event_type);
        print_serial("' listeners=");
        char cbuf[8]; int c = event_listener_count;
        int ci = 0;
        if (c == 0) { cbuf[ci++] = '0'; } else { char r[8]; int ri = 0; while(c) { r[ri++] = '0'+(c%10); c/=10; } while(ri) cbuf[ci++] = r[--ri]; }
        cbuf[ci] = 0;
        print_serial(cbuf);
        print_serial("\n");
    }
    for (int i = 0; i < event_listener_count; i++) {
        {
            char buf1[32], buf2[32];
            k_itoa_hex((uint32_t)(uintptr_t)event_listeners[i].node, buf1);
            k_itoa_hex((uint32_t)(uintptr_t)node, buf2);
            print_serial("  listener[");
            char ibuf[8]; int ii = i; int ici = 0;
            if (ii == 0) { ibuf[ici++] = '0'; } else { char r[8]; int ri = 0; while(ii) { r[ri++] = '0'+(ii%10); ii/=10; } while(ri) ibuf[ici++] = r[--ri]; }
            ibuf[ici] = 0;
            print_serial(ibuf);
            print_serial("]: registered=0x");
            print_serial(buf1);
            print_serial(" current=0x");
            print_serial(buf2);
            print_serial(" type='");
            print_serial(event_listeners[i].type);
            print_serial("' match=");
            print_serial((event_listeners[i].node == node && strcmp(event_listeners[i].type, event_type) == 0) ? "YES" : "NO");
            print_serial("\n");
        }
        if (event_listeners[i].node == node && strcmp(event_listeners[i].type, event_type) == 0) {
            print_serial("  -> FIRING event callback!\n");
            duk_push_global_stash(ctx);
            duk_push_int(ctx, event_listeners[i].stash_idx);
            duk_get_prop(ctx, -2);
            if (duk_is_function(ctx, -1)) {
                duk_push_object(ctx);
                duk_push_string(ctx, event_type);
                duk_put_prop_string(ctx, -2, "type");
                duk_push_int(ctx, mx);
                duk_put_prop_string(ctx, -2, "clientX");
                duk_push_int(ctx, my);
                duk_put_prop_string(ctx, -2, "clientY");
                duk_push_boolean(ctx, 1);
                duk_put_prop_string(ctx, -2, "bubbles");
                if (duk_pcall(ctx, 1) != 0) {
                    print_serial("JS EVENT ERROR: ");
                    print_serial(duk_safe_to_string(ctx, -1));
                    print_serial("\n");
                }
                duk_pop(ctx);
            } else {
                print_serial("  -> stash entry is NOT a function!\n");
                duk_pop(ctx);
            }
            duk_pop(ctx);
        }
    }
}

static duk_ret_t dom_node_finalizer(duk_context *ctx) {
    if (duk_has_prop_string(ctx, 0, "__handle")) {
        duk_get_prop_string(ctx, 0, "__handle");
        void *handle = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
        duk_pop(ctx);
        if (handle) {
            jsbridge_release_node(handle);
        }
    }
    return 0;
}

static void push_dom_node(duk_context *ctx, void *node) {
    if (!node) { duk_push_undefined(ctx); return; }
    duk_idx_t obj_idx = duk_push_object(ctx);
    
    char ptr_str[32];
    k_itoa_hex((uint32_t)(uintptr_t)node, ptr_str);
    duk_push_string(ctx, ptr_str);
    duk_put_prop_string(ctx, obj_idx, "__ptr");
    
    void *handle = jsbridge_retain_node(node);
    char handle_str[32];
    k_itoa_hex((uint32_t)(uintptr_t)handle, handle_str);
    duk_push_string(ctx, handle_str);
    duk_put_prop_string(ctx, obj_idx, "__handle");
    
    duk_push_c_function(ctx, dom_node_finalizer, 1);
    duk_set_finalizer(ctx, obj_idx);

    duk_push_string(ctx, jsbridge_get_tag_name(node));
    duk_put_prop_string(ctx, obj_idx, "tagName");

    duk_push_string(ctx, "textContent");
    duk_push_c_function(ctx, dom_text_getter, 0);
    duk_push_c_function(ctx, dom_text_setter, 1);
    duk_def_prop(ctx, obj_idx, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "innerHTML");
    duk_push_c_function(ctx, dom_html_getter, 0);
    duk_push_c_function(ctx, dom_html_setter, 1);
    duk_def_prop(ctx, obj_idx, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    // classList methods
    duk_push_object(ctx);
    duk_push_string(ctx, ptr_str);
    duk_put_prop_string(ctx, -2, "__ptr");
    duk_push_c_function(ctx, native_classlist_contains, 1);
    duk_put_prop_string(ctx, -2, "contains");
    duk_push_c_function(ctx, native_classlist_add, 1);
    duk_put_prop_string(ctx, -2, "add");
    duk_push_c_function(ctx, native_classlist_remove, 1);
    duk_put_prop_string(ctx, -2, "remove");
    duk_push_c_function(ctx, native_classlist_toggle, 1);
    duk_put_prop_string(ctx, -2, "toggle");
    duk_put_prop_string(ctx, obj_idx, "classList");

    duk_push_c_function(ctx, native_add_event_listener, 2);
    duk_put_prop_string(ctx, obj_idx, "addEventListener");
    duk_push_c_function(ctx, native_add_event_listener, 2);
    duk_put_prop_string(ctx, obj_idx, "removeEventListener");

    extern duk_ret_t native_query_selector(duk_context *ctx);
    duk_push_c_function(ctx, native_query_selector, 1);
    duk_put_prop_string(ctx, obj_idx, "querySelector");
    extern duk_ret_t native_query_selector_all(duk_context *ctx);
    duk_push_c_function(ctx, native_query_selector_all, 1);
    duk_put_prop_string(ctx, obj_idx, "querySelectorAll");
}

static duk_ret_t native_get_element_by_id(duk_context *ctx) {
    const char *id = duk_to_string(ctx, 0);
    if (!id) return 0;
    void *found = jsbridge_get_element_by_id(id);
    push_dom_node(ctx, found);
    return 1;
}

duk_ret_t native_query_selector(duk_context *ctx) {
    const char *selector = duk_to_string(ctx, 0);
    if (!selector) return 0;
    
    void* context_node = NULL;
    duk_push_this(ctx);
    if (duk_has_prop_string(ctx, -1, "__ptr")) {
        duk_get_prop_string(ctx, -1, "__ptr");
        context_node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
        duk_pop(ctx);
    }
    duk_pop(ctx); // pop this

    void *found = jsbridge_query_selector(context_node, selector);
    if (found) {
        push_dom_node(ctx, found);
        return 1;
    }
    
    duk_push_null(ctx);
    return 1;
}

duk_ret_t native_query_selector_all(duk_context *ctx) {
    const char *selector = duk_to_string(ctx, 0);
    if (!selector) return 0;

    void* context_node = NULL;
    duk_push_this(ctx);
    if (duk_has_prop_string(ctx, -1, "__ptr")) {
        duk_get_prop_string(ctx, -1, "__ptr");
        context_node = (void *)(uintptr_t)parse_hex(duk_to_string(ctx, -1));
        duk_pop(ctx);
    }
    duk_pop(ctx);

    void** results;
    int count = jsbridge_query_selector_all(context_node, selector, &results);
    
    duk_idx_t arr_idx = duk_push_array(ctx);
    for (int i = 0; i < count; i++) {
        push_dom_node(ctx, results[i]);
        duk_put_prop_index(ctx, arr_idx, i);
    }
    
    return 1;
}

static duk_ret_t native_create_element(duk_context *ctx) {
    return 0;
}

// element.setAttribute binding
static duk_ret_t native_set_attribute(duk_context *ctx) {
    return 0; // Simplified
}

// element.getAttribute binding
static duk_ret_t native_get_attribute(duk_context *ctx) {
    return 0; // Simplified
}

// element.style.setProperty binding
static duk_ret_t native_style_set_property(duk_context *ctx) {
    return 0; // Simplified
}

// window.getComputedStyle
static duk_ret_t native_get_computed_style(duk_context *ctx) {
    duk_push_object(ctx);
    return 1;
}

// Timer support for setTimeout
#define MAX_JS_TIMERS 16

typedef struct {
    int id;
    int active;
    int remaining_ms;
    int callback_stash_idx;
} js_timer_t;

static js_timer_t js_timers[MAX_JS_TIMERS];
static int js_next_timer_id = 1;

// Called each frame from browser draw loop to check expired timers
void js_tick_timers(void) {
    if (!ctx) return;
    for (int i = 0; i < MAX_JS_TIMERS; i++) {
        if (!js_timers[i].active) continue;
        js_timers[i].remaining_ms -= 16; // ~16ms per frame
        if (js_timers[i].remaining_ms <= 0) {
            js_timers[i].active = 0;
            duk_push_global_stash(ctx);
            duk_push_int(ctx, js_timers[i].callback_stash_idx);
            duk_get_prop(ctx, -2);
            if (duk_is_function(ctx, -1)) {
                if (duk_pcall(ctx, 0) != 0) {
                    print_serial("JS TIMER ERROR: ");
                    print_serial(duk_safe_to_string(ctx, -1));
                    print_serial("\n");
                }
                duk_pop(ctx);
            } else {
                duk_pop(ctx);
            }
            duk_pop(ctx);
        }
    }
}

static duk_ret_t native_set_timeout(duk_context *ctx) {
    if (!duk_is_function(ctx, 0)) {
        duk_push_int(ctx, 0);
        return 1;
    }
    int delay = 0;
    if (duk_is_number(ctx, 1)) delay = (int)duk_get_number(ctx, 1);
    if (delay < 0) delay = 0;

    // Stash the callback
    duk_push_global_stash(ctx);
    int stash_idx = 0;
    while (1) {
        duk_push_int(ctx, stash_idx);
        duk_get_prop(ctx, -2);
        if (duk_is_undefined(ctx, -1)) { duk_pop(ctx); break; }
        duk_pop(ctx);
        stash_idx++;
        if (stash_idx > 256) { duk_pop(ctx); duk_push_int(ctx, 0); return 1; }
    }
    duk_dup(ctx, 0);
    duk_push_int(ctx, stash_idx);
    duk_dup(ctx, -2);
    duk_put_prop(ctx, -4);
    duk_pop_2(ctx);

    // Find a timer slot
    int slot = -1;
    for (int i = 0; i < MAX_JS_TIMERS; i++) {
        if (!js_timers[i].active) { slot = i; break; }
    }
    if (slot < 0) { duk_push_int(ctx, 0); return 1; }

    int tid = js_next_timer_id++;
    js_timers[slot].id = tid;
    js_timers[slot].active = 1;
    js_timers[slot].remaining_ms = delay;
    js_timers[slot].callback_stash_idx = stash_idx;

    duk_push_int(ctx, tid);
    return 1;
}

// clearTimeout
static duk_ret_t native_clear_timeout(duk_context *ctx) {
    int tid = (int)duk_get_number(ctx, 0);
    for (int i = 0; i < MAX_JS_TIMERS; i++) {
        if (js_timers[i].active && js_timers[i].id == tid) {
            js_timers[i].active = 0;
            break;
        }
    }
    return 0;
}

// Fatal handler for Duktape
static void native_fatal(void *udata, const char *msg) {
    (void)udata;
    print_serial("JS FATAL ERROR: ");
    print_serial(msg ? msg : "no message");
    print_serial("\n");
    while(1) { kernel_poll_events(); }
}

static void *duk_alloc_wrapper(void *udata, duk_size_t size) {
    (void)udata;
    return malloc(size);
}

static void *duk_realloc_wrapper(void *udata, void *ptr, duk_size_t size) {
    (void)udata;
    return realloc(ptr, size);
}

static void duk_free_wrapper(void *udata, void *ptr) {
    (void)udata;
    free(ptr);
}

void js_init(void) {
    print_serial("JS: Initializing Duktape Engine...\n");
    if (ctx) {
        print_serial("JS: Destroying old heap...\n");
        duk_destroy_heap(ctx);
        ctx = NULL;
    }
    
    print_serial("JS: Calling duk_create_heap...\n");
    ctx = duk_create_heap(duk_alloc_wrapper, duk_realloc_wrapper, duk_free_wrapper, NULL, native_fatal);
    
    if (ctx) {
        print_serial("JS: Registering globals...\n");
        duk_push_global_object(ctx);
        
        // Register console.log
        duk_push_object(ctx);
        duk_push_c_function(ctx, native_print, 1);
        duk_put_prop_string(ctx, -2, "log");
        duk_put_prop_string(ctx, -2, "console");
        
        // Register alert
        duk_push_c_function(ctx, native_print, 1);
        duk_put_prop_string(ctx, -2, "alert");

        // Enhanced document object
        duk_push_object(ctx);
        duk_push_c_function(ctx, native_get_element_by_id, 1);
        duk_put_prop_string(ctx, -2, "getElementById");
        duk_push_c_function(ctx, native_query_selector, 1);
        duk_put_prop_string(ctx, -2, "querySelector");
        duk_push_c_function(ctx, native_query_selector_all, 1);
        duk_put_prop_string(ctx, -2, "querySelectorAll");
        duk_push_c_function(ctx, native_create_element, 1);
        duk_put_prop_string(ctx, -2, "createElement");
        // document.body stub
        duk_push_object(ctx);
        duk_push_c_function(ctx, native_add_event_listener, 2);
        duk_put_prop_string(ctx, -2, "addEventListener");
        duk_put_prop_string(ctx, -2, "body");
        // document.documentElement stub
        duk_push_object(ctx);
        duk_put_prop_string(ctx, -2, "documentElement");
        duk_put_prop_string(ctx, -2, "document");

        // window object
        duk_push_c_function(ctx, native_set_timeout, 2);
        duk_put_prop_string(ctx, -2, "setTimeout");
        duk_push_c_function(ctx, native_set_timeout, 2);
        duk_put_prop_string(ctx, -2, "setInterval");
        duk_push_c_function(ctx, native_clear_timeout, 1);
        duk_put_prop_string(ctx, -2, "clearTimeout");
        duk_push_c_function(ctx, native_get_computed_style, 1);
        duk_put_prop_string(ctx, -2, "getComputedStyle");

        // Element.prototype stubs
        duk_push_c_function(ctx, native_add_event_listener, 2);
        duk_put_prop_string(ctx, -2, "addEventListener");
        duk_push_c_function(ctx, native_set_attribute, 2);
        duk_put_prop_string(ctx, -2, "setAttribute");
        
        // location stub
        duk_push_object(ctx);
        duk_push_string(ctx, "");
        duk_put_prop_string(ctx, -2, "href");
        duk_push_string(ctx, "");
        duk_put_prop_string(ctx, -2, "hostname");
        duk_put_prop_string(ctx, -2, "location");

        duk_pop(ctx);

        // Register Math.evaluate
        duk_get_global_string(ctx, "Math");
        duk_push_c_function(ctx, native_math_evaluate, 1);
        duk_put_prop_string(ctx, -2, "evaluate");
        duk_pop(ctx);

        print_serial("JS: Duktape Ready.\n");
    } else {
        print_serial("JS ERROR: Failed to create Duktape heap!\n");
    }
}

void js_execute(const char *source) {
    if (!ctx || !source || source[0] == '\0') return;
    
    print_serial("JS: Executing script...\n");
    if (duk_peval_string(ctx, source) != 0) {
        print_serial("JS EXCEPTION: ");
        print_serial(duk_safe_to_string(ctx, -1));
        print_serial("\n");
    }
    duk_pop(ctx);
}