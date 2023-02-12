#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int foo (int n)
{
    int tid = 0;
    int x = 0, y = 0;

    if (n < 2) return n;
#pragma omp parallel
    {
#pragma omp task //shared(x)
    {
        /* Obtain thread number */
        tid = omp_get_thread_num();
        x = 10;
        printf("In-1: TID: %d. 1.1 x = %d, y = %d\n", tid, x, y);
    }
//#pragma omp taskwait
    printf("OUTSIDE-1: TID: %d. 1.2 x = %d, y = %d\n", tid, x, y);
#pragma omp task //shared(y)
    {
        /* Obtain thread number */
        tid = omp_get_thread_num();
        y = 20;
        printf("In-2: TID: %d, 2.1 x = %d, y = %d\n", tid, x, y);
    }
//#pragma omp taskwait
    printf("OUTSIDE-2: TID: %d. 1.2 x = %d, y = %d\n", tid, x, y);
#pragma omp taskwait
    }
    printf("DONE: TID: %d, 2.2 x = %d, y = %d\n", tid, x, y);
    return x + y;
}

int main (int argc, char *argv[])
{
    int n = 5;

    printf("Output #%d = %d\n", n, foo(n));

    return 0;
}