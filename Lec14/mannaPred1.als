-- https://alloy.readthedocs.io/en/latest/language/predicates-and-functions.html

some sig       U {} -- our little universe

sig S1 extends U {} -- helps define unary predicate p1
sig S2 extends U {} -- helps define unary predicate q1

one sig a extends U {} -- define a constant within U

pred p1[x: U] { x in S1 } -- arbitrary unary predicates such as "p1" can be defined this way
pred q1[x: U] { x in S2 } -- q1 defined



assert A1   {  ( (all x : U | p1[x] ) => (some x : U | p1[x]) ) }
assert nA1  { !( (all x : U | p1[x]) => (some x : U | p1[x]) ) }

assert B1   {  ( (some x : U | p1[x] ) => (all x : U | p1[x]) ) }
assert nB1  { !( (some x : U | p1[x]) => (all x : U | p1[x]) ) }

assert A2   {  (all x : U | p1[x] => p1[x]) }
assert nA2   {  !(all x : U | p1[x] => p1[x]) }

assert B2 { (all x : U | p1[x] => p1[a]) }

check A1 for 8 U 
check nA1 

check B1 
check nB1

check A2 for 8 U
check nA2 

check B2

 
