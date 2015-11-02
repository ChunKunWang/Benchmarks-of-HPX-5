#!/bin/sh
command_path="../"

command[0]="omp-icc"
command[1]="hpx-5"

input=45

log[0]="log-threads-icc"
log[1]="log-threads-hpx"

echo -e "Create new log file."

for i in 0 1
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	#echo -e "Create a new ${log[i]} file."
done


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

echo -e "${log[1]} is processing..."
for i in 1 2 4 6 8 10 15 20
do
	echo -e "\nThreads:[$i]" >> ${log[1]}
	for k in {0..4}
	do
		${command_path}${command[1]} --hpx-threads=$i $input >> ${log[1]}
	done
done

echo -e "Mission Complete!"

