#include <omp.h>
#include <list>
#include <queue>
#include <iostream>
#include "NN-chain.h"
#include <fstream>
#include <float.h>
using namespace std;
#include "tbb/tick_count.h"
using namespace tbb;

omp_lock_t chain_lock;//Critical Section I
omp_lock_t chain_Q_lock;
omp_lock_t update_Q_lock;//Critical Section II

ofstream fout;

inline void dendrogram_update(INFO *info, const int idx1, const int idx2, const float min)
{
	fout << info[idx1].clusterID << '\t' << info[idx2].clusterID << '\t' << min << endl;
}

void cluster_info_set(INFO *info, const int N);


void NN_chain( t_float *D, const int N, int thread_c, int thread_m)
{
	int i; // iterator
	int ClusterID = N;//N+1
	int chainID=0; // for chain identification.
	fout.open("dendrogram"); //ofstream fout;

	doubly_linked_list AR(N); // Valid objects List. for Matirx update and Find NN
	doubly_linked_list cluster_list(2*N); // for cluster which is not in any chain.
	int *map=new int[N]; // map[0] == idx of 0+N th cluster.
	list<CHAIN*> chain_queue; // there are chains whose dependencies are removed
	
	INFO *info=new INFO[N]; // for cluster information.
	cluster_info_set(info, N); //setting info.
	
	omp_init_lock(&chain_lock);//Critical section I
	omp_init_lock(&chain_Q_lock);// for chain ID.
	omp_init_lock(&update_Q_lock);//Critical section II
	
	queue<U_PAIR> Q; // RNN to be updated.
	queue<CHAIN*> update_queue;//queue which have RNN to be updated.
	
	omp_set_nested(1);
	omp_set_max_active_levels(3);
	
	#pragma omp parallel num_threads(2) 
	{
		#pragma omp sections
		{
			#pragma omp section
			{
				#pragma omp parallel num_threads(thread_c) 
				{
					bool tmp;
					CHAIN *NN_chain=NULL;

					while(ClusterID < 2*N-1)
					{
						//Initialize NN_chain
						tmp=Chain_Init(NN_chain, cluster_list, D, N, info, chain_queue, ClusterID, map, chainID);
						if(tmp) 
						{	
							continue;
						}
						//growing NN_chain
						Chain_Grow(NN_chain, AR, cluster_list, D, N, info, ClusterID, Q, update_queue);

					}
				}
			}
			#pragma omp section
			{
				matrix_update_section(ClusterID, N, Q, update_queue, D, info, thread_m, AR, chain_queue, map);
			}
		}
	}
	
	omp_destroy_lock(&chain_lock);
	omp_destroy_lock(&chain_Q_lock);
	omp_destroy_lock(&update_Q_lock);
	fout.close();

	delete []info;
	delete []map;
}

bool Chain_Init(CHAIN* &NN_chain, doubly_linked_list &cluster_list, t_float *D, const int N, 
		INFO *info, list<CHAIN*> &chain_queue, int &ClusterID, int *map, int &chainID)
{
	int i=0, ID;
		
	omp_set_lock(&chain_Q_lock);
	while (!chain_queue.empty())
	{	
		NN_chain=chain_queue.back(); 
		chain_queue.pop_back();
		
		if(NN_chain->EMPTY()) 
		{
			delete &*(NN_chain);
			NN_chain=NULL;
			continue;
		}
		
		omp_unset_lock(&chain_Q_lock);
		return false;
	}
	omp_unset_lock(&chain_Q_lock);
	omp_set_lock(&chain_lock);
	
	if(cluster_list.start < ClusterID-1)
	{	

		
		i=cluster_list.start;cluster_list.remove(i);
		if(i > N-1) i=map[i-N];

		ID=chainID++;
		NN_chain=new CHAIN(ID);
		info[i].chainID=ID;
		info[i].node=NN_chain;
		
		omp_unset_lock(&chain_lock);
		
		NN_chain->PUSH(i,FLT_MAX);

		return false;
	}
	omp_unset_lock(&chain_lock);
	return true;
}

