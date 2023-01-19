/* Good for understanding Buchi acceptance */
/* Depth bound for shorter traces - 1000 seems necessary */
/* Must be before exp tree to the left of "magic trace" needed */
/* Rearranging p1 and p2 might help */
active proctype p1()
{byte x;
 do
 :: x = x + 3 /* USER BEWARE: this statement is atomic, unlike in C !! */
 od
}
active proctype p2()
{byte y;
do
:: y = y + 5 od
}
never {
do
:: skip
:: (p1:x == p2:y) -> break
od;
accept: goto accept; /* not needed but looks Buchi */
}

