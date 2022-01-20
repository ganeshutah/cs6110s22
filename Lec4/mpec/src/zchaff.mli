type zchaff_solver
type zcore_database
type clause = int array
external zchaff_InitManager : unit -> zchaff_solver = "zchaff_InitManager"
external zchaff_ReleaseManager : zchaff_solver -> unit
  = "zchaff_ReleaseManager"
external zchaff_SetNumVariables : zchaff_solver -> int -> unit
  = "zchaff_SetNumVariables"
external zchaff_AddVariable : zchaff_solver -> int = "zchaff_AddVariable"
external zchaff_EnableVarBranch : zchaff_solver -> int -> unit
  = "zchaff_EnableVarBranch"
external zchaff_DisableVarBranch : zchaff_solver -> int -> unit
  = "zchaff_DisableVarBranch"
external zchaff_AddClause : zchaff_solver -> clause -> int -> unit
  = "zchaff_AddClause"
external zchaff_DeleteClauseGroup : zchaff_solver -> int -> unit
  = "zchaff_DeleteClauseGroup"
external zchaff_Reset : zchaff_solver -> unit = "zchaff_Reset"
external zchaff_MergeClauseGroup : zchaff_solver -> int -> int -> int
  = "zchaff_MergeClauseGroup"
external zchaff_AllocClauseGroupID : zchaff_solver -> int
  = "zchaff_AllocClauseGroupID"
external zchaff_IsSetClauseGroupID : zchaff_solver -> int -> int -> int
  = "zchaff_IsSetClauseGroupID"
external zchaff_SetClauseGroupID : zchaff_solver -> int -> int
  = "zchaff_SetClauseGroupID"
external zchaff_ClearClauseGroupID : zchaff_solver -> int -> int
  = "zchaff_ClearClauseGroupID"
external zchaff_GetVolatileGroupID : zchaff_solver -> int
  = "zchaff_GetVolatileGroupID"
external zchaff_GlobalGroupID : zchaff_solver -> int
  = "zchaff_GetGlobalGroupID"
external zchaff_SetTimeLimit : zchaff_solver -> float -> unit
  = "zchaff_SetTimeLimit"
external zchaff_SetMemLimit : zchaff_solver -> int -> unit
  = "zchaff_SetMemLimit"
external zchaff_GetVarAsgnment : zchaff_solver -> int -> int
  = "zchaff_GetVarAsgnment"
external zchaff_Solve : zchaff_solver -> int = "zchaff_Solve"
external zchaff_SetRandomness : zchaff_solver -> int -> unit
  = "zchaff_SetRandomness"
external zchaff_SetRandSeed : zchaff_solver -> int -> unit
  = "zchaff_SetRandSeed"
external zchaff_MakeDecision : zchaff_solver -> int -> int -> unit
  = "zchaff_MakeDecision"
external zchaff_EstimateMemUsage : zchaff_solver -> int
  = "zchaff_EstimateMemUsage"
external zchaff_GetElapseCPUTime : zchaff_solver -> float
  = "zchaff_GetElapseCPUTime"
external zchaff_GetCurrentCPUTime : zchaff_solver -> float
  = "zchaff_GetCurrentCPUTime"
external zchaff_GetCPUTime : zchaff_solver -> float = "zchaff_GetCPUTime"
external zchaff_NumLiteral : zchaff_solver -> int = "zchaff_NumLiterals"
external zchaff_NumClauses : zchaff_solver -> int = "zchaff_NumClauses"
external zchaff_NumVariables : zchaff_solver -> int = "zchaff_NumVariables"
external zchaff_InitNumLiterals : zchaff_solver -> int64
  = "zchaff_InitNumLiterals"
external zchaff_InitNumClauses : zchaff_solver -> int
  = "zchaff_InitNumClauses"
external zchaff_NumAddedLiterals : zchaff_solver -> int64
  = "zchaff_NumAddedLiterals"
external zchaff_NumAddedClauses : zchaff_solver -> int
  = "zchaff_NumAddedClauses"
external zchaff_NumDeletedClauses : zchaff_solver -> int
  = "zchaff_NumDeletedClauses"
external zchaff_NumDecisions : zchaff_solver -> int = "zchaff_NumDecisions"
external zchaff_NumImplications : zchaff_solver -> int64
  = "zchaff_NumImplications"
external zchaff_MaxDLevel : zchaff_solver -> int = "zchaff_MaxDLevel"
external zchaff_AverageBubbleMove : zchaff_solver -> float
  = "zchaff_AverageBubbleMove"
external zchaff_GetFirstClause : zchaff_solver -> int
  = "zchaff_GetFirstClause"
external zchaff_GetClauseLits : zchaff_solver -> int -> int -> unit
  = "zchaff_GetClauseLits"
external zchaff_EnableConfClsDeletion : zchaff_solver -> unit
  = "zchaff_EnableConfClsDeletion"
external zchaff_DisableConfClsDeletion : zchaff_solver -> unit
  = "zchaff_DisableConfClsDeletion"
external zchaff_SetClsDeletionInterval : zchaff_solver -> int -> unit
  = "zchaff_SetClsDeletionInterval"
external zchaff_SetMaxUnrelevance : zchaff_solver -> int -> unit
  = "zchaff_SetMaxUnrelevance"
external zchaff_SetMinClsLenForDelete : zchaff_solver -> int -> unit
  = "zchaff_SetMinClsLenForDelete"
external zchaff_SetMaxConfClsLenAllowed : zchaff_solver -> int -> unit
  = "zchaff_SetMaxConfClsLenAllowed"
external zchaff_ReadCnf : zchaff_solver -> string -> unit = "zchaff_ReadCnf"
external zchaff_HandleResult : zchaff_solver -> int -> string -> unit
  = "zchaff_HandleResult"
external zchaff_OutputStatus : zchaff_solver -> unit = "zchaff_OutputStatus"
external zchaff_VerifySolution : zchaff_solver -> unit
  = "zchaff_VerifySolution"
val zchaff_GetVarAssign : zchaff_solver -> int array
