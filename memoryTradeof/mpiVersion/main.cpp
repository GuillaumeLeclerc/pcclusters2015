/* vim: set softtabstop=2 shiftwidth=2 expandtab : */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

long syracuse_next(long n) {
  if(n % 2 == 0) return n/2;
  return 3*n + 1;
}

struct __attribute__((packed)) Count {
  int len;
  int hit;
};

struct Count* counts;
std::vector<int> buffer;
int maxCount;


int length(long n, long original, int jumps = 0) {
  int res = 0;
  if (likely(n < maxCount && counts[n].len != 0)) {
    //std::cout << original << " -> " << n << " (" << jumps << ")" << std::endl;
    counts[n].hit++;
    return counts[n].len;
  } else if (unlikely(n > 1)) {
    res = length(syracuse_next(n), original, jumps + 1) + 1;
  }

  if (n < maxCount) {
    if (counts[n].len == 0) {
      buffer.push_back(n);
      buffer.push_back(res);
    }
    counts[n].len = res;
  }
  return res;
}

void exchangeBuffers(int* bufferLengths, int rank, int size) {
  int* buffers[rank];
  bufferLengths[rank] = buffer.size();
  for(int i = 0 ; i < size; ++i) {
    MPI_Bcast(bufferLengths + i, 1, MPI_INT, i, MPI_COMM_WORLD);
    int* b;

    if (rank == i) {
      b = &buffer[0];
    } else {
      b = new int[bufferLengths[i]];
    }

    MPI_Bcast(b, bufferLengths[i], MPI_INT, i, MPI_COMM_WORLD);

    if (rank != i) {
      for (int j = 0; j < bufferLengths[i]; ++j) {
        int n = b[j++];
        int value = b[j];
        counts[n].len = value;
      }
      delete b;
    }
  }
  buffer.clear();
}

int main (int argc, char* argv[])
{
  int rank, size;
  int max;
  int to;
  int rounds;
  int cacheSize;

  int from;

  MPI_Init (&argc, &argv);      /* starts MPI */
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);        /* get number of processes */
  if (rank == 0) {
    if (argc < 4) {
      std::cout << "You must give the number of series to compute and the number of rounds" << std::endl;
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    max = atoi(argv[1]);
    rounds = atoi(argv[2]);
    cacheSize = atoi(argv[3]);
  }

  MPI_Bcast(&max, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&rounds, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int slice = (max + 1) / size + 1;
  from = slice * rank;
  to = slice * (rank + 1);

  maxCount = slice * (size + 1);
  counts = new struct Count[maxCount];

  buffer = std::vector<int>();
  int* bufferLengths = new int[size];

  int diff = to - from;
  int mod = diff/rounds;
  memset(counts, 0, maxCount * sizeof(struct Count));

  for (int i = 0; i <= diff; ++i) {
    length(from + i, from + i, 0);
    if (unlikely(i % mod == 0)) {
      exchangeBuffers(bufferLengths, rank, size);
    }
  }

  int fin = 0;

  if(rank == 0) {
    for (int i = 2; i < max; ++i) {
      //std::cout << i << " : " << counts[i].hit << std::endl;
      fin += counts[i].len;
    }
    std::cout << fin << std::endl;
  }

  delete counts;
  delete bufferLengths;

  MPI_Finalize();
}
