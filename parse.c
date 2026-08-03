//パーサ
#include "9cc.h"

Node *code[100];
LVar *locals;

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。

LVar *find_lvar(Token *tok){
	for(LVar *var = locals; var; var = var->next){
		if(var->len == tok->len && !memcmp(tok->str,var->name,var->len))
			return var;
	}
	return NULL;
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
     
Node *stmt(void);         // = expr";" | "return"expr";"
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
  Node *node;
  if(consume("return")){
		node = calloc(1,sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
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

