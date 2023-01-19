byte x; /* initialized to 0 by default */
proctype test()
{do
:: x++ ; x++
:: x==230 -> assert(0)
od }
init {
 run test();
}
