function gcd(x:nat, y:nat) : nat
requires x>=0 && y>=0
decreases x+y
{ if   (x==0)  then y
  else if (y==0) then x
  else if (x==y) then y
  else if (x>y)  then gcd(x-y,y)
  else                gcd(x,y-x)
}

method gcdm(x:nat, y:nat) returns (G:nat)
requires x>0 
requires y>0
ensures  G==gcd(x,y)
ensures  G!=0
{var X,Y := x,y; // info from the " requires " doesn't flow 
 //assert X > 0;               //-- this also helps out **
 //-passes assert (X!=0);
 //-passes assert (Y!=0);
 //assert (X > 0) && (Y > 0); -- this assertion also helps **
 while (X!=Y) //&& X>0 && Y>0 //-- why is this part needed?
  decreases X+Y
  invariant gcd(X,Y)==gcd(x,y)
  //invariant X > 0           //-- or this invariant helps **
  //- ADD invariant X > 0 to:
  //- (1) pass verification
  //- (2) remove the red underlines
  invariant X != 0   
  invariant Y != 0
 {
  if (X>Y) { X := X-Y; }
  else     { Y := Y-X; }
 }
 //if (X==0) {G:=Y;}
 //else { G:= X; }
 //-passes assert (X != 0);
 //-passes assert (Y != 0);
 G := X;
}

