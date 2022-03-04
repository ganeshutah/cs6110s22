--https://alloy.readthedocs.io/en/latest/language/predicates-and-functions.html

-- Encoding of constants in First-Order Logic (FOL) --
-- Followed by encoding propositional variables     --

one sig a,b extends U { } -- constants 'a,b' in U

some sig U {
 r0  : lone U, -- r0 is for the simulation of a zero-ary pred -- or prop variable --
 r1  : lone U, -- r1 is for another
} -- our universe  

pred R0 { some a.r0 }
pred R1 { some a.r1 }

-- Check a familiar propositional axiom --

assert r0a { R0 => (R1 => R0) }
check r0a
--


