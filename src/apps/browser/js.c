// js.c - MVJS Lexer, Parser, and Engine
#include "js.h"
#include "../../kernel/heap.h"
#include "../../kernel/string.h"
#include "dom.h"
#include "layout.h"

extern void print_serial(const char *);
extern void kernel_poll_events(void);
extern void dom_print_tree(dom_node_t *root, int depth);
extern dom_node_t *current_document;
extern render_node_t *current_render_tree;
extern int ui_dirty;

// Lexer State
static js_tok_t tokens[JS_MAX_TOKENS];
static int token_count = 0;
static int cur_tok = 0;

static int is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_digit(char c) { return (c >= '0' && c <= '9'); }

static void js_tokenize(const char *source) {
  token_count = 0;
  const char *p = source;
  int iters = 0;

  while (*p && token_count < JS_MAX_TOKENS - 1) {
    if (++iters > 100) {
      kernel_poll_events();
      iters = 0;
    }
    // Skip whitespace
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
      p++;
    if (!*p)
      break;

    js_tok_t *t = &tokens[token_count];
    t->type = JS_TOK_EOF;
    t->text[0] = '\0';
    t->number_value = 0;

    // Skip comments (//)
    if (*p == '/' && *(p + 1) == '/') {
      while (*p && *p != '\n')
        p++;
      continue;
    }

    if (is_alpha(*p)) {
      // Identifier or Keyword
      int i = 0;
      while (is_alpha(*p) || is_digit(*p)) {
        if (i < JS_MAX_STRING_LEN - 1)
          t->text[i++] = *p;
        p++;
      }
      t->text[i] = '\0';

      if (strcmp(t->text, "var") == 0 || strcmp(t->text, "let") == 0)
        t->type = JS_TOK_KEYWORD_VAR;
      else if (strcmp(t->text, "if") == 0)
        t->type = JS_TOK_KEYWORD_IF;
      else if (strcmp(t->text, "else") == 0)
        t->type = JS_TOK_KEYWORD_ELSE;
      else if (strcmp(t->text, "while") == 0)
        t->type = JS_TOK_KEYWORD_WHILE;
      else if (strcmp(t->text, "for") == 0)
        t->type = JS_TOK_KEYWORD_FOR;
      else if (strcmp(t->text, "function") == 0)
        t->type = JS_TOK_KEYWORD_FUNCTION;
      else if (strcmp(t->text, "return") == 0)
        t->type = JS_TOK_KEYWORD_RETURN;
      else
        t->type = JS_TOK_IDENTIFIER;
      token_count++;
    } else if (is_digit(*p)) {
      // Number
      int val = 0;
      while (is_digit(*p)) {
        val = val * 10 + (*p - '0');
        p++;
      }
      t->type = JS_TOK_NUMBER;
      t->number_value = val;
      token_count++;
    } else if (*p == '"' || *p == '\'') {
      // String literal
      char q = *p++;
      int i = 0;
      while (*p && *p != q) {
        if (i < JS_MAX_STRING_LEN - 1)
          t->text[i++] = *p;
        p++;
      }
      if (*p == q)
        p++;
      t->text[i] = '\0';
      t->type = JS_TOK_STRING;
      token_count++;
    } else {
      // Punctuation
      char c = *p++;
      t->text[0] = c;
      t->text[1] = '\0';

      if (c == '=') {
        if (*p == '=') {
          p++;
          t->type = JS_TOK_EQUALS;
          strncpy(t->text, "==", JS_MAX_STRING_LEN - 1);
        } else
          t->type = JS_TOK_ASSIGN;
      } else if (c == '!') {
        if (*p == '=') {
          p++;
          t->type = JS_TOK_NOT_EQUALS;
          strncpy(t->text, "!=", JS_MAX_STRING_LEN - 1);
        } else {
          token_count++;
          continue;
        } // Skip lone !
      } else if (c == '<') {
        if (*p == '=') {
          p++;
          t->type = JS_TOK_LESS_EQ;
          strncpy(t->text, "<=", JS_MAX_STRING_LEN - 1);
        } else
          t->type = JS_TOK_LESS;
      } else if (c == '>') {
        if (*p == '=') {
          p++;
          t->type = JS_TOK_GREATER_EQ;
          strncpy(t->text, ">=", JS_MAX_STRING_LEN - 1);
        } else
          t->type = JS_TOK_GREATER;
      } else if (c == '+') {
        if (*p == '+') {
          p++;
          t->type = JS_TOK_PLUS_PLUS;
          strncpy(t->text, "++", JS_MAX_STRING_LEN - 1);
        } else if (*p == '=') {
          p++;
          t->type = JS_TOK_PLUS_ASSIGN;
          strncpy(t->text, "+=", JS_MAX_STRING_LEN - 1);
        } else
          t->type = JS_TOK_PLUS;
      } else if (c == '-') {
        if (*p == '-') {
          p++;
          t->type = JS_TOK_MINUS_MINUS;
          strncpy(t->text, "--", JS_MAX_STRING_LEN - 1);
        } else if (*p == '=') {
          p++;
          t->type = JS_TOK_MINUS_ASSIGN;
          strncpy(t->text, "-=", JS_MAX_STRING_LEN - 1);
        } else
          t->type = JS_TOK_MINUS;
      } else if (c == '*')
        t->type = JS_TOK_MULTIPLY;
      else if (c == '/')
        t->type = JS_TOK_DIVIDE;
      else if (c == '(')
        t->type = JS_TOK_LPAREN;
      else if (c == ')')
        t->type = JS_TOK_RPAREN;
      else if (c == '{')
        t->type = JS_TOK_LBRACE;
      else if (c == '}')
        t->type = JS_TOK_RBRACE;
      else if (c == '[')
        t->type = JS_TOK_LBRACKET;
      else if (c == ']')
        t->type = JS_TOK_RBRACKET;
      else if (c == ';')
        t->type = JS_TOK_SEMICOLON;
      else if (c == ',')
        t->type = JS_TOK_COMMA;
      else if (c == '.')
        t->type = JS_TOK_DOT;

      token_count++;
    }
  }
  if (token_count < JS_MAX_TOKENS) {
    tokens[token_count].type = JS_TOK_EOF;
  }
}

