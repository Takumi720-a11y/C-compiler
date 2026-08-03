//パーサ
#include "9cc.h"

char *user_input;
Token *token;
Node *code[100];

// 現在着目しているトークン

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str , op, token->len))
    error_at(token->str,"'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str,"数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

bool startswith(char *p, char *q){
  return memcmp(p,q,strlen(q)) == 0;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }
    
    if(startswith(p,"==") || startswith(p,"!=") ||
       startswith(p,"<=") || startswith(p,">=")){
      cur = new_token(TK_RESERVED, cur, p,2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '>' || *p == '<' || *p == '=' || *p == ';') {
      cur = new_token(TK_RESERVED, cur, p,1);
      p += 1;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p,0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p-q;
      continue;
    }

    if('a' <= *p && *p <= 'z'){
      cur = new_token(TK_IDENT,cur,p,1);
      cur->len = 1;
      p += 1;
      continue;
    }

    error_at(p,"数ではありません");
  }

  new_token(TK_EOF, cur, p,0);
  return head.next;
}

Token *consume_ident(){
    if(token->kind != TK_IDENT)
        return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
}

Node *new_node(NodeKind kind,Node *lhs, Node *rhs){
  Node *node = calloc(1, sizeof(Node));
  node -> kind = kind;
  node -> lhs = lhs;
  node -> rhs = rhs;
  return node;
}

Node *new_node_num(int val){
  Node *node = calloc(1, sizeof(Node));
  node -> kind = ND_NUM;
  node -> val = val;
  return node;
}
     
Node *stmt(void);         // = expr";"
Node *expr(void);         // = assign
Node *assign(void);       // = equality("=" assign)?
Node *equalty(void);      // = relational("==" relational | "!=" relational)*
Node *relational(void);   // = add("<" add | "<=" add | ">" add | ">=" add)*
Node *add(void);          // = mul("+" mul | "-" mul)*
Node *mul(void);          // = unary("*" unary | "/" unary)*
Node *unary(void);        // = ("+" | "-")? primary
Node *primary(void);      // = num | ident | "("expr")"

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}// = stmt*

Node *stmt(void){
  Node *node = expr();
  expect(";");
  return node;
}

Node *expr(void){
  return assign();
}

Node *assign(void){
  Node *node = equalty();
  if(consume("="))
    node = new_node(ND_ASSIGN,node,assign());
  return node;
}

Node *equalty(void){
  Node *node = relational();

  for(;;){
    if(consume("=="))
      node = new_node(ND_EQ,node,relational());
    else if(consume("!="))
      node = new_node(ND_NE,node,relational());
  }
  return node;
}

Node *relational(void){
  Node *node = add();

  for(;;){
    if(consume("<"))
      node = new_node(ND_LT,node,add());
    else if(consume("<="))
      node = new_node(ND_LE,node,add());
    else if(consume(">"))
      node = new_node(ND_LT,add(),node);
    else if(consume(">="))
      node = new_node(ND_LE,add(),node);
    else
      return node;
  }
}

Node *add(void){
  Node *node = mul();

  for(;;){
    if(consume("+"))
      node = new_node(ND_ADD,node,mul());
    else if(consume("-"))
      node = new_node(ND_SUB,node,mul());
    else  
      return node;
  }
}

Node *mul(void){
  Node *node = unary();

  for(;;){
    if(consume("*"))
      node = new_node(ND_MUL,node,unary());
    else if(consume("/"))
      node = new_node(ND_DIV,node,unary());
    else
      return node;
  }
}

Node *unary(void){
  if(consume("+"))
    return primary();
  if(consume("-"))
    return new_node(ND_SUB,new_node_num(0),primary());
  return primary();
}

Node *primary(void){
  if(consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }
  Token *tok = consume_ident();
  if(tok){
    Node *node = calloc(1,sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) *8;
    return node;
  }
  return new_node_num(expect_number()); 
}

