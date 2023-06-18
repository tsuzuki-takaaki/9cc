#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  // create sequece of tokens
  // user_input and token is global
  user_input = argv[1];
  token = tokenize();
  // create AST
  Node *node = expr();
  // emit assembly from AST
  codegen(node);
  return 0;
}

