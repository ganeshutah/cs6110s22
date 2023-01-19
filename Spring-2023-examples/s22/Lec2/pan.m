#define rand	pan_rand
#define pthread_equal(a,b)	((a)==(b))
#if defined(HAS_CODE) && defined(VERBOSE)
	#ifdef BFS_PAR
		bfs_printf("Pr: %d Tr: %d\n", II, t->forw);
	#else
		cpu_printf("Pr: %d Tr: %d\n", II, t->forw);
	#endif
#endif
	switch (t->forw) {
	default: Uerror("bad forward move");
	case 0:	/* if without executable clauses */
		continue;
	case 1: /* generic 'goto' or 'skip' */
		IfNotBlocked
		_m = 3; goto P999;
	case 2: /* generic 'else' */
		IfNotBlocked
		if (trpt->o_pm&1) continue;
		_m = 3; goto P999;

		 /* CLAIM never_0 */
	case 3: // STATE 2 - PhilLL.pml:78 - [(!(progress))] (0:0:0 - 1)
		
#if defined(VERI) && !defined(NP)
#if NCLAIMS>1
		{	static int reported2 = 0;
			if (verbose && !reported2)
			{	int nn = (int) ((Pclaim *)pptr(0))->_n;
				printf("depth %ld: Claim %s (%d), state %d (line %d)\n",
					depth, procname[spin_c_typ[nn]], nn, (int) ((Pclaim *)pptr(0))->_p, src_claim[ (int) ((Pclaim *)pptr(0))->_p ]);
				reported2 = 1;
				fflush(stdout);
		}	}
#else
		{	static int reported2 = 0;
			if (verbose && !reported2)
			{	printf("depth %d: Claim, state %d (line %d)\n",
					(int) depth, (int) ((Pclaim *)pptr(0))->_p, src_claim[ (int) ((Pclaim *)pptr(0))->_p ]);
				reported2 = 1;
				fflush(stdout);
		}	}
#endif
#endif
		reached[3][2] = 1;
		if (!( !(((int)now.progress))))
			continue;
		_m = 3; goto P999; /* 0 */
	case 4: // STATE 7 - PhilLL.pml:80 - [(!(progress))] (0:0:0 - 5)
		
#if defined(VERI) && !defined(NP)
#if NCLAIMS>1
		{	static int reported7 = 0;
			if (verbose && !reported7)
			{	int nn = (int) ((Pclaim *)pptr(0))->_n;
				printf("depth %ld: Claim %s (%d), state %d (line %d)\n",
					depth, procname[spin_c_typ[nn]], nn, (int) ((Pclaim *)pptr(0))->_p, src_claim[ (int) ((Pclaim *)pptr(0))->_p ]);
				reported7 = 1;
				fflush(stdout);
		}	}
#else
		{	static int reported7 = 0;
			if (verbose && !reported7)
			{	printf("depth %d: Claim, state %d (line %d)\n",
					(int) depth, (int) ((Pclaim *)pptr(0))->_p, src_claim[ (int) ((Pclaim *)pptr(0))->_p ]);
				reported7 = 1;
				fflush(stdout);
		}	}
#endif
#endif
		reached[3][7] = 1;
		if (!( !(((int)now.progress))))
			continue;
		_m = 3; goto P999; /* 0 */

		 /* PROC :init: */
	case 5: // STATE 1 - PhilLL.pml:66 - [(run phil(c5,c0,0))] (0:0:0 - 1)
		IfNotBlocked
		reached[2][1] = 1;
		if (!(addproc(II, 1, 0, ((P2 *)_this)->c5, ((P2 *)_this)->c0, 0)))
			continue;
		_m = 3; goto P999; /* 0 */
	case 6: // STATE 2 - PhilLL.pml:67 - [(run fork(c0,c1))] (0:0:0 - 1)
		IfNotBlocked
		reached[2][2] = 1;
		if (!(addproc(II, 1, 1, ((P2 *)_this)->c0, ((P2 *)_this)->c1, 0)))
			continue;
		_m = 3; goto P999; /* 0 */
	case 7: // STATE 3 - PhilLL.pml:68 - [(run phil(c1,c2,1))] (0:0:0 - 1)
		IfNotBlocked
		reached[2][3] = 1;
		if (!(addproc(II, 1, 0, ((P2 *)_this)->c1, ((P2 *)_this)->c2, 1)))
			continue;
		_m = 3; goto P999; /* 0 */
	case 8: // STATE 4 - PhilLL.pml:69 - [(run fork(c2,c3))] (0:0:0 - 1)
		IfNotBlocked
		reached[2][4] = 1;
		if (!(addproc(II, 1, 1, ((P2 *)_this)->c2, ((P2 *)_this)->c3, 0)))
			continue;
		_m = 3; goto P999; /* 0 */
	case 9: // STATE 5 - PhilLL.pml:70 - [(run phil(c3,c4,2))] (0:0:0 - 1)
		IfNotBlocked
		reached[2][5] = 1;
		if (!(addproc(II, 1, 0, ((P2 *)_this)->c3, ((P2 *)_this)->c4, 2)))
			continue;
		_m = 3; goto P999; /* 0 */
	case 10: // STATE 6 - PhilLL.pml:71 - [(run fork(c4,c5))] (0:0:0 - 1)
		IfNotBlocked
		reached[2][6] = 1;
		if (!(addproc(II, 1, 1, ((P2 *)_this)->c4, ((P2 *)_this)->c5, 0)))
			continue;
		_m = 3; goto P999; /* 0 */
	case 11: // STATE 8 - PhilLL.pml:73 - [-end-] (0:0:0 - 1)
		IfNotBlocked
		reached[2][8] = 1;
		if (!delproc(1, II)) continue;
		_m = 3; goto P999; /* 0 */

		 /* PROC fork */
	case 12: // STATE 1 - PhilLL.pml:38 - [rp?are_you_free] (0:0:0 - 1)
		reached[1][1] = 1;
		if (boq != ((P1 *)_this)->rp) continue;
		if (q_len(((P1 *)_this)->rp) == 0) continue;

		XX=1;
		if (4 != qrecv(((P1 *)_this)->rp, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P1 *)_this)->rp-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P1 *)_this)->rp, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P1 *)_this)->rp);
			sprintf(simtmp, "%d", 4); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P1 *)_this)->rp))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 13: // STATE 2 - PhilLL.pml:38 - [rp!yes] (0:0:0 - 1)
		IfNotBlocked
		reached[1][2] = 1;
		if (q_len(((P1 *)_this)->rp))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P1 *)_this)->rp);
		sprintf(simtmp, "%d", 3); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P1 *)_this)->rp, 0, 3, 1);
		{ boq = ((P1 *)_this)->rp; };
		_m = 2; goto P999; /* 0 */
	case 14: // STATE 3 - PhilLL.pml:41 - [lp?are_you_free] (0:0:0 - 1)
		reached[1][3] = 1;
		if (boq != ((P1 *)_this)->lp) continue;
		if (q_len(((P1 *)_this)->lp) == 0) continue;

		XX=1;
		if (4 != qrecv(((P1 *)_this)->lp, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P1 *)_this)->lp-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P1 *)_this)->lp, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P1 *)_this)->lp);
			sprintf(simtmp, "%d", 4); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P1 *)_this)->lp))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 15: // STATE 4 - PhilLL.pml:42 - [lp!no] (0:0:0 - 1)
		IfNotBlocked
		reached[1][4] = 1;
		if (q_len(((P1 *)_this)->lp))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P1 *)_this)->lp);
		sprintf(simtmp, "%d", 2); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P1 *)_this)->lp, 0, 2, 1);
		{ boq = ((P1 *)_this)->lp; };
		_m = 2; goto P999; /* 0 */
	case 16: // STATE 5 - PhilLL.pml:43 - [rp?release] (0:0:0 - 1)
		reached[1][5] = 1;
		if (boq != ((P1 *)_this)->rp) continue;
		if (q_len(((P1 *)_this)->rp) == 0) continue;

		XX=1;
		if (1 != qrecv(((P1 *)_this)->rp, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P1 *)_this)->rp-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P1 *)_this)->rp, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P1 *)_this)->rp);
			sprintf(simtmp, "%d", 1); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P1 *)_this)->rp))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 17: // STATE 10 - PhilLL.pml:46 - [lp?are_you_free] (0:0:0 - 1)
		reached[1][10] = 1;
		if (boq != ((P1 *)_this)->lp) continue;
		if (q_len(((P1 *)_this)->lp) == 0) continue;

		XX=1;
		if (4 != qrecv(((P1 *)_this)->lp, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P1 *)_this)->lp-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P1 *)_this)->lp, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P1 *)_this)->lp);
			sprintf(simtmp, "%d", 4); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P1 *)_this)->lp))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 18: // STATE 11 - PhilLL.pml:46 - [lp!yes] (0:0:0 - 1)
		IfNotBlocked
		reached[1][11] = 1;
		if (q_len(((P1 *)_this)->lp))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P1 *)_this)->lp);
		sprintf(simtmp, "%d", 3); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P1 *)_this)->lp, 0, 3, 1);
		{ boq = ((P1 *)_this)->lp; };
		_m = 2; goto P999; /* 0 */
	case 19: // STATE 12 - PhilLL.pml:49 - [rp?are_you_free] (0:0:0 - 1)
		reached[1][12] = 1;
		if (boq != ((P1 *)_this)->rp) continue;
		if (q_len(((P1 *)_this)->rp) == 0) continue;

		XX=1;
		if (4 != qrecv(((P1 *)_this)->rp, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P1 *)_this)->rp-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P1 *)_this)->rp, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P1 *)_this)->rp);
			sprintf(simtmp, "%d", 4); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P1 *)_this)->rp))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 20: // STATE 13 - PhilLL.pml:50 - [rp!no] (0:0:0 - 1)
		IfNotBlocked
		reached[1][13] = 1;
		if (q_len(((P1 *)_this)->rp))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P1 *)_this)->rp);
		sprintf(simtmp, "%d", 2); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P1 *)_this)->rp, 0, 2, 1);
		{ boq = ((P1 *)_this)->rp; };
		_m = 2; goto P999; /* 0 */
	case 21: // STATE 14 - PhilLL.pml:51 - [lp?release] (0:0:0 - 1)
		reached[1][14] = 1;
		if (boq != ((P1 *)_this)->lp) continue;
		if (q_len(((P1 *)_this)->lp) == 0) continue;

		XX=1;
		if (1 != qrecv(((P1 *)_this)->lp, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P1 *)_this)->lp-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P1 *)_this)->lp, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P1 *)_this)->lp);
			sprintf(simtmp, "%d", 1); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P1 *)_this)->lp))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 22: // STATE 22 - PhilLL.pml:54 - [-end-] (0:0:0 - 1)
		IfNotBlocked
		reached[1][22] = 1;
		if (!delproc(1, II)) continue;
		_m = 3; goto P999; /* 0 */

		 /* PROC phil */
	case 23: // STATE 1 - PhilLL.pml:12 - [lf!are_you_free] (0:0:0 - 1)
		IfNotBlocked
		reached[0][1] = 1;
		if (q_len(((P0 *)_this)->lf))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P0 *)_this)->lf);
		sprintf(simtmp, "%d", 4); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P0 *)_this)->lf, 0, 4, 1);
		{ boq = ((P0 *)_this)->lf; };
		_m = 2; goto P999; /* 0 */
	case 24: // STATE 2 - PhilLL.pml:14 - [lf?yes] (0:0:0 - 1)
		reached[0][2] = 1;
		if (boq != ((P0 *)_this)->lf) continue;
		if (q_len(((P0 *)_this)->lf) == 0) continue;

		XX=1;
		if (3 != qrecv(((P0 *)_this)->lf, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P0 *)_this)->lf-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P0 *)_this)->lf, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P0 *)_this)->lf);
			sprintf(simtmp, "%d", 3); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P0 *)_this)->lf))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 25: // STATE 4 - PhilLL.pml:15 - [lf?no] (0:0:0 - 1)
		reached[0][4] = 1;
		if (boq != ((P0 *)_this)->lf) continue;
		if (q_len(((P0 *)_this)->lf) == 0) continue;

		XX=1;
		if (2 != qrecv(((P0 *)_this)->lf, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P0 *)_this)->lf-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P0 *)_this)->lf, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P0 *)_this)->lf);
			sprintf(simtmp, "%d", 2); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P0 *)_this)->lf))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 26: // STATE 10 - PhilLL.pml:20 - [rf!are_you_free] (0:0:0 - 1)
		IfNotBlocked
		reached[0][10] = 1;
		if (q_len(((P0 *)_this)->rf))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P0 *)_this)->rf);
		sprintf(simtmp, "%d", 4); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P0 *)_this)->rf, 0, 4, 1);
		{ boq = ((P0 *)_this)->rf; };
		_m = 2; goto P999; /* 0 */
	case 27: // STATE 11 - PhilLL.pml:22 - [rf?yes] (0:0:0 - 1)
		reached[0][11] = 1;
		if (boq != ((P0 *)_this)->rf) continue;
		if (q_len(((P0 *)_this)->rf) == 0) continue;

		XX=1;
		if (3 != qrecv(((P0 *)_this)->rf, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P0 *)_this)->rf-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P0 *)_this)->rf, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P0 *)_this)->rf);
			sprintf(simtmp, "%d", 3); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P0 *)_this)->rf))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 28: // STATE 12 - PhilLL.pml:23 - [progress = 1] (0:0:1 - 1)
		IfNotBlocked
		reached[0][12] = 1;
		(trpt+1)->bup.oval = ((int)now.progress);
		now.progress = 1;
