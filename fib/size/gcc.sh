#!/bin/sh

rm size-omp size-icc

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -O2 -fopenmp -o size-omp fib_size.c &&\

/hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc -O2 -openmp -o size-icc fib_size.c


