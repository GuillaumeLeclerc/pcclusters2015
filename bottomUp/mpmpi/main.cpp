/* vim: set softtabstop=2 shiftwidth=2 expandtab : */
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

/**
 * This represent the length of a syracuse serie
 */
typedef unsigned int length;

/**
 * This represents a value of a given syracuse serie
 */
typedef unsigned long value;

/**
 * This is the memoization array. It conains the already known length of 
 * some syracuse serie. This is like a Dynamic Programming array
 */
length* computed __attribute__((aligned(16)));

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
unsigned int N;

/**
 * The number of computation rounds
 */
unsigned int M;

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
  std::vector<value>* nextCompute;
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
    computed[n] = res;
  }
  return res;
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

  /**
   * Getting MPI information
   */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &processes);

  /**
   * Reading arguments
   */
  N = atoi(argv[1]);
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

#pragma omp parallel for reduction(+:desoptimizer)
  for (value i = 2 ; i < N; i++) {
    desoptimizer += computeLength(i);
  }

  std::cout << desoptimizer << std::endl;

  /**
   * Clean up
   */
  free(computed);
  MPI_Finalize();
}



