/* vim: set softtabstop=2 shiftwidth=2 expandtab : */

/* C Example */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

long syracuse_next(long n) {
  if(n % 2 == 0) return n/2;
  return 3*n + 1;
}

int main (int argc, char* argv[])
{
  int rank, size;

  int from;
  int to;
  int max;

  MPI_Init (&argc, &argv);      /* starts MPI */
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);        /* get number of processes */
  if (rank == 0) {
    if (argc < 2) {
      std::cout << "You must give the number of series to compute" << std::endl;
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    max = atoi(argv[1]);
  }

  MPI_Bcast(&max, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int slice = max / size + 1;
  from = slice * rank;
  to = slice * (rank + 1);

  int *counts = new int[slice * (size + 1)];

  for (int i = from; i <= to; ++i) {
    int seq = 0;
    long v = i;
    while (v > 1) {
      v = syracuse_next(v);
      seq++;
    }
    counts[i] = seq;
  }

  for(int i = 0 ; i < size; ++i) {
    MPI_Bcast(counts + i * slice, slice, MPI_INT, i, MPI_COMM_WORLD);
  }

  if(rank == 0) {
    //for (int i = 2; i < max; ++i) {
      //std::cout << counts[i] << std::endl;
    //}
  }

  delete counts;

  MPI_Finalize();
}
