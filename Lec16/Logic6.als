some sig U {
  f2 : (U some -> one U)  -- two-ary fn
}

sig S1, S2 in U {} 

pred p2[x,y:U] { x in S1 and y in S2 } -- general-enough 2-ary pred
 

assert EA { ((all y:U | some  x:U | p2[x, f2[y,y] ] )
            =>
             (some x:U | all y:U | p2[x, f2[y,y] ] ))
	  }

assert nEA { !(((all y:U | some  x:U | p2[x, f2[y,y] ] )
            =>
             (some x:U | all y:U | p2[x, f2[y,y] ] )))
	  }

check EA 
check nEA

/* explanation
f2 and p2 are as follows

     f2   p2
00 0   f
01 1   f
10 1   t
11 0   f

Thus nEA is 

all x : some y : p2(x, f2(y,y))
and
all x : some y : !p2(x, f2(y,y))

which under the above becomes
and  or p2(00) f
        or p2(01) f

and  or  p2(10) t
        or  p2(11) f
and for !pt it flips
*/


 

 
