#include <stdio.h>
// these are just for timing measurments
#include <time.h>

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

/* You should not change the value of DSIZE */
const int DSIZE = 18432;
int block_size = 32;
const float A_val = 3.0f;
const float B_val = 2.0f;


__global__ void mmul( float *A, float *B, float *C, int ds) 
{
    int idx = threadIdx.x+blockDim.x*blockIdx.x;
    int idy = threadIdx.y+blockDim.y*blockIdx.y;
    float temp = 0;
    extern __shared__ float shared[];
    for (int i =0; i < ((ds+blockDim.x-1)/blockDim.x); i = i+1) {
            if (idy < ds && (i*blockDim.x+threadIdx.x) < ds)  {shared[threadIdx.y*blockDim.x + threadIdx.x] = A[idy*ds +  i*blockDim.x+threadIdx.x];}
            else  {shared[threadIdx.y*blockDim.x + threadIdx.x] = 0;}
            if (idx < ds && (i*blockDim.x+threadIdx.y) < ds)  {shared[threadIdx.y*blockDim.x+threadIdx.x+blockDim.x*blockDim.x] = B[(i*blockDim.x+threadIdx.y)*ds+idx];}
            else  {shared[threadIdx.y*blockDim.x + threadIdx.x +  blockDim.x * blockDim.x] = 0;}
        __syncthreads();
        for (int k = 0; k < blockDim.x; ++k) {temp += shared[threadIdx.y*blockDim.x+k] * shared[k*blockDim.x+threadIdx.x+blockDim.x*blockDim.x];}
        __syncthreads();
    }
    if (idy < ds && idx < ds) {C[idy*ds + idx] = temp;}
}

int main(int argc, char *argv[])
{

  float *h_A, *h_B, *h_C, *d_A, *d_B, *d_C;

  // these are just for timing
  clock_t t0, t1, t2;
  double t1sum=0.0;
  double t2sum=0.0;

  if (argc == 2) {
      block_size = atoi(argv[1]);
      if (block_size <= 0) {
          fprintf(stderr, "Error: block_size should be >= 1\n");
          exit (1);
      }
  }

  // start timing
  t0 = clock();

  h_A = new float[DSIZE*DSIZE];
  h_B = new float[DSIZE*DSIZE];
  h_C = new float[DSIZE*DSIZE];

  for (int i = 0; i < DSIZE*DSIZE; i++){
        h_A[i] = A_val;
        h_B[i] = B_val;
        h_C[i] = 0;
  }
  printf("Init took %f seconds.  Begin compute\n", t1sum);

  // Initialization timing
  t1 = clock();
  t1sum = ((double)(t1-t0))/CLOCKS_PER_SEC;
  printf("Init took %f seconds.  Begin compute\n", t1sum);

  // Allocate device memory and copy input data over to GPU
  cudaMalloc(&d_A, DSIZE*DSIZE*sizeof(float));
  cudaCheckErrors("cudaMalloc failure");
  cudaMalloc(&d_B, DSIZE*DSIZE*sizeof(float));
  cudaCheckErrors("cudaMalloc failure");
  cudaMalloc(&d_C, DSIZE*DSIZE*sizeof(float));
  cudaCheckErrors("cudaMalloc failure");

  cudaMemcpy(d_A, h_A, DSIZE*DSIZE*sizeof(float), cudaMemcpyHostToDevice);
  cudaCheckErrors("cudaMemcpy H2D failure");
  cudaMemcpy(d_B, h_B, DSIZE*DSIZE*sizeof(float), cudaMemcpyHostToDevice);
  cudaCheckErrors("cudaMemcpy H2D failure");

  // Cuda processing sequence step 1 is complete

  // Launch kernel
  dim3 block(block_size, block_size);  // dim3 variable holds 3 dimensions
  dim3 grid((DSIZE+block.x-1)/block.x, (DSIZE+block.y-1)/block.y);
  mmul<<<grid, block, sizeof(float)*block_size*block_size*2>>>(d_A, d_B, d_C, DSIZE);
  cudaCheckErrors("kernel launch failure");

  // Cuda processing sequence step 2 is complete
  // Copy results back to host
  cudaMemcpy(h_C, d_C, DSIZE*DSIZE*sizeof(float), cudaMemcpyDeviceToHost);
  cudaCheckErrors("cudaMemcpy D2H failure");

  // GPU timing
  t2 = clock();
  t2sum = ((double)(t2-t1))/CLOCKS_PER_SEC;
  printf ("Done. Compute took %f seconds\n", t2sum);


  for (int i = 0; i < DSIZE*DSIZE; i++) if (h_C[i] != A_val*B_val*DSIZE) {printf("mismatch at index %d, was: %f, should be: %f\n", i, h_C[i], A_val*B_val*DSIZE); return -1;}
  //Free memory
  free(h_A);
  free(h_B);
  free(h_C);
  // free cuda memory;
  cudaFree(d_A);
  cudaFree(d_B);
  cudaFree(d_C);

  return 0;
}
  
