#!/bin/sh
code_1="uts_hpx"
code_2="brg_sha1"
exe_hpx="hpx-5"
path="/home/amos/amos/hpx-2.2.0/hpx"


icc -std=gnu99 -DHAVE_CONFIG_H -I. -I${path}/include -I${path}/ontrib/include/private/autogen -I${path}/ontrib/include/hwloc/autogen  -D_GNU_SOURCE -I/home/amos/amos/hpx-2.2.0/hpx/contrib/hwloc-1.11.0/include -I/home/amos/amos/hpx-2.2.0/hpx/contrib/hwloc-1.11.0/include -I${path}/contrib/libffi/include -I${path}/include  -wd32013  -fvisibility=hidden  -Wno-unused -O3 -g -MT ${code_1}.o -MD -MP -MF $depbase.Tpo -c -o ${code_1}.o ${code_1}.c &&\

icc -std=gnu99 -DHAVE_CONFIG_H -I. -I${path}/include -I${path}/ontrib/include/private/autogen -I${path}/ontrib/include/hwloc/autogen  -D_GNU_SOURCE -I/home/amos/amos/hpx-2.2.0/hpx/contrib/hwloc-1.11.0/include -I/home/amos/amos/hpx-2.2.0/hpx/contrib/hwloc-1.11.0/include -I${path}/contrib/libffi/include -I${path}/include  -wd32013  -fvisibility=hidden  -Wno-unused -O3 -g -MT ${code_2}.o -MD -MP -MF $depbase.Tpo -c -o ${code_2}.o ${code_2}.c &&\

${path}/libtool --silent --tag=CC   --mode=link icc -std=gnu99 -wd32013  -fvisibility=hidden  -Wno-unused -O3 -g -Wl,-rpath,/home/amos/amos/hpx-2.2.0/hpx/contrib/jemalloc/lib  -no-install  -o ${exe_hpx} ${code_1}.o ${code_2}.o ${path}/libhpx/libhpx.la

rm ${code_1}.o ${code_2}.o

