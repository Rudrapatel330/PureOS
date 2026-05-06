// js.h - PureJS Stack-Based Virtual Machine for PureBrowser
#ifndef JS_H
#define JS_H

#include <stdint.h>
#include <stddef.h>

#define JS_MAX_TOKENS 8192
#define JS_MAX_NODES 8192
#define JS_MAX_VAR_NAME_LEN 64
#define JS_MAX_STRING_LEN 512
#define JS_STACK_SIZE 1024
#define JS_MAX_VARS 512

// JS Value Types
typedef enum {
    JS_VAL_UNDEFINED = 0,
    JS_VAL_NULL,
    JS_VAL_NUMBER,
    JS_VAL_STRING,
    JS_VAL_BOOLEAN,
    JS_VAL_OBJECT_REF,
    JS_VAL_NATIVE_FUNC
} js_val_type_t;

// JS Variable/Value structure
typedef struct {
    char name[JS_MAX_VAR_NAME_LEN];
    js_val_type_t type;
    int num_val;
    char str_val[JS_MAX_STRING_LEN];
    void *obj_ref;
} js_var_t;

// Opcodes for the Virtual Machine
typedef enum {
    OP_PUSH_NUM = 0,
    OP_PUSH_STR,
    OP_PUSH_VAR,
    OP_STORE_VAR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_CALL,
    OP_JMP,
    OP_JMP_IF_FALSE,
    OP_RET,
    OP_POP,
    OP_HALT
} js_opcode_t;

// Bytecode Instruction
typedef struct {
    js_opcode_t op;
    int arg_num;
    char arg_str[JS_MAX_VAR_NAME_LEN];
} js_inst_t;

// Execution Context (The VM State)
typedef struct {
    js_var_t stack[JS_STACK_SIZE];
    int sp; // Stack Pointer
    
    js_var_t vars[JS_MAX_VARS];
    int var_count;
    
    js_inst_t *code;
    int pc; // Program Counter
    int code_len;
} js_vm_t;

// API
void js_init(void);
void js_execute(const char *source);

#endif
