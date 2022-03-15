// n-th Fibonacci number specified
// https://en.wikipedia.org/wiki/Fibonacci_number says 0,1,1,2,...
//
//i 0 1 2 3 4 5 6 ...
//a 0 1 1 2 3 5 8    <- a is the nth fib number
//b 1 1 2 3 5 8 13
//n             6
function fib(n: nat): nat
  requires  n >= 0
  decreases n
{  
   if n == 0 then 0 else
   if n == 1 then 1 else
                  fib(n - 1) + fib(n - 2)
}
method ComputeFib(n: nat) returns (a: nat)
   requires n>=0
   ensures a == fib(n)
{  a := 0;
   var b := 1;
   var i := 0;
   while i < n
   decreases n-i
   { a, b := b, a + b;
     i := i + 1;      }
}
