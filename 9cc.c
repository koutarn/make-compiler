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

//抽象構文木のノードの種類
typedef enum {
  NODE_ADD, // +
  NODE_SUB, // -
  NODE_MUL, // *
  NODE_DIV, // /
  NODE_NUM, // 整数
} NODE_KIND;

//構文木のノード
typedef struct NODE NODE;
struct NODE {
  NODE_KIND kind; //ノードの種類
  NODE *lhs;      //左辺
  NODE *rhs;      //右辺
  int val;        //数値
};

// 現在のトークン
TOKEN *CurrentToken;

//入力プログラム
char *user_input;

// prototype
void error_at(char *loc, char *fmt, ...);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
TOKEN *new_token(TOKEN_KIND kind, TOKEN *current, char *str);
TOKEN *tokenize(char *p);
NODE *new_node(NODE_KIND kind, NODE *lhs, NODE *rhs);
NODE *new_node_num(int val);
NODE *primary();
NODE *mul();
NODE *unary();
NODE *expr();
void gen(NODE *node);
int main(int argc, char **argv);

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

    if (strchr("+-*/()", *p)) {
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

//ノードの新規作成(左辺と右辺を受け取る
NODE *new_node(NODE_KIND kind, NODE *lhs, NODE *rhs) {
  NODE *node = calloc(1, sizeof(NODE));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

//ノードの新規作成(数値)
NODE *new_node_num(int val) {
  NODE *node = calloc(1, sizeof(NODE));
  node->kind = NODE_NUM;
  node->val = val;
  return node;
}

//==========================
//再帰下降構文解析用関数郡
//==========================
NODE *expr() {
  NODE *node = mul();

  for (;;) {
    if (consume('+')) {
      node = new_node(NODE_ADD, node, mul());
    } else if (consume('-')) {
      node = new_node(NODE_SUB, node, mul());
    } else {
      return node;
    }
  }
}

NODE *mul() {
  NODE *node = unary();
  for (;;) {
    if (consume('*')) {
      node = new_node(NODE_MUL, node, unary());
    } else if (consume('/')) {
      node = new_node(NODE_DIV, node, unary());
    } else {
      return node;
    }
  }
}

NODE *unary() {
  if (consume('+')) {
    return primary();
  }
  if (consume('-')) {
    return new_node(NODE_SUB, new_node_num(0), primary());
  }

  return primary();
}

NODE *primary() {
  //次のトークンが(なら( expr )なはず
  if (consume('(')) {
    NODE *node = expr();
    expect(')');
    return node;
  }

  //そうでなければ数値なはず
  return new_node_num(expect_number());
}

void gen(NODE *node) {
  if (node->kind == NODE_NUM) {
    printf("  push %d\n", node->val);
    return;
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
  case NODE_NUM:
    break;
  }

  printf(" push rax\n");
}

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