// AST Parser Helpers
static js_node_t *create_node(js_node_type_t type) {
  js_node_t *node = (js_node_t *)kmalloc(sizeof(js_node_t));
  if (node) {
    memset(node, 0, sizeof(js_node_t));
    node->type = type;
  }
  return node;
}

static js_tok_t *peek() { return &tokens[cur_tok]; }
static js_tok_t *consume() { return &tokens[cur_tok++]; }

static js_node_t *parse_expression(void);

// Primary: number | string | id | (expr)
static js_node_t *parse_primary() {
  js_tok_t *t = peek();

  if (t->type == JS_TOK_NUMBER) {
    consume();
    js_node_t *node = create_node(JS_NODE_LITERAL_NUMBER);
    node->num_val = t->number_value;
    return node;
  }

  if (t->type == JS_TOK_STRING) {
    consume();
    js_node_t *node = create_node(JS_NODE_LITERAL_STRING);
    strncpy(node->str_val, t->text, JS_MAX_STRING_LEN - 1);
    return node;
  }

  if (t->type == JS_TOK_IDENTIFIER) {
    consume();
    js_node_t *node = create_node(JS_NODE_IDENTIFIER);
    strncpy(node->str_val, t->text, JS_MAX_STRING_LEN - 1);
    return node;
  }

  if (t->type == JS_TOK_LPAREN) {
    consume();
    js_node_t *expr = parse_expression();
    if (peek()->type == JS_TOK_RPAREN)
      consume();
    return expr;
  }

  return 0;
}

