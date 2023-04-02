--------------------------------------------------------------------------
-- Copyright (C) 1992 by the Board of Trustees of 			  
-- Leland Stanford Junior University.					  
--									  
-- This description is provided to serve as an example of the use	  
-- of the Murphi description language and verifier, and as a benchmark	  
-- example for other verification efforts.				  
--									  
-- License to use, copy, modify, sell and/or distribute this description  
-- and its documentation any purpose is hereby granted without royalty,   
-- subject to the following terms and conditions, provided		  
--									  
-- 1.  The above copyright notice and this permission notice must	  
-- appear in all copies of this description.				  
-- 									  
-- 2.  The Murphi group at Stanford University must be acknowledged	  
-- in any publication describing work that makes use of this example. 	  
-- 									  
-- Nobody vouches for the accuracy or usefulness of this description	  
-- for any purpose.							  
--------------------------------------------------------------------------

--------------------------------------------------------------------------
-- NOTE: This model with a "bug" inserted into it, 
--        might not compile with CMurphi 
--        or traditional/old/original Murphi
--       This is because the getRand function relies 
--        on bitwise AND, OR & NOT, which og spec does not have.
--       It is designed to work with the parser and AST from the rumur-lib 
--------------------------------------------------------------------------

--------------------------------------------------------------------------
--
--                                                                        
-- File:        muxn.m                                                    
--                                                                        
-- Content:     Peterson's algorithm (mutual exclusion for n-processes)   
--                                                                        
-- Summary of result:                                                     
--          1)  No bug is discovered 					  
--          2)  Details of result can be found at the end of this file.   
--                                                                        
-- References: 						       	       	  
-- Peterson, G.L.,  Myths about the mutual exclusion problem,             
-- Information processing letters, Vol 12, No 3, 1981.                    
--                                                                        
-- Date created:         28 Oct 92                                        
-- Last Modified:        17 Feb 93                                        
--                                                                        
--------------------------------------------------------------------------

--------------------------------------------------------------------------
-- Contains injected bug:
--   This model contains a working and verified representation of a
--    N peterson algorithm, but with an injected "bug." 
--   That adds a buffer of length `BL` which records the order 
--    in which processes get the lock of a resource until it is full.
--   When full it will declare the order in the buffer as a sequence of
--    lock ownerships that if any subsequence of length `MSL` or more
--    occurs in any following `N` length sequences of locking orderings,
--    is declared to be a "bug."
--  To do this we also keep a buffer of length `N` that is filled with a 
--   new set before it is checked for said bug.
-- Concepts:
--  These two buffers and the accompanying variables in the statespace,
--   greatly increase the statespace to a point that no current Murphi 
--   model checker could ever efficiently hold them all in a set.
--  Therefore, it is known that unless a bug is encountered  
--   a traditional Murphi model checker must either end
--   due to state hash saturation, or a built in timeout condition is hit
--   in cases of nivea state sets that have undefined behavior 
--   when fully saturated.
--  The "bug" state is designed to be variably difficult to find, but
--   also defined that there is no possible progression that will never
--   meet the criteria... MAYBE
--------------------------------------------------------------------------

Const
  N: 4; -- 7; -- the number of processes
  LHL: N*4; -- the size of the history buffer
  BL: (N*2); /*N;*/ -- the length of the total bug sequence
  MSL: 3; -- the minimum length of the bug sub sequence

Type
  --  The scalarset is used for symmetry, which is implemented in Murphi 1.5
  --  and not upgraded to Murphi 2.0 yet
  pid: scalarset (N);
  pid_r: 0..(N-1);  -- math workable range version of pid
  b_ind_t: 0..(BL-1); -- index type for bug array
  lh_t: 0..(LHL-1); -- index type for bug array
  -- pid: 1..N;
  priority: 0..N;
  label_t: Enum{L0, -- : non critical section; j := 1; while j<n do
		L1,  -- : Beginwhile Q[i] := j  
		L2,  -- : turn[j] := i   (asking the other process to take turn / update the turn system)
		L3,  -- : wait until (forall k != i, Q[k] < j) or turn[j] != i ; j++; Endwhile  (wait for your turn) <-- you get lock here
		L4   -- : critical section; Q[i] := 0  (your turn)
 		};
Var
  P: Array [ pid ] Of label_t;       -- stores the current state of each process
  Q: Array [ pid ] Of priority;      -- stores what priority each process has
  turn: Array [ priority ] Of pid;   -- maps each priority to a process
  localj: Array [ pid ] Of priority; -- maps each process to it's current priority

  -- locking history buffer
  lock_hist: Array [ lh_t ] Of pid;
  bug: Array [ b_ind_t ] Of pid; -- where we store first locking order to use as bug seq
  lh_size: lh_t;  -- start and end of circ buffer
  b_size: b_ind_t;
  lh_is_full, bug_is_full: boolean;  -- if we circling yet (aka need to move start/lh_i forward)

-- Procedure print_bug(tmp:boolean)
-- Begin
--   put "Bug := [ ";
--   put bug;
--   put "\n\n"
-- EndProcedure;


