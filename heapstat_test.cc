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