#ifdef VAR_RANGES
		logval("progress", ((int)now.progress));
#endif
		;
		_m = 3; goto P999; /* 0 */
	case 29: // STATE 13 - PhilLL.pml:23 - [progress = 0] (0:0:1 - 1)
		IfNotBlocked
		reached[0][13] = 1;
		(trpt+1)->bup.oval = ((int)now.progress);
		now.progress = 0;
#ifdef VAR_RANGES
		logval("progress", ((int)now.progress));
#endif
		;
		_m = 3; goto P999; /* 0 */
	case 30: // STATE 14 - PhilLL.pml:24 - [lf!release] (0:0:0 - 1)
		IfNotBlocked
		reached[0][14] = 1;
		if (q_len(((P0 *)_this)->lf))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P0 *)_this)->lf);
		sprintf(simtmp, "%d", 1); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P0 *)_this)->lf, 0, 1, 1);
		{ boq = ((P0 *)_this)->lf; };
		_m = 2; goto P999; /* 0 */
	case 31: // STATE 15 - PhilLL.pml:25 - [rf!release] (0:0:0 - 1)
		IfNotBlocked
		reached[0][15] = 1;
		if (q_len(((P0 *)_this)->rf))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P0 *)_this)->rf);
		sprintf(simtmp, "%d", 1); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P0 *)_this)->rf, 0, 1, 1);
		{ boq = ((P0 *)_this)->rf; };
		_m = 2; goto P999; /* 0 */
	case 32: // STATE 17 - PhilLL.pml:27 - [rf?no] (0:0:0 - 1)
		reached[0][17] = 1;
		if (boq != ((P0 *)_this)->rf) continue;
		if (q_len(((P0 *)_this)->rf) == 0) continue;

		XX=1;
		if (2 != qrecv(((P0 *)_this)->rf, 0, 0, 0)) continue;
		
