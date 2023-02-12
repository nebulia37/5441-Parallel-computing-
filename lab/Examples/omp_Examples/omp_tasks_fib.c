#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int fib (int n)
{
    int x, y;

    if (n < 2) return n;
#pragma omp task shared(x)
    x = fib(n-1);
#pragma omp task shared(y)
    y = fib(n-2);
#pragma omp taskwait
    return x + y;
}

int main (int argc, char *argv[])
{
    int n = 10;

    printf("Fibonacci sequence #%d = %d\n", n, fib(n));

    return 0;
}

