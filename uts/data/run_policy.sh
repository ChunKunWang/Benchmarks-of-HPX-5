#!/bin/sh
command_path="../"
input_path="../input/"
output_path="./run_policy//"

input[0]="toy.input"
input[1]="test.input"
input[2]="tiny.input"
input[3]="small.input"
input[4]="medium.input"

command="hpx-5"
stacksize="--hpx-stacksize=262144"

log[0]=${output_path}"log-hybrid+h"
log[1]=${output_path}"log-hybrid+w"
log[2]=${output_path}"log-random+h"
log[3]=${output_path}"log-random+w"
log[4]=${output_path}"log-hier+h"
log[5]=${output_path}"log-hier+w"

option[0]="--hpx-sched-policy=default"
option[1]="--hpx-sched-policy=default --hpx-sched-wfthreshold=0"
option[2]="--hpx-sched-policy=random"
option[3]="--hpx-sched-policy=random --hpx-sched-wfthreshold=0"
option[4]="--hpx-sched-policy=hier"
option[5]="--hpx-sched-policy=hier --hpx-sched-wfthreshold=0"

echo -e "Create new log file."

for i in {0..5}
do
	if [ -f ${log[i]} ]; then
		rm ${log[i]}
	#echo -e "Remove ${log[i]} file."
	fi
	touch ${log[i]}
	#echo -e "Create a new ${log[i]} file."
done

for i in {0..5}
do
	for j in {0..4}
	do
		echo -e "" >> ${log[i]}
		for k in {1..10}
		do
			${command_path}${command} ${stacksize} ${option[i]} ${input_path}${input[j]} >> ${log[i]}
		done
	done
done

echo -e "Mission Complete!"

