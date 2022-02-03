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
	-- R01 -----------------------------------------------------	      
    	Rule "In ctrl state INIT, circulate token if root, else nothing; enter MAIN"
	(control = INIT) 
	==> Begin
    	     if (p == root_id) then --root action
               tokout := WHITE;
             endif;
	     control := MAIN;
            End;
	Endrule;

	-- R03 -----------------------------------------------------
    	Rule "In ctrl state MAIN, if I'm active, I can go passive"
	(state = MAIN) & (actpass == ACTPASS)
	==> Begin
	      actpass := PASSIVE;
            End;
	Endrule;

	-- R04 -----------------------------------------------------
    	Rule "In ctrl state MAIN, if I'm active, I can pick target & assign work"
	(state = MAIN) & (actpass == ACTIVE)
	==> var victim : procT;
            Begin
	      victim := rand_pick_work_dest(p);
	      work_array[victim] := true;
	      If upstream_victim(victim,p) 
	      Then
	         ncolor := BLACK; -- R04, R05
	      Endif;
	      actpass := PASSIVE;
	    End;

-- R06
-- 'Rule' "In ctrl state MAIN, if I'm active, I can receive incoming token"
-- This is not implemented. The token just sits at the output port of
-- my predecessor
	    
	-- R07 -----------------------------------------------------
    	Rule "In ctrl state MAIN, if passive, if in-facing work, consume, Active"
	(state = MAIN) & (actpass == PASSIVE) & (..)
	==> var victim : procT;
            Begin
	      victim := rand_pick_work_dest(p);
	      work_array[victim] := true;
	      If upstream_victim(victim,p) 
	      Then
	         ncolor := BLACK; -- R04, R05
	      Endif;
	      actpass := PASSIVE;
	    End;
	

	==> Begin
	      
	      work_array[victim] := true;
	      If upstream_victim(victim,p) 
	      Then
	         ncolor := BLACK; -- R04, R05
	      Endif;
	      actpass := PASSIVE;
	    End;          
	-------------------------------------------------------
	-------------------------------------------------------
	-------------------------------------------------------

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

