#include "9cc.h"

// 現在のトークン
TOKEN *CurrentToken;

//入力プログラム
char *user_input;

//ノード
NODE *code[100];

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
bool consume(char *op) {
  bool is_not_reserved = CurrentToken->kind != TOKEN_RESERVED;
  bool is_not_comp_len = strlen(op) != CurrentToken->len;
  if (is_not_reserved || is_not_comp_len ||
      memcmp(CurrentToken->str, op, CurrentToken->len)) {
    return false;
  }
  CurrentToken = CurrentToken->next;
  return true;
}

TOKEN *consume_ident() {
  if (CurrentToken->kind != TOKEN_IDENT) {
    return NULL;
  }
  TOKEN *token = CurrentToken;
  CurrentToken = CurrentToken->next;
  return token;
}

//次のトークンが期待している記号の時には、トークンを1つ読み進めて
//それ以外の場合はエラーを報告する
void expect(char *op) {
  bool is_not_reserved = CurrentToken->kind != TOKEN_RESERVED;
  bool is_not_comp_len = strlen(op) != CurrentToken->len;
  if (is_not_reserved || is_not_comp_len ||
      memcmp(CurrentToken->str, op, CurrentToken->len)) {
    error_at(CurrentToken->str, "'%s'ではありません", op);
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
TOKEN *new_token(TOKEN_KIND kind, TOKEN *current, char *str, int len) {
  TOKEN *token = calloc(1, sizeof(TOKEN));
  token->kind = kind;
  token->str = str;
  token->len = len;
  current->next = token;
  return token;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

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

    if ('a' <= *p && 'z' >= *p) {
      current = new_token(TOKEN_IDENT, current, p++, 1);
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
      current = new_token(TOKEN_RESERVED, current, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>;", *p)) {
      current = new_token(TOKEN_RESERVED, current, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      current = new_token(TOKEN_NUM, current, p, 0);
      char *q = p;
      current->val = strtol(p, &p, 10);
      current->len = p - q;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TOKEN_EOF, current, p, 0);
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
void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

NODE *stmt() {
  NODE *node = expr();
  expect(";");
  return node;
}

NODE *expr() { return assign(); }

NODE *assign() {
  NODE *node = equality();
  if (consume("=")) {
    node = new_node(NODE_ASSIGN, node, assign());
  }
  return node;
}

NODE *equality() {
  NODE *node = relational();
  for (;;) {
    if (consume("==")) {
      node = new_node(NODE_EQUAL, node, relational());
    } else if (consume("!=")) {
      node = new_node(NODE_NOT_EQUAL, node, relational());
    } else {
      return node;
    }
  }
}

NODE *relational() {
  NODE *node = add();
  for (;;) {
    if (consume("<")) {
      node = new_node(NODE_LT, node, add());
    } else if (consume("<=")) {
      node = new_node(NODE_LE, node, add());
    } else if (consume(">")) {
      node = new_node(NODE_LT, add(), node);
    } else if (consume(">=")) {
      node = new_node(NODE_LE, add(), node);
    } else {
      return node;
    }
  }
}

NODE *add() {
  NODE *node = mul();
  for (;;) {
    if (consume("+")) {
      node = new_node(NODE_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(NODE_SUB, node, mul());
    } else {
      return node;
    }
  }
}

NODE *mul() {
  NODE *node = unary();
  for (;;) {
    if (consume("*")) {
      node = new_node(NODE_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(NODE_DIV, node, unary());
    } else {
      return node;
    }
  }
}

NODE *unary() {
  if (consume("+")) {
    return primary();
  }
  if (consume("-")) {
    return new_node(NODE_SUB, new_node_num(0), primary());
  }

  return primary();
}

NODE *primary() {
  //次のトークンが(なら( expr )なはず
  if (consume("(")) {
    NODE *node = expr();
    expect(")");
    return node;
  }

  TOKEN *token = consume_ident();
  if (token) {
    NODE *node = calloc(1, sizeof(NODE));
    node->kind = NODE_LVAR;
    node->offset = (token->str[0] - 'a' + 1) * 8;
    return node;
  }

  //そうでなければ数値なはず
  return new_node_num(expect_number());
}
