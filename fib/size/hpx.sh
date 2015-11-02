#!/bin/sh

code="fib_hpx_size"
exe_hpx="hpx-5"

path="/home/amos/amos/HPX_Release_v1.3.0/hpx"

/hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc -std=gnu99 -DHAVE_CONFIG_H -I. -I${path}/hpx/include -I${path}/ontrib/include/private/autogen -I${path}/ontrib/include/hwloc/autogen  -D_GNU_SOURCE -I/home/amos/amos/HPX_Release_v1.3.0/hpx/contrib/hwloc-1.11.0/include -I/home/amos/amos/HPX_Release_v1.3.0/hpx/contrib/hwloc-1.11.0/include -I${path}/include  -fvisibility=hidden -I${path}/contrib/libffi/include -Wno-unused -g -O2 -MT ${code}.o -MD -MP -MF $depbase.Tpo -c -o ${code}.o ${code}.c

${path}/libtool --silent --tag=CC   --mode=link /hpc/shared/apps/RHEL-6/x86_64/intel/14.0.2/bin/icc -std=gnu99 -fvisibility=hidden -I${path}/contrib/libffi/include -Wno-unused -g -O2 -Wl,-rpath,/home/amos/amos/HPX_Release_v1.3.0/hpx/contrib/jemalloc/lib -no-install -o ${exe_hpx} ${code}.o ${path}/libhpx/libhpx.la

rm ${code}.o