// Postfix: primary (.identifier | (args))*
static js_node_t *parse_postfix() {
  js_node_t *expr = parse_primary();

  while (expr) {
    js_tok_t *t = peek();
    if (t->type == JS_TOK_DOT) {
      consume();
      js_tok_t *id = consume(); // Expect IDENTIFIER
      js_node_t *dot = create_node(JS_NODE_DOT_ACCESS);
      dot->left = expr;
      js_node_t *prop = create_node(JS_NODE_IDENTIFIER);
      strncpy(prop->str_val, id->text, JS_MAX_STRING_LEN - 1);
      dot->right = prop;
      expr = dot;
    } else if (t->type == JS_TOK_LPAREN) {
      consume();
      js_node_t *call = create_node(JS_NODE_FUNCTION_CALL);
      call->left = expr; // Function to call

      // Parse args
      js_node_t *prev_arg = 0;
      while (peek()->type != JS_TOK_RPAREN && peek()->type != JS_TOK_EOF) {
        js_node_t *arg = parse_expression();
        if (arg) {
          if (!call->args)
            call->args = arg;
          else
            prev_arg->next = arg;
          prev_arg = arg;
        }
        if (peek()->type == JS_TOK_COMMA)
          consume();
      }
      if (peek()->type == JS_TOK_RPAREN)
        consume();
      expr = call;
    } else if (t->type == JS_TOK_PLUS_PLUS || t->type == JS_TOK_MINUS_MINUS) {
      // Postfix ++/--
      js_tok_type_t op = consume()->type;
      js_node_t *inc = create_node(JS_NODE_ASSIGNMENT);
      inc->left = expr;
      js_node_t *one = create_node(JS_NODE_LITERAL_NUMBER);
      one->num_val = 1;
      js_node_t *bin = create_node(JS_NODE_BINARY_OP);
      bin->op = (op == JS_TOK_PLUS_PLUS) ? JS_TOK_PLUS : JS_TOK_MINUS;
      bin->left = expr;
      bin->right = one;
      inc->right = bin;
      expr = inc;
    } else
      break;
  }
  return expr;
}

static js_node_t *parse_term() {
  js_node_t *left = parse_postfix();
  while (peek()->type == JS_TOK_MULTIPLY || peek()->type == JS_TOK_DIVIDE) {
    js_tok_type_t op = consume()->type;
    js_node_t *right = parse_postfix();
    js_node_t *bin = create_node(JS_NODE_BINARY_OP);
    bin->op = op;
    bin->left = left;
    bin->right = right;
    left = bin;
  }
  return left;
}

static js_node_t *parse_math() {
  js_node_t *left = parse_term();
  while (peek()->type == JS_TOK_PLUS || peek()->type == JS_TOK_MINUS) {
    js_tok_type_t op = consume()->type;
    js_node_t *right = parse_term();
    js_node_t *bin = create_node(JS_NODE_BINARY_OP);
    bin->op = op;
    bin->left = left;
    bin->right = right;
    left = bin;
  }
  return left;
}

static js_node_t *parse_expression() {
  js_node_t *left = parse_math();

  // Comparison operators
  js_tok_type_t ct = peek()->type;
  if (ct == JS_TOK_EQUALS || ct == JS_TOK_NOT_EQUALS || ct == JS_TOK_LESS ||
      ct == JS_TOK_GREATER || ct == JS_TOK_LESS_EQ || ct == JS_TOK_GREATER_EQ) {
    js_tok_type_t op = consume()->type;
    js_node_t *right = parse_math();
    js_node_t *bin = create_node(JS_NODE_BINARY_OP);
    bin->op = op;
    bin->left = left;
    bin->right = right;
    return bin;
  }

  if (peek()->type == JS_TOK_ASSIGN) {
    consume();
    js_node_t *right = parse_expression();
    js_node_t *assign = create_node(JS_NODE_ASSIGNMENT);
    assign->left = left;
    assign->right = right;
    return assign;
  }

  // += and -=
  if (peek()->type == JS_TOK_PLUS_ASSIGN ||
      peek()->type == JS_TOK_MINUS_ASSIGN) {
    js_tok_type_t op =
        (consume()->type == JS_TOK_PLUS_ASSIGN) ? JS_TOK_PLUS : JS_TOK_MINUS;
    js_node_t *rhs = parse_expression();
    js_node_t *bin = create_node(JS_NODE_BINARY_OP);
    bin->op = op;
    bin->left = left;
    bin->right = rhs;
    js_node_t *assign = create_node(JS_NODE_ASSIGNMENT);
    assign->left = left;
    assign->right = bin;
    return assign;
  }

  return left;
}

