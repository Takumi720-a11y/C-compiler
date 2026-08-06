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
  node -> kind = ND_NUM;
  node -> val = val;
  return node;
}

LVar *find_lvar(Token *tok){
  for(LVar *var = locals; var; var = var->next)
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

Node *function(void);     // = ident "(" ident*? ")" stmt
Node *stmt(void);         // = expr";" | "return"expr";" | "if" "(" expr ")" stmt ("else" stmt)?
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
  Token *tok = consume_ident();
  if (!tok)
    error_at(token->str, "関数名ではありません");
  node->function_name = tok->str;
  node->function_name_len = tok->len;
  expect("(");
  int argc = 0;
  if(!consume(")")){
    for(;;){
      Token *param_tok = consume_ident();
      if (!param_tok)
        error_at(token->str, "引数名ではありません");
      if (argc >= 6)
        error_at(param_tok->str, "引数は最大6個です");
      if (find_lvar(param_tok))
        error_at(param_tok->str, "同じ引数名が重複しています");

      LVar *lvar = calloc(1,sizeof(LVar));
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
  Node *node;
  if(consume("+"))
    return primary();
  else if(consume("-"))
    return new_node(ND_SUB,new_node_num(0),primary());
  else if(consume("*")){
    return new_node(ND_DEREF,unary(),NULL);
  }else if(consume("&")){
    return new_node(ND_ADDR,unary(),NULL);
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
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if(lvar){
      node->offset = lvar->offset;
    }else{
      lvar = calloc(1,sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if(locals)
        lvar->offset = locals->offset + 8;
      else  
        lvar->offset = 8;
      node->offset = lvar->offset;
      locals = lvar;
    }
    
    return node;
  }

  return new_node_num(expect_number());
}

