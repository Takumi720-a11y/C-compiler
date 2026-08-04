//トークナイザ

#include "9cc.h"

Token *token;
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
    error_at(token->str,"'%s'ではありません", op);
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

int is_alnum(char c){
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

Token *consume_ident(){
    if(token->kind != TK_IDENT)
        return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
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


    if(('a' <= *p && *p <= 'z') ||
       ('A' <= *p && *p <= 'Z') ||
       (*p == '_')){
      char *q = p;
      while(is_alnum(*p))
        p++;
      cur = new_token(TK_IDENT,cur,q,p-q);
      continue;
    }

    error_at(p,"数ではありません");
  }

  new_token(TK_EOF, cur, p,0);
  return head.next;
}

