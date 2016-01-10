# pcclusters2015

This is my whole workspace. Everything, good, bad, stupid is here. 

The more interesting thing is the final version of the code. It is located in `final-openpm/`. and the command line to compile it is:

`mpic++ -fopenmp ./final-openpm/main.cpp -o main -lgmp -lgmpxx -pedantic -Wall -std=c++11 -O3`

It requires:

- OpenMPI
- OpenMP
- GMP
- GMPxx (C++ wrapper)
