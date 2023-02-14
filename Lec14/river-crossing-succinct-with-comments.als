// From: Peter Kriens <pkriens@gmail.com>, July 16, 2020

-- This is opening a package called orderings
-- https://homepage.cs.uiowa.edu/~tinelli/classes/181/Fall21/Notes/05-alloy-modules.pdf
-- https://alloy.readthedocs.io/en/latest/modules/ordering.html
-- Predefines for us Crossing$0, Crossing$1, etc

-- Basically, if there is a set called Crossing, its members
-- will be created by Alloy as Crossing$0, Crossing$1, etc
-- and when you instantiate the parametric relation util/ordering <-- in this path in Alloy
-- Then you get "for free" some predicates: "lt", "gt" etc
-- and some constants ordering/first and ordering/last
-- Example : order/first.lt [ordering/last] is true
-- Example : lt[ordering/first, ordering/last] is true

open util/ordering[Crossing]

-- Define the four objects of interest
enum Object { Farmer, Chicken, Grain, Fox }

-- Defines the eats relation
let eats = Chicken->Grain + Fox->Chicken

-- Defines a shorthand "safe[place]"
let safe[place] = Farmer in place or (no place.eats & place)

-- Defines the Crossing set which has some members
-- such as Crossing$0, Crossing$1, etc
-- Each such Crossing has a near, a far, and a carry
-- with the constraints that near = Object - far
-- and safe[near] and safe[far]
--
sig Crossing {
near, far : set Object,
carry :  Object
} {
near = Object - far
safe[near] 
safe[far]
}

-- Then we just run the following conjunct (conjunctions implicit in the newlines)
-- and we look for models (instances)

run {

first.near = Object -- All Object is in first.near (this dot-join gives a set of objects)
no first.far        -- Nothing in first.far (this dot-join gives a set of objects)

-- For all c such that it is Crossing minus the last crossing
-- if we define c1 to be c.next (this is why we remove "last" from above)

all c : Crossing - last, c1:c.next
  {
  Farmer in c.near  -- we want this initial state IMPLIES this c1.far for c.next
  implies           -- it is a constraint on how the total order forms
  { 
    c1.far = c.far + c.carry + Farmer
  }
  else
  {
  c1.near = c.near + c.carry + Farmer
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
