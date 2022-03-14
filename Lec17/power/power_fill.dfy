function power(x:nat, n:nat) : nat
{ if (n==0) then 1 else x * power(x, n-1) }

method g23(x:nat, n:nat) returns (P:nat)
ensures P == power(x,n)
{
 var X, N := x, n;
 P := 1;
 while (N != 0)
 decreases N
 // invariant <fill>
  {
   N := N - 1;
   P := P * X;
  }
}
