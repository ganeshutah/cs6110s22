/* A distributed locking protocol - written by Ratan Nalumasu in 1995. */
/* See our CHDL'95 paper for a description. */

#define Nprocs 4
#define Nprocs_1 3

#define PID byte

bit mutex[Nprocs];
PID po[Nprocs];
chan request[Nprocs] = [Nprocs_1] of {PID};
chan q_len_ch[Nprocs] = [1] of {byte};
chan waiters[Nprocs] = [Nprocs_1] of {PID};
byte qlen[Nprocs];   /* How many are waiting? */
bit locked[Nprocs];

byte pid_acq[Nprocs]; /* PID of acquire process  */

proctype acquire(PID me)
{
    PID thread;
    byte count;
    loop:
        /* Try to acquire the lock */
        atomic { mutex[me] == 0 ->
                 mutex[me] = 1
        };
        if
        :: po[me] == me -> locked[me] = 1;   
                            mutex[me] = 0
        :: po[me] != me ->
                        request[po[me]]!me;
                        mutex[me] = 0;
                        /* Wait until reply comes back */
                        atomic {q_len_ch[me] ? [count] && mutex[me]==0 ->
                                po[me] = me;
                                mutex[me] = 1;
                                q_len_ch[me] ? count;
					assert(qlen[me]==0);
                                qlen[me] = qlen[me]+count;
                                count = 0  // reset dead variable to unique value to prevent un-necessary state explosion!!
                        };
                        ( len(waiters[me]) == qlen[me] ); // suspend till I actually receive the promised
			  		      	          // number of waiters sent by "the other party"
							  // into the waiters[me] queue. This queue is presumably
							  // going up monotonically in response to the sends that
							  // someone else is doing...
                        locked[me] = 1; // now I've received all the waiters for the resouce, and I'm now taking the resource
                        mutex[me]=0
        fi;
/*  */
                        /* Acquire continued */
    release:
        
        atomic {
            mutex[me] == 0 -> // take this mutex so that we don't race on locked[me] with my own handle proc
            locked[me] = 0;
            mutex[me] = 1
        };
        if
        :: qlen[me] ->			// C convention; qlen[me] > 0 means "true" so that this guard can fire!
	   	    			// This means that I now need to send the waiters to the
					// "new probable owner" (i.e. the real owner)


            waiters[me] ? po[me];	// This means:
	    		  		// 1) there is a pile of waiters that have come into
					// the waiters[me] queue. Remember, this is the "promised amount of waiters".
					//
					// So I'm gonna pass the "locked" privilege to the "head waiter"
					// Thus I'll now know that the "probable owner" (i.e. real owner) is
					// the head waiter.
					//
					// So I might as well set my po[me] variable to point to the "new owner"

	    
            qlen[me] = qlen[me]-1;	// Now I have sucked up one waiter; so the qlen count must go down by 1
	    
            q_len_ch[po[me]] ! qlen[me]; // Tell the 'new party' the count to expect!
	    		       		 // which is one less because the lock was assigned to the 'new party'
					 // Its waiters is only one less now
	    
            do
            :: qlen[me] -> atomic {     // repeat till I've sent the waiter info till qlen[me] goes down to 0
                waiters[me] ? thread;   // this "thread" is just a local variable
			      		// happens to be the next waiter to be forwarded along
					// in this loop
					
                waiters[po[me]] ! thread; // "That process" (new owner of the locked privilege) will now
				  	  // get this "thread"
					  
                thread = 0;		  // hey, I'm paranoid about dead-variable clean-up!
                qlen[me] = qlen[me]-1
               }
            :: !qlen[me] -> break	// aha, all the waiters have now been sent to the 'new party'
            od
        :: !qlen[me] -> skip		// no waiters were forwarded to me, so I don't need to pass on the waiters
        fi;
        mutex[me] = 0;

	// a lot of time can pass here
	// meanwhile those processes with outdated knowledge of the probable
	// owner may still think that I have the "locked" privilege
	// So in this interim, qlen[me] can very well grow!
	
        assert (qlen[me] >= 0);		// eh? I thought it would be 0 now. why >= 0 ??
    goto loop
}


/*  */

proctype handle(int me)
{
    PID requester;
    do
    :: atomic {
            mutex[me] == 0 && request[me]?[requester]
                 ->
            mutex[me] = 1;
            request[me] ? requester
        };
        if
        :: po[me] != me -> request[po[me]] ! requester
        :: po[me] == me && locked[me] ->
                    waiters[me] ! requester;
                    qlen[me] = qlen[me] + 1;
                    assert (qlen[me] < Nprocs)
        :: po[me] == me && !locked[me] ->
                    q_len_ch[requester] ! 0;
                    assert (qlen[me] == 0);
                    po[me] = requester
        fi;
        mutex[me] = 0
    od
}

