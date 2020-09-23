# Heapstat

**Heapstat** keeps track of heap-allocated memory to detect leaks, double frees and deleting/freeing invalid pointers (to something not on the heap).

It is a **drop-in** replacement for `malloc/realloc/free`, `new/delete`, and `new[]/delete[]`. Simply include the `heapstat.hh` header file in whichever source files you want to track for heap allocation or deallocation issues, and add the `heapstat.cc` file to your project and your build target(s).

Call the function `heapstat()` to print out heap leakage info. It also returns the total number of bytes leaked.

### Example (heapstat_test.cc)

```c
#include "heapstat.hh"

int main()
{
    char* bufc = (char*)malloc(sizeof(char) * 51200);
    free(bufc);

    // 1. heap buffer not freed
    for (int i = 0; i < 10; i++) {
        double* buf = (double*)malloc(sizeof(double) * 1024);
    }

    // 2. heap buffer freed more than once
    free(bufc);

    // 3. attempt to free non-heap buffer
    char stbuf[512];
    free(stbuf);

    double* nn = new double;
    // delete nn;
    // ^ try uncommenting this

    char* nc = new char[334];
    // delete[] nc;

    heapstat();

    return 0;
}
```

Results in:
```
WARNING
freeing unknown or freed pointer 0x7feb08008000
  at heapstat_test.cc:14

WARNING
freeing unknown or freed pointer 0x7ffee69fcf50
  at heapstat_test.cc:18

HEAP ALLOCATIONS NOT FREED:
--------------------------------------------------------------
Allocations | Total Size (B) | Location
--------------------------------------------------------------
         10 |          81920 | heapstat_test.cc:10
          1 |              8 | heapstat_test.cc:21
          1 |            334 | heapstat_test.cc:23
--------------------------------------------------------------
Leaked 82262 bytes total
```

