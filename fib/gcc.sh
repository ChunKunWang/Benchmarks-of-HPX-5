#!/bin/sh
code_1="fib_omp"
code_2="fib_size"
code_3="fib_cilk"

exe_seq="basic"
exe_omp="omp"
exe_icc="omp-icc"
exe_cilk_icc="cilk-icc"

rm ${exe_seq} ${exe_omp} ${exe_icc} ${exe_cilk_icc}

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -O2 -o ${exe_seq} ${code_1}.c &&\

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -O2 -fopenmp -o ${exe_omp} ${code_1}.c &&\

/hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc -O2 -openmp -o ${exe_icc} ${code_1}.c &&\

/hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc -O2 -o ${exe_cilk_icc} ${code_3}.c


