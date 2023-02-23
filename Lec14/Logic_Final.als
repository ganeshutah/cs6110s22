one sig a,b extends U {} -- a,b disjoint in U
abstract sig U {
 f1 : U,
 f2 : U some -> one U
}

sig S, S1, S2 in U { }

pred p1[ x  :U ] {  x in S }

pred p2[ x,y:U ] {  x in S1 and y in S2 }

assert EA3 {  (some y:U | all x:U | p2[x, y])
               => 
              (all  x:U | some y:U | p2[x, y])
           }
check EA3 

assert EA5 {  (some y:U | all x:U | p2[x, f1[y]])
              => 
              (all  x:U | some y:U | p2[x, f1[y]])
           }
check EA5

assert EA5g{   --some y:U | all x:U | p2[x, f1[y]]
               (  (   p2[a,f1[a]]  &&  p2[b,f1[a]]  )
                  ||
                  (   p2[a,f1[b]]  &&  p2[b,f1[b]]  )
	       )
               => 
               --all  x:U | some y:U | p2[x, f1[y]]
               (  (   p2[a,f1[a]]  || p2[a,f1[b]]  )
                  &&
                  (   p2[b,f1[a]]  ||  p2[b,f1[b]]  )	
               )
                }
check EA5g

