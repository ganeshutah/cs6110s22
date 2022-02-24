
some sig  P          -- our little universe of people
     { l : set P }   -- the "likes" binary relation
     
sig  pat  in P {}    -- patients unary relation
sig  doc  in P {}    -- doctor unary relation ; could overlap with pat
sig  quk  in P {}    -- think of quacks as any subset of people


pred p[x: P] { x in pat } -- patient predicate denotes unary reln pat
pred d[x: P] { x in doc } -- doctor predicate  denotes unary reln doc
pred q[x: P] { x in quk } -- quack predicate

-- some patients like every doctor
fact { some x : P | p[x] and (all y : P | d[y] => x->y in l) }

-- no patient likes a quack
fact { all x, y : P | (p[x] and q[y]) => x->y not in l }

-- check that no doctor is a quack
assert docthm { all x : P | d[x] => !q[x] }
assert Ndocthm{!(all x : P | d[x] => !q[x])}

check docthm for 8
check Ndocthm for 8


