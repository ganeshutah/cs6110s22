	switch (t->back) {
	default: Uerror("bad return move");
	case  0: goto R999; /* nothing to undo */

		 /* CLAIM never_0 */
;
		;
		;
		;
		
		 /* PROC :init: */

	case 5: // STATE 1
		;
		;
		delproc(0, now._nr_pr-1);
		;
		goto R999;

	case 6: // STATE 2
		;
		;
		delproc(0, now._nr_pr-1);
		;
		goto R999;

	case 7: // STATE 3
		;
		;
		delproc(0, now._nr_pr-1);
		;
		goto R999;

	case 8: // STATE 4
		;
		;
		delproc(0, now._nr_pr-1);
		;
		goto R999;

	case 9: // STATE 5
		;
		;
		delproc(0, now._nr_pr-1);
		;
		goto R999;

	case 10: // STATE 6
		;
		;
		delproc(0, now._nr_pr-1);
		;
		goto R999;

	case 11: // STATE 8
		;
		p_restor(II);
		;
		;
		goto R999;

		 /* PROC fork */

	case 12: // STATE 1
		;
		XX = 1;
		unrecv(((P1 *)_this)->rp, XX-1, 0, 4, 1);
		;
		;
		goto R999;

	case 13: // STATE 2
		;
		_m = unsend(((P1 *)_this)->rp);
		;
		goto R999;

	case 14: // STATE 3
		;
		XX = 1;
		unrecv(((P1 *)_this)->lp, XX-1, 0, 4, 1);
		;
		;
		goto R999;

	case 15: // STATE 4
		;
		_m = unsend(((P1 *)_this)->lp);
		;
		goto R999;

	case 16: // STATE 5
		;
		XX = 1;
		unrecv(((P1 *)_this)->rp, XX-1, 0, 1, 1);
		;
		;
		goto R999;

	case 17: // STATE 10
		;
		XX = 1;
		unrecv(((P1 *)_this)->lp, XX-1, 0, 4, 1);
		;
		;
		goto R999;

	case 18: // STATE 11
		;
		_m = unsend(((P1 *)_this)->lp);
		;
		goto R999;

	case 19: // STATE 12
		;
		XX = 1;
		unrecv(((P1 *)_this)->rp, XX-1, 0, 4, 1);
		;
		;
		goto R999;

	case 20: // STATE 13
		;
		_m = unsend(((P1 *)_this)->rp);
		;
		goto R999;

	case 21: // STATE 14
		;
		XX = 1;
		unrecv(((P1 *)_this)->lp, XX-1, 0, 1, 1);
		;
		;
		goto R999;

	case 22: // STATE 22
		;
		p_restor(II);
		;
		;
		goto R999;

		 /* PROC phil */

	case 23: // STATE 1
		;
		_m = unsend(((P0 *)_this)->lf);
		;
		goto R999;

	case 24: // STATE 2
		;
		XX = 1;
		unrecv(((P0 *)_this)->lf, XX-1, 0, 3, 1);
		;
		;
		goto R999;

	case 25: // STATE 4
		;
		XX = 1;
		unrecv(((P0 *)_this)->lf, XX-1, 0, 2, 1);
		;
		;
		goto R999;

	case 26: // STATE 10
		;
		_m = unsend(((P0 *)_this)->rf);
		;
		goto R999;

	case 27: // STATE 11
		;
		XX = 1;
		unrecv(((P0 *)_this)->rf, XX-1, 0, 3, 1);
		;
		;
		goto R999;

	case 28: // STATE 12
		;
		now.progress = trpt->bup.oval;
		;
		goto R999;

	case 29: // STATE 13
		;
		now.progress = trpt->bup.oval;
		;
		goto R999;

	case 30: // STATE 14
		;
		_m = unsend(((P0 *)_this)->lf);
		;
		goto R999;

	case 31: // STATE 15
		;
		_m = unsend(((P0 *)_this)->rf);
		;
		goto R999;

	case 32: // STATE 17
		;
		XX = 1;
		unrecv(((P0 *)_this)->rf, XX-1, 0, 2, 1);
		;
		;
		goto R999;

	case 33: // STATE 18
		;
		_m = unsend(((P0 *)_this)->lf);
		;
		goto R999;

	case 34: // STATE 28
		;
		p_restor(II);
		;
		;
		goto R999;
	}

