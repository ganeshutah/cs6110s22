//-- Suppose we want to satisfy the following True Property
//-- "Henceforth (Eventually x)" in a system that exposes
//-- one bit of state
//-- 
//-- The Negated Property is "Eventually (Henceforth (not x))"
//--
//-- Here is a system that satifies the negated property
	
mtype = {A,B} // for state-names of the system automaton
bit x=0;
active proctype S()
{ mtype state=A;
  do
  :: state==A ->
     do
     //:: x=0;     // this is the self-loop at state A
     :: atomic {x=0; state=B}
                //  In one atomic step, 
     od
  :: state==B ->
     atomic {x=1;     // In one atomic move, we set x=1 and
             state=A} // transition back to A atomically
  od; // (Note: We could use d_step for efficiency in lieu of atomic)
}
//
//-- negated property automaton for <>[]!x
//
never
{do
 :: skip
 :: !x -> break
 od;
 accept: !x -> goto accept
}

/*
spin -a -X
gcc -w -o pan -D_POSIX_SOURCE -DMEMLIM=128 -DSAFETY -DXUSAFE -DNOFAIR pan.c
time ./pan -v -X -m10000 -w19 -A -c1
*/
