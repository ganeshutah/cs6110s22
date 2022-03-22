const -- configuration parameters --
NODE_NUM : 6;
DATA_NUM : 2;

type -- type decl --

NODE : scalarset(NODE_NUM);
DATA : scalarset(DATA_NUM);

CACHE_STATE : enum {I, S, E};
CACHE : record State : CACHE_STATE; Data : DATA; end;

MSG_CMD : enum {Empty, ReqS, ReqE, Inv, InvAck, GntS, GntE};
MSG : record Cmd: MSG_CMD; Data : DATA; end;

var -- state variables --

Cache : array [NODE] of CACHE;    -- Caches

Chan1 : array [NODE] of MSG;      -- Channels for Req*
Chan2 : array [NODE] of MSG;      -- Channels for Gnt* and Inv
Chan3 : array [NODE] of MSG;      -- Channels for InvAck

InvSet : array [NODE] of boolean; -- Nodes to be invalidated
ShrSet : array [NODE] of boolean; -- Nodes having S or E copies
ExGntd : boolean;                 -- E copy has been granted
CurCmd : MSG_CMD;                 -- Current request command
CurPtr : NODE;                    -- Current request node
MemData : DATA;                   -- Memory data
AuxData : DATA;                   -- Latest value of cache line

-- Initial States --

ruleset d : DATA do startstate "init"
--
-- All nodes: init all cmd channels to be empty, Cache States I, 
-- the set of nodes to be invalidated is empty
-- and nodes having S or E copies empty
-- 
  for i : NODE do
    Chan1[i].Cmd := Empty;
    Chan2[i].Cmd := Empty;
    Chan3[i].Cmd := Empty;
    Cache[i].State := I;
    InvSet[i] := false;
    ShrSet[i] := false;
  end;
  ExGntd := false;
  CurCmd := Empty;
  MemData := d;
  AuxData := d;
end end;


-- State Transitions --
--------------------------------------------
ruleset i : NODE do 
-- Any node with cmd req channel empty and cache I can request ReqS
  rule "SendReqS"
    Chan1[i].Cmd = Empty &
    Cache[i].State = I
    ==>
    Chan1[i].Cmd := ReqS;  -- raises "ReqS" semaphore
  end
end;

----------------------
ruleset i : NODE do 
-- Any node with cmd req channel empty and cache I/S can request ReqE
  rule "SendReqE"
    Chan1[i].Cmd = Empty &
   (Cache[i].State = I |
    Cache[i].State = S)
    ==>
    Chan1[i].Cmd := ReqE;  -- raises "ReqE" semaphore
  end
end;

--------------------------------------------
ruleset i : NODE do 
-- For any node that is waiting with ReqS requested, with CurCmd Empty 
-- we set CurCmd to ReqS on behalf of node i (setting CurPtr to point to it).
-- Then void Chan1 empty.
-- Now Set the nodes to be invalidated to the nodes having S or E copies.
  rule "RecvReqS" -- prep action of dir ctrlr
    CurCmd = Empty &
    Chan1[i].Cmd = ReqS
    ==>
    CurCmd := ReqS;
    CurPtr := i; -- who sent me ReqS
    Chan1[i].Cmd := Empty;       -- drain its cmd
    for j : NODE do InvSet[j] := ShrSet[j] end; -- inv = nodes with S/E
  end
end;

----------------------
ruleset i : NODE do
-- For any node that is waiting with ReqE requested, with CurCmd Empty 
-- we set CurCmd to ReqE on behalf of node i (setting CurPtr to point to it).
-- Then void Chan1 empty.
-- Now Set the nodes to be invalidated to the nodes having S or E copies.
  rule "RecvReqE" 
    CurCmd = Empty &
    Chan1[i].Cmd = ReqE
    ==>
    CurCmd := ReqE;
    CurPtr := i; -- who sent me ReqE
    Chan1[i].Cmd := Empty;       -- drain its cmd
    for j : NODE do InvSet[j] := ShrSet[j] end; -- inv = nodes with S/E
  end
end;

