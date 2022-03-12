// Find the max in an array a[0]..a[a.Length-1]
// If the array length is 0, return 0
method maxarr(a: array?<int>) returns (max:int)
 requires a != null
 ensures forall i :: 0 <= i < a.Length ==> max >= a[i]
 ensures a.Length > 0 ==> max in a[..]
 { //max := 0;
   if (a.Length==0) {return;}
   var i := 0;
   max := a[0];   
   while (i < a.Length)
   invariant 0 <= i <= a.Length
   invariant forall j :: 0 <= j < i ==> a[j] <= max
   invariant max == a[0] || max in a[..i]
   {
    if (a[i] > max) {max := a[i];}
    i := i + 1;
   }
}
