#include<stdio.h>
#include<cuda.h>

__global__ void add(int *a, int *b, int *c, int n)
{
    int index = threadIdx.x + blockIdx.x * blockDim.x;

    if (index < n) {
        c[index] = a[index] + b[index];
    }
}

int main(int argc, char *argv[])
{
    int i = 0;
    /* Host copies of a, b, and c */
    int *a = NULL, *b = NULL, *c = NULL;
    /* Device copies of a, b, and c */
    int *d_a = NULL, *d_b = NULL, *d_c = NULL;
    /* Amount of data */
    long int size = 0;
    /* Number of elements in the array */
    long int N = 0;
    /* Number of blocks */
    long int B = 0;
    /* Threads per block */
    int T = 0;
    /* Print or not */
    int print = 0;

    if (argc <= 2) {
        fprintf(stderr, "This program expects two inputs - size of array and threads per block\n");
        exit (1);
    }
    /* Read number of elements from command line */
    N = atoi(argv[1]);
    /* Read number of threads per block from command line */
    T = atoi(argv[2]);
    /* Compute the size */
    size = N*sizeof(int);
    /* Error check */
    if (size <= 0) {
        fprintf(stderr, "Size of array should be greater than 0\n");
        exit (1);
    }
    if (argc == 4) {
        /* Read print option from the command line */
        print = atoi(argv[3]);
    }

    /* Allocate space for host copies of a, b, and c */
    a = (int*) malloc(size);
    b = (int*) malloc(size);
    c = (int*) malloc(size);

    /* Allocate space for device copies of a, b, and c */
    cudaMalloc((void **) &d_a, size);
    cudaMalloc((void **) &d_b, size);
    cudaMalloc((void **) &d_c, size);

    /* Dummy input values */
    for (i = 0; i < N; ++i) {
        a[i] = b[i] = i;
        c[i] = 0;
    }

    /* Copy input to device */
    cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice);

    /* Compute number of blocks */
    B = (N+(T-1))/T;

    /* Launch kernel for addition with N blocks and T threads */
    add<<<B,T>>>(d_a, d_b, d_c, N);

    /* Copy result back to the host */
    cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost);

    /* Print result */
    if (print == 1) {
        for (i = 0; i < N; ++i) {
            printf("a[%3d] (%3d) + b[%3d] (%3d) = c[%3d] (%3d)\n",
                    i, a[i], i, b[i], i, c[i]);
        }
    }

    /* Cleanup */
    free(a);
    free(b);
    free(c);
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);

    return 0;
}
