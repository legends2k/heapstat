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