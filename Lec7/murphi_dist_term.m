------------ begin murphi_dist_term.m -----------------
Const
  Nprocs 	: 3; 
-------------------------------------------------------
Type
  procT : Scalarset (Nprocs);   -- all the processes are idential
  stateT : enum { INIT, MAIN }; -- basically 2 states
  toknT  : enum { NONE, BLACK, WHITE }; -- three states for token in-facing
  ncolorT: enum { WHITE, BLACK }; -- node colors are w/b
  actpassT: enum { ACTIVE, PASSIVE }; -- nodes are active/passive
-------------------------------------------------------
Var -- proc[p] in state_array[p] state reads work_array[p] writes token_array[p]
  root_id      : procT; -- if procid p = root_id, that is the root, else not
  work_array   : Array [procT] of Boolean;-- 'true/false': work present/not
  token_array  : Array [procT] of toknT;  -- colored token or NONE
  state_array  : Array [procT] of stateT; -- each proc in its state
  ncolor_array : Array [procT] of ncolorT; -- node colors
  actpass_array: Array [procT] of actpassT; -- active or passive
-------------------------------------------------------
Ruleset p:procT Do
-- anyone could be root, and it depends on the initialization!!
   Alias control: state_array[p]   Do
   Alias ncolor:  ncolor_array[p]  Do
   Alias actpass  actpass_array[p] Do
   Alias tokout:  token_array[p]   Do
   Alias workin:  work_array[p]    Do
    	Rule "Circulate token if root, else nothing; then enter MAIN"
	(state = INIT) 
	==> if (p == root_id) then --root action
  	     tokout := WHITE;
	    endif;
	    state := MAIN;
	Endrule;
	-------------------------------------------------------
    	Rule "If the lock is around, grab it"
		((state = TRYING) & mutex & (prob_owner = p))
			==> state := LOCKED ; mutex := false;
	Endrule;
	-------------------------------------------------------

    	Rule "If the lock isn't around, send request out"
		((state = TRYING) & mutex & (prob_owner != p))
			==> place_request(prob_owner,p);
			    state := BLOCKED;
		   	    mutex := false;
	Endrule;
	-------------------------------------------------------


	Rule "Locked -> Enter if no waiters"
		((state = LOCKED) & emptyq(waiter)) ==> state := ENTER;
	Endrule;
	-------------------------------------------------------


    	Rule "When unlocking if waiter queue isn't empty, go to EXITING state"
		((state = LOCKED) & !emptyq(waiter) & !mutex)
			==> mutex := true; 
		   	    state := EXITING;
	Endrule;
	-------------------------------------------------------


    	Rule "In EXITING state, pass hd waiter and tail of waiters along."
		((state = EXITING) & mutex)
			==>   -- set the PO variable at node frontq(waiters) and
			      -- also our own PO variable to frontq(waiters)
			      -- the former step unblocks node frontq(waiters),
			      -- giving it the lock

			      prob_owners[frontq(waiter)] := frontq(waiter);

		   	      prob_owner := frontq(waiter);

			      copytail(waiter, waiters[prob_owner]);

		   	      state := ENTER;
		   	      mutex := false
	Endrule;
	-------------------------------------------------------


    	Rule "In state HANDLE, if there is a waiting request, goto TRYGRANT"
		((hstate = HANDLE) & !mutex & !emptyq(request_buf))
			==> mutex := true;
		   	    hstate := TRYGRANT;
	Endrule;
	-------------------------------------------------------


    	Rule "In state TRYGRANT, if lock is free, grant it."
		((hstate = TRYGRANT) & (prob_owner = p) & (state != LOCKED) & mutex)
			==>
			   if (state != ENTER)
			   then Error
                              "State can't be TRYING/LOCKED/EXIT(due to mutex) or BLOCKED (due to prob_owner)"
			   else

			   if (!emptyq(waiter))
			   then Error "Lock is HERE and FREE while there is a waiter."
			   else

			    -- this step will unblock process hd(request_buf) effectively giving it the lock
			    prob_owners[frontq(request_buf)] := frontq(request_buf);
			    ar_states[frontq(request_buf)] := LOCKED;

		   	    prob_owner := frontq(request_buf);

		   	    dequeue(request_buf);
		   	    hstate := HANDLE;
		   	    mutex := false;
			   endif
			   endif
	Endrule;
	-------------------------------------------------------


    	Rule "If lock not around, pass buck along to who we think is PO"
		(hstate = TRYGRANT & prob_owner != p & mutex)
			==> place_request(prob_owner,p);
		   	    dequeue(request_buf);
		   	    hstate := HANDLE;
		   	    mutex := false;
	Endrule;
	-------------------------------------------------------


    	Rule "If lock around but busy, enqueue request"
		(hstate = TRYGRANT & prob_owner = p & state = LOCKED & mutex)
			==> enqueue(waiter,frontq(request_buf));
		   	    dequeue(request_buf);
		   	    hstate := HANDLE;
		   	    mutex := false;
	Endrule;
	-------------------------------------------------------

    Endalias;
    Endalias;
    Endalias;
    Endalias;
    Endalias;
    Endalias;
Endruleset;

-------------------------------------------------------
Ruleset n:procT Do
Startstate
For p:procT Do
  root_id := n;   -- All fight to set root_id. Finally one wins!
  work_array [p]   := false; -- no work facing anyone initially
  token_array[p]   := NONE;  -- no token facing anyone
  state_array[p]   := INIT;  -- all are in init state
  ncolor_array[p]  := WHITE; -- all nodes start white
  actpass_array[p] := ACTIVE; -- best init: "all active" (can go passive)
End;
Endstartstate;
Endruleset;

-------------------------------------------------------
-- Due to symmetry, we key-off "n"

Invariant
Exists n:procT Do
 ((ar_states[n] = LOCKED) ->
  (Forall p:procT Do
   ((p=n) | (ar_states[p] != LOCKED))
   Endforall)
 )
Endexists;

-------------------------------------------------------

-- end of locking.m
-------------------------------------------------------