--------------------------------------------
ruleset i : NODE do 
-- For every node with Chan2 Cmd empty and InvSet true (node to be invalidated)
-- and if CurCmd is ReqE or (ReqS with ExGnt true), then
-- void Chan2 Cmd to Inv, and remove node i from InvSet (invalidation already set out)
  rule "SendInv"  
    Chan2[i].Cmd = Empty &
    InvSet[i] = true &  -- Gnt* and Inv channel
    ( CurCmd = ReqE |                 -- DC: curcmd = E
      CurCmd = ReqS & ExGntd = true ) -- DC: curcmd = S & ExGntd
  ==>
    Chan2[i].Cmd := Inv; -- fill Chan2 with Inv
    InvSet[i] := false; 
  end
end;

----------------------
--
-- When a node gets invalidated, it acks, and when it was E
-- then the node (i) coughs up its cache data into Chan3
-- Then cache state is I and undefine Cache Data
-- 
ruleset i : NODE do
  rule "SendInvAck"
    Chan2[i].Cmd = Inv &
    Chan3[i].Cmd = Empty
    ==>
    Chan2[i].Cmd := Empty;
    Chan3[i].Cmd := InvAck;
    if (Cache[i].State = E) then Chan3[i].Data := Cache[i].Data end;
    Cache[i].State := I; undefine Cache[i].Data;
  end
end;

----------------------
ruleset i : NODE do
  rule "RecvInvAck"
    Chan3[i].Cmd = InvAck &
    CurCmd != Empty
    ==>
    Chan3[i].Cmd := Empty;
    ShrSet[i] := false;
    if (ExGntd = true) then ExGntd := false;
    MemData := Chan3[i].Data;
    undefine Chan3[i].Data end;
  end
end;

--------------------------------------------
ruleset i : NODE do
  rule "SendGntS"
    CurCmd = ReqS &
    CurPtr = i &
    Chan2[i].Cmd = Empty &
    ExGntd = false
    ==>
    Chan2[i].Cmd := GntS;
    Chan2[i].Data := MemData;
    ShrSet[i] := true;
    CurCmd := Empty;
    undefine CurPtr;
  end
end;

----------------------
ruleset i : NODE do
  rule "SendGntE"
    CurCmd = ReqE &
    CurPtr = i &
    Chan2[i].Cmd = Empty &
    ExGntd = false &
    forall j : NODE do ShrSet[j] = false end -- nodes having S or E status
    ==>
    Chan2[i].Cmd := GntE;
    Chan2[i].Data := MemData;
    ShrSet[i] := true;
    ExGntd := true;
    CurCmd := Empty;
    undefine CurPtr;
  end
end;

----------------------
ruleset i : NODE do
  rule "RecvGntS"
    Chan2[i].Cmd = GntS
    ==>
    Cache[i].State := S;
    Cache[i].Data := Chan2[i].Data;
    Chan2[i].Cmd := Empty;
    undefine Chan2[i].Data;
  end
end;

----------------------
ruleset i : NODE do
  rule "RecvGntE"
    Chan2[i].Cmd = GntE
    ==>
    Cache[i].State := E;
    Cache[i].Data := Chan2[i].Data;
    Chan2[i].Cmd := Empty;
    undefine Chan2[i].Data;
  end
end;

----------------------
ruleset i : NODE;         -- for every node i
        d : DATA          -- for every data d
  do 
    rule "Store"
      Cache[i].State = E  -- if node is in E
      ==> 
      Cache[i].Data := d; -- store d into Cache[i].Data
      AuxData := d;       -- Also update latest cache line value
    end                   -- The node in E can get any "D" value
end;

--------------------------------------------

---- Invariant properties ----

invariant "CtrlProp"
forall i : NODE do
  forall j : NODE do
   i!=j -> 
    (Cache[i].State = E -> Cache[j].State = I) &
    (Cache[i].State = S -> Cache[j].State = I |
                           Cache[j].State = S)
  end
end;

invariant "DataProp"
( ExGntd = false -> MemData = AuxData ) &
 forall i : NODE
 do Cache[i].State != I ->
    Cache[i].Data = AuxData
 end;

--------------------------------------------



