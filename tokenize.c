#include "9cc.h"

// input string sequence as global
char *user_input;
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

// For reading token sequence and advance(this function is not for structuring token sequence)
// if next token is expected, read next token end return true.
// else return false.
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// For reading token sequence and advance(this function is not for structuring token sequence)
// if next token is expected, read next token.
// else return panic
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    error_at(token->str, "expected \"%s\"", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  // parent token's next is this new token
  cur->next = tok;
  return tok;
}

// compare p and q by strlen's string from beginning
bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

// tokenize input argument p
// @return Token
Token *tokenize() {
  char *p = user_input;
  // create root token
  Token head;
  head.next = NULL;
  Token *cur = &head;

  // check each strings by advancing p's pointer
  while (*p) {
    // when next character is white space, read next character(advance the pointer)
    if (isspace(*p)) {
      p++;
      continue;
    }

    // for Multi-letter Punctuator
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // for Single-letter Punctuator
    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
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
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }
    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

