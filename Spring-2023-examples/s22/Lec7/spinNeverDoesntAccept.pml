bit p;

active proctype test()
{ do
  :: p != p
  od
}

never  {    /* <>[]p */
T0_init:
	do
	:: ((p)) -> goto accept_S4
	:: (1) -> goto T0_init
	od;
accept_S4:
	do
	:: ((p)) -> goto accept_S4
	od;
}