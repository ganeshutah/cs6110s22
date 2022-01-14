
byte x;
active proctype p1()
{do
 :: atomic { x++ ; x++ }
od }
active proctype p2()
{do
 :: atomic { x-- ; x-- }
od }
never {
do
:: skip
:: x==4 -> break
od
}
