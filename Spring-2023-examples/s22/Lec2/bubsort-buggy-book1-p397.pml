/* CEATL book's bubsort */
/* Run instructions at the end */

#define Size 5
#define aMinIndx 1
#define aMaxIndx (Size-1)

/* Gonna "waste" a[0] because Sedgewick uses 1-based arrays */

active proctype bubsort()
{ byte j, t;   /* Init to 0 by SPIN     */
  bit a[Size]; /* Use 1-bit abstraction */
  
  /* Nondeterministic array initialization */
  
  do ::break ::a[1]=1 ::a[2]=1 ::a[3]=1 ::a[4]=1 od;

  t=a[aMinIndx]; j=aMinIndx+1;
  
   do /* First ‘‘repeat’’ iteration */
   :: (j >(aMaxIndx)) -> break /*-- For-loop exits --*/
   :: (j<=(aMaxIndx)) ->
   if
   :: (a[j-1] > a[j]) -> t=a[j-1]; a[j-1]=a[j]; a[j]=t
   :: (a[j-1] <= a[j])
   fi;
   j++
   od;

   do /* Subsequent ‘‘repeat’’ iterations */
   :: t!=a[1] ->
      t=a[aMinIndx]; j=aMinIndx+1;
      do
      :: (j > (aMaxIndx)) -> break /*-- For-loop exits --*/
      :: (j<=(aMaxIndx)) ->
        if
        :: (a[j-1] > a[j]) -> t=a[j-1]; a[j-1]=a[j]; a[j]=t
	:: (a[j-1] <= a[j])
        fi;
        j++ /*-- for-index increments --*/
      od  /*-- end of for-loop --*/
  :: t==a[1] -> break
  od;
   
  t=1; /*-- Comb from location-1 to look for sortedness --*/
  do
  :: t < aMaxIndx-1 -> t++
  :: t > aMinIndx   -> t--
  :: a[t] > a[t+1]  -> assert(0) /*- announce there is a bug! -*/
  od
}
   
