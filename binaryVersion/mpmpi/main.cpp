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
#include <gmpxx.h>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)


mpz_class syracuseNext(mpz_class v) {
  if (v % 2 == 0) {
    return v/2;
  } else {
    return 3*v + 1;
  }
}

bool checkRecursively(unsigned char* buffer, const mpz_class& minBuffer, const mpz_class& maxBuffer, mpz_class value) {
  if (value < minBuffer) {
    return true;
  } else if (value >= maxBuffer) {
    return checkRecursively(buffer, minBuffer, maxBuffer, syracuseNext(value));
  } else {
    mpz_class offset = (value - minBuffer);
    const size_t realOffset = offset.get_ui();
    if (buffer[realOffset]) {
      return true;
    } else {
      unsigned char res = checkRecursively(buffer, minBuffer, maxBuffer, syracuseNext(value));
      buffer[realOffset] = res;
      return res;
    }
  }
}

bool computeInterval(const mpz_class& f, const mpz_class& computeFrom, const mpz_class& t) {
  mpz_class range = t - f;
  if (range.fits_ulong_p()) {
    size_t width = range.get_ui();
    unsigned char* marks = new unsigned char[width];
    memset(marks, false, width * sizeof(unsigned char));
    unsigned char ok = true;
    size_t computeWidth = mpz_class((t - computeFrom)).get_ui();
#pragma omp parallel for reduction(&&:ok)
    for (size_t i = 0 ; i < computeWidth; ++i) {
      unsigned char r = checkRecursively(marks, f, t, computeFrom + i);
      if (!r) {
        std::cout << computeFrom + i << " does not satisfy !!" << std::endl;
      }
      ok = r;
    }
    return ok;
  } else {
    std::cerr << "the range is too big for this computer" << std::endl;
  }
  return false;
}

/**
 * This is the main method
 * Arguments are:
 *  - From (assume all previous numbers correct)
 *  - To (do not check after this)
 *  - W (the width of intervals to process)
 *  - T the maximum number of threads per executable (-1 for default)
 */
int main (int argc, char* argv[]) 
{
  MPI_Init(&argc, &argv);

  mpz_class from;
  mpz_class to;
  mpz_class width;

  int currentNode;
  int totalNodes;

  if (argc < 5) {
    std::cout << "You must have 4 arguments" << std::endl;
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
  from = mpz_class(argv[1]);
  to = mpz_class(argv[2]);
  width = mpz_class(argv[3]);


  //std::cout << from << " " << to << "with width" << width << std::endl;
  const int T = atoi(argv[4]);
  if (T > 0) {
    omp_set_num_threads(T);
  }

  mpz_class current = from;
  while (current < to) {
    mpz_class f = current + currentNode * width;
    mpz_class t = f + width;
    int localOK = computeInterval(current, f, t);
    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0 ; i < totalNodes; ++i) {
      int ok;
      if (i == currentNode){
        ok = localOK;
      }
      MPI_Bcast(&ok, 1, MPI_INT, i, MPI_COMM_WORLD);
      if (!ok) {
        std::cout << "Im not continuing we found a counter example" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
        return MPI_Finalize();
      }
    }

    current += totalNodes * width;
  }
  return MPI_Finalize();
}
