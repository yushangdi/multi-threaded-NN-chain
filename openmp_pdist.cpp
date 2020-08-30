#include <iostream>
#include <omp.h>
#include <math.h>
using namespace std;

#define CPU_CHUNK 128
#define P_(i_,k_) (point[i_*DIM+k_])
#define ADDR_(r_,c_) ( (2*MAX_N-3-(r_))*(r_)>>1)-1+(c_)
#define R_(r_,c_) (result[ADDR_(r_,c_)]) 

void pdist_openmp(int &DONE, const int MAX_N, const int DIM, float *point, float *result)
{
	int START=0;
	int END=0;
	
	while (DONE < MAX_N)
	{
		#pragma omp critical(TASK)
		{
			START=DONE;
			DONE+=CPU_CHUNK;
			END=DONE;
// 			cout << "proceeding " << START << " to " << END << " on CPU " << endl;
		}
		if(END > MAX_N) END=MAX_N;
		
		#pragma omp parallel num_threads(12)
		{
			int i,j,k;
			float tmp,sum;
			#pragma omp for
			for( i=START; i < END ; i++)
			{	
				for( j=i+1; j < MAX_N ; j++)
				{
					sum=0;
					for (k=0; k<DIM;k++)
					{
						tmp=P_(i,k)-P_(j,k);//point[addr_i+k]-point[addr_j+k];
						sum+=(tmp*tmp);
					}
					R_(i,j)=sqrtf(sum);
				}
			}
		}
	}
}

