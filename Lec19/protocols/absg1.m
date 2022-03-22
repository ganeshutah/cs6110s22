const
 NODE_NUM : 2;
 DATA_NUM : 2;
type
 NODE : scalarset(NODE_NUM);
 DATA : scalarset(DATA_NUM);
 ABS_NODE : union {NODE, enum{Other}}; -- abs node
 CACHE_STATE : enum {Invld, Shrd, Excl}; -- renamed states
 CACHE : record State : CACHE_STATE; Data  : DATA; end;
 MSG_CMD : enum {Empty, ReqS, ReqE, Inv, InvAck, GntS, GntE};
 MSG : record Cmd : MSG_CMD; Data : DATA; end;

 STATE : record -- what were 'var' are now culled together into a state record
  Cache : array [NODE] of CACHE;
  Chan1 : array [NODE] of MSG; -- for Req*
  Chan2 : array [NODE] of MSG; -- Gnt* Inv
  Chan3 : array [NODE] of MSG; -- InvAck
  InvSet : array[NODE] of boolean; -- nodes to be inv
  ShrSet : array [NODE] of boolean;-- nodes having valid copies
  ExGntd : boolean; -- Excl copy gnted
  CurCnd : MSG_CMD; -- current request command
  CurPtr : ABS_NODE;-- current request node
                    -- which can be abstract
  MemData : DATA;   -- mem data
  AuxData : DATA;   -- aux var for latest data
 end;

var
 Sta : STATE;

-- initial state --

ruleset d : DATA do startstate "Init"
  undefine Sta;
  for i : NODE do
    Sta.Cache[i].State := Invld;
    Sta.Chan1[i].Cmd := Empty;
    Sta.Chan2[i].Cmd := Empty;
    Sta.Chan3[i].Cmd := Empty;
    Sta.InvSet[i] := FALSE;
    Sta.ShrSet[i] := FALSE;
  end;

  Sta.ExGntd := false;
  Sta.CurCmd := Empty;
  Sta.MemData := d;
  Sta.AuxData := d;
end end;


----------------

----------------------
ruleset i : NODE;         -- for every node i
        d : DATA          -- for every data d
  do 
    rule "Store"
      Sta.Cache[i].State = Excl  -- if node is in E
      ==>
      var NxtSta : STATE;
      begin
        NxtSta := Sta;
        NxtSta.Cache[i].Data := d; 
        NxtSta.AuxData := d;
	Sta := NtSta;
    end; end;

---
ruleset i : NODE do 
  rule "SendReqS"
    Sta.Chan1[i].Cmd = Empty &
    Sta.Cache[i].State = Invld
    ==>
    var NxtSta :STATE;
    begin
        NxtSta := Sta;
        NxtSta.Chan1[i].Cmd := ReqS;
	Sta := NxtSta;
  end; end;
---
ruleset i : NODE do 
  rule "SendReqE"
    Sta.Chan1[i].Cmd = Empty &
    Sta.Cache[i].State != Excl
    ==>
    var NxtSta : STATE;
    begin
      NxtSta := Sta;
      NxtSta.Chan1[i].Cmd := ReqE;
      Sta := NxtSta;
  end; end;

---
ruleset i : NODE do
 rule "RecvInvS"
  Sta.Cache[i].State != Excl &
  Sta.Chan2[i].Cmd = Inv &
  Sta.Chan3[i].Cmd = Empty
  ==>
  var NxtSta : STATE;
    begin
      NxtSta := Sta
      --
      NxtSta.Cache[i].State != Invld;
      undefine NxtSta.Cache[i].Data;
      NxtSta.Chan2[i].Cmd := Empty;
      NxtSta.Chan3[i].Cmd := InvAck;
      --
      Sta := NxtSta;
    end; end;      

