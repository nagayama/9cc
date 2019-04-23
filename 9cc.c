#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256,  //整数トークン
  TK_EOF,        // 入力の終わりを表わすトークン
};

typedef struct {
  int ty;       // トークンの型
  int val;      // tyがTK_NUMの場合の数値
  char *input;  // トークン文字列(エラー用)
} Token;

typedef struct Node {
  int ty;            // 演算子かND_NUM
  struct Node *lhs;  //左辺
  struct Node *rhs;  // 右辺
  int val;           // tyがND_NUMの時の数値
} Node;

// トークナイズした結果のトークンはこの配列に保存
// 上限は100まで
Token tokens[100];
int pos = 0;

Node *add();
Node *mul();
Node *term();

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // 空白文字
    if (isspace(*p)) {
      p++;
      continue;
    }
    // + か -
    if (*p == '+' || *p == '-') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }
    // 数値
    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    error("トークナイズできません： %s", p);
    exit(1);
  }
  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = TK_NUM;
  node->val = val;
  return node;
}

int consume(int ty) {
  if (tokens[pos].ty != ty) return 0;
  pos++;
  return 1;
}

Node *mul() {
  Node *node = term();
  for (;;) {
    if (consume('*'))
      node = new_node('+', node, term());
    else if (consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *term() {
  if (consume('(')) {
    Node *node = add();
    if (!consume(')'))
      error("開きカッコに対応する閉じカッコがありません: %s",
            tokens[pos].input);
    return node;
  }
  // 数値の場合はposを移動してnew_node_numを呼ぶ
  if (tokens[pos].ty == TK_NUM) return new_node_num(tokens[pos++].val);
  error("数値でも開きカッコでもないトークンです: %s", tokens[pos].input);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  if (tokens[0].ty != TK_NUM)
    error("最初の項が数字ではありません: %s", tokens[0].input);

  printf("  mov rax, %d\n", tokens[0].val);

  int i = 1;
  while (tokens[i].ty != TK_EOF) {
    if (tokens[i].ty == '+') {
      i++;
      if (tokens[i].ty != TK_NUM)
        error("予期しないトークンです: %s", tokens[i].input);
      printf("  add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }
    if (tokens[i].ty == '-') {
      i++;
      if (tokens[i].ty != TK_NUM)
        error("予期しないトークンです: %s", tokens[i].input);
      printf("  sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }
    error("予期しないトークンです: %s", tokens[i].input);
  }

  printf("  ret\n");
  return 0;
}
