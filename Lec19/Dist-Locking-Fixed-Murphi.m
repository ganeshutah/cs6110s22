-----------------------------------------------------------------------------
-- Murphi code for the locking protocol                                    
-- Author : Ganesh Gopalakrishnan, written circa year 2000 
-- Derived from Dilip Khandekar and John Carter's work     
-- Reference to the work:
-- 
-----------------------------------------------------------------------------
Const
  Nprocs 	: 7; -- >= 2 reqd to satisfy request_bufT type declaration.

-------------------------------------------------------
Type
  procT : 0..Nprocs-1; -- Scalarset (Nprocs);

  request_bufT :record
		  	Ar: Array[0..Nprocs-2] of procT;
			Count: -1..Nprocs-2 -- legal range is 0..Nprocs-2
					    -- -1 acts as empty indicator
		end;

		/* With Nprocs=1, we get 0..-1 which makes sense 
		   mathematically (empty) but perhaps not in Murphi. 
		   So, avoid Nprocs <= 1. Similar caveats apply for
		   all array declarations of the form 0..N-2. */

  stateT : enum { ENTER, TRYING, BLOCKED, LOCKED, EXITING };
  hstateT: enum { HANDLE, TRYGRANT };

-------------------------------------------------------

Var
  request_bufs 	: Array [procT] of request_bufT; 
  prob_owners  	: Array [procT] of procT;        
  waiters	: Array [procT] of request_bufT; 
  mutexes	: Array [procT] of Boolean;      

  ar_states	: Array [procT] of stateT;       
  hstates	: Array [procT] of hstateT;      

-------------------------------------------------------

procedure initq(var queue: request_bufT);
               -- queue of Array range 0..Nprocs-2
begin
  for i:0..Nprocs-2 Do
	Undefine queue.Ar[i]
  end;
  queue.Count := -1 -- empty queue
end;

function frontq(queue: request_bufT): procT;
               -- queue of Array range 0..Nprocs-2
begin
  if (queue.Count < 0)
  then Error "Front of empty queue is undefined"
  else return queue.Ar[0]
  endif
end;

function nonemptyq(queue: request_bufT) : boolean;
begin
  return (queue.Count >= 0)
end;

function emptyq(queue: request_bufT) : boolean;
begin
  return (queue.Count = -1)
end;

procedure dequeue(var queue: request_bufT);
                  -- queue of Array range 0..Nprocs-2
begin
 if queue.Count = -1
 then Error "Queue is empty"
 else queue.Count := queue.Count - 1;
      if queue.Count = -1
      then Undefine queue.Ar[0]
      else for i := 0 to queue.Count do
	     queue.Ar[i] := queue.Ar[i+1]
	   end;
           Undefine queue.Ar[queue.Count+1]
      endif
 endif
end;
	
procedure enqueue(var queue: request_bufT; pid: procT);
                  -- queue of Array range 0..Nprocs-2
begin
 if 	queue.Count = Nprocs-2
 then 	Error "Queue is full";
 else 	queue.Count := queue.Count + 1;
        queue.Ar[queue.Count] := pid
 endif;
end;

-------------------------------------------------------

procedure place_request(prob_owner, p : procT);
begin
	enqueue(request_bufs[prob_owner], p)
end;

procedure copytail(var source_queue, destination_queue : request_bufT);
/* Called only when source_queue is non_empty, i.e. source_queue.Count >= 0.
   Copies the tail of the queue "source_queue" into
   "destination_queue" (which, in actual use, happens to be
    at the new prob_owner),  and also undefines "source_queue" and
   the unused locations of "destination_queue" (at the new probable owner).
   If source_queue.Count = 0, there is no tail to be copied, and we are done.
*/
begin
	if source_queue.Count > 0 -- non-empty and has >= 1 element
	then
	   for i := 1 to source_queue.Count do
	   	destination_queue.Ar[i-1] := source_queue.Ar[i];
		Undefine source_queue.Ar[i];
	   end;
	   destination_queue.Count := source_queue.Count - 1;
	   Undefine destination_queue.Ar[source_queue.Count];
	   Undefine source_queue.Ar[0];
           source_queue.Count := -1
	else
 	   Undefine source_queue.Ar[0];
           source_queue.Count := -1
 	endif
end;

-------------------------------------------------------

