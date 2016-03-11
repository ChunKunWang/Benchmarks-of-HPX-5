#!/bin/sh
command_path="../"
input_path="../input/"

input[0]="toy.input"
input[1]="test.input"
input[2]="tiny.input"
input[3]="small.input"
input[4]="medium.input"
input[5]="large.input"
input[6]="huge.input"

command[0]="omp-icc"
command[1]="cilk-icc"

log[0]="log-omp-icc"
log[1]="log-cilk-icc"

export OMP_STACKSIZE="128M"

echo -e "set linux stack size as unlimited"
ulimit -s unlimited

echo -e "Create new log file"
for i in {0..1}
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	#echo -e "Create a new ${log[i]} file."
done

for i in {0..1}
do
	for j in {0..6}
	do
		echo -e "" >> ${log[i]}
		for k in {1..10}
		do
			${command_path}${command[i]} ${option[i]} ${input_path}${input[j]} >> ${log[i]}
		done
	done
done

echo -e "Mission Complete!"

