/* set depth-bound of DFS to around 20 */
chan ch = [0] of { byte }; /* buffering (non-rendezvous) channel */
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

//p1 :  at_x++ , at_ch!x
//p2 :  at_ch?y , at_z++

// <at_x++ , at_ch?y > -> [ ch! happens] -> < at_x++,  at_z++>

//[0] <at_x++ , at_ch?y > -> <at_ch!x, at_ch?y> > <at_x++,  at_z++> -> <at_ch!x, at_ch?y>

//[1] <at_x++ , at_ch?y > -> <<at_ch!x, at_ch?> -> <at_x++, at_ch?> -> 
