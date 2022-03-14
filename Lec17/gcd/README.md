A nice tutorial is here:
https://dafny-lang.github.io/dafny/OnlineTutorial/guide

This illustrates that sometimes you have to help Dafny out 
with "obvious invariants".

Also, the specification needs to accommodate '0' also
(i.e. specifying for x>0 and y>0 won't do).

In general, the "requires" constraints do not automatically propagate
into the loop invariant. I suppose one has to study loops as an independent
entity wrt the stated invariants.