Ruleset p:procT Do
    Alias 	request_buf: request_bufs[p] Do 
    Alias	prob_owner : prob_owners[p]  Do 
    Alias	waiter	   : waiters[p]	     Do 
    Alias	state	   : ar_states[p]    Do 
    Alias	hstate	   : hstates[p]      Do 
    Alias	mutex	   : mutexes[p]      Do 

    	Rule "Try acquiring the lock"
	 	((state = ENTER) & !mutex) 
			==> mutex := true;
			    state := TRYING;
	Endrule;
	-------------------------------------------------------

    	Rule "If the lock is around, grab it"
		((state = TRYING) & (prob_owner = p) & mutex)
			==> mutex := false;
                            state := LOCKED;
                            
	Endrule;
	-------------------------------------------------------

    	Rule "If the lock isn't around, send request out"
		((state = TRYING) & (prob_owner != p) & mutex)
			==> mutex := false;
                            place_request(prob_owner,p);
			    state := BLOCKED;
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

	-- added emptyq check for waiter 
    	Rule "In EXITING state, pass hd waiter and tail of waiters along."
		((state = EXITING) & !emptyq(waiter) & mutex)
			==>   mutex := false;
                              -- set the PO variable at node frontq(waiters) and
			      -- also our own PO variable to frontq(waiters)
			      -- the former step unblocks node frontq(waiters),
			      -- giving it the lock
		              prob_owners[frontq(waiter)] := frontq(waiter);
		   	      prob_owner := frontq(waiter);

-- added this new line to update the state of acquire's thread state when passing the lock
			      ar_states[frontq(waiter)] := LOCKED;   

			      copytail(waiter, waiters[prob_owner]);
			      state := ENTER;

	Endrule;
	-------------------------------------------------------
        -- GG : 3/23/23 : did not see a need for this rule
	-- added this new rule which will get fired when there are no waiters in the queue
	-- Rule "In EXITING state, release the lock if no waiters."
        --        ((state = EXITING) & emptyq(waiter) & mutex)
        --                ==>   
        --                    state := ENTER;
        --                    mutex := false;
        -- Endrule;
        -------------------------------------------------------


    	Rule "In state HANDLE, if there is a waiting request, goto TRYGRANT"
        	((hstate = HANDLE) & !emptyq(request_buf) & !mutex)
			==> mutex := true;
		   	    hstate := TRYGRANT;
	Endrule;
	-------------------------------------------------------


    	 Rule "In state TRYGRANT, if lock is free, grant it."
		((hstate = TRYGRANT) & (prob_owner = p) & (state != LOCKED) & mutex)
			==>
              	   	   mutex := false;

                           --if ((state != ENTER) | (state != EXITING))
			   --then Error
                           --   "State can't be TRYING/LOCKED/EXIT(due to mutex) or BLOCKED (due to prob_owner)"
			   --else

			   if (!emptyq(waiter))
			   then Error "Lock is HERE and FREE while there is a waiter."
			   else

			    -- this step will unblock process hd(request_buf) effectively giving it the lock
			    prob_owners[frontq(request_buf)] := frontq(request_buf);
			    ar_states[frontq(request_buf)] := LOCKED;

		   	    prob_owner := frontq(request_buf);

		   	    dequeue(request_buf);
		   	    hstate := HANDLE;
	
			   endif
			
	Endrule;
	-------------------------------------------------------


    	Rule "If lock not around, pass buck along to who we think is PO"
		(hstate = TRYGRANT & !emptyq(request_buf) & prob_owner != p & mutex)
			==> mutex := false;
                            place_request(prob_owner,frontq(request_buf));
		   	    dequeue(request_buf);
		   	    hstate := HANDLE;
		   	    
	Endrule;
	-------------------------------------------------------


    	Rule "If lock around but busy, enqueue request"
		(hstate = TRYGRANT & prob_owner = p & state = LOCKED & mutex)
			==> mutex := false;
                            enqueue(waiter,frontq(request_buf));
		   	    dequeue(request_buf);
		   	    hstate := HANDLE;
		   	    
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
    initq(request_bufs[p]);
    prob_owners[p] := n;  -- designate some n in procT  to be the owner
    initq(waiters[p]);
    ar_states[p] := ENTER;
    hstates[p] := HANDLE;
    mutexes[p] := false;
End;
Endstartstate;
Endruleset;

--------------------------------------------
Invariant "Bug3"
!(
(prob_owners[0]=1)&(prob_owners[1]=2)&
(prob_owners[2]=5)&
(prob_owners[3]=4)&(prob_owners[4]=5)&
(prob_owners[5]=6)&
(prob_owners[6]=6)

)



-------------------------------------------------------

-- end of locking.m
-------------------------------------------------------
