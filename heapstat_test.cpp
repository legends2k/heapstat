#include "heapstat.h"

int main(int argc, char* argv[])
{
    char* bufc = (char*)malloc(sizeof(char) * 51200);
    // free(bufc);
    double* buf[1000];

    // while (1) {
    // 1. heap buffer not freed
    for (int i = 0; i < 10; i++) {
        buf[i] = (double*)malloc(sizeof(double) * 1024);
    }
    for (int i = 0; i < 10; i++) free(buf[i]);
    // }
    // 2. heap buffer freed more than once
    // free(bufc);

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