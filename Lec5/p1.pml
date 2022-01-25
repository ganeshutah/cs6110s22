// Code the most general Kripke Structure

bit x
bool init_state = false

active proctype p() {
  if
    :: x = 0
    :: x = 1
  fi;
  //
  init_state = true;
  //
  do
  :: x = !x
  :: x = x
  od
}

ltl {  [](init_state ->  (<>x -> []x)) }  

//https://stackoverflow.com/questions/40358915/how-to-make-a-non-initialised-variable-in-spin
//

/* spin -a p1.pml ; gcc pan.c ; ./a.out -a */

/*

spin -t -p p1.pml
ltl ltl_0: [] ((! (init_state)) || ((! (<> (x))) || ([] (x))))
starting claim 1
using statement merging
Never claim moves to line 6	[(1)]
  2:	proc  0 (p:1) p1.pml:8 (state 1)	[x = 0]
  4:	proc  0 (p:1) p1.pml:12 (state 5)	[init_state = 1]
Never claim moves to line 3	[((!(!(init_state))&&!(x)))]
  6:	proc  0 (p:1) p1.pml:15 (state 6)	[x = !(x)]
spin: _spin_nvr.tmp:10, Error: assertion violated
spin: text of failed assertion: assert(!(x))
Never claim moves to line 10	[assert(!(x))]
spin: trail ends after 7 steps
#processes: 1
		x = 1
		init_state = 1
  7:	proc  0 (p:1) p1.pml:14 (state 8)
  7:	proc  - (ltl_0:1) _spin_nvr.tmp:9 (state 17)
1 processes created
[ganesh@thinmac Lec5]$ 

*/

