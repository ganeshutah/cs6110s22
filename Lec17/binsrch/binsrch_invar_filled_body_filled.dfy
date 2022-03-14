predicate sorted(a: array?<int>)
   requires a != null
   reads a
{
   forall j, k :: 0 <= j < k < a.Length ==> a[j] <= a[k]
}
method BinarySearch(a: array?<int>, value: int) returns (index: int)
   requires a != null && 0 < a.Length && sorted(a)
   // since we are "abusing" index to be -1, we need to state via an implication
   // when index >=0, what holds
   ensures 0 <= index ==> index < a.Length && a[index] == value
   ensures index < 0 ==> forall k :: 0 <= k < a.Length ==> a[k] != value
{ var low  := 0;        //incl
  var high := a.Length; //excl
  while (low < high) 
  invariant 0 <= low <= high <= a.Length
  invariant forall i :: 0 <= i < a.Length && !(low <= i < high)
                        ==> a[i] != value;
  decreases high-low 
  {index := (low+high)/2;
   if      (value< a[index]) { high := index; }
   else if (value> a[index]) { low := index+1; }
   else    { return; }
  }
  index := -1;
}
