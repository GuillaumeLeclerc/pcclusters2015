/* vim: set softtabstop=2 shiftwidth=2 expandtab : */
#include <iostream>
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <sstream>
#include <stddef.h>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

/**
 * This represent the length of a syracuse serie
 */
typedef unsigned long length;

// Corresponding MPI type
#define MPI_LENGTH MPI_UNSIGNED_LONG

/**
 * This represents a value of a given syracuse serie
 */
typedef unsigned long value;

// Corresponding MPI type
#define MPI_VALUE MPI_UNSIGNED_LONG

/**
 * This is the data that will be shared accross the network
 */
struct computation {
  length cl;
  value cv;
};

// Corresponding MPI type
MPI_Datatype mpiComputationType;
#define MPI_COMPUTATION mpiComputationType

/**
 * This is the memoization array. It conains the already known length of 
 * some syracuse serie. This is like a Dynamic Programming array
 */
length* computed __attribute__((aligned(16)));

/**
 * These are the values that haven't been shared yet to the other nodes
 */
std::vector<struct computation> toShare;

/**
 * The MPI rank of the process
 */
int rank;

/**
 * The number of mpi processes
 */
int processes;

/**
 * The number of series
 */
size_t N;

/**
 * The number of computation rounds
 */
size_t M;


/**
 * If there is a thread communicating
 */
bool sharing;

/**
 * Get all values of depth d from the depth precompute stage
 */
std::vector<value>* getNextComputeStage(std::vector<value>* previousStage, length depth) {
  std::vector<value>* nextCompute = new std::vector<value>();

  for (size_t i = 0 ; i < previousStage->size(); i++) {
    const value v = previousStage->at(i);

    computed[v] = depth;

    const value n1 = 2 * v;
    const value n2 = (v - 1)/3;
    
    if (n1 < N && n1 > 1) {
      nextCompute->push_back(n1);
    }

    if ((v - 1) % 3 == 0 && n2 % 2 == 1 && n2 > 1 && n2 < N) {
      nextCompute->push_back(n2);
    }
  }
  delete previousStage;
  return nextCompute;
}

/**
 * precompute some easy values and set it into the the compted array
 * TODO check if we get a better speedup with a global queue in a 
 *      critical section
 */
void bottomUpPrecompute() {
  const value start = 1;
  std::vector<value>* toCompute = new std::vector<value>();
  length depth = 0;

  toCompute->push_back(start);

  int threadCount;

#pragma omp parallel 
  {
    threadCount = omp_get_num_threads();
  }

  // while there are some values to compute or we can do it in parallel 
  while(toCompute->size() % threadCount != 0 && toCompute->size() > 0) {
    toCompute = getNextComputeStage(toCompute, depth);
    depth++;
  }

#pragma omp parallel for
  for (unsigned int i = 0 ; i < toCompute->size() ; i++) {
    length localDepth = depth;
    const value parallelStart = toCompute->at(i);
    std::vector<value>* parallelToCompute = new std::vector<value>();
    parallelToCompute->push_back(parallelStart);

    while(parallelToCompute->size() > 0) {
      parallelToCompute = getNextComputeStage(parallelToCompute, localDepth);
      localDepth++;
    }
    delete parallelToCompute;
  }
  delete toCompute;
}


/**
 * Get the next element of the syracuse serie
 */
value syracuse_next(value n) {
  if(n % 2 == 0) return n/2;
  return 3*n + 1;
}


length computeLength(value n) {
  int res = 0;
  if (likely(n < N && computed[n] != 0)) {
    return computed[n];
  } else if (unlikely(n > 1)) {
    res = computeLength(syracuse_next(n)) + 1;
  }

  if (n < N) {
    if (computed[n] == 0 && !sharing) {
      struct computation currentValue;
      currentValue.cv = n;
      currentValue.cl = res;
#pragma omp critical(toShare)
      toShare.push_back(currentValue);
    }
    computed[n] = res;
  }
  return res;
}

