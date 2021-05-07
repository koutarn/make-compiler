#pragma onece

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
  int len;         //トークンの長さ
};

//抽象構文木のノードの種類
typedef enum {
  NODE_EQUAL,     //==
  NODE_NOT_EQUAL, //!=
  NODE_LT,        // <
  NODE_LE,        // <=
  NODE_ADD,       // +
  NODE_SUB,       // -
  NODE_MUL,       // *
  NODE_DIV,       // /
  NODE_NUM,       // 整数
} NODE_KIND;

//構文木のノード
typedef struct NODE NODE;
struct NODE {
  NODE_KIND kind; //ノードの種類
  NODE *lhs;      //左辺
  NODE *rhs;      //右辺
  int val;        //数値
};

// prototype
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
TOKEN *new_token(TOKEN_KIND kind, TOKEN *current, char *str, int len);
bool startswith(char *p, char *q);
TOKEN *tokenize(char *p);
NODE *new_node(NODE_KIND kind, NODE *lhs, NODE *rhs);
NODE *new_node_num(int val);
NODE *expr();
NODE *equality();
NODE *relational();
NODE *add();
NODE *mul();
NODE *unary();
NODE *primary();
void gen(NODE *node);
int main(int argc, char **argv);

// 現在のトークン
extern TOKEN *CurrentToken;

//入力プログラム
extern char *user_input;
