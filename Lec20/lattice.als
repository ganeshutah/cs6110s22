-- L is a lattice
-- 1.  A lattice is a set
-- 1.1 With an operator "le" that is a partial order
-- 1.2 And operators glb and lub
--
-- Notice that I do bake in the theorem that for l1,l2,
-- glb and lub UNIQUELY take them to l3. This theorem
-- is immediately proven as soon as you run and find an
-- instance that admits glb and lub.
--
-- Another way to prove might have been to declare
-- glb, lub : L -> set L and prove that the set always
-- has size 1.
--
sig L { le       :  set L,      -- l1 can go to many ls
        glb, lub :  L -> one L  -- l1,l2 go uniquely to l3
      }
-- 1.1.1 le is reflexive
fact {  L <: iden in le }
-- 1.1.2 le is antisymmetric
fact { all a, b : L | a->b in le and b->a in le => a = b }
-- 1.1.3 le is transitive
fact { all a, b, c : L | a->b in le and b->c in le => a->c in le }

-- 1.2 And operators glb and lub satisfy their respective properties
-- 1.2.1 lub is the least upper-bound: it is lower than any other UB
--       An lub must exist for L to be a lattice - so signature fact
--       introduce a function "ub" to return a set of "ub"s
-- glb exists pairwise; trying to assert it exists for a whole set 
-- runs into higher-order quantification, and besides, we don't

--
-- ub of a set S is defined as the set of all x that ar an ub of S
--
fun ub [S : set L] : set L
{ { x : L | (all y : L | y in S => y -> x in le) } }
--
-- lb of a set S is defined as the set of all x that are an lb of S
--
fun lb [S : set L] : set L
{ { x : L | (all y : L | y in S => x -> y in le) } }
--
-- I use this predicate isglb
-- to define glb of a set S, a subset of L, to be x
--
pred isglb[x : L, S : set L]
{ x in lb[S]
  all y : L | y in lb[S] => y->x in le }
--
-- I use this predicate islub
-- to define lub of a set S, a subset of L, to be x
--
pred islub[x : L, S : set L]
{ x in ub[S]
  all y : L | y in ub[S] => x->y in le }
--
-- glbexists for a pair
-- notice that isglb[g, x+y] && glb[x,y]=g restricts
-- the "glb" in the signature
-- 
fact glbexistspair {
 all x, y :  L | x in L && y in L =>
     some g : L | isglb[g, x+y] && glb[x,y]=g 
}
--
-- lub exists for a pair
-- notice that islub[l, x+y] && lub[x,y]=l restricts
-- the "lub" in the signature
--
fact lubexistspair {
 all x, y :  L | x in L && y in L =>
     some l : L | islub[l, x+y] && lub[x,y]=l
}

-- This produces interesting lattices
run {} for exactly 4 L



