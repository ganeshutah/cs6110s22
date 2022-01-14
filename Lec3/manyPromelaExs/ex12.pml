/* set depth-bound of DFS to around 20 */
chan ch = [1] of { byte }; /* buffering (non-rendezvous) channel */
active proctype p1()
{byte x; /* local var x init to 0 */
 do
 :: x++ -> ch!x /* ; and -> are the same */
 od
}
active proctype p2()
{byte y,z; /* can be named x, but keeping distinct names */
 do
 :: ch?y -> z++ /* z tracks value of x */
 od
}
never {
do
:: skip
:: (p1:x - p2:z) > 2 -> break
od;
accept: goto accept
}
