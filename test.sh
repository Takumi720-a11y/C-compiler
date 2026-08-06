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
assert 14 'main(){2+3*4;}'
assert 12 'main(){(5+3)*2-4;}'
assert 1 'main(){3+4==7;}'
assert 0 'main(){5>10;}'
assert 3 'main(){a=3;}'
assert 5 'main(){b=5;}'
assert 3 'main(){a=3;}'
assert 8 'main(){a=3;b=5;a+b;}'
assert 8 'main(){abc = 8;}'
assert 5 'main(){return 5;}'
assert 67 'main(){a = 15;b = 4;return 7+a*b;}'
assert 3 'main(){a = 1;if(a) return 3;}'
assert 4 'main(){a = 0;if(a) return 6; else return 4;}'
assert 5 'main(){a = 1;while(a == 1)return 5;} '
assert 3 'main(){a=0;while(a<3)a=a+1;a;}'
assert 10 'main(){a=0;for(i = 0; i < 10;i = i + 1)a = a + 1; a;}'
assert 3 'main(){a=1;b=2;return a+b;}'
assert 3 'main(){if(1){a=1;b=2;return a+b;}}'
assert 5 'main(){return 5;}'
echo OK