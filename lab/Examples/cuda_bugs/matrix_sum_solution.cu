#include<stdio.h>
#include<cuda.h>

#define N 512
#define BLOCK_DIM 512

// error checking macro
#define cudaCheckErrors(msg) \
    do { \
        cudaError_t __err = cudaGetLastError(); \
        if (__err != cudaSuccess) { \
            fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
                msg, cudaGetErrorString(__err), \
                __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
            exit(1); \
        } \
    } while (0)

__global__ void matrixAdd (int *a, int *b, int *c)
{
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int index = col + row * N;

    if (col < N && row < N) {
        c[index] = a[index] + b[index];
    }
}

int main()
{
    int wrapSize = 32;
    int device_id = 0;
    int i = 0, j = 0;
    int a[N][N], b[N][N], c[N][N];
    int *dev_a, *dev_b, *dev_c;
    int size = N * N * sizeof(int);

    // CUDA device properties variable
    cudaDeviceProp prop;

    // Query GPU properties
    cudaGetDeviceProperties(&prop, device_id);

    printf("maxThreadsDim x,y,z = %d,%d,%d\n",
            prop.maxThreadsDim[0],
            prop.maxThreadsDim[1],
            prop.maxThreadsDim[2]);
    printf("maxGridSize x,y,z = %d,%d,%d\n",
            prop.maxGridSize[0],
            prop.maxGridSize[1],
            prop.maxGridSize[2]);
    printf("maxThreadsPerBlock = %d, maxThreadsPerMultiProcessor = %d, maxBlocksPerMultiProcessor = %d\n",
            prop.maxThreadsPerBlock,
            prop.maxThreadsPerMultiProcessor,
            prop.maxBlocksPerMultiProcessor);
    printf("reservedSharedMemPerBlock = %d, sharedMemPerBlock = %d\n",
            prop.reservedSharedMemPerBlock,
            prop.sharedMemPerBlock);

    // initialize a and b with real values (NOT SHOWN)
    cudaMalloc((void**)&dev_a, size);
    cudaMalloc((void**)&dev_b, size);
    cudaMalloc((void**)&dev_c, size);

    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            a[i][j] = b [i][j] = 1;
        }
    }
    cudaMemcpy(dev_a, a, size, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_b, b, size, cudaMemcpyHostToDevice);

    dim3 dimBlock(prop.maxThreadsPerBlock/wrapSize,wrapSize);
    dim3 dimGrid((int)ceil(N/dimBlock.x),(int)ceil(N/dimBlock.y));

    printf("dimBlock.x = %d, dimBlock.y = %d, dimBlock.z = %d\n",
            dimBlock.x, dimBlock.y, dimBlock.z);
    printf("dimGrid.x = %d, dimGrid.y = %d, dimGrid.z = %d\n",
            dimGrid.x, dimGrid.y, dimGrid.z);

    matrixAdd<<<dimGrid,dimBlock>>>(dev_a,dev_b,dev_c);
    cudaCheckErrors("kernel launch failure");

    cudaDeviceSynchronize();

    cudaMemcpy(c, dev_c, size, cudaMemcpyDeviceToHost);

    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            if (c[i][j] != 2) {
                printf("Data validation error at location c[%d][%d]. Expected: 2, Actual: %d (%d, %d)\n",
                        i, j, c[i][j], a[i][j], b[i][j]);
                exit(-1);
            }
        }
    }
    cudaFree(dev_a);
    cudaFree(dev_b);
    cudaFree(dev_c);
}
