CC = icpc -fPIC
NVCC=nvcc
CCFLAGS = -O3
NVCCFLAGS = -O3 -Xptxas="-v" -gencode=arch=compute_35,code=sm_35
# OBJECTS = NN-chain.o
CUDAHOME    = /usr/local/cuda-5.5
INCLUDEDIR  = -I$(CUDAHOME)/include
INCLUDELIB  = -L$(CUDAHOME)/lib64 -lcudart -ltbb
OMPLIB = -openmp -liomp5

NNOBJECTS =  main.o cuda_pdist.o pdist.o openmp_pdist.o NN-chain.o

NN-chain: $(NNOBJECTS) Makefile
	$(CC) $(CCFLAGS) $(NNOBJECTS) -o $@ $(INCLUDELIB) $(OMPLIB) -lrt

NN-chain.o: NN-chain.cpp NN-chain.h
	$(CC) $(CCFLAGS) -c $< $(OMPLIB) 

pdist.o: pdist.cpp pdist.h cuda_pdist.cuh
	$(CC) $(CCFLAGS) -c $< $(OMPLIB) $(INCLUDEDIR)

cuda_pdist.o: cuda_pdist.cu cuda_pdist.cuh openmp_pdist.h
	$(NVCC) $(NVCCFLAGS) -c $< 

openmp_pdist.o: openmp_pdist.cpp openmp_pdist.h
	$(CC) $(CCFLAGS) -c $< $(OMPLIB)

main.o: main.cpp
	$(CC) $(CCFLAGS) -c $< $(INCLUDEDIR) -ltbb

#$< $(INCLUDEDIR)
# 
# linkage.o: linkage.cpp linkage.h
# 	$(CC) $(CCFLAGS) -c linkage.cpp 
clean:
	rm -f NN-chain *.o
