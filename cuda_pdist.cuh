#ifndef cuda_pdist_h
#define cuda_pdist_h

#define BLOCK_DIM 256
#define CHUNK 1024

void launch_pdist_gpu(float *output, float* input, const int DIM, const int MAX_N, const int START, const int END, cudaStream_t &stream);

__global__ void pdist_gpu(float *output, float* input, int size_x, int size_y, const int start, const int end);

void pdist_CUDA(int &DONE, const int MAX_N, const int DIM, const int GPU_N, 
	       cudaStream_t *stream, float *result, float *point_gpu[], float *result_gpu[]);

#endif
