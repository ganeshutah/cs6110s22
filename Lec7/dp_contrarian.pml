//
// spin -a dp_contrarian.pml
// gcc -O2 -w -o pan pan.c
// ./pan -a -m20000 /* no issues! */
//
//

mtype = {are_you_free, yes, no, release}

byte progress;
bit prop1, prop2, prop3;

proctype phil(chan lf, rf; int philno)
{
  //printf("pid is %d\n", _pid); // in case you need to know the PID, use this!
  // I knew which PIDs to expect, thus:
  //
  // init is always PID 0
  //
  // Then I ran the three phils, so they become PID 1,2,3
  // This is easier to debug!
  //
  // I fiddle with the prop1 and prop2 bits whenver that PID is scheduled
  //
  do
  ::
    if
    :: _pid==1 -> prop1=1; prop1=1 // fiddle with prop1 when PID=1 runs
    :: _pid==2 -> prop2=1; prop2=0 // fiddle with prop2 when PID=2 runs
    fi;
    do
    :: lf!are_you_free -> 
      if
        :: lf?yes -> break 
        :: lf?no
      fi
    od;
    do
    :: rf!are_you_free -> 
      if
        :: rf?yes -> 
 	    progress = 1; 
	    lf!release; 
	    rf!release;
 	    progress = 0; 
	    break
	    
        :: rf?no ->
	   lf!release;
	   break
      fi
    od
  od
}

proctype altphil(chan lf, rf; int philno)
{ // printf("pid is %d\n", _pid); 1 is the pid
  do
  ::
    if
    :: _pid==3 -> prop3=1; prop3=0 // I fiddle with the prop3 bits whenver that PID is scheduled
    fi;
    do
    :: rf!are_you_free ->
      if
        :: rf?yes -> break
        :: rf?no
      fi
    od;
    do
    :: lf!are_you_free ->
      if
        :: lf?yes ->
   	   progress = 1;
	   rf!release;
	   lf!release;
	   progress = 0;
	   break
	
        :: lf?no ->
	   rf!release;
   	   break
      fi
    od
  od
}


proctype fork(chan lp, rp)
{ do
  :: rp?are_you_free -> rp!yes -> 
    do
    :: lp?are_you_free -> lp!no
    :: rp?release -> break
    od
  :: lp?are_you_free -> lp!yes -> 
    do
    :: rp?are_you_free -> rp!no
    :: lp?release -> break
    od
  od
}

init {
      chan c0 = [0] of { mtype }; chan c1 = [0] of { mtype };
      chan c2 = [0] of { mtype }; chan c3 = [0] of { mtype };
      chan c4 = [0] of { mtype }; chan c5 = [0] of { mtype };
      atomic {
	run altphil(c5, c0, 0); // run phils first - just to debug easily
	run phil(c1, c2, 1);
	run phil(c3, c4, 2);
	
	run fork(c0, c1);      // then the forks
	run fork(c2, c3);
	run fork(c4, c5);
      }
}

/* Obtained from the LTL2BA tool : ran on their web-interface */
/* URL: http://www.lsv.fr/~gastin/ltl2ba/index.php */
/* SPIN BA generation takes forever */

// Property desired is
// []<>prop1 && []<>!prop1 &&  []<>prop2 && []<>!prop2 &&  []<>prop3 && []<>!prop3 -> !([]<>progress)  
// I negated it manually and obtained never and pasted it in!
//
// Thus, "never have all processes running inf. often AND have a hosed progress!
//
// This is true! never does not accept!


never { /* (   []<>prop1 && []<>!prop1 &&  []<>prop2 && []<>!prop2 &&  []<>prop3 && []<>!prop3 && (<>[]!progress) )  */
T0_init :    /* init */
	if
	:: (1) -> goto T0_init
	:: (!progress) -> goto T0_S55
	fi;
T0_S55 :    /* 1 */
	if
	:: (!progress) -> goto T0_S55
	:: (prop1 && !progress) -> goto T1_S55
	fi;
T1_S55 :    /* 2 */
	if
	:: (!progress) -> goto T1_S55
	:: (!prop1 && !progress) -> goto T2_S55
	:: (!prop1 && prop2 && !progress) -> goto T3_S55
	fi;
T2_S55 :    /* 3 */
	if
	:: (!progress) -> goto T2_S55
	:: (prop2 && !progress) -> goto T3_S55
	fi;
T3_S55 :    /* 4 */
	if
	:: (!progress) -> goto T3_S55
	:: (!prop2 && !progress) -> goto T4_S55
	:: (!prop2 && prop3 && !progress) -> goto T5_S55
	fi;
T4_S55 :    /* 5 */
	if
	:: (!progress) -> goto T4_S55
	:: (prop3 && !progress) -> goto T5_S55
	fi;
T5_S55 :    /* 6 */
	if
	:: (!progress) -> goto T5_S55
	:: (!prop3 && !progress) -> goto accept_S55
	fi;
accept_S55 :    /* 7 */
	if
	:: (!progress) -> goto T0_S55
	:: (prop1 && !progress) -> goto T1_S55
	fi;
}

// -- phew!
