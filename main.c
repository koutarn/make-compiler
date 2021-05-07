#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  //トークナイズしてパースする
  user_input = argv[1];
  CurrentToken = tokenize(argv[1]);
  NODE *node = expr();

  //アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  //抽象構文木を下りながらコードを生成
  gen(node);

  //スタックトップに式全体の値が残っているはずなので
  //それをRAXにロードして関数の戻り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
