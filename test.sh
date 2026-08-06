#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 67 'main(){a = 15;b = 4;return 7+a*b;}'
assert 3 'main(){a = 1;if(a) return 3;}'
assert 4 'main(){a = 0;if(a) return 6; else return 4;}'
assert 5 'main(){a = 1;while(a == 1)return 5;} '
assert 3 'main(){a=0;while(a<3)a=a+1;a;}'
assert 10 'main(){a=0;for(i = 0; i < 10;i = i + 1)a = a + 1; a;}'
assert 3 'main(){a=1;b=2;return a+b;}'
assert 3 'main(){if(1){a=1;b=2;return a+b;}}'
assert 5 'main(){return 5;}'
assert 105 'main(a,b){a=20;b=5;c=5;return c+a*b;}'
echo OK