static js_node_t *parse_statement(void);

static js_node_t *parse_block() {
  consume(); // {
  js_node_t *blk = create_node(JS_NODE_BLOCK);
  js_node_t *last = 0;

  while (peek()->type != JS_TOK_RBRACE && peek()->type != JS_TOK_EOF) {
    int before = cur_tok;
    js_node_t *stmt = parse_statement();
    if (stmt) {
      if (!blk->left)
        blk->left = stmt;
      else
        last->next = stmt;
      last = stmt;
    } else if (cur_tok == before) {
      // parse_statement didn't advance - skip token to avoid infinite loop
      consume();
    }
  }
  if (peek()->type == JS_TOK_RBRACE)
    consume();
  return blk;
}

static js_node_t *parse_statement() {
  js_tok_t *t = peek();

  if (t->type == JS_TOK_KEYWORD_VAR) {
    consume();
    js_node_t *decl = create_node(JS_NODE_VAR_DECL);
    t = consume(); // Identifier
    strncpy(decl->str_val, t->text, JS_MAX_STRING_LEN - 1);
    if (peek()->type == JS_TOK_ASSIGN) {
      consume();
      decl->right = parse_expression();
    }
    if (peek()->type == JS_TOK_SEMICOLON)
      consume();
    return decl;
  }

  if (t->type == JS_TOK_KEYWORD_IF) {
    consume();
    js_node_t *if_node = create_node(JS_NODE_IF_STATEMENT);
    if (peek()->type == JS_TOK_LPAREN)
      consume();
    if_node->left = parse_expression();
    if (peek()->type == JS_TOK_RPAREN)
      consume();
    if (peek()->type == JS_TOK_LBRACE)
      if_node->right = parse_block();
    else {
      if_node->right = parse_statement();
    }
    if (peek()->type == JS_TOK_KEYWORD_ELSE) {
      consume();
      if (peek()->type == JS_TOK_LBRACE)
        if_node->alt = parse_block();
      else
        if_node->alt = parse_statement();
    }
    return if_node;
  }

  // for (init; cond; step) { body }
  if (t->type == JS_TOK_KEYWORD_FOR) {
    consume();
    js_node_t *for_node = create_node(JS_NODE_FOR_LOOP);
    if (peek()->type == JS_TOK_LPAREN)
      consume();
    // init
    for_node->left = parse_statement(); // consumes trailing ;
    // cond
    for_node->right = parse_expression();
    if (peek()->type == JS_TOK_SEMICOLON)
      consume();
    // step
    for_node->alt = parse_expression();
    if (peek()->type == JS_TOK_RPAREN)
      consume();
    // body
    if (peek()->type == JS_TOK_LBRACE)
      for_node->args = parse_block();
    else
      for_node->args = parse_statement();
    return for_node;
  }

  // function name() { body }
  if (t->type == JS_TOK_KEYWORD_FUNCTION) {
    consume();
    js_node_t *fn = create_node(JS_NODE_FUNC_DECL);
    if (peek()->type == JS_TOK_IDENTIFIER) {
      strncpy(fn->str_val, consume()->text, JS_MAX_STRING_LEN - 1);
    }
    if (peek()->type == JS_TOK_LPAREN) {
      consume();
      while (peek()->type != JS_TOK_RPAREN && peek()->type != JS_TOK_EOF)
        consume(); // skip params for now
      if (peek()->type == JS_TOK_RPAREN)
        consume();
    }
    if (peek()->type == JS_TOK_LBRACE)
      fn->right = parse_block();
    return fn;
  }

  if (t->type == JS_TOK_LBRACE) {
    return parse_block();
  }

  // Default expression stmt
  int before = cur_tok;
  js_node_t *expr = parse_expression();
  if (peek()->type == JS_TOK_SEMICOLON)
    consume();
  // Safety: if we didn't advance at all and got null, consume one token
  if (!expr && cur_tok == before) {
    consume();
  }
  return expr;
}

