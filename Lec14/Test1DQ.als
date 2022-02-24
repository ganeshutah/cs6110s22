some P { l : set P }

sig  pat  in P {} 
sig  doc  in P {} 
sig  quk  in P {}

pred p[x: P] { x in pat } -- patient predicate denotes unary reln pat
pred d[x: P] { x in doc } -- doctor predicate  denotes unary reln doc
pred q[x: P] { x in quk } -- quack predicate

-- some patients like every doctor : state as a fact
pred P1 { some x : pat | all y : doc | x->y in l }

pred P2 { some x : P | p[x] and (all y : P | d[y] => x->y in l) }

assert Main { P1 iff P2 }

check Main for 8
-- NO COUNTEREXAMPLES - SO EQUIV!
-- BUT we may prefer the P2 form, as we have a tangible unary predicate we can query/debug



