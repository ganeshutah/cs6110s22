/* setup of https://haslab.wordpress.com/2011/07/21/model-checking-graph-properties-using-alloy/
While developing an algorithm for fast distributed computation of a graph diameter and radius, some of my colleagues had a conjecture about graph topology that, if true, would enable an aggressive optimization of the algorithm.
*/

-- OTHER RESOURCES
-- http://reports-archive.adm.cs.cmu.edu/anon/ml2008/CMU-ML-08-117.pdf - soc graphs - diameter est
-- https://en.wikipedia.org/wiki/MapReduce
-- https://en.wikipedia.org/wiki/Antipodes - NZ antipode is Spain
-- https://www.cs.cmu.edu/~emc/papers/Books%20and%20Edited%20Volumes/Bounded%20Model%20Checking.pdf
-- https://www.princeton.edu/~chaff/publication/SAT04_search_qbf_diameter.pdf
-- https://reference.wolfram.com/language/ref/GraphDiameter.html
-- https://mathworld.wolfram.com/GraphPeriphery.html
-- https://mathworld.wolfram.com/GraphEccentricity.html
-- https://mathworld.wolfram.com/Antipode.html

-- Graph Properties Haslab --
-->> GraphProp4 <<--

-- Literally typing-in https://haslab.wordpress.com/2011/07/21/model-checking-graph-properties-using-alloy/ --
/*
The distance between two nodes is the length of the shortest path between them;
The eccentricity of a node is the greatest distance between that node and all others;
The diameter of a graph is the maximum eccentricity found in the graph;
A node is peripheral if its eccentricity is equal to the diameter;
The antipode of a node is the set of nodes at distance equal to its eccentricity.
*/

open util/ordering[Nat]  -- (5) Generic total ordering, instantiated by Nat; gives first, next, last etc for free!

/*
 --A Graph --
 sig Node {
  adj : set Node -- page 96, Alloy Red Book    -- (1)
 } 
*/


-- Signature facts constraining Node and adj --
fact {
 adj = ~adj       -- symm adj rel  -- (2)
 no iden & adj   -- no self-loop  -- (3)
 all n : Node | Node in n.*adj -- the graph is connected -- (4) -- notice it is Node in .. !!
}


sig Nat {}                     -- (6) It is our own Nat, not anything else


sig Node { -- (1a) -- (1) refined 
 adj : set Node,     -- same as before
 dist : Node -> one Nat  -- (7) Note from Page 96 that "one" can be omitted, but good to have it
}

fact {
 all n : Node | n.dist[n] = first  -- [1] All nodes have distance 0 to themselves
 all disj n,m : Node | n.dist[m] = min[n.adj.dist[m]].next -- [2] How distances are BFS distances
}

/* -- eccentricity -- */

fun ecc [] : Node -> one Nat { -- {1} -- ecc is a mapping of node to nat
 { n : Node, d : Nat | d = max[n.dist[Node]] } -- {2} -- the set of n->d pairs s.t. n is furthest from Node which incl n but OK
}

fun diam [] : one Nat { -- {2} -- diameter is a property over ALL nodes - the eccentricity of the most eccentric node!! is that me? :-)
 max[Node.ecc]
}

pred peripheral [n : Node] { -- {3} -- yes, they are all pushing to the "periphory" - makes sense
 n.ecc = diam
}

fun antipode [] : Node -> Node { -- {4} As the English says!
 { n, m : Node | n.dist[m] = n.ecc }
}
 
fun radius [] : one Nat { -- {5} makes sense
 min[Node.ecc]
}

pred central [ n : Node ] {
  n.ecc = radius
}

--> orig conj <--
-- For every node, all nodes in its antipode are peripheral.

check { --{6} -- author's conjecture--For every node, some node in its antipode is peripheral--
 all n : Node | all m : antipode[n] | peripheral[m]
} for 7 -- increase scopeto find cex --

--> new conj <--

check { --{6} -- author's conjecture--For every node, some node in its antipode is peripheral--
 all n : Node | some m : antipode[n] | peripheral[m]
} for 7 -- increase scopeto find cex --



/* --- THIS STORY CONTINUES NOW IN GraphProp5 which is not created yet! --- */
 