/*---

[ganesh@vpn-155-94-98-155 soln]$ spin -a bubsort-buggy-book1-p397.pml
[ganesh@vpn-155-94-98-155 soln]$ gcc -DMEMLIM=1024 -O2 -w -o pan pan.c
[ganesh@vpn-155-94-98-155 soln]$ ./pan

pan:1: assertion violated 0 (at depth 11)
pan: wrote bubsort-buggy-book1-p397.pml.trail

(Spin Version 6.4.5 -- 1 January 2016)
Warning: Search not completed
	+ Partial Order Reduction

Full statespace search for:
	never claim         	- (none specified)
	assertion violations	+
	acceptance   cycles 	- (not selected)
	invalid end states	+

State-vector 20 byte, depth reached 20, errors: 1
       44 states, stored
        7 states, matched
       51 transitions (= stored+matched)
        0 atomic steps
hash conflicts:         0 (resolved)

Stats on memory usage (in Megabytes):
    0.002	equivalent memory usage for states (stored*(State-vector + overhead))
    0.287	actual memory usage for states
  128.000	memory used for hash table (-w24)
    0.534	memory used for DFS stack (-m10000)
  128.730	total actual memory usage


pan: elapsed time 0 seconds

[ganesh@vpn-155-94-98-155 soln]$ spin --help
use: spin [-option] ... [-option] file
	Note: file must always be the last argument
	-A apply slicing algorithm
	-a generate a verifier in pan.c
	-B no final state details in simulations
	-b don't execute printfs in simulation
	-C print channel access info (combine with -g etc.)
	-c columnated -s -r simulation output
	-d produce symbol-table information
	-Dyyy pass -Dyyy to the preprocessor
	-Eyyy pass yyy to the preprocessor
	-e compute synchronous product of multiple never claims (modified by -L)
	-f "..formula.."  translate LTL into never claim
	-F file  like -f, but with the LTL formula stored in a 1-line file
	-g print all global variables
	-h at end of run, print value of seed for random nr generator used
	-i interactive (random simulation)
	-I show result of inlining and preprocessing
	-J reverse eval order of nested unlesses
	-jN skip the first N steps in simulation trail
	-k fname use the trailfile stored in file fname, see also -t
	-L when using -e, use strict language intersection
	-l print all local variables
	-M generate msc-flow in tcl/tk format
	-m lose msgs sent to full queues
	-N fname use never claim stored in file fname
	-nN seed for random nr generator
	-O use old scope rules (pre 5.3.0)
	-o1 turn off dataflow-optimizations in verifier
	-o2 don't hide write-only variables in verifier
	-o3 turn off statement merging in verifier
	-o4 turn on rendezvous optiomizations in verifier
	-o5 turn on case caching (reduces size of pan.m, but affects reachability reports)
	-o6 revert to the old rules for interpreting priority tags (pre version 6.2)
	-o7 revert to the old rules for semi-colon usage (pre version 6.3)
	-Pxxx use xxx for preprocessing
	-p print all statements
	-pp pretty-print (reformat) stdin, write stdout
	-qN suppress io for queue N in printouts
	-r print receive events
	-replay  replay an error trail-file found earlier
		if the model contains embedded c-code, the ./pan executable is used
		otherwise spin itself is used to replay the trailfile
		note that pan recognizes different runtime options than spin itself
	-search  (or -run) generate a verifier, and compile and run it
	      options before -search are interpreted by spin to parse the input
	      options following a -search are used to compile and run the verifier pan
		    valid options that can follow a -search argument include:
		    -bfs	perform a breadth-first search
		    -bfspar	perform a parallel breadth-first search
		    -bcs	use the bounded-context-switching algorithm
		    -bitstate	or -bit, use bitstate storage
		    -biterate	use bitstate with iterative search refinement (-w18..-w35)
		    -swarmN,M like -biterate, but running all iterations in parallel
				perform N parallel runs and increment -w every M runs
				default value for N is 10, default for M is 1
		    -link file.c  link executable pan to file.c
		    -collapse	use collapse state compression
		    -hc  	use hash-compact storage
		    -noclaim	ignore all ltl and never claims
		    -p_permute	use process scheduling order random permutation
		    -p_rotateN	use process scheduling order rotation by N
		    -p_reverse	use process scheduling order reversal
		    -ltl p	verify the ltl property named p
		    -safety	compile for safety properties only
		    -i	    	use the dfs iterative shortening algorithm
		    -a	    	search for acceptance cycles
		    -l	    	search for non-progress cycles
		similarly, a -D... parameter can be specified to modify the compilation
		and any valid runtime pan argument can be specified for the verification
	-S1 and -S2 separate pan source for claim and model
	-s print send events
	-T do not indent printf output
	-t[N] follow [Nth] simulation trail, see also -k
	-Uyyy pass -Uyyy to the preprocessor
	-uN stop a simulation run after N steps
	-v verbose, more warnings
	-w very verbose (when combined with -l or -g)
	-[XYZ] reserved for use by xspin interface
	-V print version number and exit

[ganesh@vpn-155-94-98-155 soln]$ spin -p -t bubsort-buggy-book1-p397.pml
using statement merging
  1:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:13 (state 2)	[a[1] = 1]
  2:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:13 (state 3)	[a[2] = 1]
  3:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:13 (state 1)	[goto :b0]
  3:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:15 (state 9)	[t = a[1]]
  3:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:15 (state 10)	[j = (1+1)]
  4:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:19 (state 13)	[((j<=(5-1)))]
  5:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:22 (state 18)	[((a[(j-1)]<=a[j]))]
  5:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:24 (state 21)	[j = (j+1)]
  6:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:19 (state 13)	[((j<=(5-1)))]
  7:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:21 (state 14)	[((a[(j-1)]>a[j]))]
  7:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:21 (state 15)	[t = a[(j-1)]]
  7:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:21 (state 16)	[a[(j-1)] = a[j]]
  7:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:21 (state 17)	[a[j] = t]
  7:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:24 (state 21)	[j = (j+1)]
  8:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:19 (state 13)	[((j<=(5-1)))]
  9:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:21 (state 14)	[((a[(j-1)]>a[j]))]
  9:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:21 (state 15)	[t = a[(j-1)]]
  9:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:21 (state 16)	[a[(j-1)] = a[j]]
  9:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:21 (state 17)	[a[j] = t]
  9:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:24 (state 21)	[j = (j+1)]
 10:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:18 (state 11)	[((j>(5-1)))]
 11:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:39 (state 42)	[((t==a[1]))]
 11:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:42 (state 47)	[t = 1]
 12:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:46 (state 52)	[((a[t]>a[(t+1)]))]
spin: bubsort-buggy-book1-p397.pml:46, Error: assertion violated
spin: text of failed assertion: assert(0)
 12:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:46 (state 53)	[assert(0)]
spin: trail ends after 12 steps
#processes: 1
 12:	proc  0 (bubsort:1) bubsort-buggy-book1-p397.pml:43 (state 54)
1 process created
[ganesh@vpn-155-94-98-155 soln]$

--*/


