--https://alloy.readthedocs.io/en/latest/language/predicates-and-functions.html

-- p1(x) => q1(y) - validity : univ closure; sat : exist closure

-- exists x : exists y : p2(x,y) => exists x : p2(x,x) -- check valid/sat

-- exists y : forall x : p2(x,y) => forall x : exists y: p2(x,y)

some sig U {}

sig S1 extends U {}
sig S2 extends U {}

pred p1[x:U]   { x in S1 } -- general-enough 1-ary pred
pred p2[x,y:U] { x in S1 and y in S2 } -- general-enough 2-ary pred

-- run { #U = 2 }


assert EA { some y:U | all x:U | p2[x,y]
            =>
	    all  x:U | some y:U | p2[x,y] 
	  } 
assert nEA { !(some y:U | all x:U | p2[x,y]
            =>
	    all  x:U | some y:U | p2[x,y])
	  }
check EA 
check nEA

 

 
