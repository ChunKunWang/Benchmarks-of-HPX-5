#!/bin/sh
command_path="../"
input_path="../input/"

command[0]="omp"
command[1]="omp-icc"

input[0]="toy.input"
input[1]="test.input"
input[2]="tiny.input"
input[3]="small.input"
input[4]="medium.input"

log[0]="log-threads-omp"
log[1]="log-threads-icc"

echo -e "Create new log file."

for i in 1
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	#echo -e "Create a new ${log[i]} file."
done

export OMP_STACKSIZE="64M"

for n in 1 
do
	echo -e "${log[n]} is processing..."
	for i in {0..4}
	do
		for j in 1 2 4 6 8 10 15 20
		do
			echo -e "\nThreads:[$j]" >> ${log[n]}
				for k in {0..4}
				do
					${command_path}${command[n]} $j ${input_path}${input[i]} >> ${log[n]}
				done
		done
	done
done

echo -e "Mission Complete!"

