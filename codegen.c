//コードジェネレーター
#include "9cc.h"

static char *argreg64[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9",}; //引数を代入するレジスタの配列
int label_seq = 0;
void gen_lval(Node *node){
  if(node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");
  printf("mov rax, rbp\n");
  printf("sub rax, %d\n",node->offset);
  printf("push rax\n");
}

void gen(Node *node){
  if(node->kind == ND_NUM){
    printf("push %d\n",node->val);
    return;
  }
  switch(node->kind){
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return; 
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_IF:
      int seq_1 = label_seq++;
      gen(node->cond);
      printf("	pop rax\n");
		  printf("	cmp rax, 0\n");

		  if(node->els){
			  printf("	je .Lelse%d\n",seq_1);
			  gen(node->then);
			  printf("	jmp .Lend%d\n",seq_1);
			  printf(".Lelse%d:\n",seq_1);
			  gen(node->els);
			  printf(".Lend%d:\n",seq_1);
		  }else{
		    printf("	je .Lend%d\n",seq_1);
			  gen(node->then);
			  printf(".Lend%d:\n",seq_1);
		  }
		  return;
	  case ND_WHILE:
		  int seq_2 = label_seq++;
		  printf(".Lbegin%d:\n",seq_2);
		  gen(node->cond);
		  printf("	pop rax\n");
		  printf("	cmp rax, 0\n");
		  printf("	je .Lend%d\n",seq_2);
		  gen(node->then);
		  printf("	jmp .Lbegin%d\n",seq_2);				printf(".Lend%d:\n",seq_2);
		  return;
	  case ND_FOR:
		  int seq_3 = label_seq++;
		  gen(node->for_ident);
		  printf(".Lbegin%d:\n",seq_3);
		  gen(node->for_cond);
      printf("	pop rax\n");
		  printf("	cmp rax, 0\n");
		  printf("	je .Lend%d\n",seq_3);
		  gen(node->then);
		  gen(node->for_else);
		  printf("	jmp .Lbegin%d\n",seq_3);
		  printf(".Lend%d:\n",seq_3);
		  return;
	  case ND_BLOCK:
		  for(int i = 0; node->code_stm[i]; i++){
			  gen(node->code_stm[i]);
			  printf("	pop rax\n");
		  }
		  return;
    case ND_ADDR: //単項&
      gen_lval(node->lhs);
      return;
    case ND_DEREF: //単項*
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rax,[rax]\n");
      printf("  push rax\n");
      return;
  }
  gen(node->lhs);
  gen(node->rhs);

  printf("pop rdi\n");
  printf("pop rax\n");

  switch(node->kind){
    case ND_ADD:
      printf("add rax, rdi\n");
      break;
    case ND_SUB:
      printf("sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("cqo\n");
      printf("idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("push rax\n");
}
void gen_function(Node *node){
  printf(".globl %.*s\n",node->function_name_len,node->function_name);
  printf("%.*s:\n",node->function_name_len,node->function_name);
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for(int i = 0; i < node->argc; i++){
    printf("  mov [rbp-%d], %s\n",node->args[i]->offset,argreg64[i]);
  }

  gen(node->function_body);

  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}