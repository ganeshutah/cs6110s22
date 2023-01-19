byte x;
active proctype test()
{do
:: atomic { x++ ; x++ }
od }
never { do
:: skip
:: x==3 -> break
od
}

