WIP

--https://alloy.readthedocs.io/en/latest/language/predicates-and-functions.html

some sig U {
  f1 : U,             -- one-ary fn from U to U
  f2 : U some -> one U  -- one-ary fn from U to many-to-one maps U->U
                     -- f2[U] is acting as a non-functional relation!!
                     -- this diag is confusing , table OK

}

sig S in U {} 


pred p1[x:U]   { x in S } -- general-enough 1-ary pred
pred p2[x,y:U] { x in S and y in S } -- general-enough 2-ary pred


assert EA3{  some y:U | all x:U  | p2[x,y]
             =>
	     all  x:U | some y:U | p2[x,y] 
	  } 

assert nEA3{!(some y:U | all x:U  | p2[x,y]
             =>
	     all  x:U | some y:U | p2[x,y] )
	   } 

check EA3

check nEA3


 
assert EA5{  some y:U | all x:U  | p2[x,f1[y] ]
             =>
	     all  x:U | some y:U | p2[x,f1[y] ] 
	  } 

assert nEA5{!(some y:U | all x:U  | p2[x,f1[y] ]
             =>
	     all  x:U | some y:U | p2[x,f1[y] ] )
	   } 

check EA5

check nEA5


 


