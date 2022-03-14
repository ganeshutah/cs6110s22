method g20(x : nat, y : nat) returns (S: nat)
requires x > 0
requires y > 0
ensures S == x * y
{var X, Y;
 X := x;
 Y := y;
 S := 0;
 while (X != 0)
 invariant ((x*y - X*Y) == S) 
 decreases X
 {
   while (X%2 == 0)
   invariant ((x*y - X*Y) == S)
   invariant X >= 0
   invariant Y >= 0
   invariant S >= 0      
   decreases X
   {
    Y := Y + Y; // was 2 * Y; 
    X := X / 2;
   }
   S := S+Y;
   X := X-1;
 }
}
