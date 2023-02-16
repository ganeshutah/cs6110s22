-- From Bradley and Manna, Example 2.13

-- a non-empty set U
some sig U {}

-- S is a subset of U
sig S in U {} 

-- A general 2-ary predicate
pred p[x,y:U] { x in S and y in S } -- general-enough 2-ary pred

-- The Bradley/Manna assertion 
assert EA { (all x:U | p[x,x] 
             =>
	     some x:U | all y:U | p[x,y]
	    )
	  }

-- Negation of the Bradley/Manna assertion 	  
assert nEA {!(all x:U | p[x,x] 
             =>
	     some x:U | all y:U | p[x,y]
	    )
	   }
check EA 
check nEA

 

 
