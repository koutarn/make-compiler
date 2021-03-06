#include "9cc.h"

void gen_lval(NODE *node) {
  if (node->kind != NODE_LVAR) {
    printf("代入の左辺値が変数ではありません");
    return;
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(NODE *node) {

  switch (node->kind) {
  case NODE_NUM:
    printf("  push %d\n", node->val);
    break;
  case NODE_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax,[rax]\n");
    printf("  push rax\n");
  case NODE_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax],rdi\n");
    printf("  push rdi\n");
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case NODE_ADD:
    printf("  add rax, rdi\n");
    break;
  case NODE_SUB:
    printf("  sub rax, rdi\n");
    break;
  case NODE_MUL:
    printf("  imul rax, rdi\n");
    break;
  case NODE_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case NODE_EQUAL:
    printf("  cmp rax,rdi\n");
    printf("  sete al\n");
    printf("  movzb rax,al\n");
    break;
  case NODE_NOT_EQUAL:
    printf("  cmp rax,rdi\n");
    printf("  setne al\n");
    printf("  movzb rax,al\n");
    break;
  case NODE_LT:
    printf("  cmp rax,rdi\n");
    printf("  setl al\n");
    printf("  movzb rax,al\n");
    break;
  case NODE_LE:
    printf("  cmp rax,rdi\n");
    printf("  setle al\n");
    printf("  movzb rax,al\n");
    break;
  }

  printf("  push rax\n");
}
