#include <stdlib.h>
#include "pdist.h"
#include "error_check.h"
#include <iostream>
using namespace std;

#include "tbb/tick_count.h"
using namespace tbb;

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
	
	
	tick_count tt1,tt2, tt3;
	
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
// 		cout << point[i] << endl;
	}

	tt1=tick_count::now();
	
	pdist(point, result, MAX_N, DIM);
	
	tt2=tick_count::now();
// 	cout << (tt2-tt1).seconds() << endl;
// 	cout << "Total Running Time = " << (tt2-tt1).seconds() << "sec" << endl;
	
	
	NN_chain(result, MAX_N, thread_c, thread_m);
	

	tt3=tick_count::now();
// 	cout << "NN = " << (tt3-tt2).seconds() << "sec" << endl;
	cout << (tt3-tt2).seconds() << endl;

	
// 	delete [] point;
// 	delete [] result;
	checkCudaErrors( cudaFreeHost(point) );
	checkCudaErrors( cudaFreeHost(result) );
	fcloseall();
	return 0;
}