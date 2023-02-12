#include<stdio.h>
#include<cuda.h>

__global__ void add(int *a, int *b, int *c)
{
    *c = *a + *b;
}

int main(void)
{
    /* Host copies of a, b, and c */
    int a = 0, b = 0, c = 0;
    /* Device copies of a, b, and c */
    int *d_a = NULL, *d_b = NULL, *d_c = NULL;
    /* Amount of data */
    int size = sizeof(int);

    /* Print initial values */
    printf("Begin: a (%d) + b (%d) = c (%d)\n", a, b, c);

    /* Allocate space for device copies of a, b, and c */
    cudaMalloc((void **) &d_a, size);
    cudaMalloc((void **) &d_b, size);
    cudaMalloc((void **) &d_c, size);

    /* Dummy input values */
    a = 10;
    b = 20;

    /* Copy input to device */
    cudaMemcpy(d_a, &a, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, &b, size, cudaMemcpyHostToDevice);

    /* Launch kernel for addition */
    add<<<1,1>>>(d_a, d_b, d_c);

    /* Copy result back to the host */
    cudaMemcpy(&c, d_c, size, cudaMemcpyDeviceToHost);

    /* Cleanup */
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);

    /* Print result */
    printf("End: a (%d) + b (%d) = c (%d)\n", a, b, c);

    return 0;
}
