//main関数
#include "9cc.h"



int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s:引数の個数が正しくありません",argv[0]);
    return 1;
  }

  //文字列全体を保存
  user_input = argv[1];
  // トークナイズする
  token = tokenize();
  program();
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  //先頭の式から淳にコード生成
  for(int i = 0; code_prog[i];i++){
    gen_function(code_prog[i]);
    //式の評価結果としてスタックに一つの値が残っている
    //そのためスタックが溢れないようにポップしておく
    
  }
  return 0;
}