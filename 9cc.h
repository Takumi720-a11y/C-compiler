//ヘッダファイル
//通常、ヘッダファイルには関数の宣言だけを書きます。
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    //識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;         //トークンの長さ
};

//ノードの種類
typedef enum{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_ASSIGN, // =
  ND_LVAR, // ローカル変数
  ND_RETURN, // return
  ND_IF, // if else
  ND_WHILE, // while
  ND_FOR, // for
  ND_BLOCK, // block
  ND_FUNCTION, //関数
  ND_ADDR,  //アドレスを整数をして返す
  ND_DEREF,  //アドレスから値を呼んでくる
} NodeKind;

//ノードの型
struct Node{
    NodeKind kind; //ノードの型
    Node *lhs;     //左辺
    Node *rhs;     //右辺
    Node *cond;    //条件式
    Node *then;    //条件が真の時
    Node *els;     //条件が偽の時
    Node *for_ident;  //for文の条件式で使う変数
    Node *for_cond;   //for文の条件式
    Node *for_else;   //for文の条件式が満たされなかったときの式
    Node **code_stm;   //blockに含まれる式
    Node *function_body;  //関数の中身
    LVar *args[6];  //引数
    char *function_name;    //関数の名前
    int function_name_len; //関数の名前の長さ
    int val;       //kindがND_NUMの場合のみ使う
    int offset;    //kindがND_LVARの場合のみ使う
    int argc;      //引数の数
};

//ローカル変数の型
struct LVar{
  LVar *next;   //次の変数 or NULL
  char *name;  //変数の名前
  int len;      //名前の長さ
  int offset;   //RBPからのオフセット
};



//複数ファイルで共有されている変数
extern char *user_input;
extern Token *token;
extern Node *code_prog[100];
extern LVar *locals;



//複数ファイルで共有されている関数
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize(void);
Token *consume_ident();
void program(void);
void gen_function(Node *node);