js_node_t *js_parse(const char *source) {
  print_serial("JS: Tokenizing...\n");
  js_tokenize(source);
  cur_tok = 0;

  print_serial("JS: Parsing...\n");
  js_node_t *prog = create_node(JS_NODE_PROGRAM);
  js_node_t *last = 0;

  while (peek()->type != JS_TOK_EOF) {
    kernel_poll_events();
    int before = cur_tok;
    js_node_t *stmt = parse_statement();
    if (stmt) {
      if (!prog->left)
        prog->left = stmt;
      else
        last->next = stmt;
      last = stmt;
    } else if (cur_tok == before) {
      // Unparseable, skip token to prevent infinite loop
      consume();
    }
  }
  return prog;
}

void js_free_ast(js_node_t *root) {
  if (!root)
    return;
  js_free_ast(root->left);
  js_free_ast(root->right);
  js_free_ast(root->args);
  js_free_ast(root->alt);
  js_free_ast(root->next);
  kfree(root);
}

// Engine Execution Context
static js_context_t global_ctx;

void js_init(void) {
  global_ctx.var_count = 0;
  print_serial("JS: Engine Initialized.\n");
}

static js_var_t *js_find_var(const char *name) {
  for (int i = 0; i < global_ctx.var_count; i++) {
    if (strcmp(global_ctx.vars[i].name, name) == 0)
      return &global_ctx.vars[i];
  }
  return 0;
}

static js_var_t *js_create_var(const char *name) {
  js_var_t *v = js_find_var(name);
  if (v)
    return v;
  if (global_ctx.var_count < JS_MAX_VARS) {
    v = &global_ctx.vars[global_ctx.var_count++];
    strncpy(v->name, name, JS_MAX_VAR_NAME_LEN - 1);
    v->name[JS_MAX_VAR_NAME_LEN - 1] = 0;
    v->type = JS_VAL_UNDEFINED;
    return v;
  }
  return 0;
}

// Recursive dom search by ID
static dom_node_t *find_dom_by_id(dom_node_t *curr, const char *id) {
  if (!curr)
    return 0;

  static int find_iters = 0;
  if (++find_iters > 50) {
    kernel_poll_events();
    find_iters = 0;
  }

  if (curr->type == DOM_NODE_ELEMENT) {
    const char *my_id = dom_get_attribute(curr, "id");
    if (my_id && strcmp(my_id, id) == 0)
      return curr;
  }
  dom_node_t *child = curr->first_child;
  while (child) {
    dom_node_t *f = find_dom_by_id(child, id);
    if (f)
      return f;
    child = child->next_sibling;
  }
  return 0;
}

