byte p1=0; byte p2=0;
active proctype P1() {
do
:: p1==0 -> p1=22; printf("P1, case 1\n") //disables p1==0 forever
:: p2==0 -> p1=55; printf("P1, case 2\n") // disables p1==0 forever
od
}
active proctype P2() {
do
:: p1==1 -> p1==44; printf("P2, case 1\n") // never taken, so no effect at any point
:: p2==0 -> p2=44; printf("P2, case 2\n") // taken once, disables p2==0 of P1
od
}
