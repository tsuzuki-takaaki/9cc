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
  int len;
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

extern char *user_input;
extern Token *token;

// ----------
// Parser
// ----------

// kinds of AST nodes
typedef enum {
  ND_ADD, // +
  ND_SUB, // - 
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
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

Node *expr();

// ----------
// Code generator
// ----------

void codegen(Node *node);

