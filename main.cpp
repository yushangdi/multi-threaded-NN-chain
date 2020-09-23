#include <stdlib.h>
#include "pdist.h"
#include <stdio.h>
#include <stdlib.h>
// #include "error_check.h"
#include <iostream>
using namespace std;


#include "NN-chain.h"
#include "gettime.h"


#include "common.h"

int main(int argc, char *argv[])
{
	intT i;
	FILE *pFile=fopen(argv[1],"rb");
	intT MAX_N=atol(argv[2]); //compile with -Wall flag
	int DIM=atoi(argv[3]);
	float f;
	int thread_c=atoi(argv[4]);
	int thread_m=atoi(argv[5]);

	timer t;t.start();

	float *point = new float[MAX_N*DIM];
	float *result = new float[MAX_N*MAX_N/2];

	// checkCudaErrors( cudaHostAlloc((void**)&point,sizeof(float)*MAX_N*DIM, cudaHostAllocWriteCombined) );//use page-locked memory
	// checkCudaErrors( cudaHostAlloc((void**)&result,sizeof(float)*MAX_N*(MAX_N-1)/2, cudaHostAllocPortable) );//use page-locked memmory

	/************object generation*************/
	srand(time(0));
	for(i=0;i<MAX_N*DIM;i++)
	{
		fscanf(pFile, "%f", &f);
		point[i] = f;
	}
	cout << "init " << t.next() << endl;


	pdist(point, result, MAX_N, DIM);

	cout << "distM " << t.next() << endl;

	NN_chain(result, MAX_N, thread_c, thread_m);

	cout << "NNchain " << t.next() << endl;

// 	delete [] point;
// 	delete [] result;
	// checkCudaErrors( cudaFreeHost(point) );
	// checkCudaErrors( cudaFreeHost(result) );
	// fcloseall();
	fclose(pFile);
	t.reportTotal("total");
	return 0;
}
