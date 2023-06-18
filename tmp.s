.intel_syntax noprefix
.globl main
main:
  push 0
  push 0
  push 10
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  pop rax
  ret
