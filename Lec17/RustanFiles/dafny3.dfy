method g20(x: nat, y: nat) returns (S: nat)
  ensures S == x * y
{
  var X, Y := x, y;
  S := 0;
  while X != 0
    invariant x*y - X*Y == S
  {
    while X % 2 == 0
      invariant x*y - X*Y == S
      invariant X != 0
      decreases X
    {
      Y := 2 * Y;
      X := X / 2;
    }
    S := S + Y;
    X := X - 1;
  }
}
