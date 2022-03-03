--https://alloy.readthedocs.io/en/latest/language/predicates-and-functions.html
one sig a,b extends U { } -- constants 'a' in U

 
 
some sig U {
 p1 : set U, -- p1 is an arbitrary unary predicate
 q1 : set U, -- q1 is an arbitrary unary predicate
 r0  : lone U, -- r0 is for the simulation of a zero-ary pred
 r1  : lone U, -- r1 is for .. another zero-ary pred
} -- our universe  

pred R0 { some a.r0 }
pred R1 { some a.r1 }
assert r0a { R0 => (R1 => R0) }
check r0a

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

