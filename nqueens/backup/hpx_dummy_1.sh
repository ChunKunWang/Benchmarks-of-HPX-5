#!/bin/sh

/hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -std=gnu99 -DHAVE_CONFIG_H -I. -I/home/amos/amos/HPX_Release_v1.1.0/hpx/include  -D_GNU_SOURCE -I/home/amos/amos/HPX_Release_v1.1.0/hpx/contrib/libffi/include -I/home/amos/amos/HPX_Release_v1.1.0/hpx/include   -g -O2 -MT nqueens_dummy_1.o -MD -MP -MF $depbase.Tpo -c -o nqueens_dummy_1.o nqueens_dummy_1.c

/home/amos/amos/HPX_Release_v1.1.0/hpx/libtool --silent --tag=CC   --mode=link /hpc_shared/apps/RHEL-6/x86_64/gcc/gcc-4.9.1/bin/gcc -std=gnu99  -g -O2 -Wl,-rpath,/home/amos/amos/HPX_Release_v1.1.0/hpx/contrib/jemalloc/lib -L/home/amos/amos/HPX_Release_v1.1.0/hpx/contrib/jemalloc/lib  -no-install  -o hpx-d1 nqueens_dummy_1.o /home/amos/amos/HPX_Release_v1.1.0/hpx/libhpx/libhpx.la

rm nqueens_dummy_1.o

