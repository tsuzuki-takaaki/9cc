#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ----------
// Tokenizer
// ----------

// Kinds of token as TokenKind
typedef enum {
  TK_RESERVED, // for operator
  TK_NUM, // for int
  TK_EOF, // for eof
} TokenKind;

// Type of token
typedef struct Token Token;
struct Token {
  TokenKind kind; // kind of token(TokenKind)
  Token *next; // next token
  int val; // val when token is TK_EOF
  char *str;
};

// current token as global
Token *token;

// input string sequence as global
char *user_input;

// error handling function
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  // pos = pointer headed to current pos - pointer headed to the beginning(user_input is string(array) and point the beginning)
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// if next token is expected, read next token end return true.
// else return false.
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// if next token is expected, read next token.
// else return panic
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "expected '%c'", op);
  token = token->next;
}

// if next token is int, read next token and return the integer
// else return panic
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  // read next token as global
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// create new token and link cur
// @cur parent tokne
// @str next token(like integer token after calcuation operator)
// @return Token
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  // parent token's next is this new token
  cur->next = tok;
  return tok;
}

// tokenize input argument p
// @return Token
Token *tokenize() {
  char *p = user_input;
  // create root token
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // when next character is white space, read next character(advance the pointer)
    if (isspace(*p)) {
      p++;
      continue;
    }

    // for Punctuator
    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    // ex: 5 and parent token is head
    // Token of head {
    // 	next: Token of 5,
    // }
    //
    // Token of 5 {
    // 	kind: TK_NUM,
    // 	next: not clear until next token is read
    // 	val: 5,
    // 	str: '5',
    // }
    // for Integer Literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

// ----------
// Parser
// ----------

// kinds of AST nodes
typedef enum {
  ND_ADD, // +
  ND_SUB, // - 
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // int
} NodeKind;

// Type of AST node
typedef struct Node Node;
struct Node {
  NodeKind kind; // node kind like ND_ADD
  Node *lhs; // left side Node
  Node *rhs; // right side Node
  int val; // when kind is ND_NUM, use this member
};

// initialize Node
Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

// For node that has lhs, rhs
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
} 

// For int ND_NUM
Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

// prototype
Node *expr();
Node *mul();
Node *primary();

// expr = mul ("+" mul | "-" mul)*
Node *expr() {
  Node *node = mul();
  
  for(;;) {
    // consume: when token matches with argument, advance token
    if (consume('+'))
      node = new_binary(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = primary ("*" primary | "/" primary)*
Node *mul() {
  Node *node = primary();

  for (;;) {
    if (consume('*'))
      node = new_binary(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_binary(ND_DIV, node, primary());
    else
      return node;
  }
}

// primary = num | "(" expr ")"
Node *primary() {
  // when token is (, next is maybe expr
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  return new_num(expect_number());
}

// ----------
// Code generator
// ----------

// receive AST and return asm output
void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  // create sequece of tokens
  user_input = argv[1];
  token = tokenize();
  // create AST
  Node *node = expr();
  
  // prepare of output assembly
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // generate asm code based on AST
  gen(node);

  // pop to finish
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

