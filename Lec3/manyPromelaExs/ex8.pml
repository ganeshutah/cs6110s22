/* Reductions don't work */
byte x,y;
active proctype p1()
{do
 :: atomic { x++ ; x++ }
od }
active proctype p2()
{do
 :: atomic { y++ ; y++ }
od }
never {
do
:: skip
:: (x==232)&&(y==2) -> break /* observe both vars to introduce
                              * state explosion
                              */
od }