void Chain_Grow(CHAIN* &NN_chain, doubly_linked_list &AR, doubly_linked_list &cluster_list,  
		t_float *D, const int N, INFO *info, int &ClusterID, queue<U_PAIR> &Q, queue<CHAIN*> &update_queue)
{
	int i;
	int idx0;
	int N1, N2;
	tick_count tt1,tt2;
	t_float min=NN_chain->D_TOP();

	int idx1=NN_chain->List.top(), idx2;
	
	find_NN(AR, idx1, idx2, min, D, N, info);
	
	while( NN_chain->D_TOP() > min )
	{
		N1=idx1;
		N2=idx2;
		if(N1>N2) 
		{
			N1=idx2;
			N2=idx1;
		}

		omp_set_lock(&chain_lock);
		
		if(info[idx2].chainID == VALID && D_(N1,N2)==min)
		{
			info[idx2].chainID=NN_chain->chainID;
			cluster_list.remove(info[idx2].clusterID);
			info[idx2].node=NN_chain;
			omp_unset_lock(&chain_lock);
			
			NN_chain->PUSH(idx2,min);
		}
		else if (info[idx2].chainID == INVALID || D_(N1,N2) != min)
		{
			omp_unset_lock(&chain_lock);	
			if (ClusterID < 2*N-1)
			{
				min=NN_chain->D_TOP();
				find_NN(AR, idx1, idx2, min, D, N, info);
				continue;
			}
			else 
			{
				delete &*(NN_chain);
				NN_chain=NULL;
				return;
			}
		}
		
		else if (info[idx2].node->dependentONcluster == info[idx1].clusterID ) //  case: mutual dependency
		{
			info[idx2].chainID = NN_chain->chainID;
			info[idx2].node->POP();
			info[idx2].node=NN_chain;
			omp_unset_lock(&chain_lock);
			
			NN_chain->PUSH(idx2, min);
		}
		else // case: starvation
		{
			NN_chain->dependentONcluster=info[idx2].clusterID;
			info[idx2].dependency_queue.push_back(NN_chain);
			omp_unset_lock(&chain_lock);
			NN_chain=NULL;
			return;
		}
		min=NN_chain->D_TOP();
		idx1 = idx2;
		find_NN(AR, idx1, idx2, min, D, N, info);
	}
	
	if (NN_chain->SIZE_1()) return;
	
	
	U_PAIR u_pair;	
	NN_chain->RNN(u_pair.idx1, u_pair.idx2, min);
										
	#pragma omp critical(DEND) 
	{ dendrogram_update(info, u_pair.idx1, u_pair.idx2, min);}
	
	omp_set_lock(&update_Q_lock);
	Q.push(u_pair);
	update_queue.push(NN_chain);
	omp_unset_lock(&update_Q_lock);

	NN_chain=NULL;
}

void matrix_update_section(int &ClusterID, const int N,  std::queue<U_PAIR> &Q,  std::queue<CHAIN*> &update_queue, t_float *D, INFO *info,
			   const int thread_m, doubly_linked_list &AR, std::list<CHAIN*> &chain_queue, int *map)
{
	#pragma omp parallel num_threads(1)
	{
		int idx1, idx2;
		CHAIN *NN_chain;
		
		while( (ClusterID < 2*N-1) )
		{
			if(Q.empty()) continue;
			
			omp_set_lock(&update_Q_lock);
			idx1=Q.front().idx1;
			idx2=Q.front().idx2;
			Q.pop();
			NN_chain=update_queue.front();
			update_queue.pop();
			omp_unset_lock(&update_Q_lock);
			
			matrix_update(D, idx1, idx2, N, info, thread_m, AR);
			
			omp_set_lock(&chain_lock);
			omp_set_lock(&chain_Q_lock);
			if(NN_chain->EMPTY()) {	delete &*(NN_chain);NN_chain=NULL;}
			else chain_queue.push_back(NN_chain);
			
			
			if(!info[idx1].dependency_queue.empty())
			{
				chain_queue.splice(chain_queue.end(), info[idx1].dependency_queue);
			}
			if(!info[idx2].dependency_queue.empty())
			{
				chain_queue.splice(chain_queue.end(), info[idx2].dependency_queue);
			}
			omp_unset_lock(&chain_Q_lock);
			
			map[ClusterID-N]=idx2;
			Info_update(info, idx1, idx2, ClusterID);
			AR.remove(idx1);
			omp_unset_lock(&chain_lock);
		}
	}
}

void matrix_update(t_float *D, const int idx1, const int idx2, const int N, INFO *info, const int thread_m, doubly_linked_list &AR)
{
	
	int size1 = info[idx1].length;
	int size2 = info[idx2].length;
	t_float s =(t_float) size1/(size1+size2);
	t_float t =(t_float) size2/(size1+size2);
	
	#pragma omp parallel num_threads(thread_m)
	{
		int i;
		#pragma omp for schedule(guided)
		for(i=AR.start; i<N; i++)
		{
			if( info[i].chainID==INVALID ) continue;
			if( i < idx1) f_average(&D_(i, idx2), D_(i, idx1), s, t );
			else if( i > idx2) f_average(&D_(idx2, i), D_(idx1, i), s, t );
			else if (i > idx1 && i < idx2) f_average(&D_(i, idx2), D_(idx1, i), s, t );
		}
	}
}

void find_NN(doubly_linked_list &AR, const int idx2, int &idx1, t_float &min, t_float *D, const int N, INFO *info)
{
	int i;
	for (i=AR.start; i<idx2; i=AR.succ[i]) 
	{
 		if ( (info[i].chainID==info[idx2].chainID)) continue;
		if (D_(i,idx2) < min) 
		{
			min = D_(i,idx2);
			idx1 = i;
		}	
	}
	
	for (i=AR.succ[idx2]; i<N; i=AR.succ[i]) 
	{
		if ((info[i].chainID==info[idx2].chainID)) continue;
		if (D_(idx2,i) < min) 
		{
			min = D_(idx2,i);
			idx1 = i;
		}
	}
}

void Info_update(INFO *info, int &idx1, int &idx2, int &ClusterID)
{
	info[idx2].length+=info[idx1].length;
	info[idx2].clusterID=ClusterID++;
	info[idx1].clusterID=-1;
	info[idx1].chainID=INVALID;
	info[idx2].chainID=VALID;
}

void cluster_info_set(INFO *info, const int N)
{
	#pragma omp parallel for schedule(guided)
	for(int i=0;i<N;i++) 
	{
		info[i].length=1;
		info[i].clusterID=i;//i+1
 		info[i].chainID=VALID;
		info[i].node=NULL;
	}
}
