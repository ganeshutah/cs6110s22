-- Graph Properties Haslab --
-->> GraphProp1 <<--

-- Literally typing-in https://haslab.wordpress.com/2011/07/21/model-checking-graph-properties-using-alloy/ --
/*
The distance between two nodes is the length of the shortest path between them;
The eccentricity of a node is the greatest distance between that node and all others;
The diameter of a graph is the maximum eccentricity found in the graph;
A node is peripheral if its eccentricity is equal to the diameter;
The antipode of a node is the set of nodes at distance equal to its eccentricity.
*/

open util/ordering[Nat]  -- (5) Generic total ordering, instantiated by Nat; gives first, next, last etc for free!


 --A Graph --
 sig Node {
  adj : set Node -- page 96, Alloy Red Book    -- (1)
 } 


-- Signature facts constraining Node and adj --
fact {
 adj = ~adj       -- symm adj rel  -- (2)
 no iden & adj   -- no self-loop  -- (3)
 all n : Node | Node in n.*adj -- the graph is connected -- (4) -- notice it is Node in .. !!
}


sig Nat {}                     -- (6) It is our own Nat, not anything else

/*
sig Node { -- (1a) -- (1) refined 
 adj : set Node,     -- same as before
 dist : Node -> one Nat  -- (7) Note from Page 96 that "one" can be omitted, but good to have it
}
*/

/*--- at this point, type Nat, first, last, first.next , first.next.next , etc */
/* --- if you type next, it is ambiguous -- thus type Nat <: next         */

/* --- THIS STORY CONTINUES NOW IN GraphProp2 --- */
 
