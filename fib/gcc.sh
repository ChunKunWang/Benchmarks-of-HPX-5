#!/bin/sh

rm basic omp omp-icc

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -O2 -o basic fib-0.c &&\

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -O2 -fopenmp -o omp fib-0.c &&\

/hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc -O2 -openmp -o omp-icc fib-0.c &&\

/hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc -O2 -openmp -o size-icc fib_size.c