/*  */

init {
    byte cnt;

    atomic {
        do
        :: cnt == Nprocs-> break
        :: cnt != Nprocs->
        	      pid_acq[cnt] = run acquire(cnt);
                      run handle(cnt);
                      cnt = cnt+1
        od
    }
}

#define at_release_0   (acquire[pid_acq[0]]@release)
#define at_release_1   (acquire[pid_acq[1]]@release)
#define at_release_2   (acquire[pid_acq[2]]@release)
#define at_release_3   (acquire[pid_acq[3]]@release)

	/*
	 * Formula As Typed: ([]  <>  at_release_0) || ([]  <>  at_release_1) || ([]  <>  at_release_2) || ([]  <>  at_release_3)
	 * The Never Claim Below Corresponds
	 * To The Negated Formula !(([]  <>  at_release_0) || ([]  <>  at_release_1) || ([]  <>  at_release_2) || ([]  <>  at_release_3))
	 * (formalizing violations of the original)
	 */

never {    /* !(([]  <>  at_release_0) || ([]  <>  at_release_1) || ([]  <>  at_release_2) || ([]  <>  at_release_3)) */
T0_init:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1932
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2201
	:: (! ((at_release_0)) && ! ((at_release_1))) -> goto T0_S1512
	:: (! ((at_release_0)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2214
	:: (! ((at_release_0)) && ! ((at_release_2))) -> goto T0_S1956
	:: (! ((at_release_0)) && ! ((at_release_3))) -> goto T0_S2225
	:: (! ((at_release_0))) -> goto T0_S845
	:: (! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2246
	:: (! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1988
	:: (! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2257
	:: (! ((at_release_1))) -> goto T0_S1580
	:: (! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2276
	:: (! ((at_release_2))) -> goto T0_S2024
	:: (! ((at_release_3))) -> goto T0_S2293
	:: (1) -> goto T0_init
	fi;
accept_S2193:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	fi;
T0_S845:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1932
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2201
	:: (! ((at_release_0)) && ! ((at_release_1))) -> goto T0_S1512
	:: (! ((at_release_0)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2214
	:: (! ((at_release_0)) && ! ((at_release_2))) -> goto T0_S1956
	:: (! ((at_release_0)) && ! ((at_release_3))) -> goto T0_S2225
	:: (! ((at_release_0))) -> goto T0_S845
	fi;
T0_S1512:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1932
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2201
	:: (! ((at_release_0)) && ! ((at_release_1))) -> goto T0_S1512
	fi;
T0_S1580:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1932
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2201
	:: (! ((at_release_0)) && ! ((at_release_1))) -> goto T0_S1512
	:: (! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2246
	:: (! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1988
	:: (! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2257
	:: (! ((at_release_1))) -> goto T0_S1580
	fi;
T0_S1932:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1932
	fi;
T0_S1956:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1932
	:: (! ((at_release_0)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2214
	:: (! ((at_release_0)) && ! ((at_release_2))) -> goto T0_S1956
	fi;
T0_S1988:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1932
	:: (! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2246
	:: (! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1988
	fi;
T0_S2024:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1932
	:: (! ((at_release_0)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2214
	:: (! ((at_release_0)) && ! ((at_release_2))) -> goto T0_S1956
	:: (! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2246
	:: (! ((at_release_1)) && ! ((at_release_2))) -> goto T0_S1988
	:: (! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2276
	:: (! ((at_release_2))) -> goto T0_S2024
	fi;
T0_S2201:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2201
	fi;
T0_S2214:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2214
	fi;
T0_S2225:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2201
	:: (! ((at_release_0)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2214
	:: (! ((at_release_0)) && ! ((at_release_3))) -> goto T0_S2225
	fi;
T0_S2246:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2246
	fi;
T0_S2257:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2201
	:: (! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2246
	:: (! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2257
	fi;
T0_S2276:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2214
	:: (! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2246
	:: (! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2276
	fi;
T0_S2293:
	if
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto accept_S2193
	:: (! ((at_release_0)) && ! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2201
	:: (! ((at_release_0)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2214
	:: (! ((at_release_0)) && ! ((at_release_3))) -> goto T0_S2225
	:: (! ((at_release_1)) && ! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2246
	:: (! ((at_release_1)) && ! ((at_release_3))) -> goto T0_S2257
	:: (! ((at_release_2)) && ! ((at_release_3))) -> goto T0_S2276
	:: (! ((at_release_3))) -> goto T0_S2293
	fi;
}

#ifdef NOTES
Use Load to open a file or a template.
#endif
#ifdef RESULT

#endif

