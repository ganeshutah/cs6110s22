
-- Now, try a 2-ary function encoding --

some sig U {
f2 : (U set -> one U)  -- two-ary fn
}

-- must ensure that f2 indeed is simulating a fully general 2-ary function --

sig S1, S2 in U {} 

pred p2[x,y:U] { x in S1 and y in S2 } -- general-enough 2-ary pred
 

-- Now a valid sentence --

assert EA { ((some y:U | all     x:U | p2[x, f2[y,y] ] )
            =>
	        (all      x:U | some y:U | p2[x, f2[y,y] ] ))
	  }


assert nEA {!( (some y:U | all x:U | p2[x, f2[y,y] ])
            =>
	            (all  x:U |    some y:U | p2[x, f2[y,y] ]) )
	  }

check EA 
check nEA


 

 
