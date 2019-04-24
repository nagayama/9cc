#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  ND_NUM = 256,
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

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

// トークナイズした結果のトークンはこの配列に保存
// 上限は100まで
// Token tokens[100];

// トークナイズした結果のトークンはこの可変ベクタに保存
// 挿入時にはvec_pushを使う
Vector *tokens;

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

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
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
    if (*p == '(' || *p == ')' || *p == '+' || *p == '-' || *p == '*' ||
        *p == '/') {
      Token *t = malloc(sizeof(Token));
      t->ty = *p;
      t->input = p;
      vec_push(tokens, t);
      i++;
      p++;
      continue;
    }
    // 数値
    if (isdigit(*p)) {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_NUM;
      t->input = p;
      t->val = strtol(p, &p, 10);
      vec_push(tokens, t);
      i++;
      continue;
    }

    error("トークナイズできません： %s", p);
    exit(1);
  }
  Token *t = malloc(sizeof(Token));
  t->ty = TK_EOF;
  t->input = p;
  vec_push(tokens, t);
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
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int consume(int ty) {
  Token *t = tokens->data[pos];
  if (t->ty != ty) return 0;
  pos++;
  return 1;
}

Node *mul() {
  Node *node = term();
  for (;;) {
    if (consume('*'))
      node = new_node('*', node, term());
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
    if (!consume(')')) error("開きカッコに対応する閉じカッコがありません");
    return node;
  }
  // 数値の場合はposを移動してnew_node_numを呼ぶ
  Token *t = tokens->data[pos];
  if (t->ty == TK_NUM) {
    Token *t2 = tokens->data[pos++];
    return new_node_num(t2->val);
  }
  error("数値でも開きカッコでもないトークンです: %s", t->input);
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
    case '+':
      printf("  add rax, rdi\n");
      break;
    case '-':
      printf("  sub rax, rdi\n");
      break;
    case '*':
      printf("  mul rdi\n");
      break;
    case '/':
      printf("  mov rdx, 0\n");
      printf("  div rdi\n");
      break;
  }
  printf("  push rax\n");
}

int expect(int line, int expected, int actual) {
  if (expected == actual) return 0;
  fprintf(stderr, "%d: %d exptected, but got %d\n", line, expected, actual);
  exit(1);
}

void runtest() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);
  for (int i = 0; i < 100; i++) vec_push(vec, (void *)(long)i);

  expect(__LINE__, 100, (long)vec->len);
  expect(__LINE__, 0, (long)vec->data[0]);
  expect(__LINE__, 50, (long)vec->data[50]);
  expect(__LINE__, 99, (long)vec->data[99]);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  if (strcmp(argv[1], "-test") == 0) {
    runtest();
    return 0;
  }

  tokens = new_vector();

  // トークナイズしてパースする
  tokenize(argv[1]);
  Node *node = add();

  // アセンブリの前半部分
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード静止
  gen(node);

  // スタックトップに式全体の値が残っているので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
