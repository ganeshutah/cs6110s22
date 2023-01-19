bit mutex = 0;
bit bomb = 0;
int c = 0;

proctype prod(int tmp; int id)
{ atomic { mutex == 0 -> mutex = 1 };
  if :: c > 0 -> c++
     :: c<= 0 -> do :: tmp == 0 -> break
                    :: tmp > 0  -> c++ ; tmp-- od;
  fi;
  mutex = 0
}

proctype cons(int id)
{ if
  :: c >  0 -> c-- ; if :: c>= 0
                        :: c < 0 -> bomb = 1
	          fi
  :: c <= 0 
  fi
}

init
{
 atomic {
  run prod(5, 0); run prod(5, 1); run cons(0); run cons(1);
 };
}

never {
do
:: skip
:: bomb -> break
od
}


