--https://alloy.readthedocs.io/en/latest/language/predicates-and-functions.html

-- p1(x) => q1(y) - validity : univ closure; sat : exist closure

-- exists x : exists y : p2(x,y) => exists x : p2(x,x) -- check valid/sat

-- exists y : forall x : p2(x,y) => forall x : exists y: p2(x,y)

some sig U {
  f1 : U,       -- one-ary fn
  f2 : U -> U,  -- two-ary fn
}

sig S in U {} 


pred p1[x:U]   { x in S } -- general-enough 1-ary pred
pred p2[x,y:U] { x in S and y in S } -- general-enough 2-ary pred

 
assert EA { some y:U | all x:U | p2[x,f1[y] ]
            =>
	    all  x:U | some y:U | p2[x,f1[y] ] 
	  } 
assert nEA { !(some y:U | all x:U | p2[x, f2[y,y] ]
            =>
	    all  x:U | some y:U | p2[x, f2[y,y]  ] )
	  }
check EA 
check nEA

 

 
