
(******************************   mpec.ml   ****************************************)
(*  If genTransTotalOrder is false, then constraints for Transitive Order and      *)
(*    total order not generated, but constraints for Reflexivity generated.        *)
(*                                                                                 *)
(*  Adds support for default data for memory locations.                            *)
(*                                                                                 *)
(*  The default-value rule is simplified - any value read that's not               *)
(*  written is assumed to be an initial memory value -- but then added to an       *)
(* init-mem-value-HT and written out into a file as a "TCC" - users are advised    *)
(*  to check.                                                                      *)
(*                                                                                 *)
(*  Version that also writes implied unit-clauses for WOO, PO, MDD, DFD, and SUC   *)
(*                                                                                 *)
(*  Over and above main11.ml, this version does a little bit of clean-up (e.g.,    *)
(*  var_at is used instead of explicit formulae) and also suppresses generating    *)
(*  trans constraints.                                                             *)
(***********************************************************************************)


type rulename =
    Reflexive
  | TransitiveOrder
  | TotalOrder
  | WriteOperationOrder
  | ProgramOrder
  | MemoryDataDependence
  | DataFlowDependence
  | Coherence
  | SequentialUC
  | ReadValue
  | AtomicWBRelease
  | NoUCBypass
  | NoRule

(*************************************************************************)
(*  Each constraint is annoted with a max of 3 ops that were involved    *)
(*  in it's generation and a rule that generated it                      *)
(*************************************************************************)
type constinfo = { op1: int; op2: int; op3: int; op4: int; rule: rulename }


type constr =
    Tt of constinfo
  | Ff of constinfo
  | Iv  of int * constinfo (* For variables that are already assigned - we just want to build constraints
			      on them *)
  | Not of constr * constinfo
  | And of constr * constr * constinfo
  | Or  of constr * constr * constinfo
  | Neq of constr * constr * constinfo
  | Imp of constr * constr * constinfo

type ikind = LdAcq | StRel | Ld | St | Mf

type wtype = Local | Remote | DontCare


(* Using strings for "var", "data", etc, to handle litmus tests that contain
   numbers in Hex, Octal, etc.
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*)
type instrn = {proc: int; pc: int; op: ikind; var: string; 
               data: string; 
               wrID:int;  wrType: wtype; wrProc: int; 
               reg: string; useReg: bool; id: int} 



(************* the external file ******************)
external gentuple_parse_input_file : string -> int = "parse_input_file"
external gentuple_get_tuple_sum: unit -> int = "get_tuple_sum"
external gentuple_get_init_vals: unit->string * string array = "get_init_vals"
external gentuple_get_tuple: unit-> instrn = "get_tuple"
external gentuple_get_tuple_array : unit -> instrn array = "get_tuple_array"
external test_val : ikind -> unit = "test_val"
external set_val : ikind -> int -> ikind = "set_val"


(*

class input_reader:
  object
    val file_name : string
    method set_file_name : string -> unit
    method get_tuple_array : unit -> instrn array
    method get_init_vals : unit -> string * string array
  end

class constraint_generator:
  object
    val output_file_name  : string
    method set_tuple_list : instr list -> unit 
    method set_init_vals  : string * string -> unit
    method generate_cnf_file : unit -> unit 
  end

*)

(*#print_depth 1000000;;
  #print_length 1000000;; *)


let genTransTotalOrder  = true (* false *)

let genClauseAnnotation = true (* false *)

let input_file : string  =  
   try  Sys.argv.(1)  with 
     Invalid_argument("index out of bounds") -> 
       begin print_string("Usage: mpec snapshot_file \n"); exit(0) end

let array_tTups =   
  let _ = gentuple_parse_input_file (input_file) in
     gentuple_get_tuple_array ()

let tName = input_file

let tLen  = Array.length(array_tTups) 


let start_values_list = 
[
  ("8d59ba","e304");  ("7dd9c4","bf6a");
  ("41cd2","9090");   ("88da18","1d85d0440415e304");
  ("733a74","415e304");
  ("2b5a34","ee76");
]

let start_values_ht = Hashtbl.create 30

let rec populate_start_values_ht svl =
  match svl with
      [] -> start_values_ht
    | (address,defval)::svl1 -> 
       let _ = Hashtbl.add start_values_ht address defval in
	 populate_start_values_ht svl1

let _ = populate_start_values_ht start_values_list








let transHT = Hashtbl.create 0 (* Enter pairs with known orderings *) (*-- automatic resizing will fix size --*)

let init_Trans() = Hashtbl.clear transHT

let var_at(i)(j) = j + (i-1)*tLen

let add_Trans(i)(j) = Hashtbl.add transHT (i,j) true

let in_Trans(i)(j) = Hashtbl.mem transHT (i,j)		     

let isWr(i) = let v = (array_tTups.(i-1)).op in (v = St || v = StRel)
		  
let isRd(i) = let v = (array_tTups.(i-1)).op in (v = Ld || v = LdAcq)
      
let wrID(i) = (array_tTups.(i-1)).wrID
    
let isLocalWrType(i) =  (array_tTups.(i-1)).wrType = Local
  
let isRemoteWrType(i) = (array_tTups.(i-1)).wrType = Remote
					      
let wrProc(i) = (array_tTups.(i-1)).wrProc
  
let proc(i) = (array_tTups.(i-1)).proc 
  
let pc(i) = (array_tTups.(i-1)).pc
	      
let isLdAcqOp(i) = (array_tTups.(i-1)).op = LdAcq
						  
let isMfOp(i) = (array_tTups.(i-1)).op = Mf
  
let isStRelOp(i) = (array_tTups.(i-1)).op = StRel
						  
let var(i) = (array_tTups.(i-1)).var

let data(i) = (array_tTups.(i-1)).data
	       
let reg(i) = (array_tTups.(i-1)).reg
	       
let useReg(i) = (array_tTups.(i-1)).useReg

let isAttrWb(i) = true (* assume *)

let isAttrUc(i) = false (* assume *)

let default_data(i) =  
  if   Hashtbl.mem (start_values_ht) (var(i))
  then Hashtbl.find (start_values_ht) (var(i))
  else "0"


(*******************************************************************)
(*   CNF clauses are encodes as integer lists. For example,        *)
(*   (4 or ~3) and (6) and (~7 or 3 or ~4) is encoded as           *)
(*    [[4 ;  -3] ;  [6] ;  [-7  ; 3   ; -4]].                      *)
(*                                                                 *)
(*******************************************************************)
   
type nvnc = { nv:int; nc:int} (* the number of vars and clauses allocated *)
type literal = int

(* a clause has a literal list and a constinfo for annotation *)
type clause   = literal list * constinfo
type cnffmla  = clause list
type gatetype = Andg | Org | Neqg | Impg | Notg
type gateinfo = {gatetype: gatetype; inputs: (literal * literal) ; output: literal}
type gate = gateinfo * constinfo

(* nvgen counts new vars generated in "al" + intermediate nodes for gates *)
type cnfRslt = 
    { nvgen: int;       andGs: gate list; orGs : gate list;   
      notGs: gate list; neqGs: gate list; impGs: gate list; gateOut : literal}


(*---------------------------------------------------------------------------*)
let rec gen_gate_clause(  {andGs = andGs; orGs = orGs; notGs = notGs; neqGs = neqGs; impGs = impGs} as cnf) =
  ( List.fold_right (@) ( List.map gen_andG_clauses (andGs)) []) @
  ( List.fold_right (@) ( List.map gen_orG_clauses  (orGs))  []) @
  ( List.fold_right (@) ( List.map gen_notG_clauses (notGs)) []) @
  ( List.fold_right (@) ( List.map gen_neqG_clauses (neqGs)) []) @
  ( List.fold_right (@) ( List.map gen_impG_clauses (impGs)) [])

(* Since we only have two-input and-gates, we can make the input list 
 * into an input pair and pattern-match
 *)

(*  Maybe we can handle special cases here and avoid generating 2-3 clauses always. 
 *  If |i1| == |i2|, then special cases i1 != i2, i1 == i2. Then again we need to do 
 *  this only if the SAT solver (zchaff) doesn't do this simplification right at the beginning 
 *)

and gen_andG_clauses  (gate_rec, ginfo) = 
      let i1 = fst(gate_rec.inputs) in
      let i2 = snd(gate_rec.inputs) in 
      let out = gate_rec.output     in 
         match gate_rec.gatetype with
           Andg ->  [ ([-i1; -i2; out], ginfo); 
		      ([-out; i1], ginfo);
		      ([-out; i2], ginfo) ]
         | _    ->  print_string "Error: inconsisten gate type !!!\n"; []

and gen_orG_clauses ({ gatetype = _;  inputs = (i1,i2); output = output} as  orG, ginfo) =
      match orG.gatetype with
        Org ->   [ ([i1; i2; -output],ginfo); 
		   ([output; -i1],ginfo); 
		   ([output; -i2],ginfo) ]
      | _   ->  print_string "Error: inconsisten gate type, Or gate expected!!!\n"; []

and gen_notG_clauses({gatetype = _; inputs = (i1,i2); output = output} as notG, ginfo) =
      match notG.gatetype with
        Notg -> [ ([i1; output],ginfo); ([-output; -i1],ginfo) ]
      | _    -> print_string "Error: inconsisten gate type, Not gate expected!!!\n"; []

and gen_neqG_clauses ({gatetype = _;  inputs = (i1,i2); output = output} as neqG, ginfo) =
      match neqG.gatetype with
        Neqg -> [ ([ i1;  i2; -output],ginfo); 
		  ([-i1; -i2; -output],ginfo); 
		  ([-i1;  i2;  output],ginfo); 
		  ([ i1; -i2;  output],ginfo) ]
      | _ -> print_string "Error: inconsisten gate type, NotEq gate expected!!!\n"; []

and gen_impG_clauses({gatetype = _; inputs = (i1,i2); output = output} as impG, ginfo) =
      match impG.gatetype with
         Impg -> [ ([i1; output],ginfo); 
		   ([-i2; output],ginfo); 
		   ([-i1; i2; -output],ginfo) ]
	| _   -> print_string "Error: inconsisten gate type, Imply gate expected!!!\n"; []

and gate_clauses2str_and_out(gate_clauses)(cchan) =
  match gate_clauses with
    | [] -> ()
    | (clause,{op1=op1;op2=op2;op3=op3;op4=op4;rule=rule})::gate_clauses1 ->
	(if genClauseAnnotation
	 then
	   (Printf.fprintf(cchan)("c op1 = %d; op2 = %d; op3 = %d; op4 = %d;")(op1)(op2)(op3)(op4);
	    match rule with
	      | Reflexive            -> Printf.fprintf(cchan)(" rule = %s\n")("Reflexive")
	      | TransitiveOrder      -> Printf.fprintf(cchan)(" rule = %s\n")("TransitiveOrder")
	      | TotalOrder           -> Printf.fprintf(cchan)(" rule = %s\n")("TotalOrder")
	      | WriteOperationOrder  -> Printf.fprintf(cchan)(" rule = %s\n")("WriteOperationOrder")
	      | ProgramOrder         -> Printf.fprintf(cchan)(" rule = %s\n")("ProgramOrder")
	      | MemoryDataDependence -> Printf.fprintf(cchan)(" rule = %s\n")("MemoryDataDependence")
	      | DataFlowDependence   -> Printf.fprintf(cchan)(" rule = %s\n")("DataFlowDependence")
	      | Coherence            -> Printf.fprintf(cchan)(" rule = %s\n")("Coherence")
	      | SequentialUC         -> Printf.fprintf(cchan)(" rule = %s\n")("SequentialUC")
	      | ReadValue            -> Printf.fprintf(cchan)(" rule = %s\n")("ReadValue")
	      | AtomicWBRelease      -> Printf.fprintf(cchan)(" rule = %s\n")("AtomicWBRelease")
	      | NoUCBypass           -> Printf.fprintf(cchan)(" rule = %s\n")("NoUCBypass")
	      | NoRule               -> Printf.fprintf(cchan)(" rule = %s\n")("NoRule")
	   )
	 else ();
	 let _ = List.map (fun (x) -> (Printf.fprintf(cchan)("%d ")(x))) (clause) in
	 let _ = Printf.fprintf(cchan)("0\n") in           (* dimacs lines end with zero *)
	   gate_clauses2str_and_out(gate_clauses1)(cchan)
	)

      
(*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Given a constraint formula, generate a
  tailor-made CNF fmla for it - basically via 2-input gates
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*)
let rec gen_cnf(qbf)(nvi) = 
  (match qbf with
     | Tt(cinfo) -> { 
              nvgen  = 1;  (* The "1" is one new var that models the OR-gate output *)
	      andGs  = [];
	      orGs   = [ ({gatetype=Org; inputs=(1, -1); output=nvi},cinfo) ];
	      notGs  = [];
	      neqGs  = [];
	      impGs  = [];
	      gateOut= nvi }

     | Ff(cinfo) -> {nvgen  = 1;  (* The "1" is one new var that models the AND-gate output *)
	      andGs  = [ ({gatetype=Andg; inputs=(1, -1); output=nvi},cinfo) ];
	      orGs   = [];
	      notGs  = [];
	      neqGs  = [];
	      impGs  = [];
	      gateOut=nvi}

     | Iv(n,cinfo) ->
             {nvgen  = 1;
	      andGs  = [];
	      orGs   = [ ({gatetype=Org; inputs=(n, n); output=nvi},cinfo) ];
	      notGs  = [];
	      neqGs  = [];
	      impGs  = [];
	      gateOut=nvi}

     | Not(q,cinfo) ->
	 let {nvgen=nvgen;
	      andGs=andGs;
	      orGs=orGs;
	      notGs=notGs;
	      neqGs=neqGs;
	      impGs=impGs;
	      gateOut=gateOut} as gatei = gen_cnf(q)(nvi) in

	 let nvi'   = nvgen + nvi    in (* This is where the output of the 'not' gate exists *)

	   {nvgen=nvgen + 1; (* One new var - for the not-gate output - generated *)
	    andGs=andGs;
	    orGs =orGs;
	    notGs= ({gatetype=Notg; inputs=(gateOut, -1); output=nvi'},cinfo) :: notGs ;
	    neqGs=neqGs;
	    impGs=impGs;
	    gateOut=nvi'}	   
	 
     | And(q1,q2,cinfo) | Or(q1,q2,cinfo) | Neq(q1,q2,cinfo) | Imp(q1,q2,cinfo) -> 
	 let {nvgen=nvgen1;
	      andGs=andGs1;
	      orGs=orGs1;
	      notGs=notGs1;
	      neqGs=neqGs1;
	      impGs=impGs1;
	      gateOut=gateOut1} as gatei1 = gen_cnf(q1)(nvi) in

	 let nvi'   = nvgen1 + nvi    in	   
	  
	 let {nvgen=nvgen2;
	      andGs=andGs2;
	      orGs=orGs2;
	      notGs=notGs2;
	      neqGs=neqGs2;
	      impGs=impGs2;	      
	      gateOut=gateOut2} as gatei2 = gen_cnf(q2)(nvi') in

	 let nvi'2    = nvgen2 + nvi'    in
	 let andGs12  = andGs1 @ andGs2  in
	 let orGs12   = orGs1  @ orGs2   in
	 let notGs12  = notGs1 @ notGs2  in
	 let neqGs12  = neqGs1 @ neqGs2  in
	 let impGs12  = impGs1 @ impGs2  in	   
	 let inputs12 = (gateOut1, gateOut2) in
	   
	   {nvgen=nvgen1 + nvgen2 + 1; (* One new var - for the x-gate output - generated *)
	    andGs= (match qbf with
		      | And(_,_,cinfo) -> ({gatetype=Andg; inputs=inputs12; output=nvi'2},cinfo) :: andGs12
		      | _ -> andGs12
		   );
	     orGs= (match qbf with
		      |  Or(_,_,cinfo) -> ({gatetype= Org; inputs=inputs12; output=nvi'2},cinfo) ::  orGs12
		      | _ ->  orGs12
		   );
		     
	    notGs=notGs12;
	     
	    neqGs= (match qbf with
		      |  Neq(_,_,cinfo) -> ({gatetype= Neqg; inputs=inputs12; output=nvi'2},cinfo) :: neqGs12
		      | _ ->  neqGs12
		   );
		     
	    impGs= (match qbf with
		      | Imp(_,_,cinfo) -> ({gatetype=Impg; inputs=inputs12; output=nvi'2},cinfo) :: impGs12
		      | _ -> impGs12
		   );
	    gateOut=nvi'2}	   
  )



(*---------------------------------------------------------------------------*)
(* Generate CNF string of formula + generate nv and nc                       *)
(*---------------------------------------------------------------------------*)

let cnfstr (cfmla) (vnext) (cchan) =
  (match cfmla with
     | Tt(_) -> {nv=0; nc=0}
     | _ ->
	 let cnf                      = gen_cnf(cfmla)(vnext)                       in
	 let {gateOut = final_output} = cnf                                         in
	 let gate_clauses             = ([final_output],{op1= -1;op2= -1;op3= -1;op4= -1;rule=NoRule}) ::
					  (gen_gate_clause cnf)     in
	 let n_clauses                = List.length gate_clauses                    in
	 let n_vars                   = final_output - vnext + 1                    in
	   
	 let _ = gate_clauses2str_and_out(gate_clauses)(cchan) in
	   {nv=n_vars; nc=n_clauses}
  )
  

(*---------------------------------------------------------------------------*)
(*       requireTotalOrder                                                   *)
(*---------------------------------------------------------------------------*)

let requireTotalOrder(i)(j)(vnext)(cchan) =
    if i <> j
    then
      let i1 = var_at(i)(j) in
      let i2 = var_at(j)(i) in

      let clauses = [ ([i1;i2],{op1=i;op2=j;op3= -1;op4= -1;rule=TotalOrder}) ] in	
      (* Emit or relation *)
      let _ = gate_clauses2str_and_out(clauses)(cchan) in
	(* 0 new vars and 1 new clause generated *)
	{nv=0; nc=1}
	
    else
      (* vars and clauses are 0 and 0 resply; nothing emitted to CNF file *)
      {nv=0; nc=0} 


(*---------------------------------------------------------------------------*)
(*       requireTransitiveOrder                                              *)
(*---------------------------------------------------------------------------*)

let rec requireTransitiveOrder1(i)(j)(k)(vnext)(cchan) =
  if k > tLen
  then (* 1 clause per tLen iterations this will go thru - so tLen clauses -- except it won't visit j=k;
	  0 vars    per tLen iterations this will go thru - so 0 vars
       *)
    { nv=0; nc=tLen-1 }
  else
    if ( j=k )
    then requireTransitiveOrder1(i)(j)(k+1)(vnext)(cchan)
    else

      if ( (in_Trans(j)(i)) or (in_Trans(k)(j)) )
      then
	requireTransitiveOrder1(i)(j)(k+1)(vnext)(cchan)

      else
	let ante1  = var_at(i)(j) in (* i-j condition *)
	let ante2  = var_at(j)(k) in (* j-k condition *)
	let conseq = var_at(i)(k) in (* i-k condition *)

	let clauses = [ ([-ante1; -ante2; conseq],{op1=i;op2=j;op3=k;op4= -1;rule=TransitiveOrder}) ] in
      
	(* Emit the transitivity constraints: i,j and j,k ==> i,k ordered *)
	let _ = gate_clauses2str_and_out(clauses)(cchan) in
	  requireTransitiveOrder1(i)(j)(k+1)(vnext)(cchan)


(*---------------------------------------------------------------------------*)
let requireTransitiveOrder(i)(j)(vnext)(cchan) =
  requireTransitiveOrder1(i)(j)(1)(vnext)(cchan)

(*---------------------------------------------------------------------------*)
(*       requireWriteOperationOrder                                          *)
(*---------------------------------------------------------------------------*)        
let requireWriteOperationOrder(i)(j)(vnext)(cchan) = 
  if (isWr(i) & isWr(j) & (wrID(i) = wrID(j))
      & ( ( isLocalWrType(i) & isRemoteWrType(j) & (wrProc(i) = wrProc(j)) )
	  ||
	  ( isRemoteWrType(i) & isRemoteWrType(j) & (wrProc(i) = proc(i)) & (wrProc(j) <> proc(i)) )
	) 
     )
  then
    let _ = add_Trans(i)(j) in
      
    (* simply generate i less than j wrt the i',j' positions below *)
    let constr_pos = var_at(i)(j) in

(*--new UC --*)
    let constr_pos' = var_at(j)(i) in
(*--new UC --*)
      
    let lt_clause  = [ ([ constr_pos ],{op1=i;op2=j;op3= -1;op4= -1;rule=WriteOperationOrder}) ]   in

(*--new UC --*)
    let lt_clause' = [ ([ -constr_pos' ],{op1=i;op2=j;op3= -1;op4= -1;rule=WriteOperationOrder}) ] in
(*--new UC --*)

    (* Emit that unit-clause *)
    let _          = gate_clauses2str_and_out(lt_clause)(cchan) in


(*--new UC --*)
    let _          = gate_clauses2str_and_out(lt_clause')(cchan) in      
(*--new UC --*)            
      
      (* generated no new vars, but one clause *)
(*--new UC {nv=0; nc=1}  --*){nv=0; nc=2} 
      
  else
    {nv=0; nc=0}

  
(*---------------------------------------------------------------------------*)
(*       requireProgramOrder                                                 *)
(*---------------------------------------------------------------------------*)

let orderedByProgram(i)(j) = (proc(i) = proc(j)) & ( pc(i) < pc(j) ) 

let orderedByAcquire(i)(j) = orderedByProgram(i)(j)  & isLdAcqOp(i) 

let orderedByFence (i) (j) = orderedByProgram(i)(j)  & ( isMfOp(i) || isMfOp(j) )

let orderedByRelease(i)(j) =
  orderedByProgram(i)(j) & isStRelOp(j)  & 
  (
     if isWr(i)
     then ((isLocalWrType(i) & isLocalWrType(j))
	   ||(isRemoteWrType(i) & isRemoteWrType(j) & (wrProc(i) = wrProc(j)) )
	  )
     else true
  )
    
let requireProgramOrder(i)(j)(vnext)(cchan) =
  if ( orderedByAcquire(i)(j) || orderedByRelease(i)(j) || orderedByFence(i)(j) )
  then
    let _          = add_Trans(i)(j) in
    let constr_pos = var_at(i)(j) in
    let lt_clause  = [ ([ constr_pos ],{op1=i;op2=j;op3= -1;op4= -1;rule=ProgramOrder}) ] in

(*--new UC --*)
    let constr_pos' = var_at(j)(i) in
    let lt_clause'  = [ ([ -constr_pos' ],{op1=i;op2=j;op3= -1;op4= -1;rule=ProgramOrder}) ] in      
(*--new UC --*)                  
      
    (* Emit that unit-clause *)
    let _          = gate_clauses2str_and_out(lt_clause)(cchan) in

(*--new UC --*)
    let _          = gate_clauses2str_and_out(lt_clause')(cchan) in
(*--new UC --*)      
      
      (* generated no new vars, but one clause *)
(*--new UC       {nv=0; nc=1} --*){nv=0; nc=2}
      
  else
    {nv=0; nc=0}    

(*---------------------------------------------------------------------------*)
(*       requireMemoryDataDependence                                         *)
(*---------------------------------------------------------------------------*)
let orderedByMemoryData(i)(j) =  orderedByProgram(i)(j) & var(i) = var(j) 
	       
let orderedByRAW(i)(j) = 
  orderedByMemoryData(i)(j) & isWr(i) & isRd(j)  & isLocalWrType(i) 

let orderedByWAR(i)(j) =
  orderedByMemoryData(i)(j) & isRd(i) & isWr(j)  & isLocalWrType(j) 
  
let orderedByWAW(i)(j) =
  orderedByMemoryData(i)(j) & isWr(i) & isWr(j)  
  & (  ( isLocalWrType(i) & isLocalWrType(j) )
     ||(isRemoteWrType(i) & isRemoteWrType(j) & (wrProc(i) = proc(i)) & (wrProc(j) = proc(i)))
    ) 

let requireMemoryDataDependence(i)(j)(vnext)(cchan) =
  if ( orderedByRAW(i)(j) || orderedByWAR(i)(j) || orderedByWAW(i)(j) )
  then
    let _          = add_Trans(i)(j) in    
    let constr_pos = var_at(i)(j)    in
    let lt_clause  = [ ([ constr_pos ],{op1=i;op2=j;op3= -1;op4= -1;rule=MemoryDataDependence}) ]                        in
(*--new UC --*)
    let constr_pos' = var_at(j)(i) in
    let lt_clause'  = [ ([ constr_pos' ],{op1=i;op2=j;op3= -1;op4= -1;rule=MemoryDataDependence}) ]                        in      
(*--new UC --*)            
      
    (* Emit that unit-clause *)
    let _          = gate_clauses2str_and_out(lt_clause)(cchan) in

(*--new UC --*)
    let _          = gate_clauses2str_and_out(lt_clause')(cchan) in      
(*--new UC --*)                  
      
      (* generated no new vars, but one clause *)
(*--new UC {nv=0; nc=1}  --*){nv=0; nc=2}                         
      
  else
    {nv=0; nc=0}    

    
(*---------------------------------------------------------------------------*)
(*       requireDataFlowDependence                                           *)
(*---------------------------------------------------------------------------*)
let orderedByLocalDependence(i)(j) =
  orderedByProgram(i)(j)
  & (reg(i) = reg(j))
  & ( (isRd(i) & isRd(j))
      || ( isWr(i) & isRd(j) & isLocalWrType(i) & useReg(i) )
      || ( isRd(i) & isWr(j) & isLocalWrType(j) & useReg(j) )
      || ( isWr(i) & isWr(j) & isLocalWrType(i) & isLocalWrType(j) & useReg(i) & useReg(j))
    ) 
      
let requireDataFlowDependence(i)(j)(vnext)(cchan) =
  if orderedByLocalDependence(i)(j)
  then

    let _          = add_Trans(i)(j) in    
    let constr_pos = var_at(i)(j) in
    let lt_clause  = [ ([ constr_pos ],{op1=i;op2=j;op3= -1;op4= -1;rule=DataFlowDependence}) ]                        in

(*--new UC --*)
    let constr_pos' = var_at(j)(i) in
    let lt_clause'  = [ ([ -constr_pos' ],{op1=i;op2=j;op3= -1;op4= -1;rule=DataFlowDependence}) ]                        in      
(*--new UC --*)                        
      
    (* Emit that unit-clause *)
    let _          = gate_clauses2str_and_out(lt_clause)(cchan) in

(*--new UC --*)
    let _          = gate_clauses2str_and_out(lt_clause')(cchan) in      
(*--new UC --*)
      
      (* generated no new vars, but one clause *)
(*--new UC {nv=0; nc=1} --*){nv=0; nc=2}
      
  else
    {nv=0; nc=0}    


(*---------------------------------------------------------------------------*)
(*       requireCoherence                                                    *)
(*---------------------------------------------------------------------------*)


(*---------------------------------------------------------------------------*)
(* Generate clauses corresp to the constraints                               *)
(*---------------------------------------------------------------------------*)        
let cohCondition1(i)(j)(p)(q) =
  if isWr(p) & isWr(q)  & (wrID(p) = wrID(i)) & (wrID(q) = wrID(j))
    & isRemoteWrType(p) & isRemoteWrType(q)   & (wrProc(p) = wrProc(q))
  then 
    let constr_pos = var_at(p)(q) in
      Iv(constr_pos,{op1=i;op2=j;op3=p;op4=q;rule=Coherence})
  else 
    Tt({op1=i;op2=j;op3=p;op4=q;rule=Coherence})
    

let rec cohCondition(i)(j)(p)(q)(constr) =
  if p=tLen & q > tLen then constr
  else if q > tLen then cohCondition(i)(j)(p+1)(1)(constr)
  else
    let c = cohCondition1(i)(j)(p)(q) in
      match c with
	| Tt(_) -> cohCondition(i)(j)(p)(q+1)(constr)
	| _ -> match constr with
	    | Tt(_) -> cohCondition(i)(j)(p)(q+1)(c)
	    | _ -> cohCondition(i)(j)(p)(q+1)(And(c, constr,{op1=i;op2=j;op3=p;op4=q;rule=Coherence}))
  

let requireCoherence(i)(j)(vnext)(cchan) =
  if ( isWr(i) & isWr(j) & (var(i) = var(j)) & (isAttrWb(i) || isAttrUc(i))
       & (  (isLocalWrType(i) & isLocalWrType(j) & (proc(i) = proc(j)) )
	  ||( isRemoteWrType(i) & isRemoteWrType(j) & (wrProc(i) = wrProc(j)) )
	 )
     )
  then
    let cc = cohCondition(i)(j)(1)(1)(Tt({op1=i;op2=j;op3= -1;op4= -1;rule=Coherence})) in
      match cc with 
	| Tt(_) -> {nv=0; nc=0}
	| _ -> 

 	let lt_constr     = var_at(i)(j) in
	let not_lt_constr = -lt_constr       in
	  match cc with 
	    | Ff(_) -> 
	    (* simply generate NOT(i less than j) wrt the i',j' positions below *)
	    let notlt_clause  = [ ([ not_lt_constr ],{op1=i;op2=j;op3= -1;op4= -1;rule=Coherence}) ] in

	    (* Emit that unit-clause *)
	    let _          = gate_clauses2str_and_out(notlt_clause)(cchan) in
	    
	      (* generated no new vars, but one clause *)
	      {nv=0; nc=1} 
	    
	    | _ -> cnfstr(Imp(Iv(lt_constr,{op1=i;op2=j;op3= -1;op4= -1;rule=Coherence}), cc,
                          {op1=i;op2=j;op3= -1;op4= -1;rule=Coherence}))(vnext)(cchan)
	    
  else
    {nv=0; nc=0}

(*---------------------------------------------------------------------------*)
(*       requireReadValue                                                    *)
(*---------------------------------------------------------------------------*)
let rec existsk7(j)(i7)(k7)(constr) =
  if k7 > tLen
  then constr
  else
    let c =
      if not(isWr(k7))
      then Ff({op1=j;op2=i7;op3=k7;op4= -1;rule=ReadValue})
      else
	if not(isLocalWrType(k7))
	then Ff({op1=j;op2=i7;op3=k7;op4= -1;rule=ReadValue})
	else
	  if not(var(k7) = var(j))
	  then Ff({op1=j;op2=i7;op3=k7;op4= -1;rule=ReadValue})
	  else
	    if not(proc(k7) = proc(j))
	    then Ff({op1=j;op2=i7;op3=k7;op4= -1;rule=ReadValue})
	    else
	      let lt_i7k7 = var_at(i7)(k7) in
	      let lt_k7j  = var_at(k7)(j)  in
		And(
		    Iv(lt_i7k7,{op1=j;op2=i7;op3=k7;op4= -1;rule=ReadValue}),
		    Iv(lt_k7j,{op1=j;op2=i7;op3=k7;op4= -1;rule=ReadValue}),
		    {op1=j;op2=i7;op3=k7;op4= -1;rule=ReadValue}
		   )
    in
      match c with
	| Ff(_) -> existsk7(j)(i7)(k7+1)(constr)
	| _ -> match constr with 
	    | Ff(_) -> existsk7(j)(i7)(k7+1)(c)
	    | _ -> existsk7(j)(i7)(k7+1)(Or(c,constr,{op1=j;op2=i7;op3=k7;op4= -1;rule=ReadValue}))

let rec existsValidLocalWr1(j)(i7)(constr) =
  if i7 > tLen
  then constr
  else
    let c =
      if not(isWr(i7))
      then Ff({op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue})
      else 
	if not(isLocalWrType(i7))
	then Ff({op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue})
	else
	  if not(var(i7) = var(j))
	  then Ff({op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue})
	  else
	    if not(proc(i7) = proc(j))
	    then Ff({op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue})
	    else
	      if not(data(i7) = data(j))
	      then Ff({op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue})
	      else
		let lt_i7j = var_at(i7)(j) in
		  And(
		       Iv(lt_i7j,{op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue}), 
		       Not(existsk7(j)(i7)(1)
			     (Ff({op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue})),
			     {op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue}
			  ),
		       {op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue}
		     )
    in
      match c with
	| Ff(_) -> existsValidLocalWr1(j)(i7+1)(constr)
	| _ -> match constr with 
	    | Ff(_) -> existsValidLocalWr1(j)(i7+1)(c)
	    | _ -> existsValidLocalWr1(j)(i7+1)(Or(c,constr,{op1=j;op2=i7;op3= -1;op4= -1;rule=ReadValue}))
		
let validLocalWr(j) = existsValidLocalWr1(j)(1)(Ff({op1=j;op2= -1;op3= -1;op4= -1;rule=ReadValue}))

let rec forallk8(j)(i8)(k8)(constr) =
  if k8 > tLen
  then constr
  else
    let c =
      if not(isWr(k8))
      then Tt({op1=j;op2=i8;op3=k8;op4= -1;rule=ReadValue})
      else
        if not(isRemoteWrType(k8))
	then Tt({op1=j;op2=i8;op3=k8;op4= -1;rule=ReadValue})
	else 
	  if not(var(k8) = var(j))
	  then Tt({op1=j;op2=i8;op3=k8;op4= -1;rule=ReadValue})
	  else
	    if not(wrProc(k8) = proc(j))
	    then Tt({op1=j;op2=i8;op3=k8;op4= -1;rule=ReadValue})
	    else
	      let not_i8k8 = -(var_at(i8)(k8))  in
	      let not_k8j  = -(var_at(k8)(j))   in

		Or(Iv(not_i8k8,{op1=j;op2=i8;op3=k8;op4= -1;rule=ReadValue}), 
		   Iv(not_k8j,{op1=j;op2=i8;op3=k8;op4= -1;rule=ReadValue}),
		  {op1=j;op2=i8;op3=k8;op4= -1;rule=ReadValue}
		  )

    in
      match c with 
	| Tt(_) -> forallk8(j)(i8)(k8+1)(constr)
	| _ -> match constr with
	    | Tt(_) -> forallk8(j)(i8)(k8+1)(c)
	    | _ -> forallk8(j)(i8)(k8+1)(And(c,constr,{op1=j;op2=i8;op3=k8;op4= -1;rule=ReadValue}))			


let existsValidRemoteWr2(j)(i8) =
  if not(isWr(i8))
  then Ff({op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue})
  else
    if not(isRemoteWrType(i8))
    then Ff({op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue})
    else
      if not(wrProc(i8) = proc(j))
      then Ff({op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue})
      else
	if not(var(i8) = var(j))
	then Ff({op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue})
	else
	  if not(data(i8) = data(j))
	  then Ff({op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue})
	  else
	    let not_ji8 = -(var_at(j)(i8))  in
	      
	      And(Iv(not_ji8,{op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue}), 
		  forallk8(j)(i8)(1)
		    (Tt({op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue})),
		  {op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue}
		 )

	      
let rec existsValidRemoteWr1(j)(i8)(constr) =
  if i8 > tLen
  then constr
  else
    let c = existsValidRemoteWr2(j)(i8) in
      match c with 
	| Ff(_) -> existsValidRemoteWr1(j)(i8+1)(constr)
	| _ -> match constr with
	    | Ff(_) -> existsValidRemoteWr1(j)(i8+1)(c)
	    | _ -> existsValidRemoteWr1(j)(i8+1)(Or(c,constr,{op1=j;op2=i8;op3= -1;op4= -1;rule=ReadValue}))
	  
let validRemoteWr(j) = existsValidRemoteWr1(j)(1)(Ff({op1=j;op2= -1;op3= -1;op4= -1;rule=ReadValue}))
    
let rec existsValidDefaultWr1(j)(i9)(constr) =
  if i9 > tLen
  then constr
  else
    let c =
      if not(isWr(i9))
      then Ff({op1=j;op2=i9;op3= -1;op4= -1;rule=ReadValue})
      else
	if not(var(i9) = var(j))
	then Ff({op1=j;op2=i9;op3= -1;op4= -1;rule=ReadValue})
	else
	  if
	    not
	      ( (isLocalWrType(i9) & (proc(i9) = proc(j)))
		||
		(isRemoteWrType(i9) & (wrProc(i9) = proc(j)))
	      )
	  then Ff({op1=j;op2=i9;op3= -1;op4= -1;rule=ReadValue})
	  else
	    let lt_i9j = (var_at(i9)(j))  in
	      
	      (Iv(lt_i9j,{op1=j;op2=i9;op3= -1;op4= -1;rule=ReadValue}))
	      
    in
      match c with
	| Ff(_) -> existsValidDefaultWr1(j)(i9+1)(constr)
	| _ -> match constr with
	    | Ff(_) -> existsValidDefaultWr1(j)(i9+1)(c)
	    | _ ->  existsValidDefaultWr1(j)(i9+1)(Or(c,constr,{op1=j;op2=i9;op3= -1;op4= -1;rule=ReadValue}))
	
let validDefaultWr(j) =
  if not(data(j) = default_data(j))
  then Ff({op1=j;op2= -1;op3= -1;op4= -1;rule=ReadValue})
  else Not(
            existsValidDefaultWr1(j)(1)(Ff({op1=j;op2= -1;op3= -1;op4= -1;rule=ReadValue})),
	    {op1=j;op2= -1;op3= -1;op4= -1;rule=ReadValue}
	  )

let invalidRd3(i)(ia)(ka) =
  ( isRd(ka) & (reg(ka) = reg(i)) & orderedByProgram(ia)(ka)  & orderedByProgram(ka)(i)  )

let rec invalidRd2(i)(ia)(ka)(constr) =
  if (ka > tLen)  then constr
  else
    let c = invalidRd3(i)(ia)(ka) in
      invalidRd2(i)(ia)(ka+1)(c || constr)

let validRd1(i)(ia) =
  if not(isRd(ia))
  then false
  else if not(reg(ia) = reg(i))
       then false
       else if not(orderedByProgram(ia)(i))
            then false
            else if not(data(ia) = data(i))
	         then false
   	         else not(invalidRd2(i)(ia)(1)(false))
	    
let rec validRd(i)(ia)(constr) =
  if (ia > tLen)
  then constr
  else
    let c = validRd1(i)(ia) in
      validRd(i)(ia+1)(c || constr)

let rdOrOther(i) =
  if isRd(i)
  then
    (Or(validLocalWr(i),
	Or(validRemoteWr(i),
	   validDefaultWr(i),{op1=i;op2= -1;op3= -1;op4= -1;rule=ReadValue}
	  ),{op1=i;op2= -1;op3= -1;op4= -1;rule=ReadValue}
       )
    )
  else Tt({op1=i;op2= -1;op3= -1;op4= -1;rule=ReadValue})    
	
let requireReadValue(i)(vnext)(cchan) =
  if (isWr(i) & useReg(i)) (* This is a read instruction if it "writes" a register *)
  then 
    if validRd(i)(1)(false)
    then cnfstr(rdOrOther(i))(vnext)(cchan)
    else cnfstr(Ff({op1=i;op2= -1;op3= -1;op4= -1;rule=ReadValue}))(vnext)(cchan)
  else cnfstr(rdOrOther(i))(vnext)(cchan)

(*---------------------------------------------------------------------------*)
(*       requireAtomicWBRelease                                              *)
(*---------------------------------------------------------------------------*)
let requireAtomicWBRelease3(ib)(jb)(kb) =
  if not(isStRelOp(kb))
  then Tt({op1=ib;op2=jb;op3=kb;op4= -1;rule=AtomicWBRelease})
  else
    if not(isRemoteWrType(kb))
    then Tt({op1=ib;op2=jb;op3=kb;op4= -1;rule=AtomicWBRelease})
    else
      if not(wrID(ib) = wrID(kb))
      then Tt({op1=ib;op2=jb;op3=kb;op4= -1;rule=AtomicWBRelease})
      else
	let not_lt_ibjb = var_at(ib)(jb) in
	let not_lt_jbkb = var_at(jb)(kb) in
	  
	  Not(
	       And(
		   Iv(not_lt_ibjb,{op1=ib;op2=jb;op3=kb;op4= -1;rule=AtomicWBRelease}), 
		   Iv(not_lt_jbkb,{op1=ib;op2=jb;op3=kb;op4= -1;rule=AtomicWBRelease}),
		   {op1=ib;op2=jb;op3=kb;op4= -1;rule=AtomicWBRelease}
		  ),
	       {op1=ib;op2=jb;op3=kb;op4= -1;rule=AtomicWBRelease}
	     )

let rec requireAtomicWBRelease2(ib)(jb)(kb)(constr) =
  if kb > tLen
  then constr
  else
    let c = requireAtomicWBRelease3(ib)(jb)(kb)
    in
      match c with 
	| Tt(_) -> requireAtomicWBRelease2(ib)(jb)(kb+1)(constr)
	| _ -> match constr with
	    | Tt(_) -> requireAtomicWBRelease2(ib)(jb)(kb+1)(c)
	    | _ -> requireAtomicWBRelease2(ib)(jb)(kb+1)(And(c,constr,{op1=ib;op2=jb;op3=kb;op4= -1;rule=AtomicWBRelease}))	    

let rec requireAtomicWBRelease1(ib)(jb)(constr) =
  if jb > tLen
  then constr
  else
    if (isStRelOp(jb) & isRemoteWrType(jb) & (wrID(jb) = wrID(ib)))
    then
      requireAtomicWBRelease1(ib)(jb+1)(constr)
    else
      let c = requireAtomicWBRelease2(ib)(jb)(1)(Tt({op1=ib;op2=jb;op3= -1;op4= -1;rule=AtomicWBRelease})) in
	match c with
	  | Tt(_) -> requireAtomicWBRelease1(ib)(jb+1)(constr)
	  | _ -> match constr with 
	      | Tt(_) -> requireAtomicWBRelease1(ib)(jb+1)(c)
	      | _ ->  requireAtomicWBRelease1(ib)(jb+1)(And(c,constr,{op1=ib;op2=jb;op3= -1;op4= -1;rule=AtomicWBRelease}))

  
let rec requireAtomicWBRelease(ib)(vnext)(cchan) = 
  if not(isStRelOp(ib))
  then {nv=0; nc=0}
  else 
    if not(isRemoteWrType(ib))
    then {nv=0; nc=0}
    else
      if not(isAttrWb(ib))
      then {nv=0; nc=0}
      else cnfstr(requireAtomicWBRelease1(ib)(1)(Tt({op1=ib;op2= -1;op3= -1;op4= -1;rule=AtomicWBRelease})))(vnext)(cchan)
  
(*---------------------------------------------------------------------------*)
(*       requireSequentialUC                                                 *)
(*---------------------------------------------------------------------------*)
let orderedByUC(i)(j) =
    orderedByProgram(i)(j)  &   isAttrUc(i)   &    isAttrUc(j)
    & ( ( isRd(i) & isRd(j) )   ||
        ( isRd(i) & isWr(j) & isLocalWrType(j) )   ||
        ( isWr(i) & isRd(j) & isLocalWrType(i) )   ||
        ( isWr(i) & isWr(j) & isLocalWrType(i) & isLocalWrType(j) ) )

let requireSequentialUC(i)(j)(vnext)(cchan) =
  if orderedByUC(i)(j)  then
    let _          = add_Trans(i)(j) in    
    let constr_pos = var_at(i)(j) in
    let lt_clause  = [ ([ constr_pos ],{op1=i;op2=j;op3= -1;op4= -1;rule=SequentialUC}) ]  in

(*--new UC --*)
    let constr_pos' = var_at(j)(i) in
    let lt_clause'  = [ ([ -constr_pos' ],{op1=i;op2=j;op3= -1;op4= -1;rule=SequentialUC}) ] in      
(*--new UC --*)      
      
    (* Emit that unit-clause *)
    let _          = gate_clauses2str_and_out(lt_clause)(cchan) in

(*--new UC --*)
    let _          = gate_clauses2str_and_out(lt_clause')(cchan) in      
(*--new UC --*)            
      
      (* generated no new vars, but one clause *)
(*--new UC {nv=0; nc=1} --*){nv=0; nc=2} 
      
  else
    {nv=0; nc=0}    

(*---------------------------------------------------------------------------*)
(*       requireNoUCBypass                                                   *)
(*---------------------------------------------------------------------------*)
let requireNoUCBypass3(ie)(je)(ke) =
  if not(isWr(ke))
  then Tt({op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass})
  else 
    if not(isRemoteWrType(ke))
    then Tt({op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass})
    else
      if not(wrProc(ke) = proc(ke))
      then Tt({op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass})
      else
	if not(wrID(ke) = wrID(ie))
	then Tt({op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass})
	else
	  if ( not(wrProc(ke) = proc(je)) || not(var(ie) = var(je)))
	  then Tt({op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass})
	  else
	    let lt_ieje = var_at(ie)(je) in
	    let lt_jeke = var_at(je)(ke) in
	      
	      Not(
		  And(
		      Iv(lt_ieje,{op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass}),
		      Iv(lt_jeke,{op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass}),
		      {op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass}
		     ),
		  {op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass}
		 )


let rec requireNoUCBypass2(ie)(je)(ke)(constr) =
  if ke > tLen
  then constr
  else
    let c = requireNoUCBypass3(ie)(je)(ke) in
      match c with
	| Tt(_) -> requireNoUCBypass2(ie)(je)(ke+1)(constr)
	| _ -> match constr with
	    | Tt(_) -> requireNoUCBypass2(ie)(je)(ke+1)(c)
	    | _ -> requireNoUCBypass2(ie)(je)(ke+1)(And(c,constr,{op1=ie;op2=je;op3=ke;op4= -1;rule=NoUCBypass}))

let rec requireNoUCBypass1(ie)(je)(constr) =
  if je > tLen
  then constr
  else
    if not(isRd(je))
    then
      requireNoUCBypass1(ie)(je+1)(constr)
    else
      let c = requireNoUCBypass2(ie)(je)(1)(Tt({op1=ie;op2=je;op3= -1;op4= -1;rule=NoUCBypass})) in
	match c with
	  | Tt(_) -> requireNoUCBypass1(ie)(je+1)(constr)
	  | _ -> match constr with
	      | Tt(_) -> requireNoUCBypass1(ie)(je+1)(c)
	      | _ -> requireNoUCBypass1(ie)(je+1)(And(c,constr,{op1=ie;op2=je;op3= -1;op4= -1;rule=NoUCBypass}))

let rec requireNoUCBypass(ie)(vnext)(cchan) =
  if not(isWr(ie))
  then {nv=0; nc=0}
  else
    if not(isLocalWrType(ie))
    then {nv=0; nc=0}
    else
      if not(isAttrUc(ie)) 
      then {nv=0; nc=0}
      else cnfstr(requireNoUCBypass1(ie)(1)(Tt({op1=ie;op2= -1;op3= -1;op4= -1;rule=NoUCBypass})))(vnext)(cchan)


(*---------------------------------------------------------------------------*)
(* cg - the "main" function - 'constraint gen'                               *)
(*---------------------------------------------------------------------------*)
let rec cg_main(i1)(i2)(vnext)(cchan)( {nv=nv; nc=nc} as nvnc ) =
  if i1=tLen & i2 > tLen
  then
    (* All iterations over; report accumulated allocation *)
    nvnc
  else
    if i2 > tLen
    then cg_main(i1+1)(1)(vnext)(cchan)(nvnc)
    else
      if (i1 = i2)
      then cg_main(i1)(i2+1)(vnext)(cchan)(nvnc)
      else
	let {nv=nv2; nc=nc2} =
	  if (genTransTotalOrder) then requireTotalOrder(i1)(i2)(vnext)(cchan)
	  else {nv=0; nc=0} in
	let vnext = vnext + nv2 in      
	  
	let {nv=nv3; nc=nc3} = requireWriteOperationOrder(i1)(i2)(vnext)(cchan) in
	let vnext = vnext + nv3 in
	  
	let {nv=nv4; nc=nc4} = requireProgramOrder(i1)(i2)(vnext)(cchan) in
	let vnext = vnext + nv4 in
	  
	let {nv=nv5; nc=nc5} = requireMemoryDataDependence(i1)(i2)(vnext)(cchan) in
	let vnext = vnext + nv5 in

	let {nv=nv6; nc=nc6} = requireDataFlowDependence(i1)(i2)(vnext)(cchan) in
	let vnext = vnext + nv6 in

	let {nv=nv7; nc=nc7} = requireCoherence(i1)(i2)(vnext)(cchan) in
	let vnext = vnext + nv7 in

	let {nv=nv8; nc=nc8} = requireSequentialUC(i1)(i2)(vnext)(cchan) in
	let vnext = vnext + nv8 in

	  cg_main (i1) (i2+1) (vnext) (cchan)( {nv=nv+nv2+nv3+nv4+nv5+nv6+nv7+nv8;
					        nc=nc+nc2+nc3+nc4+nc5+nc6+nc7+nc8} )

(*-------------------------------------------------------*)
(* Constraint gen for the atomicwb  rule                 *)
(*-------------------------------------------------------*)		  
let rec cg_atomicwb(i1)(vnext)(cchan)({nv=nv; nc=nc} as nvnc) =
  if i1 > tLen
  then nvnc
  else
    let {nv=nv1; nc=nc1} = requireAtomicWBRelease(i1)(vnext)(cchan) in
    let vnext = vnext + nv1                                         in
      cg_atomicwb(i1+1)(vnext)(cchan)({nv=nv+nv1; nc=nc+nc1})

(*-------------------------------------------------------*)
(* Constraint gen for the nouncaheable bypass rule       *)
(*-------------------------------------------------------*)		
let rec cg_noucb(i1)(vnext)(cchan)({nv=nv; nc=nc} as nvnc) =
  if i1 > tLen
  then nvnc
  else
    let {nv=nv1; nc=nc1} = requireNoUCBypass(i1)(vnext)(cchan) in
    let vnext = vnext + nv1                                    in
      cg_noucb(i1+1)(vnext)(cchan)({nv=nv+nv1; nc=nc+nc1})

(*-------------------------------------------------------*)
(* Constraint gen for the reflexive rule                 *)
(*-------------------------------------------------------*)	
let rec cg_reflexive (i)(vnext)(cchan) =
  if i > tLen
  then (* Done; report vars and clauses allocated.
	  No vars allocated; one clause per iteration - so tLen total *)
    {nv=0; nc=tLen}
  else 
    let refl_pos = var_at(i)(i) in
    let refl_clause = [ ([ -refl_pos ],{op1=i;op2= -1;op3= -1;op4= -1;rule=Reflexive}) ] in
    let _ = gate_clauses2str_and_out(refl_clause)(cchan) in
      cg_reflexive (i+1)(vnext)(cchan)

(*-------------------------------------------------------*)
(*        Constraint gen for read rule                   *)
(*-------------------------------------------------------*)		
let rec cg_read(i1)(vnext)(cchan)({nv=nv; nc=nc} as nvnc) =
  if i1 > tLen
  then nvnc
  else
    let {nv=nv1; nc=nc1} = requireReadValue(i1)(vnext)(cchan) in
    let vnext = vnext + nv1                                   in
      cg_read(i1+1)(vnext)(cchan)({nv=nv+nv1; nc=nc+nc1})


(*---------------------------------------------------------------------*)
(* Constraint gen for transitivity rule                                *)
(* This is "pulled out" of cg_main to allow us to call it "at the end" *)
(* to benefit from the max number of unit-clauses                      *)   
(*---------------------------------------------------------------------*)		
let rec cg_transitive(i1)(i2)(vnext)(cchan)({nv=nv;nc=nc} as nvnc) =
  if ( i1 = tLen & i2 > tLen )
  then 
    (* All iterations over; report accumulated allocation *)
    nvnc
  else
    if i2 > tLen
    then cg_transitive(i1+1)(1)(vnext)(cchan)(nvnc)
    else
      if (i1 = i2)
      then cg_transitive(i1)(i2+1)(vnext)(cchan)(nvnc)
      else
	let {nv=nv1; nc=nc1} =
	  if (genTransTotalOrder) then requireTransitiveOrder(i1)(i2)(vnext)(cchan)
	  else {nv=0; nc=0}
	in
	let vnext = vnext + nv1 in	  
	  cg_transitive(i1)(i2+1)(vnext)(cchan)({nv=nv+nv1; nc=nc+nc1})


(*******************************************************************)
(*   To generator the constraint cnf file, the path is cnf_file.   *)
(*   Two temporary file, bCNF_temp and pCNF_temp, are generated    *) 
(*   in  this process.                                             *)
(*******************************************************************)

let cnf_file = "/tmp/cnf_temp"

let cnf_generator() =
  let clause_file = "/tmp/bCNF_temp" in
  let summary_file = "/tmp/pCNF_temp"  in
  
  let _ = init_Trans() in  
  let _ = print_string "Generating CNF file ... \n "; flush stdout in 
  let cchan  = open_out ( clause_file ) in
  let vnext  = 1 + var_at (tLen) (tLen) in

  let _ = print_string "... reflextive rules ... \n "; flush stdout in   
  let {nv=nv1; nc=nc1} = cg_reflexive(1)(vnext)(cchan) in
  let vnext = vnext + nv1                              in
     
  let {nv=nv2; nc=nc2} = cg_main(1)(1)(vnext)(cchan)({nv=0;nc=0}) in
  let vnext = vnext + nv2                                         in
    
  let _ = print_string "... read rules ... \n "; flush stdout in   
  let {nv=nv3; nc=nc3} = cg_read(1)(vnext)(cchan)({nv=0;nc=0}) in
  let vnext = vnext + nv3                                      in
    
  let _ = print_string "... write ruls ... \n "; flush stdout in   
  let {nv=nv4; nc=nc4} = cg_atomicwb(1)(vnext)(cchan)({nv=0;nc=0}) in
  let vnext = vnext + nv4                                          in
    
  let _ = print_string "... uncacheable ruls ... \n "; flush stdout in   
  let {nv=nv5; nc=nc5} = cg_noucb(1)(vnext)(cchan)({nv=0;nc=0}) in
  let vnext = vnext + nv5                                       in

  let _ = print_string "... transtive ruls ... \n "; flush stdout in   
  let {nv=nv6; nc=nc6} = cg_transitive(1)(1)(vnext)(cchan)({nv=0;nc=0}) in
    
  let _ = close_out cchan                           in
  let nvars    = var_at(tLen)(tLen) + nv1 + nv2 + nv3 + nv4 + nv5 + nv6 in
  let nclauses =                      nc1 + nc2 + nc3 + nc4 + nc5 + nc6 in

  (* Emit the "problem" line into a different file - pCNF *)
    
  let pchan = open_out ( summary_file ) in
  let _ =  Printf.fprintf(pchan)("c CNF file for memory model test %s \n")(tName);
           Printf.fprintf(pchan)("c\n");
           Printf.fprintf(pchan)("p cnf %d %d \n")(nvars)(nclauses) in
  let _ =  close_out pchan  in
  let _ =  Unix.system ("cat " ^ summary_file ^ " " ^ clause_file ^ " > " ^ cnf_file) in
  let _ =  Unix.system ("rm -f " ^ summary_file ^ "  " ^ clause_file) in        
  let _ =  print_string "DONE\n"; flush stdout in
    ()
     
let _ = cnf_generator () 

(*****************************************************************)
(*  Use zchaff to solve the constraints                          *)
(*  If it is satisfiable, then give out a satisfiable instance   *)
(*  Otherwise, a unsat core will be extracted                    *)
(*      yu @ Oct 2, 2004                                         *)
(*****************************************************************)

let sat_model : int array = 
  let mpec_solver = Zchaff.zchaff_InitManager () in
  let _ = print_string "\nSolving the constraints with zchaff ..."; flush stdout in
  let sat_result =    (* 1 -- unsatistiable, 2 -- satisfiable *) 
       Zchaff.zchaff_ReadCnf (mpec_solver) (cnf_file);
       Zchaff.zchaff_Solve (mpec_solver) in
  let _ = print_string "done.\n"; flush stdout in

    if (sat_result = 2) then
       Zchaff.zchaff_GetVarAssign (mpec_solver)
    else [| |]

      
  (*
    print_string ("result =");
    print_int (sat_result);
    flush stdout
  *)



