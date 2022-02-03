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
                                count = 0
                        };
                        ( len(waiters[me]) == qlen[me] );
                        locked[me] = 1;
                        mutex[me]=0
        fi;
/*  */
                        /* Acquire continued */
    release:
        
        atomic {
            mutex[me] == 0 ->
            locked[me] = 0;
            mutex[me] = 1
        };
        if
        :: qlen[me] ->
            waiters[me] ? po[me];
            qlen[me] = qlen[me]-1;
            q_len_ch[po[me]] ! qlen[me];
            do
            :: qlen[me] -> atomic {
                waiters[me] ? thread;
                waiters[po[me]] ! thread;
                thread = 0;
                qlen[me] = qlen[me]-1
               }
            :: !qlen[me] -> break
            od
        :: !qlen[me] -> skip
        fi;
        mutex[me] = 0;
        assert (qlen[me] >= 0);
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

