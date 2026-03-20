// js.h - Minimum Viable JavaScript (MVJS) Engine for PureBrowser
#ifndef JS_H
#define JS_H

#define JS_MAX_TOKENS 256
#define JS_MAX_NODES 256
#define JS_MAX_VAR_NAME_LEN 16
#define JS_MAX_STRING_LEN 64

// Internal Token Types
typedef enum {
  JS_TOK_EOF = 0,
  JS_TOK_IDENTIFIER,
  JS_TOK_NUMBER,
  JS_TOK_STRING,
  JS_TOK_KEYWORD_VAR,
  JS_TOK_KEYWORD_IF,
  JS_TOK_KEYWORD_ELSE,
  JS_TOK_KEYWORD_WHILE,
  JS_TOK_KEYWORD_FOR,
  JS_TOK_KEYWORD_FUNCTION,
  JS_TOK_KEYWORD_RETURN,
  JS_TOK_ASSIGN,       // =
  JS_TOK_PLUS,         // +
  JS_TOK_MINUS,        // -
  JS_TOK_MULTIPLY,     // *
  JS_TOK_DIVIDE,       // /
  JS_TOK_EQUALS,       // ==
  JS_TOK_NOT_EQUALS,   // !=
  JS_TOK_LESS,         // <
  JS_TOK_GREATER,      // >
  JS_TOK_LESS_EQ,      // <=
  JS_TOK_GREATER_EQ,   // >=
  JS_TOK_PLUS_ASSIGN,  // +=
  JS_TOK_MINUS_ASSIGN, // -=
  JS_TOK_PLUS_PLUS,    // ++
  JS_TOK_MINUS_MINUS,  // --
  JS_TOK_LPAREN,       // (
  JS_TOK_RPAREN,       // )
  JS_TOK_LBRACE,       // {
  JS_TOK_RBRACE,       // }
  JS_TOK_LBRACKET,     // [
  JS_TOK_RBRACKET,     // ]
  JS_TOK_COMMA,        // ,
  JS_TOK_SEMICOLON,    // ;
  JS_TOK_DOT           // .
} js_tok_type_t;

// Token struct
typedef struct {
  js_tok_type_t type;
  char text[JS_MAX_STRING_LEN];
  int number_value;
} js_tok_t;

// AST Node Types
typedef enum {
  JS_NODE_PROGRAM = 0,
  JS_NODE_BLOCK,
  JS_NODE_VAR_DECL,
  JS_NODE_ASSIGNMENT,
  JS_NODE_BINARY_OP,
  JS_NODE_LITERAL_NUMBER,
  JS_NODE_LITERAL_STRING,
  JS_NODE_IDENTIFIER,
  JS_NODE_DOT_ACCESS,
  JS_NODE_FUNCTION_CALL,
  JS_NODE_IF_STATEMENT,
  JS_NODE_FOR_LOOP,
  JS_NODE_FUNC_DECL
} js_node_type_t;

// AST Node
typedef struct js_node {
  js_node_type_t type;
  js_tok_type_t op; // For binary ops
  char str_val[JS_MAX_STRING_LEN];
  int num_val;

  struct js_node *left;  // LHS of assignment/binop, or object in dot access, or
                         // condition in IF
  struct js_node *right; // RHS of assignment/binop, or property in dot access,
                         // or then_branch in IF
  struct js_node *args;  // Linked list of arguments for func calls
  struct js_node *next;  // For lists (blocks, args)
  struct js_node *alt;   // else_branch in IF
} js_node_t;

// Runtime Types
typedef enum {
  JS_VAL_UNDEFINED = 0,
  JS_VAL_NUMBER,
  JS_VAL_STRING,
  JS_VAL_OBJECT_REF, // Simplified reference to a DOM obj for now
  JS_VAL_NATIVE_FUNC // Like document.getElementById
} js_val_type_t;

// Variable struct
typedef struct {
  char name[JS_MAX_VAR_NAME_LEN];
  js_val_type_t type;
  int num_val;
  char str_val[JS_MAX_STRING_LEN];
  void *obj_ref;
} js_var_t;

#define JS_MAX_VARS 16

// Execution Context
typedef struct {
  js_var_t vars[JS_MAX_VARS];
  int var_count;
} js_context_t;

// API
void js_init(void);
js_node_t *js_parse(const char *source);
void js_execute(const char *source);
void js_free_ast(js_node_t *root);

#endif // JS_H
