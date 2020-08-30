#include "pdist.h"
#include "openmp_pdist.h"
#include "error_check.h"
#include "cuda_pdist.cuh"
#include <omp.h>
#include <iostream>
using namespace std;
#include "tbb/tick_count.h"
using namespace tbb;



void pdist(float *point, float *result, const int MAX_N, const int DIM)
{
	int i=0;
	tick_count tt1,tt2, tt3;
	
	//multi-gpu//
	int GPU_N = 1;
	
	cudaStream_t *stream=new cudaStream_t[GPU_N];
	float *point_gpu[GPU_N];
	float *result_gpu[GPU_N];
	
// 	tt1=tick_count::now();
	#pragma omp parallel num_threads(GPU_N)
	{
		int i=omp_get_thread_num();
		checkCudaErrors( cudaSetDevice(i) );
		checkCudaErrors( cudaStreamCreate(&stream[i]) );
		//Allocate memory
		checkCudaErrors( cudaMalloc((void**)&point_gpu[i], sizeof(float)*MAX_N*DIM) );
 		checkCudaErrors( cudaMalloc((void**)&result_gpu[i], sizeof(float)*CHUNK*(MAX_N-1)) );
		checkCudaErrors( cudaMemsetAsync(result_gpu[i], 0, sizeof(float)*CHUNK*(MAX_N-1),stream[i]) );//GPU
		checkCudaErrors( cudaMemcpyAsync(point_gpu[i],point,sizeof(float)*MAX_N*DIM,cudaMemcpyHostToDevice,stream[i]) );//GPU
	}
	
 	
	int DONE=0;
	
	omp_set_nested(1);
	omp_set_max_active_levels(2);
	#pragma omp parallel num_threads(2) 
	{
		int tid=omp_get_thread_num();
		#pragma omp sections
		{
			
			#pragma omp section
			{
 				pdist_CUDA(DONE, MAX_N, DIM, GPU_N, stream, result, &*point_gpu, &*result_gpu);
			}
			#pragma omp section
			{
// 				pdist_openmp(DONE, MAX_N, DIM, point, result);
			}
			
		}
 
	}
	
// 	for(i = 0; i < GPU_N; i++)  
// 	{
// 		checkCudaErrors( cudaSetDevice(i) );
// 		cudaStreamSynchronize(stream[i]);
// 		
// 		checkCudaErrors( cudaFree(point_gpu[i]) );
// 		checkCudaErrors( cudaFree(result_gpu[i]) );
// 	}
	
	//pdist_result
// 	int j,k=0;
// 	for (i=0; i<MAX_N; i++)
// 	{
// 		for(j=i; j<MAX_N-1; j++)
// 		{
// 			cout << result[k++] << '\t';
// 		}
// 		cout << endl;
// 	}
// 	cout << endl;
	
// 	tt3=tick_count::now();
// 	cout << "Elapsed Time = " << (tt3-tt2).seconds() << "sec" << endl;
	
	//memory de-allocation
// 	delete []stream;
}






