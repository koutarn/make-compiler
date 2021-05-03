#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TOKEN_RESERVED, //記号
  TOKEN_NUM,      //整数トークン
  TOKEN_EOF,      //入力の終り
} TOKEN_KIND;

typedef struct TOKEN TOKEN;
struct TOKEN {
  TOKEN_KIND kind; //トークンの型
  TOKEN *next;     //次の入力トークン
  int val;         // TOKEN_NUMの場合の数値
  char *str;       //トークンの文字列
};

// 現在のトークン
TOKEN *CurrentToken;

//入力プログラム
char *user_input;

//エラー報告用
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

//次のトークンが期待している記号の時には、トークンを1つ読み進めて
//真を返す。それ以外の場合は偽を返す
bool consume(char op) {
  if (CurrentToken->kind != TOKEN_RESERVED || CurrentToken->str[0] != op) {
    return false;
  }
  CurrentToken = CurrentToken->next;
  return true;
}

//次のトークンが期待している記号の時には、トークンを1つ読み進めて
//それ以外の場合はエラーを報告する
void expect(char op) {
  if (CurrentToken->kind != TOKEN_RESERVED || CurrentToken->str[0] != op) {
    error_at(CurrentToken->str, "'%c'ではありません", op);
  }
  CurrentToken = CurrentToken->next;
}

// 次のトークンが数値の場合、トークンを一つ読み進めてその数値を返す
//それ以外の場合はエラーを報告する
int expect_number() {
  if (CurrentToken->kind != TOKEN_NUM) {
    error_at(CurrentToken->str, "数ではありません");
  }
  int val = CurrentToken->val;
  CurrentToken = CurrentToken->next;
  return val;
}

bool at_eof() { return CurrentToken->kind == TOKEN_EOF; }

// 新しいトークンを作成して現在のトークンと繋げる
TOKEN *new_token(TOKEN_KIND kind, TOKEN *current, char *str) {
  TOKEN *token = calloc(1, sizeof(TOKEN));
  token->kind = kind;
  token->str = str;
  current->next = token;
  return token;
}

//入力文字列pをトークナイズしてそれを返す。
TOKEN *tokenize(char *p) {
  TOKEN head;
  head.next = NULL;
  TOKEN *current = &head;

  while (*p) {
    //空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      current = new_token(TOKEN_RESERVED, current, p++);
      continue;
    }

    if (isdigit(*p)) {
      current = new_token(TOKEN_NUM, current, p);
      current->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TOKEN_EOF, current, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  //入力を保持する
  user_input = argv[1];

  //トークナイズする
  CurrentToken = tokenize(argv[1]);

  //アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  //式の最初は数値なはずなのでそれをチェックして最初のmov命令を出力
  printf("  mov rax, %d\n", expect_number());

  //+<数値>あるいは-<数値>というトークンの並びを消費しつつアセンブリを出力
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
