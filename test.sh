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
assert 14 '2+3*4;'
assert 12 '(5+3)*2-4;'
assert 1 '3+4==7;'
assert 0 '5>10;'
assert 3 'a=3;'
assert 5 'b=5;'
assert 3 'a=3;'
assert 8 'a=3;b=5;a+b;'
assert 8 'abc = 8;'

echo OK