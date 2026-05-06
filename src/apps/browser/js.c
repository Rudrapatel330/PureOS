// js.c - Duktape Bridge for PureBrowser
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

// Compatibility stubs for Duktape
void abort(void) {
    print_serial("FATAL: JS engine called abort()!\n");
    while(1) kernel_poll_events();
}

time_t mktime(struct tm *tp) {
    (void)tp;
    return 0; // Stub
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

// document.getElementById binding
static duk_ret_t native_get_element_by_id(duk_context *ctx) {
    const char *id = duk_to_string(ctx, 0);
    // Note: We need a way to wrap the DOM node in a JS object
    // For now, just return undefined or a dummy
    return 0; 
}

// Fatal handler for Duktape
static void native_fatal(void *udata, const char *msg) {
    (void)udata;
    print_serial("JS FATAL ERROR: ");
    print_serial(msg ? msg : "no message");
    print_serial("\n");
    
    // Enter infinite loop to prevent further corruption
    while(1) {
        kernel_poll_events();
    }
}

// Allocator wrappers for Duktape (to match its expected signature with udata)
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
    
    // Create heap using our allocator wrappers and fatal handler
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

        // Register document object (very basic)
        duk_push_object(ctx);
        duk_push_c_function(ctx, native_get_element_by_id, 1);
        duk_put_prop_string(ctx, -2, "getElementById");
        duk_put_prop_string(ctx, -2, "document");
        
        duk_pop(ctx);

        // Register Math.evaluate
        duk_get_global_string(ctx, "Math");
        duk_push_c_function(ctx, native_math_evaluate, 1);
        duk_put_prop_string(ctx, -2, "evaluate");
        duk_pop(ctx);

        print_serial("JS: Duktape Ready.\n");
    } else {
        print_serial("JS ERROR: Failed to create Duktape heap (returned NULL)!\n");
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
