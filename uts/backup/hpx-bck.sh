#!/bin/sh

code_1="uts_hpx_bck"
code_2="brg_sha1"

exe_hpx="hpx-bck"

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -std=gnu99 -DHAVE_CONFIG_H -I. -I/home/amos/amos/HPX_Release_v1.1.0/hpx/include  -D_GNU_SOURCE -I/home/amos/amos/HPX_Release_v1.1.0/hpx/contrib/libffi/include -I/home/amos/amos/HPX_Release_v1.1.0/hpx/include  -lm -g -O2 -MT ${code_1}.o -MD -MP -MF $depbase.Tpo -c -o ${code_1}.o ${code_1}.c

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -std=gnu99 -DHAVE_CONFIG_H -I. -I/home/amos/amos/HPX_Release_v1.1.0/hpx/include  -D_GNU_SOURCE -I/home/amos/amos/HPX_Release_v1.1.0/hpx/contrib/libffi/include -I/home/amos/amos/HPX_Release_v1.1.0/hpx/include  -lm -g -O2 -MT ${code_2}.o -MD -MP -MF $depbase.Tpo -c -o ${code_2}.o ${code_2}.c

/home/amos/amos/HPX_Release_v1.1.0/hpx/libtool --silent --tag=CC   --mode=link /hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -std=gnu99  -lm -g -O2 -Wl,-rpath,/home/amos/amos/HPX_Release_v1.1.0/hpx/contrib/jemalloc/lib -L/home/amos/amos/HPX_Release_v1.1.0/hpx/contrib/jemalloc/lib  -no-install  -o ${exe_hpx} ${code_1}.o ${code_2}.o /home/amos/amos/HPX_Release_v1.1.0/hpx/libhpx/libhpx.la

rm ${code_1}.o ${code_2}.o

