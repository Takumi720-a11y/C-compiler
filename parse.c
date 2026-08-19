//パーサ
#include "9cc.h"

Node *code_prog[100];
LVar *locals;
char *user_input;
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



Node *new_node(NodeKind kind,Node *lhs, Node *rhs){
  Node *node = calloc(1, sizeof(Node));
  node -> kind = kind;
  node -> lhs = lhs;
  node -> rhs = rhs;
  return node;
}

Node *new_node_num(int val){
  Node *node = calloc(1, sizeof(Node));
  Type *ty = calloc(1,sizeof(Type));
  node -> kind = ND_NUM;
  node -> val = val;
  ty->ty = INT;
  node->ty = ty;
  return node;
}

Node *new_node_addr(Node *lhs){
  Node *node = new_node(ND_ADDR, lhs, NULL);
  Type *ty = calloc(1,sizeof(Type));
  ty->ty = PTR;
  ty->ptr_to = lhs->ty;
  node->ty = ty;
  return node;
}

LVar *find_lvar(Token *tok){
  for(LVar *var = locals; var; var = var->next)
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

Type *basetype(){
  expect("int");
  Type *ty = calloc(1,sizeof(Type));
  ty->ty = INT;
  while(consume("*")){
    Type *ptr = calloc(1,sizeof(Type));
    ptr->ty = PTR;
    ptr->ptr_to = ty;
    ty = ptr;
  }
  return ty;
}



Node *function(void);     // = "int" ident "(" params? ")" stmt
Node *stmt(void);         // = "int" ident ";" | expr";" | "return"expr";" | "if" "(" expr ")" stmt ("else" stmt)?
Node *expr(void);         // = assign
Node *assign(void);       // = equality("=" assign)?
Node *equalty(void);      // = relational("==" relational | "!=" relational)*
Node *relational(void);   // = add("<" add | "<=" add | ">" add | ">=" add)*
Node *add(void);          // = mul("+" mul | "-" mul)*
Node *mul(void);          // = unary("*" unary | "/" unary)*
Node *unary(void);        // = ("+" | "-")? primary | "*" unary | "&" unary
Node *primary(void);      // = num | ident | "("expr")"

void program() {
  int i = 0;
  while (!at_eof())
    code_prog[i++] = function();
  code_prog[i] = NULL;
}// = function*

Node *function(){
  locals = NULL;
  Node *node;
  node = calloc(1,sizeof(Node));
  node->kind = ND_FUNCTION;
  Type *ty = basetype();
  Token *tok = consume_ident();
  if (!tok)
    error_at(token->str, "関数名ではありません");
  node->function_name = tok->str;
  node->function_name_len = tok->len;
  expect("(");
  int argc = 0;
  if(!consume(")")){
    for(;;){
      Type *ty = basetype();
      Token *param_tok = consume_ident();
      if (!param_tok)
        error_at(token->str, "引数名ではありません");
      if (argc >= 6)
        error_at(param_tok->str, "引数は最大6個です");
      if (find_lvar(param_tok))
        error_at(param_tok->str, "同じ引数名が重複しています");

      LVar *lvar = calloc(1,sizeof(LVar));
      lvar->ty = ty;
      lvar->name = param_tok->str;
      lvar->len = param_tok->len;
      lvar->next = locals;
      if(locals)
        lvar->offset = locals->offset + 8;
      else
        lvar->offset = 8;
      locals = lvar;
      node->args[argc++] = locals; //args[]の要素一つ一つにLVarが代入されている
      if(consume(")"))
        break;
      expect(",");
    }
  }
  
  node->argc = argc;
  node->function_body = stmt();
  if (node->function_body->kind != ND_BLOCK)
    error("関数本体はブロックである必要があります");
  return node;
}

Node *stmt(void){
  Node *node;
  if(consume("{")){
    node = calloc(1,sizeof(Node));
    node->kind = ND_BLOCK;
    node->code_stm = calloc(100,sizeof(Node *));
    int i = 0;
    while(!consume("}")){
      if(at_eof())
        error_at(token->str,"'}'がありません");
      node->code_stm[i++] = stmt();
    }
    node->code_stm[i] = NULL;
    return node;
  }else if(consume("int")){
    Type *ty = calloc(1,sizeof(Type));
    ty->ty = INT;
    while(consume("*")){
      Type *ptr = calloc(1,sizeof(Type));
      ptr->ty = PTR;
      ptr->ptr_to = ty;
      ty = ptr; 
    }
    Token *tok = consume_ident();
    if(!tok)
      error_at(token->str,"変数名ではありません");
    if(find_lvar(tok))
      error_at(tok->str,"変数が重複して定義されています");
    LVar *lvar = calloc(1,sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->ty = ty;
    if(locals)
      lvar->offset = locals->offset + 8;
    else
      lvar->offset = 8;
    locals = lvar;
    expect(";");
    node = calloc(1,sizeof(Node));
    node->kind = ND_VAR_DEF;
    return node;
  }else if(consume("return")){
		node = calloc(1,sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
	}else if(consume("if")){
    node = calloc(1,sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if(consume("else")){
      node->els = stmt();
    }
    return node;
  }else if(consume("while")){
    node = calloc(1,sizeof(Node));
    node->kind = ND_WHILE;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }else if(consume("for")){
    node = calloc(1,sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if(!consume(";")){
      node->for_ident = expr();
      expect(";");
    }
    if(!consume(";")){
      node->for_cond = expr();
      expect(";");
    }
    if(!consume(")")){
      node->for_else = expr();
      expect(")");
    }
    node->then = stmt();
    return node;
  }else{
		node = expr();
	}
	if(!consume(";"))
		error_at(token->str,"';'ではないトークンです");
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
    else
      return node;
  }
 
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
    if(consume("+")){
      Node *rhs = mul();
      if(node->ty->ty == PTR)
        rhs = new_node(ND_MUL,rhs,new_node_num(4));
      node = new_node(ND_ADD,node,rhs);
      continue;
    } 
    if(consume("-")){
      Node *rhs = mul();
      if(node->ty->ty == PTR)
        rhs = new_node(ND_MUL,rhs,new_node_num(4));
      node = new_node(ND_SUB,node,rhs);
      continue;
    }
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
  Node *node;
  if(consume("+"))
    return primary();
  else if(consume("-"))
    return new_node(ND_SUB,new_node_num(0),primary());
  else if(consume("*")){
    Node *lhs = unary();
    Node *node = new_node(ND_DEREF,lhs,NULL);
    if(!lhs->ty || lhs->ty->ty != PTR)
      error("ポインタではない値をデリファレンスしています");
    node->ty = lhs->ty->ptr_to;
    return node;
  }else if(consume("&")){
    return new_node_addr(unary());
  }
  return primary();
}

Node *primary(void) {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  Token *tok = consume_ident();
  if (tok) {
    LVar *lvar = find_lvar(tok);
    if(!lvar)
      error_at(tok->str, "未定義の変数です");
    Node *node = calloc(1,sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = lvar->offset;
    node->ty = lvar->ty;
    return node;
  }

  return new_node_num(expect_number());
}