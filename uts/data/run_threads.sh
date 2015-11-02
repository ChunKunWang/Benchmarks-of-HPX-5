#!/bin/sh
command_path="../"
input_path="../input/"

command="hpx-5"

input[0]="toy.input"
input[1]="test.input"
input[2]="tiny.input"
input[3]="small.input"
input[4]="medium.input"

log="log-threads"

stacksize1=65536
stacksize2=262144

echo -e "Create new log file."

if [ -f ${log} ]; then
	rm ${log}
#echo -e "Remove ${log} file."
fi
touch ${log}
#echo -e "Create a new ${log} file."

for i in {0..4}
do
	for j in 1 2 4 6 8 10 15 20
	do
		echo -e "\nThreads:[$j]" >> ${log}
		if [ "$i" == 0 ]; then
			for k in {0..4}
			do
				${command_path}${command} --hpx-stacksize=$stacksize1 --hpx-threads=$j ${input_path}${input[i]} >> ${log}
			done
		else
			for k in {0..4}
			do
				${command_path}${command} --hpx-stacksize=$stacksize2 --hpx-threads=$j ${input_path}${input[i]} >> ${log}
			done
			
		fi
	done
done

echo -e "Mission Complete!"

