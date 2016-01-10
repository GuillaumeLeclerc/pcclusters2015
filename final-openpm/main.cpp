/* vim: set softtabstop=2 shiftwidth=2 expandtab : */
#include <iostream>
#include <algorithm>
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <sstream>
#include <stddef.h>
#include <gmpxx.h>
#include <map>
#include <sstream>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

std::vector<mpz_class> computed;
std::vector<std::vector<size_t> > newValuesArray;
size_t savings = 0;
size_t nn = 0;
int thread_number;

std::vector<size_t> packData() {
  std::vector<size_t> result;
  std::vector<size_t> startPoints;
  for (auto& newValues : newValuesArray) {
    startPoints.push_back(result.size());
    result.resize(result.size() + 2*newValues.size());
  }
#pragma omp parallel
  {
    size_t myId = omp_get_thread_num();
    size_t previous = startPoints[myId];
    for (size_t offset = 0; offset < newValuesArray[myId].size(); offset++) {
      size_t index = newValuesArray[myId][offset];
      mpz_class val = computed[index];
      if (val.fits_ulong_p()) {
        result[2*offset + previous] = index;
        result[2*offset + previous + 1] = val.get_ui();
      }
    }
  }
  return result;
}

void unpackData(std::vector<size_t> data) {
  for (size_t i = 0 ; i < data.size() ; ++i) {
    size_t index = data[i];
    ++i;
    size_t value = data[i];
    if (computed[index] == 0) {
      computed[index] = value;
      nn++;
    }
  }
}

mpz_class syracuseNext(mpz_class v) {
  if (v % 2 == 0) {
    return v/2;
  } else {
    return 3*v + 1;
  }
}

mpz_class memoizeLength(mpz_class v, size_t &steps) {
  if (v.fits_ulong_p()) {
    size_t index = v.get_ui();
    if (computed.size() > index) {
      mpz_class value = computed.at(index);
      if (value != 0) {
        return value;
      }
    }
  }
  if (v < 2) {
    return 1;
  }
  steps++;
  mpz_class result = memoizeLength(syracuseNext(v), steps) + 1;
  if (v.fits_ulong_p()) {
    size_t index = v.get_ui();
    if (computed.size() > index) {
      computed[index] = result;
/*#pragma omp critical 
      {
      printf("%lu - %s\n", index, result.get_str().c_str());
      fflush(stdin);
      }
      */
      newValuesArray[omp_get_thread_num()].push_back(index);
    } 
  }
  return result;
}

mpz_class length(mpz_class v) {
  if(v < 2) return 0;
  return length(syracuseNext(v)) + 1;
}

void computeInterval(mpz_class from, mpz_class to) {
  for (mpz_class i(from); i < to; ++i) {
    size_t steps = 0;
    mpz_class value = memoizeLength(i, steps);
    savings += value.get_ui() - steps;
  }
}

int main (int argc, char* argv[]) 
{
  MPI_Init(&argc, &argv);

  mpz_class range;
  mpz_class rounds;

  int currentNode;
  int totalNodes;

  if (argc < 3) {
    std::cout << "You must have 2 arguments" << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
    return MPI_Finalize();
  } 

  /**
   * Getting MPI information
   */
  MPI_Comm_rank(MPI_COMM_WORLD, &currentNode);
  MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);

  /**
   * Reading arguments
   */
  range = mpz_class(argv[1]);
  rounds = mpz_class(argv[2]);

  if (rounds > range) {
    std::cerr << "It is impossible to have more rounds than values to compute" << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
    return MPI_Finalize();
  }

  if (rounds < 1) {
    std::cerr << "This is not a vlid round number" << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
    return MPI_Finalize();
  }

#pragma omp parallel
  {
     thread_number = omp_get_num_threads(); 
  }

  mpz_class start(2);
  mpz_class valuesPerRound = (range - start - 1)/rounds + 1;
  std::cout << "vpr " << valuesPerRound << std::endl;
  mpz_class valuesPerNode  = (valuesPerRound - 1)/totalNodes + 1;
  std::cout << "vpn " << valuesPerNode << std::endl;
  mpz_class valuesPerThread = (valuesPerNode - 1)/ thread_number + 1;
  std::cout << "vpt " << valuesPerThread << std::endl;


  if (range.fits_ulong_p()) {
    computed = std::vector<mpz_class>(range.get_ui());
  } else {
    computed = std::vector<mpz_class>(0);
  }


  for (int tid = 0 ; tid < thread_number; ++tid) {
    newValuesArray.push_back(std::vector<size_t>());
  }

  std::fill(computed.begin(), computed.end(), 0);

  for (mpz_class i(0); i < rounds; ++i) {

    for (auto& newValues : newValuesArray) {
      newValues.clear();
    }

    std::cout << "round " << i + 1 << std::endl;

    mpz_class from = start + valuesPerRound * i + valuesPerNode * currentNode;
    mpz_class to = from + valuesPerNode;

    size_t diff = mpz_class(to - from).get_ui();

#pragma omp parallel for schedule(dynamic, 1)
    for (size_t dd = 0; dd < diff; dd++) {
      size_t s = 0;
       memoizeLength(from + dd, s);
    }
    nn = 0;
    std::vector<size_t> data = packData();
    for (int node = 0; node < totalNodes; ++node) {
      size_t dataLen;
      if (node == currentNode) {
        dataLen = data.size();
      } 
      MPI_Bcast(&dataLen, 1, MPI_UINT64_T, node, MPI_COMM_WORLD);
      if (dataLen != 0) {
        if (node == currentNode) {
          MPI_Bcast(&data[0], dataLen, MPI_UINT64_T, node, MPI_COMM_WORLD);
        } else {
          size_t* rawData = new size_t[dataLen];
          MPI_Bcast(rawData, dataLen, MPI_UINT64_T, node, MPI_COMM_WORLD);
          std::vector<size_t> toUnpack(rawData, rawData + dataLen);
          unpackData(toUnpack);
          delete rawData;
        }
      }
    }
    //std::cout << "round " << i << ", node: " << currentNode << ", from: " << from << ", to: " << to << " toSend: " << newValues.size() << ", learned: " << nn <<  std::endl;
  }

  // I am master
  if (currentNode == 0) {
    for (int i = 1 ; i < totalNodes; ++i) {
      size_t peerSavings;
      MPI_Status stat;
      MPI_Recv(&peerSavings, 1, MPI_UNSIGNED_LONG, i, 1, MPI_COMM_WORLD, &stat);
      savings += peerSavings;
    }
    //std::cout << savings << std::endl;
  } else {
      MPI_Send(&savings, 1, MPI_UNSIGNED_LONG, 0, 1, MPI_COMM_WORLD);
  }
  return MPI_Finalize();
}
