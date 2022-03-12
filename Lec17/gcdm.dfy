function gcd(x:nat, y:nat) : nat
requires x>=0 && y>=0
decreases x+y
{ if      (x==0)  then y
  else if (y==0)  then x
  else if (x==y)  then y //could be x
  else if (x > y) then gcd(x-y,y)
  else                 gcd(x, y-x)
}

method gcdm(x:nat, y:nat) returns (G:nat)
requires x>=0 && y>=0
ensures  G==gcd(x,y)

