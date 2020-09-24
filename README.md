# Heapstat

**Heapstat** keeps track of heap-allocated memory to detect leaks.

## How to use

### 1. Add to your project

Simply include the `heapstat.hh` header file in whichever source files you want to track for heap allocation or deallocation issues, and add the `heapstat.cc` file to your project and your build target(s).

All calls to the default heap allocators are automatically intercepted. Heapstat works with:
- `malloc`
- `realloc`
- `free`
- `new`
- `delete`
- `new[]`
- `delete[]`

**You do not have to change anything in your source code**, as long as you use the default allocators.

It does **not** work with custom allocators, unless for instance your custom allocator calls `malloc` and you track the custom allocator routine itself. It is not magic.

### 2. Print allocation summary

Call the function `heapstat()` at any time to print out a summary of heap pointers not freed. It also returns the total number of bytes leaked as a result.

## Example (heapstat_test.cc)

```cpp
#include "heapstat.hh"

int main()
{
    char* bufc = (char*)malloc(sizeof(char) * 51200);
    // free(bufc);

    for (int i = 0; i < 10; i++) {
        double* buf = (double*)malloc(sizeof(double) * 1024);
        // free(buf);
    }

    double* nn = new double;
    // delete nn;

    char* nc = new char[334];
    // delete[] nc;

    heapstat();

    return 0;
}
```

Results in:
```
HEAP ALLOCATIONS LEAKED:
--------------------------------------------------------------
      Count |       Size (B) | Location
==============================================================
          1 |            334 | heapstat_test.cc:16
          1 |              8 | heapstat_test.cc:13
         10 |         81'920 | heapstat_test.cc:9
          1 |         51'200 | heapstat_test.cc:5
--------------------------------------------------------------
         13 |        133'462 | total
```

