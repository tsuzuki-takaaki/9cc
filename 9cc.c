#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Kinds of token as TokenKind
typedef enum {
  TK_RESERVED, // for operator
  TK_NUM, // for int
  TK_EOF, // for eof
} TokenKind;

typedef struct Token Token;

// Type of token struct
struct Token {
  TokenKind kind; // kind of token(TokenKind)
  Token *next; // next token
  int val; // val when token is TK_EOF
  char *str;
};

// current token as global
Token *token;

// error handling function
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// if next tokne is expected, read next token end return true.
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
    error("Not '%c'", op);
  token = token->next;
}

// if next token is int, read next token and return the integer
// else return panic
int expect_number() {
  if (token->kind != TK_NUM)
    error("Not integer");
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
Token *tokenize(char *p) {
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

    if (*p == '+' || *p == '-') {
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
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error("Can not tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid argument count\n");
    return 1;
  }

  // create sequece of tokens
  token = tokenize(argv[1]);
  
  // prepare of output assembly
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // first token is integer, so check the integer and create mov instruction
  printf("  mov rax, %d\n", expect_number());

  // read tokens until eof token
  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}