#ifndef BFS_PAR
		if (q_flds[((Q0 *)qptr(((P0 *)_this)->rf-1))->_t] != 1)
			Uerror("wrong nr of msg fields in rcv");
#endif
		;
		qrecv(((P0 *)_this)->rf, XX-1, 0, 1);
		
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[32];
			sprintf(simvals, "%d?", ((P0 *)_this)->rf);
			sprintf(simtmp, "%d", 2); strcat(simvals, simtmp);
		}
#endif
		if (q_zero(((P0 *)_this)->rf))
		{	boq = -1;
#ifndef NOFAIR
			if (fairness
			&& !(trpt->o_pm&32)
			&& (now._a_t&2)
			&&  now._cnt[now._a_t&1] == II+2)
			{	now._cnt[now._a_t&1] -= 1;
#ifdef VERI
				if (II == 1)
					now._cnt[now._a_t&1] = 1;
#endif
#ifdef DEBUG
			printf("%3ld: proc %d fairness ", depth, II);
			printf("Rule 2: --cnt to %d (%d)\n",
				now._cnt[now._a_t&1], now._a_t);
#endif
				trpt->o_pm |= (32|64);
			}
#endif

		};
		_m = 4; goto P999; /* 0 */
	case 33: // STATE 18 - PhilLL.pml:27 - [lf!release] (0:0:0 - 1)
		IfNotBlocked
		reached[0][18] = 1;
		if (q_len(((P0 *)_this)->lf))
			continue;
#ifdef HAS_CODE
		if (readtrail && gui) {
			char simtmp[64];
			sprintf(simvals, "%d!", ((P0 *)_this)->lf);
		sprintf(simtmp, "%d", 1); strcat(simvals, simtmp);		}
#endif
		
		qsend(((P0 *)_this)->lf, 0, 1, 1);
		{ boq = ((P0 *)_this)->lf; };
		_m = 2; goto P999; /* 0 */
	case 34: // STATE 28 - PhilLL.pml:32 - [-end-] (0:0:0 - 1)
		IfNotBlocked
		reached[0][28] = 1;
		if (!delproc(1, II)) continue;
		_m = 3; goto P999; /* 0 */
	case  _T5:	/* np_ */
		if (!((!(trpt->o_pm&4) && !(trpt->tau&128))))
			continue;
		/* else fall through */
	case  _T2:	/* true */
		_m = 3; goto P999;
#undef rand
	}

