CC = g++
NVCC=nvcc
CCFLAGS = -O3  -Wall
NVCCFLAGS = -O3 -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_61,code=sm_61 -gencode arch=compute_70,code=sm_70 -gencode arch=compute_70,code=compute_70
# OBJECTS = NN-chain.o
CUDAHOME    = /usr/local/cuda
INCLUDEDIR  = 
# -I$(CUDAHOME)/include
INCLUDELIB  = 
# -L$(CUDAHOME)/lib64 -lcudart -ltbb
OMPLIB = -fopenmp

NNOBJECTS =  main.o pdist.o openmp_pdist.o NN-chain.o
# cuda_pdist.o 

NN-chain: $(NNOBJECTS) Makefile
	$(CC) $(CCFLAGS) $(NNOBJECTS) -o $@ $(INCLUDELIB) $(OMPLIB)

NN-chain.o: NN-chain.cpp NN-chain.h
	$(CC) $(CCFLAGS) -c $< $(OMPLIB)

pdist.o: pdist.cpp pdist.h cuda_pdist.cuh
	$(CC) $(CCFLAGS) -c $< $(OMPLIB) $(INCLUDEDIR)

# cuda_pdist.o: cuda_pdist.cu cuda_pdist.cuh openmp_pdist.h
# 	$(NVCC) $(NVCCFLAGS) -c $<

openmp_pdist.o: openmp_pdist.cpp openmp_pdist.h
	$(CC) $(CCFLAGS) -c $< $(OMPLIB)

main.o: main.cpp
	$(CC) $(CCFLAGS) -c $< $(INCLUDEDIR)

clean:
	rm -f NN-chain *.o
