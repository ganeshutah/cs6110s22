#include <klee/klee.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void print_data(int arr[], int size, int target) {
  printf("searching for %d in:\n[", target);
    for (int i=0; i < size-1; i++) {
      printf("%d, ", arr[i]);
    }
    printf("%d]\n", arr[size-1]);
}

int binary_search(int arr[], int size, int target) {
  #ifdef PRINTON
    print_data(arr, size, target);
  #endif
    int low = 0;
    int high = size - 1;
    int mid;
    while (low <= high) {
        mid = (low + high)/2;
        if (arr[mid] == target) {
            return mid;
        }
        if (arr[mid] < target) {
            low = mid + 1;
        }
        if (arr[mid] > target) {
            high = mid - 1;
        }
    }
    return -1;
}

int main() {

  int a[4]; // was a[10]
  int x;
  
  #ifdef KLEEON
  klee_make_symbolic(&a, sizeof(a), "a");
  klee_assume(a[0] <= a[1]);
  klee_assume(a[1] <= a[2]);
  klee_assume(a[2] <= a[3]);

  klee_make_symbolic(&x, sizeof(x), "x");
  #endif
  
  #ifdef INITON
  a[0]=rand()%30;
  a[1]=a[0]+rand()%30;
  a[2]=a[1]+rand()%30;
  a[3]=a[2]+rand()%30;
  #endif

  int result = binary_search(a, 4, x); // was 10                                                    
  #ifdef PRINTON
  printf("result = %d\n", result);
  #endif
  // check correctness                                                                              
  if (result != -1) {
    assert(a[result] == x);
  } else {
    // if result == -1, then we didn't find it. Therefore, it shouldn't be in the array             
    for (int i = 0; i < 3; i++) { // was 10                                                         
      assert(a[i] != x);
    }
  }
  return 1;
}
