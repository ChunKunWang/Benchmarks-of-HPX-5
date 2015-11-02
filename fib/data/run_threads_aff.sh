#!/bin/sh
command_path="../"

command[0]="omp-icc"

input=45

log[0]="log-threads-icc-aff"

echo -e "Create new log file."

for i in 0 
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	#echo -e "Create a new ${log[i]} file."
done

export KMP_AFFINITY=granularity=core,compact

echo -e "${log[0]} is processing..."
for i in 1 2 4 6 8 10 15 20
do
	export OMP_NUM_THREADS=$i
	echo -e "\nThreads:[$i]" >> ${log[0]}
	for j in {0..4}
	do
		${command_path}${command[0]} $input >> ${log[0]}
	done
done

export KMP_AFFINITY=

echo -e "Mission Complete!"