// Global runtime execution eval
static void js_eval(js_node_t *node, js_var_t *ret_val) {
  if (!node)
    return;

  if (node->type == JS_NODE_PROGRAM || node->type == JS_NODE_BLOCK) {
    js_node_t *stmt = node->left;
    while (stmt) {
      kernel_poll_events();
      js_eval(stmt, ret_val);
      stmt = stmt->next;
    }
    return;
  }

  if (node->type == JS_NODE_VAR_DECL) {
    js_var_t *v = js_create_var(node->str_val);
    if (v && node->right) {
      js_var_t rh = {0};
      js_eval(node->right, &rh);
      v->type = rh.type;
      v->num_val = rh.num_val;
      strncpy(v->str_val, rh.str_val, JS_MAX_STRING_LEN - 1);
      v->obj_ref = rh.obj_ref;
    }
    return;
  }

  if (node->type == JS_NODE_LITERAL_NUMBER) {
    ret_val->type = JS_VAL_NUMBER;
    ret_val->num_val = node->num_val;
    return;
  }

  if (node->type == JS_NODE_LITERAL_STRING) {
    ret_val->type = JS_VAL_STRING;
    strncpy(ret_val->str_val, node->str_val, JS_MAX_STRING_LEN - 1);
    return;
  }

  if (node->type == JS_NODE_IDENTIFIER) {
    if (strcmp(node->str_val, "document") == 0) {
      ret_val->type = JS_VAL_OBJECT_REF;
      ret_val->obj_ref = current_document;
    } else if (strcmp(node->str_val, "alert") == 0) {
      ret_val->type = JS_VAL_NATIVE_FUNC;
      strncpy(ret_val->str_val, "js_alert", JS_MAX_STRING_LEN - 1);
    } else {
      js_var_t *v = js_find_var(node->str_val);
      if (v) {
        *ret_val = *v;
      } else {
        ret_val->type = JS_VAL_UNDEFINED;
      }
    }
    return;
  }

  if (node->type == JS_NODE_BINARY_OP) {
    js_var_t lh = {0}, rh = {0};
    js_eval(node->left, &lh);
    js_eval(node->right, &rh);

    // String concatenation with +
    if (node->op == JS_TOK_PLUS &&
        (lh.type == JS_VAL_STRING || rh.type == JS_VAL_STRING)) {
      ret_val->type = JS_VAL_STRING;
      ret_val->str_val[0] = 0;
      if (lh.type == JS_VAL_STRING) {
        strncpy(ret_val->str_val, lh.str_val, JS_MAX_STRING_LEN - 1);
      } else {
        char nb[16];
        int ni = 0, nv = lh.num_val;
        if (nv == 0) {
          nb[ni++] = '0';
        } else {
          if (nv < 0) {
            nb[ni++] = '-';
            nv = -nv;
          }
          char t2[16];
          int j = 0;
          while (nv > 0 && j < 15) {
            t2[j++] = '0' + (nv % 10);
            nv /= 10;
          }
          while (j > 0)
            nb[ni++] = t2[--j];
        }
        nb[ni] = 0;
        strncpy(ret_val->str_val, nb, JS_MAX_STRING_LEN - 1);
      }
      ret_val->str_val[JS_MAX_STRING_LEN - 1] = 0;

      char next_part[JS_MAX_STRING_LEN];
      next_part[0] = 0;
      if (rh.type == JS_VAL_STRING) {
        strncpy(next_part, rh.str_val, JS_MAX_STRING_LEN - 1);
      } else {
        char nb[16];
        int ni = 0, nv = rh.num_val;
        if (nv == 0) {
          nb[ni++] = '0';
        } else {
          if (nv < 0) {
            nb[ni++] = '-';
            nv = -nv;
          }
          char t2[16];
          int j = 0;
          while (nv > 0 && j < 15) {
            t2[j++] = '0' + (nv % 10);
            nv /= 10;
          }
          while (j > 0)
            nb[ni++] = t2[--j];
        }
        nb[ni] = 0;
        strncpy(next_part, nb, JS_MAX_STRING_LEN - 1);
      }
      next_part[JS_MAX_STRING_LEN - 1] = 0;

      int current_len = strlen(ret_val->str_val);
      int space = JS_MAX_STRING_LEN - 1 - current_len;
      if (space > 0) {
        strncat(ret_val->str_val, next_part, space);
      }
      return;
    }

    // Numeric operations
    ret_val->type = JS_VAL_NUMBER;
    int a = lh.num_val, b = rh.num_val;
    switch (node->op) {
    case JS_TOK_PLUS:
      ret_val->num_val = a + b;
      break;
    case JS_TOK_MINUS:
      ret_val->num_val = a - b;
      break;
    case JS_TOK_MULTIPLY:
      ret_val->num_val = a * b;
      break;
    case JS_TOK_DIVIDE:
      ret_val->num_val = (b != 0) ? a / b : 0;
      break;
    case JS_TOK_EQUALS:
      ret_val->num_val = (a == b) ? 1 : 0;
      break;
    case JS_TOK_NOT_EQUALS:
      ret_val->num_val = (a != b) ? 1 : 0;
      break;
    case JS_TOK_LESS:
      ret_val->num_val = (a < b) ? 1 : 0;
      break;
    case JS_TOK_GREATER:
      ret_val->num_val = (a > b) ? 1 : 0;
      break;
    case JS_TOK_LESS_EQ:
      ret_val->num_val = (a <= b) ? 1 : 0;
      break;
    case JS_TOK_GREATER_EQ:
      ret_val->num_val = (a >= b) ? 1 : 0;
      break;
    default:
      ret_val->num_val = 0;
      break;
    }
    return;
  }

  if (node->type == JS_NODE_DOT_ACCESS) {
    js_var_t lh = {0};
    js_eval(node->left, &lh);

    if (lh.type == JS_VAL_OBJECT_REF && lh.obj_ref == current_document) {
      if (node->right && strcmp(node->right->str_val, "getElementById") == 0) {
        ret_val->type = JS_VAL_NATIVE_FUNC;
        strncpy(ret_val->str_val, "doc_getById", JS_MAX_STRING_LEN - 1);
        return;
      }
    }

    // console.log binding
    if (node->left->type == JS_NODE_IDENTIFIER &&
        strcmp(node->left->str_val, "console") == 0 && node->right &&
        strcmp(node->right->str_val, "log") == 0) {
      ret_val->type = JS_VAL_NATIVE_FUNC;
      strncpy(ret_val->str_val, "console_log", JS_MAX_STRING_LEN - 1);
      return;
    }

    // Element dot access? (e.g. el.textContent)
    if (lh.type == JS_VAL_OBJECT_REF && lh.obj_ref != current_document) {
      dom_node_t *el = (dom_node_t *)lh.obj_ref;
      if (node->right && strcmp(node->right->str_val, "textContent") == 0) {
        // Return text node child if exists
        ret_val->type = JS_VAL_STRING;
        ret_val->str_val[0] = 0;
        dom_node_t *child = el->first_child;
        while (child) {
          if (child->type == DOM_NODE_TEXT && child->text_content) {
            strncpy(ret_val->str_val, child->text_content,
                    JS_MAX_STRING_LEN - 1);
            break;
          }
          child = child->next_sibling;
        }
        return;
      }
    }
    return;
  }

  if (node->type == JS_NODE_FUNCTION_CALL) {
    js_var_t func = {0};
    js_eval(node->left, &func);

    if (func.type == JS_VAL_NATIVE_FUNC) {
      if (strcmp(func.str_val, "doc_getById") == 0) {
        if (node->args) {
          js_var_t arg1 = {0};
          js_eval(node->args, &arg1);
          if (arg1.type == JS_VAL_STRING) {
            dom_node_t *f = find_dom_by_id(current_document, arg1.str_val);
            if (f) {
              ret_val->type = JS_VAL_OBJECT_REF;
              ret_val->obj_ref = f;
            } else {
              ret_val->type = JS_VAL_UNDEFINED;
            }
          }
        }
      } else if (strcmp(func.str_val, "console_log") == 0) {
        if (node->args) {
          js_var_t arg1 = {0};
          js_eval(node->args, &arg1);
          print_serial("JS console.log: ");
          if (arg1.type == JS_VAL_STRING)
            print_serial(arg1.str_val);
          else if (arg1.type == JS_VAL_NUMBER) {
            char nbuf[16];
            int ni = 0, nv = arg1.num_val;
            if (nv < 0) {
              nbuf[ni++] = '-';
              nv = -nv;
            }
            if (nv == 0)
              nbuf[ni++] = '0';
            else {
              char tmp[16];
              int j = 0;
              while (nv > 0) {
                tmp[j++] = '0' + (nv % 10);
                nv /= 10;
              }
              while (j > 0)
                nbuf[ni++] = tmp[--j];
            }
            nbuf[ni] = 0;
            print_serial(nbuf);
          }
          print_serial("\n");
        }
      } else if (strcmp(func.str_val, "js_alert") == 0) {
        if (node->args) {
          js_var_t arg1 = {0};
          js_eval(node->args, &arg1);
          if (arg1.type == JS_VAL_STRING) {
            extern char status_text[];
            strncpy(status_text, "Alert: ", 127);
            strncat(status_text, arg1.str_val, 100);
            ui_dirty = 1;
          }
        }
      }
    }
    return;
  }

  // FOR loop: left=init, right=cond, alt=step, args=body
  if (node->type == JS_NODE_FOR_LOOP) {
    if (node->left) {
      js_var_t d = {0};
      js_eval(node->left, &d);
    }
    int iters = 0;
    while (iters < 100) { // Safety limit
      kernel_poll_events();
      if (node->right) {
        js_var_t cond = {0};
        js_eval(node->right, &cond);
        if (cond.type == JS_VAL_NUMBER && cond.num_val == 0)
          break;
      }
      if (node->args) {
        js_var_t d = {0};
        js_eval(node->args, &d);
      }
      if (node->alt) {
        js_var_t d = {0};
        js_eval(node->alt, &d);
      }
      iters++;
    }
    return;
  }

  // FUNC_DECL: skip function declarations gracefully
  if (node->type == JS_NODE_FUNC_DECL) {
    return;
  }

  if (node->type == JS_NODE_ASSIGNMENT) {
    js_var_t rh = {0};
    js_eval(node->right, &rh);

    if (node->left->type == JS_NODE_IDENTIFIER) {
      js_var_t *v = js_find_var(node->left->str_val);
      if (!v)
        v = js_create_var(node->left->str_val);
      if (v)
        *v = rh;
    } else if (node->left->type == JS_NODE_DOT_ACCESS) {
      js_var_t obj = {0};
      js_eval(node->left->left, &obj);
      if (obj.type == JS_VAL_OBJECT_REF && obj.obj_ref != 0 &&
          obj.obj_ref != current_document) {
        dom_node_t *el = (dom_node_t *)obj.obj_ref;
        if (node->left->right &&
            strcmp(node->left->right->str_val, "textContent") == 0) {
          dom_node_t *child = el->first_child;
          while (child) {
            if (child->type == DOM_NODE_TEXT) {
              kfree(child->text_content);
              int slen = strlen(rh.str_val);
              char *nstr = (char *)kmalloc(slen + 1);
              strncpy(nstr, rh.str_val, JS_MAX_STRING_LEN - 1);
              child->text_content = nstr;
              ui_dirty = 1;
              return;
            }
            child = child->next_sibling;
          }
          dom_node_t *tnode = (dom_node_t *)kmalloc(sizeof(dom_node_t));
          memset(tnode, 0, sizeof(dom_node_t));
          tnode->type = DOM_NODE_TEXT;
          char *nstr = (char *)kmalloc(strlen(rh.str_val) + 1);
          strncpy(nstr, rh.str_val, JS_MAX_STRING_LEN - 1);
          nstr[strlen(rh.str_val)] = 0; // Ensure termination
          tnode->text_content = nstr;
          dom_append_child(el, tnode);
          ui_dirty = 1;
        }
      }
    }
    return;
  }
}

void js_execute(const char *source) {
  if (!source || source[0] == '\0')
    return;

  // Safety: skip scripts that are too complex for our tiny engine
  int src_len = strlen(source);
  if (src_len > 16384) {
    print_serial("JS: Script too large, skipping (");
    char buf[16];
    int i = 0, v = src_len;
    if (v == 0) {
      buf[i++] = '0';
    } else {
      char tmp[16];
      int j = 0;
      while (v > 0) {
        tmp[j++] = '0' + (v % 10);
        v /= 10;
      }
      while (j > 0)
        buf[i++] = tmp[--j];
    }
    buf[i] = 0;
    print_serial(buf);
    print_serial(" bytes)\n");
    return;
  }

  js_node_t *ast = js_parse(source);
  if (ast) {
    print_serial("JS: Executing script...\n");
    js_var_t dump_ret = {0};
    js_eval(ast, &dump_ret);
    js_free_ast(ast);

    // After JS runs, invalidate render tree so layout recalculates
    __asm__ volatile("cli");
    if (current_render_tree) {
      layout_free_tree(current_render_tree);
      current_render_tree = 0;
    }
    __asm__ volatile("sti");
  }
}
