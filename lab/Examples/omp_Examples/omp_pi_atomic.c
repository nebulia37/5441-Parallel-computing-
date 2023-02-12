#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NTHREADS    8


int main (int argc, char *argv[])
{
    int nthreads;
    long num_steps = 1000000000;
    double pi = 0.0, step = 0.0;

    step = 1.0/(double) num_steps;
    /* Fork a team of threads giving them their own copies of variables */
#pragma omp parallel
    {
        int i = 0, tid = 0, nthrds = 0;
        double x = 0.0, sum = 0.0;

        nthrds = omp_get_num_threads();
        if (nthrds > MAX_NTHREADS) {
            fprintf(stderr, "Only support %d OMP threads\n", MAX_NTHREADS);
            exit(-1);
        }

        /* Obtain thread number */
        tid = omp_get_thread_num();
        /* Only master does this */
        if (tid == 0) {
            nthreads = nthrds;
        }

        nthreads = omp_get_num_threads();
        for (i = tid; i < num_steps; i = i + nthrds) {
            x = (i+0.5)*step;
            sum += 4.0/(1.0+x*x);
        }
        sum *= step;
#pragma omp atomic
            pi += sum;
    }  /* All threads join master thread and disband */
    printf("PI = %f, Computed by %d threads\n", pi, nthreads);

    return 0;
}
