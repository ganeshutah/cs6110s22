some sig U {}

sig S1,S2 extends U {}

-- Encode a 1-ary and a two-ary predicate in Alloy --

pred p1[x:U]   { x in S1 }             -- general-enough 1-ary pred
pred p2[x,y:U] { x in S1 and y in S2 } -- general-enough 2-ary pred

-- run { #U = 2 }

-- An expected FOL-valid sentence being checked --

assert EA { ((some y:U | all x:U | p2[x,y])
            =>
	     (all  x:U | some y:U | p2[x,y]))
	  }
assert nEA {!((some y:U | all x:U | p2[x,y])
            =>
	     (all  x:U | some y:U | p2[x,y]))
	  } 	  
check EA 
check nEA
--

 

 