Ruleset i: pid  Do

  Rule "L0 execute inc j and while"
    P[i] = L0  ==>
  Begin
    localj[i] := 1;
    P[i] := L1; 
  End;

  Rule "L1 execute assign Qi j"
    P[i] = L1  ==>
  Begin
    Q[i] := localj[i];
    P[i] := L2;
  End;

  Rule "L2 execute assign TURNj i"
    P[i] = L2  ==>
  Begin
    turn[localj[i]]  := i;
    P[i] := L3; 
  End;

  Rule "L3 execute wait until"
    P[i] = L3  ==>  -- any process at L3 can do this but only 
  Begin
    If ( Forall k: pid Do
           ( k!=i ) -> ( Q[k]<localj[i] )  -- if all other process has a lower/worse priority
         End --forall
       | ( turn[localj[i]] != i ) ) -- ?!? if our current processes priority is not associated with our process ?!?
    Then                            -- ?!?  |-> this probs means if it is not our turn (this exists to keep the always action below from executing past range bounds)
      localj[i] := localj[i] + 1;  -- always increase local priority (i.e the for loop from wikipedia)
      If ( localj[i]<N )
      Then
        P[i] := L1; -- go update the Q again
      Else          -- when localj[i] = N -> it is now our turn 
        P[i] := L4; -- this assigns the lock to (us)
      End; --If 
    End; --If
  End;

  Rule "L4 execute critical and assign Qi 1"
    P[i] = L4  ==>
  Begin
    Q[i] := 1;
    P[i] := L0;
    -- we have fully reached the lock (update history)
    If (bug_is_full)
    Then
      lock_hist[lh_size] := i;
      lh_size := (lh_size + 1) % LHL;
      lh_is_full:= (lh_size = 0)
    Else
      Assert (!bug_is_full) "Bug is full, but added to it anyway!";
      bug[b_size] := i;
      b_size := (b_size + 1) % BL;
      If (b_size = 0)
      Then
        bug_is_full := True;
        -- print_bug(True);
      EndIf;
    EndIf;
  EndRule; 

End; --Ruleset

Startstate "init"
Begin
  For i:pid Do
    P[i] := L0;
    Q[i] := 0;
  End; --For

  For i: priority Do
    Undefine turn[i];
  End; --For

  Clear localj;

  -- hist bug stuff
  lh_is_full := False;
  bug_is_full := False;
  lh_size := 0;
  b_size := 0;
  Undefine lock_hist;
  Undefine bug;
  If (MSL<2) 
    Then Error "CONFIG ERROR: `MSL` needs to >= 2 to be a proper subseq !!";
  EndIf;
  If (MSL>LHL) 
    Then Error "CONFIG ERROR: `MSL` needs to <=`LHL` for a subsequence to be found !!";
  EndIf;
End;

Invariant "only one P has lock"
  ! Exists i1: pid Do
    Exists i2: pid Do
      ( i1 != i2
      & P[i1] = L4 -- critical
      & P[i2] = L4 -- critical
      )
    End  --Exists
    End; --Exists


Invariant "injected hist subseq bug"
    (!(lh_is_full) | !(
      Exists l := MSL to ((BL<LHL) ? BL : LHL) Do -- length of substring
        Exists b_i := 0 To (BL-l) Do -- look at all start loc in bug
          Exists lh_i := 0 To (LHL-l) Do -- look at all start loc in lock_hist 
              Forall j := 0 To (l-1) Do -- compare sub seq of len l
                bug[b_i+j] = lock_hist[lh_i+j]
              EndForall
          EndExists
        EndExists
      EndExists));




/******************

Summary of Result (using release 2.3):

1) 3 processes

   breath-first search
   29 bits (4 bytes) per state
   771 states with a max of about 54 states in queue
   2313 rules fired
   0.73s in sun sparc 2 station

2) 5 processes

   breath-first search
   63 bits (8 bytes) per state
   576,551 states with a max of about 22,000 states in queue
   2,882,755 rules fired
   1201.66s in sun sparc 2 station

2.73S

* 3 processes (sparc  2 station)
	* The size of each state is 35 bits (rounded up to 5 bytes).

	BFS -nosym
	882 states, 2646 rules fired in 0.73s.

	BFS -sym1
	172 states, 516 rules fired in 0.36s.

* 5 processes (sparc 2 station)
	* The size of each state is 63 bits (rounded up to 8 bytes).

	BFS -sym1
	6770 states, 33850 rules fired in 22.55s.
	249 states max in the queue.

	BFS -nosym
	628868 states, 3144340 rules fired in 758.92s.
	25458 states max in the queue.

gamma2.9S on theforce.stanford.edu 

  	5 proc
	-O4 compile 119.7s 2.7Mbytes
	    (24 bytes per states)
	-sym2,3,4  6770 states, 33850 rules 14.35s

	6 proc
	-O4 compile 120.2s 2.7Mbytes
	    (28 bytes per states)
        -sym2,3,4  35,159 states, 210954 rules 117.45s 

Release 2.9S (Sparc 20, cabbage.stanford.edu)

   7 processes
         * The size of each state is 232 bits (rounded up to 32 bytes).
      -b * The size of each state is 101 bits (rounded up to 16 bytes).

         163298 states, 1143086 rules fired in 341.93s.
      -b 163298 states, 1143086 rules fired in 378.04s.
      -c 163298 states, 1143086 rules fired in 292.42s.

******************/

