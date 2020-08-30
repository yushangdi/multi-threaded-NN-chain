#ifndef NN_chain_h
#define NN_chain_h

#include <list>
#include <queue>
#include <stack>

#define VALID -1
#define INVALID -2
#define D_(r_,c_) ( D[(static_cast<size_t>(2*N-3-(r_))*(r_)>>1)-1+(c_)] )

typedef float t_float;

typedef struct OBJECT_INFO INFO;
class doubly_linked_list;
class CHAIN;
typedef struct update_pair U_PAIR;
// class auto_array_ptr;

bool Chain_Init(CHAIN* &NN_chain, doubly_linked_list &AR2,  t_float *D, const int N, INFO *info, 
		std::list<CHAIN*> &chain_queue, int &ClusterID, int *map, int &chainID);
		
void Chain_Grow(CHAIN* &NN_chain, doubly_linked_list &AR, doubly_linked_list &AR2, t_float *D, 
		const int N, INFO *info, int &ClusterID, std::queue<U_PAIR> &Q,  std::queue<CHAIN*> &update_queue);


void matrix_update(t_float *D, const int idx1, const int idx2, const int N, INFO *info, const int thread_m, doubly_linked_list &AR);
void Info_update(INFO *info, int &idx1, int &idx2, int &ClusterID);
void find_NN(doubly_linked_list &AR, const int idx2, int &idx1, t_float &min, t_float *D, const int N, INFO *info);

void matrix_update_section(int &ClusterID, const int N,  std::queue<U_PAIR> &Q,  std::queue<CHAIN*> &update_queue, t_float *D, 
			   INFO *info, const int thread_m, doubly_linked_list &AR, std::list<CHAIN*> &chain_queue, int *map);

void NN_chain(t_float *result, const int N, int thread_c, int thread_m);

inline static void f_average( t_float * const b, const t_float a, const t_float s, const t_float t)
{
  *b = s*a + t*(*b);
}

template <typename type>
class auto_array_ptr
{
private:
	type * ptr;
public:
	auto_array_ptr() { ptr = NULL; }
	template <typename index>
	auto_array_ptr(index size) { init(size); }
	template <typename index, typename value>
	auto_array_ptr(index size, value val) { init(size, val); }
	~auto_array_ptr() { delete [] ptr; }
	void free() 
	{
		delete [] ptr;
		ptr = NULL;
	}
	template <typename index>
	void init(index size) { ptr = new type [size]; }
	template <typename index, typename value>
	void init(index size, value val)
	{
		init(size);
		for (index i=0; i < size; i++) ptr[i] = val;
	}
	operator type *() { return ptr; }
	
};

class CHAIN
{
public:
	CHAIN(int _ID)
	{
		chainID=_ID;
		dependentONcluster=-2;
	}

	void PUSH(int idx, t_float min)	{	List.push(idx);		dist.push(min);	}
	void POP()	{		List.pop();		dist.pop();	}
	bool SIZE_1()	{	return List.size()==1;	}
	bool EMPTY()	{	return List.empty();	}
	t_float D_TOP()	{	return dist.top();		}
	void RNN(int &idx1, int &idx2, t_float &min)
	{
			idx1=List.top();List.pop();
			idx2=List.top();List.pop();
			min=dist.top();dist.pop();dist.pop();
						
			if (idx1>idx2) 
			{
				int tmp = idx1;
				idx1 = idx2;
				idx2 = tmp;
			}
	}

	std::stack<int> List;
	std::stack<t_float> dist;

	int chainID;
	int dependentONcluster;
};

class doubly_linked_list 
{
public:
	int start;
	auto_array_ptr<int> succ;

private:
	auto_array_ptr<int> pred;

public:
	doubly_linked_list(const int size) 
	{
		// Initialize to the given size.
		pred.init(size+1);
		succ.init(size+1);
		for (int i=1; i<size+1; i++)
			pred[i] = i-1;
		// pred[0] is never accessed!
		for (int i=0; i<size; i++)
			succ[i] = i+1;
		//succ[size] is never accessed!
		start = 0;
		SIZE=size;
	}

	void remove(int idx) 
	{
		// Remove an index from the list.
		if (idx==start) start = succ[idx];
		else 
		{
			succ[pred[idx]] = succ[idx];
			pred[succ[idx]] = pred[idx];
		}
		SIZE--;
	}
	int SIZE;
};

typedef struct OBJECT_INFO
{
	int clusterID;
	int length;
	int chainID;
	CHAIN *node;
	std::list<CHAIN*> dependency_queue;
}INFO;

typedef struct update_pair //pairs to be merged
{	
	int idx1;
	int idx2;
}U_PAIR;

#endif