#!/bin/sh
threads=20
stacksize=32768
exe="../hpx-s"
input="../input/toy.input"

while getopts "t:i:s:" flag;
do	case $flag in
t)	threads=${OPTARG};;
i)	input=${OPTARG};;
s)	stacksize=${OPTARG}
	esac
done

#echo ${input}

for i in {0..20}
do
	echo -e "[$i]:"
	${exe} --hpx-stacksize=$stacksize --hpx-threads=$threads ${input}
done

echo -e "Mission Complete!"

