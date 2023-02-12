#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS     5

void PrintHello(int threadid)
{
    int nthreads;

    printf("Hello World! It's me, thread #%d!\n", threadid);
    /* Only master thread does this */
    if (threadid == 0)
    {
        nthreads = omp_get_num_threads();
        printf("Number of threads = %d\n", nthreads);
    }
}

int main (int argc, char *argv[])
{
    int nthreads, tid;

#pragma omp parallel private(nthreads, tid)
    {
        /* Obtain thread number */
        tid = omp_get_thread_num();
        PrintHello(tid);
    }
}
