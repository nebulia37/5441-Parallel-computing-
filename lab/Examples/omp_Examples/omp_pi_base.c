#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    int i = 0;
    long num_steps = 1000000000;
    double pi = 0.0, step = 0.0, x = 0.0, sum = 0.0;

    step = 1.0/(double) num_steps;

    {
        for (i = 0; i < num_steps; i++) {
            x = (i+0.5)*step;
            sum += 4.0/(1.0+x*x);
        }
        pi = sum * step;
    }  /* All threads join master thread and disband */
    printf("PI = %f\n", pi);

    return 0;
}
