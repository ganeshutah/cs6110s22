/*
 * An error that is hard to hit
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "klee/klee.h"

int rare(int x) {
  if (x < RAND_MAX/2) {
    if (x > RAND_MAX/2 - 3) {
	 assert(0);
      }
  }
  else 
    return(1);
} 

int main() {
  int a, i;
  
  #ifdef KLEEON
  klee_make_symbolic(&a, sizeof(a), "a");
  rare(a);  
  #endif
  

  #ifdef PRINTON
  #ifndef KLEEON
  printf("randmax = %d\n", RAND_MAX);
  for (i=0; i < 10000000; i++) {
  a = rand(); //% 100000000;
  rare(a);
  }
  #endif
  #endif
}

