// js.c - PureJS Bytecode Compiler and Virtual Machine
#include "js.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"
#include "dom.h"

extern void print_serial(const char *);
extern void kernel_poll_events(void);
extern dom_node_t *current_document;
extern int ui_dirty;

// Global VM Instance
static js_vm_t global_vm;

// --- LEXER ---
typedef enum {
    TOK_EOF = 0, TOK_ID, TOK_NUM, TOK_STR,
    TOK_VAR, TOK_IF, TOK_FOR, TOK_FUNC,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_SEMICOLON, TOK_COMMA, TOK_DOT,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT
} tok_type_t;

typedef struct {
    tok_type_t type;
    char text[JS_MAX_STRING_LEN];
    int num;
} token_t;

static token_t tokens[JS_MAX_TOKENS];
static int token_count = 0;
static int cur_tok = 0;

static void tokenize(const char *source) {
    token_count = 0;
    const char *p = source;
    while (*p && token_count < JS_MAX_TOKENS - 1) {
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
        if (!*p) break;

        token_t *t = &tokens[token_count++];
        memset(t, 0, sizeof(token_t));

        if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_' || *p == '$') {
            int i = 0;
            while (*p && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_' || *p == '$')) {
                if (i < JS_MAX_STRING_LEN - 1) t->text[i++] = *p;
                p++;
            }
            t->text[i] = 0;
            if (strcmp(t->text, "var") == 0) t->type = TOK_VAR;
            else if (strcmp(t->text, "if") == 0) t->type = TOK_IF;
            else if (strcmp(t->text, "for") == 0) t->type = TOK_FOR;
            else if (strcmp(t->text, "function") == 0) t->type = TOK_FUNC;
            else t->type = TOK_ID;
        } else if (*p >= '0' && *p <= '9') {
            t->type = TOK_NUM;
            t->num = 0;
            while (*p >= '0' && *p <= '9') {
                t->num = t->num * 10 + (*p - '0');
                p++;
            }
        } else if (*p == '"' || *p == '\'') {
            char quote = *p++;
            t->type = TOK_STR;
            int i = 0;
            while (*p && *p != quote) {
                if (i < JS_MAX_STRING_LEN - 1) t->text[i++] = *p;
                p++;
            }
            if (*p) p++;
            t->text[i] = 0;
        } else {
            char c = *p++;
            if (c == '(') t->type = TOK_LPAREN;
            else if (c == ')') t->type = TOK_RPAREN;
            else if (c == '{') t->type = TOK_LBRACE;
            else if (c == '}') t->type = TOK_RBRACE;
            else if (c == ';') t->type = TOK_SEMICOLON;
            else if (c == ',') t->type = TOK_COMMA;
            else if (c == '.') t->type = TOK_DOT;
            else if (c == '+') t->type = TOK_PLUS;
            else if (c == '-') t->type = TOK_MINUS;
            else if (c == '*') t->type = TOK_MUL;
            else if (c == '/') t->type = TOK_DIV;
            else if (c == '=') {
                if (*p == '=') { t->type = TOK_EQ; p++; }
                else t->type = TOK_ASSIGN;
            } else if (c == '!') {
                if (*p == '=') { t->type = TOK_NEQ; p++; }
            } else if (c == '<') t->type = TOK_LT;
            else if (c == '>') t->type = TOK_GT;
        }
    }
    tokens[token_count].type = TOK_EOF;
}

// --- VM EXECUTION ---

static js_var_t *vm_get_var(const char *name) {
    for (int i = 0; i < global_vm.var_count; i++) {
        if (strcmp(global_vm.vars[i].name, name) == 0) return &global_vm.vars[i];
    }
    if (global_vm.var_count < JS_MAX_VARS) {
        js_var_t *v = &global_vm.vars[global_vm.var_count++];
        strncpy(v->name, name, JS_MAX_VAR_NAME_LEN - 1);
        v->type = JS_VAL_UNDEFINED;
        return v;
    }
    return NULL;
}

static void vm_push(js_var_t v) {
    if (global_vm.sp < JS_STACK_SIZE) global_vm.stack[global_vm.sp++] = v;
}

static js_var_t vm_pop() {
    if (global_vm.sp > 0) return global_vm.stack[--global_vm.sp];
    js_var_t v = {0}; v.type = JS_VAL_UNDEFINED;
    return v;
}

