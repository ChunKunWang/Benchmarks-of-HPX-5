#!/bin/sh
command_path="../"
input_path="../input/"
output_path="./run_policy_num/"

input="medium.input"

command="hpx-5"
stacksize="--hpx-stacksize=262144"

log[0]="log-pu-hybrid+h"
log[1]="log-pu-hybrid+w"
log[2]="log-pu-random+h"
log[3]="log-pu-random+w"
log[4]="log-pu-hier+h"
log[5]="log-pu-hier+w"

option[0]="--hpx-sched-policy=default"
option[1]="--hpx-sched-policy=default --hpx-sched-wfthreshold=0"
option[2]="--hpx-sched-policy=random"
option[3]="--hpx-sched-policy=random --hpx-sched-wfthreshold=0"
option[4]="--hpx-sched-policy=hier"
option[5]="--hpx-sched-policy=hier --hpx-sched-wfthreshold=0"

echo -e "Create new log file."

for i in {0..5}
do
	if [ -f ${output_path}${log[i]} ]; then
		rm ${output_path}${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${output_path}${log[i]}
	#echo -e "Create a new ${log[i]} file."
done

for i in {0..5}
do
	for j in 1 2 4 8 12 16 20
	do
		echo -e "\nThreads:[$j]" >> ${output_path}${log[i]}
		for k in {1..10}
		do
			${command_path}${command} ${stacksize} --hpx-threads=$j ${option[i]} ${input_path}${input} >> ${output_path}${log[i]}
		done
	done
done

echo -e "Mission Complete!"

