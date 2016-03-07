#!/bin/sh

code_1="uts_task"
code_2="brg_sha1"
code_3="uts_cilk"

exe_seq="basic"
exe_omp="omp"
exe_icc="omp-icc"
exe_cilk="cilk"
exe_cilk_icc="cilk-icc"

option=""

rm ${exe_seq} ${exe_omp} ${exe_icc} ${exe_cilk} ${exe_cilk_icc}

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc ${option} -O2 -lm -o ${exe_seq} ${code_1}.c ${code_2}.c &&\

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc ${option} -O2 -lm -fopenmp -o ${exe_omp} ${code_1}.c ${code_2}.c &&\

/hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc ${option} -O2 -openmp -o ${exe_icc} ${code_1}.c ${code_2}.c &&\

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc ${option} -O2 -ldl -lcilkrts -fcilkplus -o ${exe_cilk} ${code_3}.c ${code_2}.c 

/hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc ${option} -O2 -ldl -lcilkrts -o ${exe_cilk_icc} ${code_3}.c ${code_2}.c 

