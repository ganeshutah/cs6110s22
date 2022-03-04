-- Now try a two-ary predicate, a 1-ary function and a valid sentence --

sig S1, S2 in U {}

some sig U {
  f1 : U -- this is implicitly 'one U' so we get a 1-ary function
}

pred p2[x,y:U] { x in S1 and y in S2 }
 
assert EA { ((some y:U | all x:U | p2[x,f1[y] ])
             =>
             (all  x:U | some y:U | p2 [x,f1[y] ]))
	  }
assert nEA {!((some y:U | all x:U | p2[x,f1[y] ])
             =>
             (all  x:U | some y:U | p2 [x,f1[y] ]))
	  }	  

check EA 
check nEA
--

 

 
