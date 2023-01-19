byte x; /* initialized to 0 by default */
proctype test()
{do
:: x++ ; x++
od }
init {
 run test();
}
