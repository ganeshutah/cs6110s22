// Tic-Tac-Toe -- INCOMPLETE
// Keep a state array and let the processes race to fill them
// This Tic-Tac-Toe plays even after winning till all cells are filled
// Doing otherwise is a pain (Promela syntax)


mtype = {B, X, O} // Filling X and O, B being blank

typedef row {
 mtype c[3]
};

row r[3];

bit drew;
bit won;
bit turn;

proctype PO ()
{byte i,j;
 do
 :: i < 3 -> i++
 :: i > 0 -> i--
 :: j < 3 -> j++
 :: j > 0 -> j--
 :: atomic { (turn==0) && (r[i].c[j] == B) -> r[i].c[j] = O ; turn = 1}
 od
}

proctype PX ()
{byte i,j;
 do
 :: i < 3 -> i++
 :: i > 0 -> i--
 :: j < 3 -> j++
 :: j > 0 -> j--
 :: atomic { r[i].c[j] == B -> r[i].c[j] = O }
 od
}

init {
  byte i,j;
  i=0;j=0;
  do
  :: i < 3 -> do
              :: j < 3  -> r[i].c[j] = B ; j++ 
	      :: j == 3 -> break 
	      od;
	      i++
  :: i == 3 -> break 	      
  od;
  atomic { run PO(); run PX(); }
}