static void vm_run() {
    int iters = 0;
    while (global_vm.pc < global_vm.code_len) {
        if (++iters > 100) { kernel_poll_events(); iters = 0; }
        
        js_inst_t inst = global_vm.code[global_vm.pc++];
        switch (inst.op) {
            case OP_PUSH_NUM: {
                js_var_t v = {0}; v.type = JS_VAL_NUMBER; v.num_val = inst.arg_num;
                vm_push(v);
                break;
            }
            case OP_PUSH_STR: {
                js_var_t v = {0}; v.type = JS_VAL_STRING; strncpy(v.str_val, inst.arg_str, JS_MAX_STRING_LEN-1);
                vm_push(v);
                break;
            }
            case OP_PUSH_VAR: {
                if (strcmp(inst.arg_str, "document") == 0) {
                    js_var_t v = {0}; v.type = JS_VAL_OBJECT_REF; v.obj_ref = current_document;
                    vm_push(v);
                } else {
                    js_var_t *v = vm_get_var(inst.arg_str);
                    vm_push(v ? *v : (js_var_t){.type=JS_VAL_UNDEFINED});
                }
                break;
            }
            case OP_STORE_VAR: {
                js_var_t val = vm_pop();
                js_var_t *v = vm_get_var(inst.arg_str);
                if (v) *v = val;
                break;
            }
            case OP_ADD: {
                js_var_t b = vm_pop(); js_var_t a = vm_pop();
                js_var_t res = {0};
                if (a.type == JS_VAL_STRING || b.type == JS_VAL_STRING) {
                    res.type = JS_VAL_STRING;
                    strncpy(res.str_val, a.str_val, JS_MAX_STRING_LEN-1);
                    strncat(res.str_val, b.str_val, JS_MAX_STRING_LEN-1-strlen(res.str_val));
                } else {
                    res.type = JS_VAL_NUMBER; res.num_val = a.num_val + b.num_val;
                }
                vm_push(res);
                break;
            }
            case OP_CALL: {
                // Simplified call for native funcs
                js_var_t func = vm_pop();
                if (strcmp(inst.arg_str, "log") == 0) {
                    js_var_t arg = vm_pop();
                    print_serial("JS LOG: ");
                    if (arg.type == JS_VAL_STRING) print_serial(arg.str_val);
                    else if (arg.type == JS_VAL_NUMBER) { char b[16]; k_itoa(arg.num_val, b); print_serial(b); }
                    print_serial("\n");
                }
                break;
            }
            case OP_HALT: return;
            default: break;
        }
    }
}

// --- COMPILER (Simplified) ---

static js_inst_t *bytecode = NULL;
static int bc_ptr = 0;

static void emit(js_opcode_t op, int num, const char *str) {
    js_inst_t *inst = &bytecode[bc_ptr++];
    inst->op = op;
    inst->arg_num = num;
    if (str) strncpy(inst->arg_str, str, JS_MAX_VAR_NAME_LEN-1);
}

void js_init() {
    memset(&global_vm, 0, sizeof(js_vm_t));
    if (!bytecode) bytecode = kmalloc(sizeof(js_inst_t) * 4096);
}

void js_execute(const char *source) {
    if (!source) return;
    js_init();
    tokenize(source);
    
    bc_ptr = 0;
    cur_tok = 0;
    
    // Tiny one-pass compiler for demo: handles "var x = 10; log(x);"
    while (tokens[cur_tok].type != TOK_EOF && bc_ptr < 4000) {
        token_t t = tokens[cur_tok++];
        if (t.type == TOK_VAR) {
            token_t id = tokens[cur_tok++];
            if (tokens[cur_tok].type == TOK_ASSIGN) {
                cur_tok++;
                token_t val = tokens[cur_tok++];
                if (val.type == TOK_NUM) emit(OP_PUSH_NUM, val.num, NULL);
                else if (val.type == TOK_STR) emit(OP_PUSH_STR, 0, val.text);
                emit(OP_STORE_VAR, 0, id.text);
            }
        } else if (t.type == TOK_ID) {
            if (tokens[cur_tok].type == TOK_LPAREN) {
                cur_tok++;
                if (tokens[cur_tok].type != TOK_RPAREN) {
                    token_t arg = tokens[cur_tok++];
                    if (arg.type == TOK_ID) emit(OP_PUSH_VAR, 0, arg.text);
                    else if (arg.type == TOK_NUM) emit(OP_PUSH_NUM, arg.num, NULL);
                }
                if (tokens[cur_tok].type == TOK_RPAREN) cur_tok++;
                emit(OP_CALL, 0, t.text);
            }
        }
    }
    emit(OP_HALT, 0, NULL);
    
    global_vm.code = bytecode;
    global_vm.code_len = bc_ptr;
    global_vm.pc = 0;
    global_vm.sp = 0;
    
    print_serial("JS: PureJS VM starting...\n");
    vm_run();
}
