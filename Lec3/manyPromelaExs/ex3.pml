byte x;
active proctype test()
{do
:: x++ ; x++
od }
never { do
:: skip
:: x==3 -> break
od
}

