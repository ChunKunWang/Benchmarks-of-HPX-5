#!/bin/sh
command_path="../"

command[0]="omp"
command[1]="omp-icc"
command[2]="hpx-5"

log[0]="log-omp"
log[1]="log-icc"
log[2]="log-hpx"
log[3]="log-err"

start=10
end=10000000
step=10

echo -e "Create new log file."

for i in {0..2}
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
	for ((j=$start; j<=$end; j*=$step))
	do
		echo -e "\nInput:[$j]" >> ${log[i]}
		for k in 0 1 2
		do
			#exec 2>> log-err && ${command_path}${command[i]} $j 2>&1 >> ${log[i]} &
			${command_path}${command[i]} $j >> ${log[i]}  
		done
	done
done

echo -e "Mission Complete!"

