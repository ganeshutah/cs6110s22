--https://alloy.readthedocs.io/en/latest/language/predicates-and-functions.html
one sig a,b extends U { } -- constants 'a' in U
some sig U {
 r0  : lone U, -- r0 is for the simulation of a zero-ary pred
 r1  : lone U, -- r1 is for .. another zero-ary pred
 pr1 : set U,   -- helps define pred p1 as an arbitrary predicate
 qr1 : set U,   -- helps define pred q1 as an arbitrary predicate
} -- our universe  

pred R0 { some a.r0 } -- bool var R0
pred R1 { some a.r1 } -- bool var R1

pred p1[x:U] { x in a.pr1 } -- one-ary predicate p1
pred q1[x:U] { x in a.qr1 } -- one-ary predicate p1

assert r0a { R0 => (R1 => R0) } -- check Boolean identities
assert contr1 { R1 => !R1 } -- contra

assert AE   {  ( (all x : U | p1[x] ) => (some x : U | p1[x]) ) } -- valid
assert nAE  {  !( (all x : U | p1[x] ) => (some x : U | p1[x]) ) } -- contra assr

assert EA   {  ( (some x : U | p1[x] ) => (all x : U | p1[x]) ) } -- invalid assr
assert nEA  {  !( (some x : U | p1[x] ) => (all x : U | p1[x]) ) } -- !invalid assr

check r0a
check contr1
check AE  -- no cex for valid - hence valid
check nAE -- cex  for contra - hence not valid
check EA  -- cex for invalid - hence not valid
check nEA -- cex -- sat of EA


//assert r0true { some a.r0 }
//check r0true

/*
fact {
 no x : U | (x->X1s in p1  or X1s->x in p1)
 no y : U | (y->X1s in q1   or X1s->y in q1)
}
*/

/*
run { some p1
        some q1
       #U = 3 }
*/

