// From: Peter Kriens <pkriens@gmail.com>, July 16, 2020

open util/ordering[Crossing]

enum Object { Farmer, Chicken, Grain, Fox }

let eats = Chicken->Grain + Fox->Chicken

let safe[place] = Farmer in place or (no place.eats & place)

sig Crossing {

near, far : set Object,

carry :  Object

} {

near = Object - far

safe[near] 

safe[far]

}

run {

first.near = Object

no first.far

all c : Crossing - last, c':c.next {

Farmer in c.near implies { 

c'.far = c.far + c.carry + Farmer

} else {

c'.near = c.near + c.carry + Farmer

}

}

some c : Crossing | c.far = Object

} for 8

// ordering
// // Ordering places an ordering on the parameterized signature.
// open util/ordering[A]

// sig A {}

// run {
//  some first -- first in ordering
//  some last  -- last in ordering
//  first.lt[last]
// }
// ordering can only be instantiated once per signature. You can, however, call it for two different signatures:

// open util/module[Thing1] as u1
// open util/module[Thing2] as u2

// sig Thing1 {}
// sig Thing2 {}

// ordering forces the signature to be exact. This means that the following model has no instances:

// open util/ordering[S]

// sig S {}

// run {#S = 2} for 3
