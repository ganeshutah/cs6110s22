byte x;
active proctype test()
{do
:: x++ ; x++
od }
never { do
:: skip
:: x==4 -> break
od;
accept: goto accept
}

