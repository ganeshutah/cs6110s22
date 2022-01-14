/* set depth-bound of DFS to around 20 */
chan ch = [1] of { byte }; /* buffering (non-rendezvous) channel */
byte x, z;
active proctype p1()
{
 do
 :: x++ -> ch!x /* ; and -> are the same */
 od
}
active proctype p2()
{byte y; /* can be named x, but keeping distinct names */
 do
 :: ch?y -> z++ /* z tracks value of x */
 od
}
never {
do
:: skip
:: (x - z) > 2 -> break
od;
accept: goto accept
}