size_t sharing_round = 1;
void shareProgress() {
  printf("start sharing(%lu) by thread %d on node %d\n", sharing_round, omp_get_thread_num(), rank);
  fflush(stdout);
  struct computation* computationBuffer;
  //filling the buffer in parallel
  sharing = true;
  size_t toShareSize;
#pragma omp critical(toShare) 
  toShareSize = toShare.size();
  computationBuffer = new struct computation[toShareSize];
  {
    for (size_t i = 0 ; i < toShare.size(); ++i) {
      computationBuffer[i] = toShare.at(i);
    }
  }
  sharing = false;

  // for each node
  for (int node = 0; node < processes; ++node) {
    size_t sizeOfDataToTransmit;
    if (rank == node) { // I am the one transmitting
      sizeOfDataToTransmit = toShareSize;
    }

    MPI_Bcast(&sizeOfDataToTransmit, 1, MPI_INT, node, MPI_COMM_WORLD);

    printf("size of data to transmit %lu (r:%d)\n", sizeOfDataToTransmit, rank);
    fflush(stdout);

    //alocating memory for what we are about to receive
    
    struct computation* currentRoundComputation;
    if (rank == node) {
      currentRoundComputation = computationBuffer;
    } else { 
      currentRoundComputation = new struct computation[sizeOfDataToTransmit];
    }

    printf("start broadcasting values\n");
    //broadcast the values
    MPI_Bcast(currentRoundComputation, sizeOfDataToTransmit, MPI_COMPUTATION, node, MPI_COMM_WORLD);
    printf("end broadcasting values\n");

    if (rank != node) { // if I am not the sender
      for (size_t i = 0; i < sizeOfDataToTransmit; ++i) {
        struct computation currentComputation = currentRoundComputation[i];
        computed[currentComputation.cv] = currentComputation.cl;
      }
    }
  }
  std::cout << "sharing done by " << omp_get_thread_num() << std::endl;
  sharing_round++;
}

/**
 * Generate the non native MPI Types
 */

void generateMPITypes() {
  /**
   * Generate MPI type for struct Computation
   */
    const int nitems=2;
    int          blocklengths[2] = {1,1};
    MPI_Datatype types[2] = {MPI_VALUE, MPI_LENGTH};
    MPI_Aint     offsets[2];

    offsets[0] = offsetof(struct computation, cl);
    offsets[1] = offsetof(struct computation, cv);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpiComputationType);
    MPI_Type_commit(&mpiComputationType);
}

/**
 * This is the main method
 * Arguments are:
 *  - N the number of series to compute (2 to N)
 *  - M the number of computation rounds
 *  - T the maximum number of threads per executable (-1 for default)
 */
int main (int argc, char* argv[]) 
{
  MPI_Init(&argc, &argv);

  if (argc < 4) {
    std::cout << "You must have 3 arguments" << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
    return MPI_Finalize();
  } 

  generateMPITypes();

  /**
   * Getting MPI information
   */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &processes);

  /**
   * Reading arguments
   */
  std::istringstream iss(argv[1]);
  iss >> N;
  std::cout << N <<  std::endl;
  M = atoi(argv[2]);
  const int T = atoi(argv[3]);
  if (T > 0) {
    omp_set_num_threads(T);
  }

  /**
   * Allocating memory and setting default values
   */
  computed = (length*) calloc(N, sizeof(length)); 
  if (computed == NULL) {
    std::cout << "Unable to allocate memory on process " << rank << std::endl;
    MPI_Abort(MPI_COMM_WORLD, errno);
  } else {
    std::cout << "Memory allocated correctly on process " << rank << std::endl;
  }
  memset(computed, 0, N * sizeof(length));

  /**
   * Precomputed easy values
   * Seem to slow down the whole thing
   */

  //bottomUpPrecompute();


  /**
   * Computing the complex values (going outside the interval)
   */

  /**
   * to remove over agressive optimisaitons that does not even compute
   * anything
   */
  length desoptimizer = 0;

  value mod = N / M;
  // only the master thread of master node
  if (rank == 0 && omp_get_thread_num() == 0) {
    printf("mod %d",  10000/2);
  }


#pragma omp parallel for reduction(+:desoptimizer)
  for (value i = 2 ; i < N; i++) {
    desoptimizer += computeLength(i);
    if (processes > 1 && i % mod == 1) { // no need to share if there is only one node
      printf("share at %lu\n", i);
#pragma omp critical(sharing) // no two threads should be sharing at the same time !
      shareProgress();
    }
  }
  // last sharing
  if (processes > 1) {
#pragma omp critical(sharing) // no two threads should be sharing at the same time !
    shareProgress();
  }

  // only the master thread of master node
  if (rank == 0 && omp_get_thread_num() == 0) {
    std::cout << desoptimizer << std::endl;
  }

  /**
   * Clean up
   */
  free(computed);
  MPI_Finalize();
}



