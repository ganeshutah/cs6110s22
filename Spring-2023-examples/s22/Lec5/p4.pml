// Code the most general Kripke Structure

bit x,y;
bit ready;

active proctype p() {
  do
  :: break
  :: x = !x
  :: y = !y
  od;
  ready = 1;
  do
  :: x = x
  :: y = y
  :: x = !x
  :: y = !y
  od
}

ltl  { [](ready -> ((<>[]x -> []<>y) -> ([]<>x -> []<>y))) }

/*--
spin -a p6b.pml ; gcc pan.c ; ./a.out -a
ltl ltl_0: [] ((! (ready)) || ((! ((! (<> ([] (x)))) || ([] (<> (y))))) || ((! ([] (<> (x)))) || ([] (<> (y))))))
pan:1: acceptance cycle (at depth 14)
pan: wrote p6b.pml.trail

(Spin Version 6.4.5 -- 1 January 2016)
Warning: Search not completed
	+ Partial Order Reduction

Full statespace search for:
	never claim         	+ (ltl_0)
	assertion violations	+ (if within scope of claim)
	acceptance   cycles 	+ (fairness disabled)
	invalid end states	- (disabled by never claim)

State-vector 28 byte, depth reached 25, errors: 1
       24 states, stored
       45 states, matched
       69 transitions (= stored+matched)
        0 atomic steps
hash conflicts:         0 (resolved)

Stats on memory usage (in Megabytes):
    0.001	equivalent memory usage for states (stored*(State-vector + overhead))
    0.272	actual memory usage for states
  128.000	memory used for hash table (-w24)
    0.534	memory used for DFS stack (-m10000)
  128.730	total actual memory usage



pan: elapsed time 0 seconds
[ganesh@thinmac Lec5]$ spin -t -p p6b.pml
ltl ltl_0: [] ((! (ready)) || ((! ((! (<> ([] (x)))) || ([] (<> (y))))) || ((! ([] (<> (x)))) || ([] (<> (y))))))
starting claim 1
using statement merging
Never claim moves to line 11	[(1)]
  2:	proc  0 (p:1) p6b.pml:8 (state 1)	[goto :b0]
  4:	proc  0 (p:1) p6b.pml:12 (state 7)	[ready = 1]
Never claim moves to line 6	[(((!(!(ready))&&!(x))&&!(y)))]
  6:	proc  0 (p:1) p6b.pml:14 (state 8)	[x = x]
Never claim moves to line 66	[((!(x)&&!(y)))]
  8:	proc  0 (p:1) p6b.pml:16 (state 10)	[x = !(x)]
Never claim moves to line 67	[((!(y)&&x))]
 10:	proc  0 (p:1) p6b.pml:14 (state 8)	[x = x]
Never claim moves to line 40	[((!(y)&&x))]
 12:	proc  0 (p:1) p6b.pml:16 (state 10)	[x = !(x)]
Never claim moves to line 39	[((!(x)&&!(y)))]
 14:	proc  0 (p:1) p6b.pml:14 (state 8)	[x = x]
  <<<<<START OF CYCLE>>>>>
Never claim moves to line 24	[(!(y))]
 16:	proc  0 (p:1) p6b.pml:14 (state 8)	[x = x]
Never claim moves to line 82	[((!(x)&&!(y)))]
 18:	proc  0 (p:1) p6b.pml:14 (state 8)	[x = x]
Never claim moves to line 56	[(!(y))]
 20:	proc  0 (p:1) p6b.pml:16 (state 10)	[x = !(x)]
Never claim moves to line 57	[((!(y)&&x))]
 22:	proc  0 (p:1) p6b.pml:14 (state 8)	[x = x]
Never claim moves to line 31	[(!(y))]
 24:	proc  0 (p:1) p6b.pml:16 (state 10)	[x = !(x)]
Never claim moves to line 28	[((!(x)&&!(y)))]
 26:	proc  0 (p:1) p6b.pml:14 (state 8)	[x = x]
spin: trail ends after 26 steps
#processes: 1
		x = 0
		y = 0
		ready = 1
 26:	proc  0 (p:1) p6b.pml:13 (state 12)
 26:	proc  - (ltl_0:1) _spin_nvr.tmp:20 (state 39)
1 processes created
[ganesh@thinmac Lec5]$

--*/
