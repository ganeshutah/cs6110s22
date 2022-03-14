// Sum an array a[0]..a[a.Length-1]

function recsum(a: array?<int>,Max:nat) : int // sum [0..Max)
reads a
requires a != null
requires 0 <= Max <= a.Length
decreases Max
{ if a.Length == 0 then 0
  else if 0 == Max then 0
  else a[Max-1]+recsum(a,Max-1)
}

method sumarr(a: array?<int>) returns (sum:int)
 requires a != null
 ensures sum == recsum(a,a.Length) //sum [0..a.Length)
 {sum := 0;
  if (a.Length==0) {return;}
  var i : int := 0;
  while (i < a.Length)
  invariant ...
  invariant ...
  {
   sum := sum + a[i]; i:=i+1;
  }
}
