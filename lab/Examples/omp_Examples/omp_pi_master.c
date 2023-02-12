#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NTHREADS    8


int main (int argc, char *argv[])
{
    int nthreads;
    long num_steps = 1000000000;
    double pi = 0.0, step = 0.0, sum[MAX_NTHREADS] = {0.0};

    step = 1.0/(double) num_steps;
    /* Fork a team of threads giving them their own copies of variables */
#pragma omp parallel
    {
        int i = 0, tid = 0, nthrds = 0;
        double x = 0.0;

        nthrds = omp_get_num_threads();
        if (nthrds > MAX_NTHREADS) {
            fprintf(stderr, "Only support %d OMP threads\n", MAX_NTHREADS);
            exit(-1);
        }

        /* Obtain thread number */
        tid = omp_get_thread_num();
        /* Only master does this */
#pragma omp master
        {
            nthreads = nthrds;
            printf("TID %d is here\n", tid);
        }

        nthreads = omp_get_num_threads();
        for (i = tid, sum[tid] = 0.0; i < num_steps; i = i + nthrds) {
            x = (i+0.5)*step;
            sum[tid] += 4.0/(1.0+x*x);
        }
    }  /* All threads join master thread and disband */
    {
        int i = 0;
        for (i = 0; i < nthreads; ++i) {
            pi += sum[i] * step;
        }
    }
    printf("PI = %f, Computed by %d threads\n", pi, nthreads);

    return 0;
}
