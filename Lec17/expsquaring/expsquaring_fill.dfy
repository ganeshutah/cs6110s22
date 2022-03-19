function expn(x:nat, n:nat) : nat
requires x >= 0
{ if (n==0) then 1 else x * expn(x, n-1) }
method expsqr(x:nat, n:nat) returns (P:nat)
requires x > 0
requires n > 0
ensures P == expn(x,n)
{
   //var X, N := x, n;
//if (n < 0) { X := 1 / X; N := -N; }
 P := 1;
 if (N == 0) { return; }
 var Y := 1; 
 while (N > 1)
 invariant N >= 0
 decreases N
 invariant Y * expn(X, N) == expn(x, n)
 invariant ((N % 2) == 0) ==> (Y * expn(X, N) == Y * expn(X * X, N / 2))
 invariant ((N % 2) != 0) ==> (Y * expn(X, N) == Y * X * expn(X * X, (N - 1) / 2))
 { if (N % 2) == 0
   {
     X := X * X;
     N := N / 2;
   }
   else
   {
     Y := X * Y; // Y accumulates one X whenever N is odd
     X := X * X; // With 'one N consumed', we act per "even (N-1)"
     N := (N - 1) / 2;
   }
 }
 P := Y * X;
}