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

ltl {  [](init_state ->  ([]x -> <>x)) }  

//https://stackoverflow.com/questions/40358915/how-to-make-a-non-initialised-variable-in-spin
//

/* spin -a p2.pml ; gcc pan.c ; ./a.out -a */


/* State-vector 28 byte, depth reached 7, errors: 0 */
