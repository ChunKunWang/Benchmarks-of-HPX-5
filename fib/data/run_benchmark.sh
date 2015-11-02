#!/bin/sh
command_path="../"

command[0]="omp"
command[1]="omp-icc"
command[2]="hpx-5"

log[0]="log-omp"
log[1]="log-icc"
log[2]="log-hpx"

start=10
end=45
step=5

echo -e "Create new log file."

for i in 0 1 2
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	#echo -e "Create a new ${log[i]} file."
done

for i in 0 1 2
do
	for ((j=$start; j<=$end; j+=$step))
	do
		echo -e "\nInput:[$j]" >> ${log[i]}
		for k in 0 1 2
		do
			${command_path}${command[i]} $j >> ${log[i]}
		done
	done
done

echo -e "Mission Complete!"

