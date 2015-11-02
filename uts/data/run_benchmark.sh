#!/bin/sh
command_path="../"
input_path="../input/"

input[0]="toy.input"
input[1]="test.input"
input[2]="tiny.input"
input[3]="small.input"
input[4]="medium.input"

command[0]="omp"
command[1]="omp-icc"
command[2]="cilk"
command[3]="hpx-5"

option[0]=""
option[1]=""
option[2]=""
option[3]="--hpx-stacksize=262144"

log[0]="log-omp"
log[1]="log-icc"
log[2]="log-cilk"
log[3]="log-hpx"

echo -e "Create new log file."

for i in {0..3}
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	#echo -e "Create a new ${log[i]} file."
done

for i in {0..3}
do
	for j in {0..4}
	do
		echo -e "" >> ${log[i]}
		for k in {1..5}
		do
			${command_path}${command[i]} ${option[i]} ${input_path}${input[j]} >> ${log[i]}
		done
	done
done

echo -e "Mission Complete!"

