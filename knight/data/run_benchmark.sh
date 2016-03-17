#!/bin/sh
command_path="../"

command[0]="omp-icc"
command[1]="cilk-icc"
command[2]="hpx-5"

log[0]="log-omp-icc"
log[1]="log-cilk-icc"
log[2]="log-hpx"

start=3
end=8
step=1

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
		for k in {1..10}
		do
			${command_path}${command[i]} $j >> ${log[i]}
		done
	done
done

echo -e "Mission Complete!"

