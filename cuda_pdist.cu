#include <cuda.h>
#include "error_check.h"
#include "cuda_pdist.cuh"
#include <iostream>
using namespace std;

#define P_(i_,k_) (point[i_*DIM+k_])
#define ADDR_(r_,c_) ( static_cast<size_t>(2*MAX_N-3-(r_))*(r_)>>1)-1+(c_)
#define R_(r_,c_) (result[ADDR_(r_,c_)]) 

#define SIZE 16

void pdist_CUDA(int &DONE, const int MAX_N, const int DIM, const int GPU_N, 
	       cudaStream_t *stream, float *result, float *point_gpu[], float *result_gpu[])
{
	
	int i=0;
	int START=0;
	int END=0;
	int TransferSize=0;
	int S,E;
// 	DONE=8;
	while(DONE < MAX_N)
	{
		checkCudaErrors( cudaSetDevice(i) );//Set device
		cudaStreamSynchronize(stream[i]);
		
		#pragma omp critical(TASK)
		{
			START=DONE;
			DONE+=CHUNK;
			END=DONE;
// 			cout << "proceeding " << START << " to " << END << " on GPU " << i << endl;
		}
		if(END > MAX_N) END=MAX_N;
		if(START > MAX_N) break;
		
		launch_pdist_gpu( result_gpu[i], point_gpu[i], DIM, MAX_N, START, END, stream[i] );
		getLastCudaError("pdist_gpu() execution failed.\n");


		S=ADDR_(START,START+1);
		E=ADDR_(END-1,MAX_N-1);
		TransferSize=E-S+1;

		checkCudaErrors( cudaMemcpyAsync(result+ADDR_(START,START+1),result_gpu[i], sizeof(float)*TransferSize,cudaMemcpyDeviceToHost,stream[i]) );
	}
}

__global__ void pdist_gpu(float *out, float* in, int m, const int MAX_N, const int START, const int END)
{
	__shared__ float Ys[SIZE][SIZE];
	__shared__ float Xs[SIZE][SIZE];
	
	int bx= blockIdx.x;
	int by= blockIdx.y;
	
	int tx= threadIdx.x;
	int ty= threadIdx.y;
	
	int yBegin= (START*m) + by * SIZE * m;
	int xBegin=bx * SIZE * m;
	
	int yEnd = yBegin + m -1, y, x, k, o;
	float tmp, s=0;
	
	float dim_bound=0;
	int xIndex=bx*SIZE;
	int yIndex=START+by*SIZE;
	
	int addr_y= START+by*SIZE + ty;
	int addr_x= bx*SIZE + tx;
	
	
	if(  START/SIZE+by <= bx)
	{
		for (y=yBegin, x=xBegin; y<=yEnd ; x+=SIZE, y+=SIZE, dim_bound+=SIZE)
		{
			Ys[tx][ty]=0;
			Xs[tx][ty]=0;
			__syncthreads();
			
			if(dim_bound+tx < m && (yIndex+ty < END) )
				Ys[ty][tx] = in[y+(ty*m)+tx];
						
			if(dim_bound+tx < m && (xIndex+ty < MAX_N) )
				Xs[tx][ty] = in[x+(ty*m)+tx];
			__syncthreads();
			
			if ( START+by*SIZE+ty < bx*SIZE+tx ) //half
			{
				for( k = 0 ; k < SIZE ; k++)
				{
					tmp=Ys[ty][k] - Xs[k][tx];
					s+=tmp*tmp;
				}
			}
			__syncthreads();
		}
		o= (ADDR_(addr_y, addr_x))-(ADDR_(START,START+1));//
		
		if (addr_y < addr_x && addr_x < MAX_N && addr_y < END)
			out[o] = sqrtf(s);
	}
}

void launch_pdist_gpu(float *output, float* input, const int DIM, const int MAX_N, const int START, const int END, cudaStream_t &stream)
{
	int TASK=END-START;
	dim3 grid((MAX_N%SIZE == 0 ? 0 : 1)+MAX_N/SIZE, (TASK%SIZE == 0 ? 0 : 1)+TASK/SIZE);
	dim3 threads(SIZE,SIZE);
	pdist_gpu<<< grid, threads, 0, stream>>>( output, input, DIM, MAX_N, START, END);
}

