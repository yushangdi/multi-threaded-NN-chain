#include <stdlib.h>
#include "pdist.h"
#include "error_check.h"
#include <iostream>
using namespace std;


#include "NN-chain.h"

int main(int argc, char *argv[])
{
	int i;
	FILE *pFile=fopen(argv[1],"rb");
	int MAX_N=atoi(argv[2]);
	int DIM=atoi(argv[3]);
	float f;
	int thread_c=atoi(argv[4]);
	int thread_m=atoi(argv[5]);
	
	float *point;//=new float[MAX_N*DIM];
	float *result;//=new float[MAX_N*MAX_N/2];
	
	checkCudaErrors( cudaHostAlloc((void**)&point,sizeof(float)*MAX_N*DIM, cudaHostAllocWriteCombined) );//use page-locked memory
	checkCudaErrors( cudaHostAlloc((void**)&result,sizeof(float)*MAX_N*(MAX_N-1)/2, cudaHostAllocPortable) );//use page-locked memmory
	
	/************object generation*************/
	srand(time(0));
	for(i=0;i<MAX_N*DIM;i++)
	{
		fscanf(pFile, "%f", &f);
		point[i] = f;
	}

	
	pdist(point, result, MAX_N, DIM);
	
	NN_chain(result, MAX_N, thread_c, thread_m);
	
// 	delete [] point;
// 	delete [] result;
	checkCudaErrors( cudaFreeHost(point) );
	checkCudaErrors( cudaFreeHost(result) );
	fcloseall();
	return 0;
}
